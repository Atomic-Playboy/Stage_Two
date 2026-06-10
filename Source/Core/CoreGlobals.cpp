#include "Core/CoreGlobals.h"
#include "Core/AppState.h"
#include "Config/ConfigParser.h"
#include "UI/MainWindow.h"
#include <set>
#include <windows.h>

namespace CoreGlobals {
    std::unique_ptr<FileWatcher> watcher;
    std::unique_ptr<EvaluationEngine> engine;
    ThreadSafeLogger logger;
    void reloadConfiguration() {
        if (!watcher) return;
        std::string jsonPath = watcher->getTargetDashboardJson();
        std::string transPath = watcher->getTargetTransformerJson();

        logger.log("[INFO] --- System Configuration Reload Initiated ---");
        
        AppState& state = AppState::getInstance();
        std::set<std::string> requestedInPorts;
        std::set<std::string> requestedOutPorts;
        
        // 1. Load Dashboard
        WorkspaceConfig nextWorkspace;
        if (ConfigParser::loadWorkspaceJson(jsonPath, nextWorkspace, logger)) {
            state.setDashWorkspace(nextWorkspace);
            if (!state.isInterfaceOverridden()) {
                state.setActiveDashInterface(nextWorkspace.systemTuning.midiInterface);
            }
            if (!state.isChannelOverridden()) {
                state.setActiveDashChannel(nextWorkspace.systemTuning.defaultChannel);
            }
            state.loadPresetMatrixBlocks(state.getCurrentProgramChange());
            if (state.getActiveDashInterface() != "ANY" && !state.getActiveDashInterface().empty()) {
                requestedInPorts.insert(state.getActiveDashInterface());
            }
        } else {
            logger.log("[ERROR] Failed to load Dashboard JSON.");
        }

        // 2. Load Transformer
        if (engine) {
            auto rules = ConfigParser::loadTransformerJson(transPath, logger);
            engine->setTransformerRules(rules);
            engine->updateDashboardInterfaces(state.getActiveDashInterface(), state.getActiveDashChannel());
            engine->updateTuningGlobals(nextWorkspace.systemTuning);
            engine->selectActiveHardwareInterface(state.getActiveDashInterface());
            
            for (const auto& rule : rules) {
                if (rule.trigger.interfaceName != "ANY" && !rule.trigger.interfaceName.empty()) {
                    requestedInPorts.insert(rule.trigger.interfaceName);
                }
                for (const auto& trans : rule.transforms) {
                    if (trans.interfaceName != "ANY" && !trans.interfaceName.empty() && trans.interfaceName.find("GTR_SLOT_") != 0 && trans.interfaceName != "Stage One Dashboard") {
                        requestedOutPorts.insert(trans.interfaceName);
                    }
                }
            }
        }

        // 3. Load GTR Slots
        char buffer[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, buffer);
        for (int i = 0; i < 4; ++i) {
            std::string targetFile = MainWindow::searchValidWebPath("gtr" + std::to_string(i + 1) + ".json");
            GtrWorkspaceConfig gtrConfig;
            if (ConfigParser::loadGtrJsonConfig(i, targetFile, gtrConfig, logger)) {
                state.setGtrWorkspace(i, gtrConfig);
                if (!gtrConfig.inPort1.empty()) requestedInPorts.insert(gtrConfig.inPort1);
                if (!gtrConfig.inPort2.empty()) requestedInPorts.insert(gtrConfig.inPort2);
                if (!gtrConfig.outPort1.empty()) requestedOutPorts.insert(gtrConfig.outPort1);
                if (!gtrConfig.outPort2.empty()) requestedOutPorts.insert(gtrConfig.outPort2);
            }
        }

        // 4. Teardown and Rebuild Ports
        if (engine) {
            engine->rebuildPorts(requestedInPorts, requestedOutPorts);
        }

        HWND hwnd = state.getHWnd();
        if(hwnd) {
            MainWindow::SetupConsoleScrollbar(hwnd);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        logger.log("[INFO] --- Configuration Reload Complete ---");
    }
}
