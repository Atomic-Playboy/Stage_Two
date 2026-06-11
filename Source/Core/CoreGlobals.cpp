/**
 * CoreGlobals.cpp
 * Coordinates the teardown and reconstruction of the application's physical routing 
 * when the hot-reloader detects JSON modifications.
 */

#include "Core/CoreGlobals.h"
#include "Core/AppState.h"
#include "Core/AppConstants.h"
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
        
        // 1. Load Dashboard matrix
        DashboardConfig nextDashboard;
        if (ConfigParser::loadDashboardJson(jsonPath, nextDashboard, logger)) {
            state.setDashboardConfig(nextDashboard);
            if (!state.isInterfaceOverridden()) {
                state.setActiveDashInterface(nextDashboard.systemTuning.midiInterface);
            }
            if (!state.isChannelOverridden()) {
                state.setActiveDashChannel(nextDashboard.systemTuning.defaultChannel);
            }
            state.loadPresetMatrixBlocks(state.getCurrentProgramChange());
            if (state.getActiveDashInterface() != "ANY" && !state.getActiveDashInterface().empty()) {
                requestedInPorts.insert(state.getActiveDashInterface());
            }
        } else {
            logger.log("[ERROR] Failed to load Dashboard JSON.");
        }

        // 2. Load Transformer rules
        if (engine) {
            auto rules = ConfigParser::loadTransformerJson(transPath, logger);
            engine->setTransformerRules(rules);
            engine->updateDashboardInterfaces(state.getActiveDashInterface(), state.getActiveDashChannel());
            engine->updateTuningGlobals(nextDashboard.systemTuning);
            engine->selectActiveHardwareInterface(state.getActiveDashInterface());
            
            for (const auto& rule : rules) {
                if (rule.trigger.interfaceName != "ANY" && !rule.trigger.interfaceName.empty()) {
                    requestedInPorts.insert(rule.trigger.interfaceName);
                }
                for (const auto& trans : rule.transforms) {
                    // Bypass internal virtual components during port whitelisting
                    if (trans.interfaceName != "ANY" && !trans.interfaceName.empty() && 
                        trans.interfaceName.find("GTR_SLOT_") != 0 && 
                        trans.interfaceName != AppConstants::InternalDashboardID) {
                        requestedOutPorts.insert(trans.interfaceName);
                    }
                }
            }
        }

        // 3. Load GTR Hardware Slots
        char buffer[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, buffer);
        for (int i = 0; i < AppConstants::MAX_HARDWARE_SLOTS; ++i) {
            std::string targetFile = MainWindow::searchValidWebPath("gtr" + std::to_string(i + 1) + ".json");
            GtrSlotConfig gtrConfig;
            if (ConfigParser::loadGtrJsonConfig(i, targetFile, gtrConfig, logger)) {
                state.setGtrSlotConfig(i, gtrConfig);
                if (!gtrConfig.inPort1.empty()) requestedInPorts.insert(gtrConfig.inPort1);
                if (!gtrConfig.inPort2.empty()) requestedInPorts.insert(gtrConfig.inPort2);
                if (!gtrConfig.outPort1.empty()) requestedOutPorts.insert(gtrConfig.outPort1);
                if (!gtrConfig.outPort2.empty()) requestedOutPorts.insert(gtrConfig.outPort2);
            }
        }

        // 4. Teardown and Rebuild Physical Ports based on JSON requirements
        if (engine) {
            engine->rebuildPorts(requestedInPorts, requestedOutPorts);
        }

        // [STUDY GUIDE: Petzold, Chapter 4 - "An Intro to GDI" -> Invalidating the Window]
        // Invalidating the rect forces Windows to post a priority WM_PAINT message, updating the UI.
        HWND hwnd = state.getHWnd();
        if(hwnd) {
            MainWindow::SetupConsoleScrollbar(hwnd);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        logger.log("[INFO] --- Configuration Reload Complete ---");
    }
}
