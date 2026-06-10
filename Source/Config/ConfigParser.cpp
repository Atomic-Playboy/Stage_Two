#include "Config/ConfigParser.h"
#include "Core/Utils.h"
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool ConfigParser::loadWorkspaceJson(const std::string& filePath, WorkspaceConfig& workspace, ThreadSafeLogger& logger) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        logger.log("[ERROR] Could not open Workspace config path: " + filePath);
        return false;
    }
    try {
        json j; file >> j;
        if (j.contains("system_tuning")) {
            auto st = j["system_tuning"];
            if (st.contains("midi_interface")) workspace.systemTuning.midiInterface = st["midi_interface"].get<std::string>();
            if (st.contains("default_channel")) workspace.systemTuning.defaultChannel = st["default_channel"].get<int>();
            if (st.contains("cc_on_default_value")) workspace.systemTuning.ccOnDefaultValue = st["cc_on_default_value"].get<int>();
            if (st.contains("cc_off_default_value")) workspace.systemTuning.ccOffDefaultValue = st["cc_off_default_value"].get<int>();
            if (st.contains("debug_history_limit")) workspace.systemTuning.debugHistoryLimit = st["debug_history_limit"].get<int>();
            if (st.contains("crawl_speed")) workspace.systemTuning.crawlSpeed = st["crawl_speed"].get<int>();
            if (st.contains("orb_blink_duration")) workspace.systemTuning.orbBlinkDuration = st["orb_blink_duration"].get<int>();
            if (st.contains("filter_midi_clock")) workspace.systemTuning.filterMidiClock = (st["filter_midi_clock"].get<int>() != 0);
            if (st.contains("filter_active_sensing")) workspace.systemTuning.filterActiveSensing = (st["filter_active_sensing"].get<int>() != 0);
        }
        if (j.contains("program_change_dashboard_matrix")) {
            workspace.programChangeDashboardMatrix.clear();
            auto matrixJson = j["program_change_dashboard_matrix"];
            for (auto& item : matrixJson.items()) {
                int pcNumber = std::stoi(item.key());
                auto pcNode = item.value();
                PresetMatrixNode preset;
                if (pcNode.contains("active")) preset.active = pcNode["active"].get<bool>();
                if (pcNode.contains("song_title")) preset.songTitle = pcNode["song_title"].get<std::string>();
                if (pcNode.contains("interactive_tiles") && pcNode["interactive_tiles"].is_array()) {
                    for (const auto& tileJson : pcNode["interactive_tiles"]) {
                        DashboardBlock block;
                        if (tileJson.contains("effect")) block.effect = tileJson["effect"].get<std::string>();
                        if (tileJson.contains("cc")) block.cc = tileJson["cc"].get<int>();
                        
                        if (tileJson.contains("Tile colours")) block.colorName = tileJson["Tile colours"].get<std::string>();
                        else if (tileJson.contains("color")) block.colorName = tileJson["color"].get<std::string>();
                        
                        if (tileJson.contains("Text colour")) block.textColorName = tileJson["Text colour"].get<std::string>();
                        else if (tileJson.contains("text_color")) block.textColorName = tileJson["text_color"].get<std::string>();
                        
                        if (tileJson.contains("load_state")) block.loadState = tileJson["load_state"].get<std::string>();
                        block.transientState = (block.loadState == "Bright");
                        if (tileJson.contains("cc_on_override") && !tileJson["cc_on_override"].is_null()) block.ccOnOverride = tileJson["cc_on_override"].get<int>();
                        if (tileJson.contains("cc_off_override") && !tileJson["cc_off_override"].is_null()) block.ccOffOverride = tileJson["cc_off_override"].get<int>();
                        preset.interactiveTiles.push_back(block);
                    }
                }
                workspace.programChangeDashboardMatrix[pcNumber] = preset;
            }
        }
        if (j.contains("theme_colors")) {
            for (auto& item : j["theme_colors"].items()) {
                std::string colorName = item.key();
                auto node = item.value();
                if (node.contains("dim") && node.contains("bright")) {
                    auto dimArr = node["dim"];
                    auto brightArr = node["bright"];
                    DualColorNode colorNode;
                    colorNode.dim = RGB(dimArr[0].get<int>(), dimArr[1].get<int>(), dimArr[2].get<int>());
                    colorNode.bright = RGB(brightArr[0].get<int>(), brightArr[1].get<int>(), brightArr[2].get<int>());
                    workspace.themeColors[colorName] = colorNode;
                }
            }
        }
        logger.log("[INFO] Workspace Dashboard configuration loaded from: " + filePath);
        return true;
    } catch (...) {
        logger.log("[ERROR] Exceptions caught during parsing Workspace JSON: " + filePath);
        return false;
    }
}

std::vector<MIDITrigger> ConfigParser::loadTransformerJson(const std::string& filePath, ThreadSafeLogger& logger) {
    std::vector<MIDITrigger> rules;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        logger.log("[ERROR] Could not open Transformer configuration: " + filePath);
        return rules;
    }
    try {
        json j; file >> j;
        if (j.contains("triggers") && j["triggers"].is_array()) {
            for (const auto& ruleJson : j["triggers"]) {
                MIDITrigger rule;
                if (ruleJson.contains("priority")) rule.priority = ruleJson["priority"].get<std::string>();
                if (ruleJson.contains("comment")) rule.comment = ruleJson["comment"].get<std::string>();
                if (ruleJson.contains("trigger")) {
                    auto t = ruleJson["trigger"];
                    if (t.contains("int")) rule.trigger.interfaceName = t["int"].get<std::string>();
                    if (t.contains("chan")) rule.trigger.channel = t["chan"].get<std::string>();
                    if (t.contains("type")) rule.trigger.type = t["type"].get<std::string>();
                    if (t.contains("details") && t["details"].is_array()) {
                        for (const auto& d : t["details"]) rule.trigger.details.push_back(d.get<std::string>());
                    }
                }
                if (ruleJson.contains("transforms") && ruleJson["transforms"].is_array()) {
                    for (const auto& tJson : ruleJson["transforms"]) {
                        Transform trans;
                        if (tJson.contains("int")) trans.interfaceName = tJson["int"].get<std::string>();
                        if (tJson.contains("chan")) trans.channel = tJson["chan"].get<std::string>();
                        if (tJson.contains("type")) trans.type = tJson["type"].get<std::string>();
                        if (tJson.contains("details") && tJson["details"].is_array()) {
                            for (const auto& d : tJson["details"]) trans.details.push_back(d.get<std::string>());
                        }
                        rule.transforms.push_back(trans);
                    }
                }
                rules.push_back(rule);
            }
        }
        logger.log("[INFO] Transformer configurations loaded successfully from: " + filePath);
    } catch (...) {
        logger.log("[ERROR] Exceptions caught during parsing Transformer configuration: " + filePath);
    }
    return rules;
}

bool ConfigParser::loadGtrJsonConfig(int slotIndex, const std::string& filePath, GtrWorkspaceConfig& config, ThreadSafeLogger& logger) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        logger.log("[WARNING] Optional GTR config path not found on slot " + std::to_string(slotIndex + 1) + ": " + filePath);
        return false;
    }
    try {
        json j; file >> j;
        config.filepath = filePath;
        size_t slashPos = filePath.find_last_of("/\\");
        config.filename = (slashPos != std::string::npos) ? filePath.substr(slashPos + 1) : filePath;
        if (j.contains("midi_config")) {
            auto mc = j["midi_config"];
            config.inPort1 = mc.value("in_port", "");
            config.inPort2 = mc.value("in_port_2", "");
            config.outPort1 = mc.value("out_port", "");
            config.outPort2 = mc.value("out_port_2", "");
            config.inChan = mc.value("in_chan", "omni");
            config.inChan2 = mc.value("in_chan_2", "omni");
            config.outChan = mc.value("out_chan", "omni");
            config.outChan2 = mc.value("out_chan_2", "omni");
        }

        config.presets.clear();
        if (j.contains("presets") && j["presets"].is_array()) {
            for (const auto& p : j["presets"]) {
                Preset preset;
                preset.name = p.value("preset_name", "Unknown");
                preset.pcTrigger = p.value("pc_trigger", -1);
                preset.sendPc = (p.value("send_PC", 0) == 1);
                preset.sendConfig = (p.value("send_config", 0) == 1);
                if (p.contains("ab_box")) {
                    auto ab = p["ab_box"];
                    preset.abOffState = safeStoi(ab.value("off_state", "0"));
                    preset.abOnState = safeStoi(ab.value("on_state", "0"));
                    preset.abInitialState = safeStoi(ab.value("initial_state", "0"));
                }

                if (p.contains("displays") && p["displays"].is_array()) {
                    auto dispArray = p["displays"];
                    for (int k = 0; k < 7 && k < (int)dispArray.size(); ++k) {
                        preset.displays[k].text = dispArray.at(k).value("text", "");
                        preset.displays[k].isActive = (dispArray.at(k).value("enabled", "inactive") == "active");
                        preset.displays[k].isBright = (dispArray.at(k).value("brightness", "dim") == "bright");
                    }
                }
                config.presets.push_back(preset);
            }
        }
        config.loaded = true;
        logger.log("[INFO] GTR slot " + std::to_string(slotIndex + 1) + " workspace initialized from: " + filePath);
        return true;
    } catch (...) {
        logger.log("[ERROR] Parse crash during loading GTR configuration layout: " + filePath);
        return false;
    }
}
