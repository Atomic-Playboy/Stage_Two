#include "MIDI/EvaluationEngine.h"
#include "Core/AppState.h"
#include "Core/Utils.h"
#include "MIDI/MidiUtils.h" // <--- Upgraded Include
#include "Core/AppConstants.h"
#include "MIDI/GtrSendLights.h"
#include <iostream>
#include <sstream>
#include <iomanip>

EvaluationEngine* g_CurrentEngineInstance = nullptr; std::mutex g_RouterLock; std::function<void(int, int)> g_GlobalStateCallback = nullptr; std::function<void(const std::string&)> g_GlobalCrawlCallback = nullptr;
void globalRtMidiStaticRouter(double deltatime, std::vector<unsigned char>* message, void* userData);

static bool parseRawMidi(const std::vector<unsigned char>& bytes, const std::string& name, RawMidiMessage& incomingMsg) {
    incomingMsg.interfaceName = name; incomingMsg.type = "UNKNOWN"; unsigned char statusByte = bytes[0];
    if (statusByte >= 0x80 && statusByte <= 0xEF) {
        incomingMsg.channel = (statusByte & 0x0F) + 1; unsigned char typeNibble = statusByte & 0xF0;
        if (typeNibble == 0xB0 && bytes.size() >= 3) { incomingMsg.type = "CC"; incomingMsg.ccNumber = bytes[1]; incomingMsg.ccValue = bytes[2]; }
        else if (typeNibble == 0xC0 && bytes.size() >= 2) { incomingMsg.type = "PC"; incomingMsg.programNumber = bytes[1] + 1; }
        else if (typeNibble == 0x90 && bytes.size() >= 3) { incomingMsg.type = "NOTE_ON"; incomingMsg.ccNumber = bytes[1]; incomingMsg.ccValue = bytes[2]; if (incomingMsg.ccValue == 0) incomingMsg.type = "NOTE_OFF"; }
        else if (typeNibble == 0x80 && bytes.size() >= 3) { incomingMsg.type = "NOTE_OFF"; incomingMsg.ccNumber = bytes[1]; incomingMsg.ccValue = bytes[2]; }
    } else if (statusByte == 0xF0) { incomingMsg.type = "SYSEX"; incomingMsg.channel = 0; }
    return incomingMsg.type != "UNKNOWN";
}

static void evaluateGtrHardware(const RawMidiMessage& incomingMsg, unsigned char statusByte, AppState& state) {
    for (int slot = 0; slot < AppConstants::MAX_HARDWARE_SLOTS; ++slot) {
        GtrSlotConfig gtrCfg = state.getGtrSlotConfig(slot); if (!gtrCfg.loaded) continue;
        if (containsSubstringIgnoreCase(incomingMsg.interfaceName, gtrCfg.inPort1) || containsSubstringIgnoreCase(incomingMsg.interfaceName, gtrCfg.inPort2)) {
            if (incomingMsg.type == "CC" && parseChannelMatch(gtrCfg.inChan, statusByte)) {
                int currentPc = gtrCfg.activePcNum;
                for (auto& preset : gtrCfg.presets) {
                    if (preset.pcTrigger == currentPc) {
                        if (incomingMsg.ccNumber >= 16 && incomingMsg.ccNumber <= 21) { if (incomingMsg.ccValue > 0) { int btnIndex = incomingMsg.ccNumber - 15; if (preset.displays[btnIndex].isActive) { preset.displays[btnIndex].isBright = !preset.displays[btnIndex].isBright; activatePreset(slot, preset, gtrCfg); } } }
                        else if (incomingMsg.ccNumber == 22) { if (incomingMsg.ccValue > 0) { preset.abInitialState = (preset.abInitialState == preset.abOffState) ? preset.abOnState : preset.abOffState; activatePreset(slot, preset, gtrCfg); } }
                        break;
                    }
                }
            } else if (incomingMsg.type == "PC" && parseChannelMatch(gtrCfg.inChan, statusByte)) { for (const auto& preset : gtrCfg.presets) { if (preset.pcTrigger == incomingMsg.programNumber - 1) { activatePreset(slot, preset, gtrCfg); break; } } }
        }
    }
}

static bool evaluateTransformer(const RawMidiMessage& incomingMsg, AppState& state, std::string& transformerLog) {
    bool ruleMatched = false; std::string matchPriorityNum = ""; const MIDITrigger* matchedRulePtr = nullptr;
    for (const auto& rule : g_CurrentEngineInstance->transformerRules) {
        bool matchInt = (rule.trigger.interfaceName == "ANY" || incomingMsg.interfaceName.find(rule.trigger.interfaceName) != std::string::npos);
        bool matchChan = (rule.trigger.channel == "ANY" || std::to_string(incomingMsg.channel) == rule.trigger.channel);
        bool matchType = (rule.trigger.type == incomingMsg.type || (rule.trigger.type == "NOTE" && (incomingMsg.type == "NOTE_ON" || incomingMsg.type == "NOTE_OFF")));
        bool matchDetails = true;
        if (!rule.trigger.details.empty()) {
            if (incomingMsg.type == "CC") { if (rule.trigger.details[0] != "ANY" && std::to_string(incomingMsg.ccNumber) != rule.trigger.details[0]) matchDetails = false; if (rule.trigger.details.size() >= 2 && rule.trigger.details[1] != "ANY" && std::to_string(incomingMsg.ccValue) != rule.trigger.details[1]) matchDetails = false; }
            else if (incomingMsg.type == "PC") { if (rule.trigger.details[0] != "ANY" && std::to_string(incomingMsg.programNumber) != rule.trigger.details[0]) matchDetails = false; }
            else if (incomingMsg.type == "NOTE_ON" || incomingMsg.type == "NOTE_OFF") { if (rule.trigger.details[0] != "ANY" && std::to_string(incomingMsg.ccNumber) != rule.trigger.details[0]) matchDetails = false; if (rule.trigger.details.size() >= 2 && rule.trigger.details[1] != "ANY" && std::to_string(incomingMsg.ccValue) != rule.trigger.details[1]) matchDetails = false; }
        }
        if (matchInt && matchChan && matchType && matchDetails) { ruleMatched = true; matchPriorityNum = rule.priority; matchedRulePtr = &rule; break; }
    }
    transformerLog += (ruleMatched ? (" -> matched by priority " + matchPriorityNum) : " -> unmatched");
    if (ruleMatched && matchedRulePtr) {
        for (const auto& trans : matchedRulePtr->transforms) {
            std::stringstream tsStream; tsStream << "\r\n     [" << trans.interfaceName << "] Ch:" << trans.channel << " " << trans.type;
            if (!trans.details.empty()) {
                if (trans.type == "CC" && trans.details.size() >= 2) tsStream << " N:" << trans.details[0] << " V:" << trans.details[1];
                else if (trans.type == "PC" && !trans.details.empty()) tsStream << " P:" << trans.details[0];
                else if ((trans.type == "NOTE_ON" || trans.type == "NOTE_OFF") && trans.details.size() >= 2) tsStream << " N:" << trans.details[0] << " V:" << trans.details[1];
                else for (const auto& d : trans.details) tsStream << " " << d;
            }
            transformerLog += tsStream.str();
            if (trans.interfaceName == AppConstants::InternalDashboardID) {
                if (trans.type == "CC" && trans.details.size() >= 2) { bool changed = false; state.updateBlockState(safeStoi(trans.details[0]), safeStoi(trans.details[1]) > 0, changed); if (g_GlobalStateCallback) g_GlobalStateCallback(1, -1); }
                else if (trans.type == "PC" && trans.details.size() >= 1) { int pcVal = safeStoi(trans.details[0]); state.loadPresetMatrixBlocks(pcVal); if (g_GlobalStateCallback) g_GlobalStateCallback(1, pcVal); }
            }
            else if (trans.interfaceName.find("GTR_SLOT_") == 0) {
                int targetSlot = safeStoi(trans.interfaceName.substr(9), 1) - 1;
                if (targetSlot >= 0 && targetSlot < AppConstants::MAX_HARDWARE_SLOTS) {
                    GtrSlotConfig targetCfg = state.getGtrSlotConfig(targetSlot);
                    if (targetCfg.loaded && trans.type == "PC" && !trans.details.empty()) { int pcVal = safeStoi(trans.details[0], -1) - 1; for (const auto& preset : targetCfg.presets) { if (preset.pcTrigger == pcVal) { activatePreset(targetSlot, preset, targetCfg); break; } } }
                }
            } else {
                unsigned char cByte = getOutputChannelByte((trans.type == "CC" ? 0xB0 : (trans.type == "PC" ? 0xC0 : 0x90)), trans.channel); std::vector<unsigned char> outMsg;
                if (trans.type == "CC" && trans.details.size() >= 2) outMsg = {cByte, static_cast<unsigned char>(safeStoi(trans.details[0])), static_cast<unsigned char>(safeStoi(trans.details[1]))};
                else if (trans.type == "PC" && trans.details.size() >= 1) outMsg = {cByte, static_cast<unsigned char>(safeStoi(trans.details[0]) - 1)};
                if (!outMsg.empty()) g_CurrentEngineInstance->transmitMidi(trans.interfaceName, outMsg);
            }
        }
    }
    return ruleMatched;
}

static bool evaluateDashboard(const RawMidiMessage& incomingMsg, AppState& state) {
    bool interfaceMatch = (g_CurrentEngineInstance->dashInterface == "ANY" || incomingMsg.interfaceName.find(g_CurrentEngineInstance->dashInterface) != std::string::npos); bool channelMatch = (g_CurrentEngineInstance->dashChannel == -1 || incomingMsg.channel == g_CurrentEngineInstance->dashChannel); bool isDashboardMsg = false;
    if (interfaceMatch && channelMatch) { if (incomingMsg.type == "PC") isDashboardMsg = true; else if (incomingMsg.type == "CC") { for (const auto& block : state.getBlockData()) { if (block.cc == incomingMsg.ccNumber) { isDashboardMsg = true; break; } } } }
    if (isDashboardMsg && incomingMsg.type == "CC") { bool changed = false; state.updateBlockState(incomingMsg.ccNumber, incomingMsg.ccValue > 0, changed); }
    return isDashboardMsg;
}

EvaluationEngine::EvaluationEngine(ThreadSafeLogger& systemLogger, std::function<void(const std::string&)> onPatchUpdate, std::function<void(const std::string&)> onRawMidiCrawl, std::function<void(int, int)> onStateChange) : logger(systemLogger), patchCallback(onPatchUpdate), crawlCallback(onRawMidiCrawl), stateCallback(onStateChange) { dashInterface = "ANY"; dashChannel = -1; g_CurrentEngineInstance = this; g_GlobalStateCallback = onStateChange; g_GlobalCrawlCallback = onRawMidiCrawl; try { midiInScanner = std::make_unique<RtMidiIn>(); queryAvailableHardware(); } catch (...) {} }
EvaluationEngine::~EvaluationEngine() { std::lock_guard<std::mutex> lock(engineMutex); for (auto& portPair : activeInputPorts) if (portPair.second) portPair.second->closePort(); activeInputPorts.clear(); for (auto& portPair : activeOutputPorts) if (portPair.second) portPair.second->closePort(); activeOutputPorts.clear(); std::lock_guard<std::mutex> glock(g_RouterLock); if (g_CurrentEngineInstance == this) { g_CurrentEngineInstance = nullptr; g_GlobalStateCallback = nullptr; g_GlobalCrawlCallback = nullptr; } }
void EvaluationEngine::queryAvailableHardware() { if (!midiInScanner) return; discoveredInputNames.clear(); unsigned int portCount = midiInScanner->getPortCount(); for (unsigned int i = 0; i < portCount; ++i) { try { discoveredInputNames.push_back(midiInScanner->getPortName(i)); } catch (...) {} } }
std::vector<std::string> EvaluationEngine::getDiscoveredInputs() const { return discoveredInputNames; }
void EvaluationEngine::selectActiveHardwareInterface(const std::string& interfaceName) { std::lock_guard<std::mutex> lock(engineMutex); dashInterface = interfaceName; }
void EvaluationEngine::transmitMidi(const std::string& portName, const std::vector<unsigned char>& message) { std::lock_guard<std::mutex> lock(engineMutex); if (activeOutputPorts.count(portName) && activeOutputPorts[portName]) { activeOutputPorts[portName]->sendMessage(&message); } }

void EvaluationEngine::rebuildPorts(const std::set<std::string>& requestedInPorts, const std::set<std::string>& requestedOutPorts) {
    std::lock_guard<std::mutex> lock(engineMutex);
    for (auto& portPair : activeInputPorts) if (portPair.second) portPair.second->closePort(); activeInputPorts.clear();
    for (auto& portPair : activeOutputPorts) if (portPair.second) portPair.second->closePort(); activeOutputPorts.clear();
    logger.log("[MIDI HUB] Whitelisting Required Hardware Ports...");
    if (!requestedInPorts.empty()) {
        try {
            RtMidiIn tempIn;
            for (const auto& req : requestedInPorts) {
                bool found = false;
                for (unsigned int i = 0; i < tempIn.getPortCount(); ++i) {
                    if (containsSubstringIgnoreCase(tempIn.getPortName(i), req)) {
                        auto port = std::make_unique<RtMidiIn>(); port->openPort(i); port->ignoreTypes(false, false, false); std::string* heapName = new std::string(tempIn.getPortName(i)); port->setCallback(&globalRtMidiStaticRouter, heapName); activeInputPorts[req] = std::move(port); logger.log("[CAPTURED MIDI IN] " + tempIn.getPortName(i)); found = true; break;
                    }
                }
                if (!found && req != "ANY") logger.log("[WARNING] Requested Input '" + req + "' not found.");
            }
        } catch (...) {}
    }
    if (!requestedOutPorts.empty()) {
        try {
            RtMidiOut tempOut;
            for (const auto& req : requestedOutPorts) {
                if (req == AppConstants::InternalDashboardID) continue; bool found = false;
                for (unsigned int i = 0; i < tempOut.getPortCount(); ++i) { if (containsSubstringIgnoreCase(tempOut.getPortName(i), req)) { auto port = std::make_unique<RtMidiOut>(); port->openPort(i); activeOutputPorts[req] = std::move(port); logger.log("[CAPTURED MIDI OUT] " + tempOut.getPortName(i)); found = true; break; } }
                if (!found && req != "ANY") logger.log("[WARNING] Requested Output '" + req + "' not found.");
            }
        } catch (...) {}
    }
}

void globalRtMidiStaticRouter(double deltatime, std::vector<unsigned char>* message, void* userData) {
    (void)deltatime; std::lock_guard<std::mutex> glock(g_RouterLock); if (!message || message->empty() || !userData || !g_CurrentEngineInstance) return;
    std::string name = *static_cast<std::string*>(userData); std::vector<unsigned char> bytes = *message; unsigned char statusByte = bytes[0];
    if (statusByte == 0xF8 && g_CurrentEngineInstance->liveGlobals.filterMidiClock) return; if (statusByte == 0xFE && g_CurrentEngineInstance->liveGlobals.filterActiveSensing) return;
    RawMidiMessage incomingMsg; if (!parseRawMidi(bytes, name, incomingMsg)) return;
    std::stringstream textStream; textStream << "[" << incomingMsg.interfaceName << "] ";
    if (incomingMsg.type == "SYSEX") { textStream << "SYSEX:"; for (size_t i = 0; i < bytes.size(); ++i) { char hexBuf[3]; sprintf_s(hexBuf, " %02X", bytes[i]); textStream << hexBuf; } }
    else { textStream << "Ch:" << incomingMsg.channel << " " << incomingMsg.type; if (incomingMsg.type == "CC" || incomingMsg.type == "NOTE_ON" || incomingMsg.type == "NOTE_OFF") textStream << " N:" << incomingMsg.ccNumber << " V:" << incomingMsg.ccValue; else if (incomingMsg.type == "PC") textStream << " P:" << incomingMsg.programNumber; }
    std::string formattedEvent = textStream.str(); if (g_GlobalCrawlCallback) g_GlobalCrawlCallback(formattedEvent);
    AppState& state = AppState::getInstance(); evaluateGtrHardware(incomingMsg, statusByte, state);
    std::string transformerLog = formattedEvent; bool ruleMatched = evaluateTransformer(incomingMsg, state, transformerLog); g_CurrentEngineInstance->getLogger().log(transformerLog);
    bool isDashboardMsg = evaluateDashboard(incomingMsg, state); if (isDashboardMsg) g_CurrentEngineInstance->getLogger().log(formattedEvent + " -> Dashboard");
    int passPcValue = (isDashboardMsg && incomingMsg.type == "PC") ? incomingMsg.programNumber : -1;
    if (ruleMatched) { if (g_GlobalStateCallback) g_GlobalStateCallback(2, passPcValue); } else if (isDashboardMsg) { if (g_GlobalStateCallback) g_GlobalStateCallback(1, passPcValue); } else { if (g_GlobalStateCallback) g_GlobalStateCallback(3, passPcValue); }
}
