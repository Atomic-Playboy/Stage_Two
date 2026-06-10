#include "Logic/PresetManager.h"
#include "Core/AppState.h"
#include "Core/Utils.h"
#include "Core/CoreGlobals.h"
#include <thread>
#include <chrono>
#include <vector>

std::vector<unsigned char> generateTextSysEx(unsigned char address, const std::string& textStr) {
    std::vector<unsigned char> sysex(15, 0x00);
    sysex[0] = 0xF0;
    sysex[1] = 0x7F;
    sysex[2] = 0x00;
    sysex[3] = 0x20;
    sysex[4] = 0x66;
    sysex[5] = address;
    sysex[6] = 0x00;
    sysex[7] = 0x00;

    std::string paddedText = textStr;
    while (paddedText.length() < 3) paddedText += " ";
    for (int k = 0; k < 3; ++k) {
        unsigned char outerByte, innerByte;
        mapAsciiToGtrSegments(paddedText[k], outerByte, innerByte);
        sysex[8 + (k * 2)] = outerByte;
        sysex[9 + (k * 2)] = innerByte;
    }

    sysex[14] = 0xF7;
    return sysex;
}

std::vector<unsigned char> generateLedSysEx(unsigned char commandByte, unsigned char value) {
    std::vector<unsigned char> sysex(8, 0x00);
    sysex[0] = 0xF0;
    sysex[1] = 0x7F;
    sysex[2] = 0x00;
    sysex[3] = 0x20;
    sysex[4] = 0x66;
    sysex[5] = commandByte;
    sysex[6] = value;
    sysex[7] = 0xF7;
    return sysex;
}

void dispatchMidiOut(const std::string& portName, const std::vector<unsigned char>& msg) {
    if (CoreGlobals::engine && !portName.empty()) {
        CoreGlobals::engine->transmitMidi(portName, msg);
    }
}

void activatePreset(int slotIndex, const Preset& preset, GtrWorkspaceConfig& config) {
    std::string actionLog = "Hardware Out 1 updated.";
    AppState& state = AppState::getInstance();

    for(int k = 0; k < 7; k++) {
        state.setGtrDisplayState(slotIndex, k, preset.displays[k]);
    }
    config.activePresetName = preset.name;
    config.activePcNum = preset.pcTrigger;
    config.activeAbState = preset.abInitialState;
    state.setGtrWorkspace(slotIndex, config);
    auto msg = generateTextSysEx(0x47, preset.displays[0].isActive ? preset.displays[0].text : "");
    dispatchMidiOut(config.outPort1, msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    for (int k = 1; k <= 6; ++k) {
        msg = generateTextSysEx(static_cast<unsigned char>(0x40 + k), preset.displays[k].isActive ? preset.displays[k].text : "");
        dispatchMidiOut(config.outPort1, msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    msg = generateLedSysEx(0x57, preset.displays[0].isActive ? (preset.displays[0].isBright ? 0x0F : 0x08) : 0x00);
    dispatchMidiOut(config.outPort1, msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));

    for (int k = 1; k <= 6; ++k) {
        msg = generateLedSysEx(static_cast<unsigned char>(0x50 + k), preset.displays[k].isActive ? (preset.displays[k].isBright ? 0x0F : 0x08) : 0x00);
        dispatchMidiOut(config.outPort1, msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    msg = generateLedSysEx(0x62, static_cast<unsigned char>(preset.abInitialState));
    dispatchMidiOut(config.outPort1, msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    if (preset.sendPc && preset.pcTrigger >= 0 && preset.pcTrigger <= 127) {
        unsigned char channelByte = getOutputChannelByte(0xC0, config.outChan2);
        std::vector<unsigned char> pcMsg = {channelByte, static_cast<unsigned char>(preset.pcTrigger)};
        dispatchMidiOut(config.outPort2, pcMsg);
        actionLog += " PC " + std::to_string(preset.pcTrigger) + " forwarded.";
    }

    if (preset.sendConfig) {
        unsigned char channelByte = getOutputChannelByte(0xB0, config.outChan2);
        for (int k = 0; k < 6; k++) {
            if (preset.displays[k + 1].isActive) {
                unsigned char ccNum = static_cast<unsigned char>(16 + k);
                unsigned char ccVal = preset.displays[k + 1].isBright ? 127 : 0;
                std::vector<unsigned char> ccMsg = {channelByte, ccNum, ccVal};
                dispatchMidiOut(config.outPort2, ccMsg);
                state.setPhysicalButtonState(ccNum, (ccVal == 127));
            }
        }
        unsigned char abVal = (preset.abInitialState == preset.abOnState) ? 127 : 0;
        std::vector<unsigned char> ccAbMsg = {channelByte, 22, abVal};
        dispatchMidiOut(config.outPort2, ccAbMsg);
        actionLog += " Config CCs synced (Active only).";
    }
    CoreGlobals::logger.log("[Preset Triggered: " + preset.name + "] " + actionLog);
}

void manualForceSendLights(int slotIndex) {
    AppState& state = AppState::getInstance();
    GtrWorkspaceConfig config = state.getGtrWorkspace(slotIndex);
    if (!config.loaded) return;
    DisplayState upper = state.getGtrDisplayState(slotIndex, 0);
    auto msg = generateTextSysEx(0x47, upper.isActive ? upper.text : "");
    dispatchMidiOut(config.outPort1, msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    msg = generateLedSysEx(0x57, upper.isActive ? (upper.isBright ? 0x0F : 0x08) : 0x00);
    dispatchMidiOut(config.outPort1, msg);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    for (int k = 1; k <= 6; ++k) {
        auto btn = state.getGtrDisplayState(slotIndex, k);
        msg = generateTextSysEx(static_cast<unsigned char>(0x40 + k), btn.isActive ? btn.text : "");
        dispatchMidiOut(config.outPort1, msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        msg = generateLedSysEx(static_cast<unsigned char>(0x50 + k), btn.isActive ? (btn.isBright ? 0x0F : 0x08) : 0x00);
        dispatchMidiOut(config.outPort1, msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    msg = generateLedSysEx(0x62, static_cast<unsigned char>(config.activeAbState));
    dispatchMidiOut(config.outPort1, msg);
    CoreGlobals::logger.log("[Send Lights] Forced Hardware Lights completed on slot " + std::to_string(slotIndex + 1));
}

void manualForceSendMidi(int slotIndex) {
    AppState& state = AppState::getInstance();
    GtrWorkspaceConfig config = state.getGtrWorkspace(slotIndex);
    if (!config.loaded) return;
    bool sendPc = false;
    bool sendConfig = false;
    int abOnState = 0;
    int pcTrigger = config.activePcNum;
    for (const auto& preset : config.presets) {
        if (preset.pcTrigger == config.activePcNum) {
            sendPc = preset.sendPc;
            sendConfig = preset.sendConfig;
            abOnState = preset.abOnState;
            break;
        }
    }

    if (sendPc && pcTrigger >= 0 && pcTrigger <= 127) {
        unsigned char channelByte = getOutputChannelByte(0xC0, config.outChan2);
        std::vector<unsigned char> pcMsg = {channelByte, static_cast<unsigned char>(pcTrigger)};
        dispatchMidiOut(config.outPort2, pcMsg);
    }

    if (sendConfig) {
        unsigned char channelByte = getOutputChannelByte(0xB0, config.outChan2);
        for (int k = 0; k < 6; k++) {
            DisplayState ds = state.getGtrDisplayState(slotIndex, k + 1);
            if (ds.isActive) {
                unsigned char ccNum = static_cast<unsigned char>(16 + k);
                unsigned char ccVal = ds.isBright ? 127 : 0;
                std::vector<unsigned char> ccMsg = {channelByte, ccNum, ccVal};
                dispatchMidiOut(config.outPort2, ccMsg);
            }
        }
        unsigned char abVal = (config.activeAbState == abOnState) ? 127 : 0;
        std::vector<unsigned char> ccAbMsg = {channelByte, 22, abVal};
        dispatchMidiOut(config.outPort2, ccAbMsg);
    }
    CoreGlobals::logger.log("[Send MIDI] Panic Resync completed on slot " + std::to_string(slotIndex + 1));
}

void manualStepProgramChange(int slotIndex, int direction) {
    AppState& state = AppState::getInstance();
    GtrWorkspaceConfig config = state.getGtrWorkspace(slotIndex);
    if (!config.loaded || config.presets.empty()) return;

    int targetPc = config.activePcNum + direction;
    if (targetPc < 0) targetPc = 127;
    if (targetPc > 127) targetPc = 0;

    config.activePcNum = targetPc;
    bool found = false;
    for (const auto& preset : config.presets) {
        if (preset.pcTrigger == targetPc) {
            CoreGlobals::logger.log("[Manual Step] Opening PC " + std::to_string(targetPc));
            activatePreset(slotIndex, preset, config);
            found = true;
            break;
        }
    }

    if (!found) {
        for (int k = 0; k < 7; ++k) {
            DisplayState blank;
            state.setGtrDisplayState(slotIndex, k, blank);
        }
        config.activePresetName = "Blank";
        config.activeAbState = 0;
        state.setGtrWorkspace(slotIndex, config);
    }
}
