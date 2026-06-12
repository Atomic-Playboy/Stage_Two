/**
 * CoreGlobals.cpp
 * Coordinates the teardown and reconstruction of the application's physical routing 
 * when the hot-reloader detects JSON modifications.
 */

#include "Core/CoreGlobals.h"
#include "Core/AppStateDashboard.h"
#include "Core/AppStateGTR.h"
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
        
        
        std::set<std::string> requestedInPorts;
        std::set<std::string> requestedOutPorts;

        // 1. Load Dashboard matrix
        DashboardConfig nextDashboard;
        if (ConfigParser::loadDashboardJson(jsonPath, nextDashboard, logger)) {
            AppStateDashboard::getInstance().setDashboardConfig(nextDashboard);
            if (!AppStateDashboard::getInstance().isInterfaceOverridden()) {
                AppStateDashboard::getInstance().setActiveDashInterface(nextDashboard.systemTuning.midiInterface);
            }
            if (!AppStateDashboard::getInstance().isChannelOverridden()) {
                AppStateDashboard::getInstance().setActiveDashChannel(nextDashboard.systemTuning.defaultChannel);
            }
            AppStateDashboard::getInstance().loadPresetMatrixBlocks(AppStateDashboard::getInstance().getCurrentProgramChange());
            if (AppStateDashboard::getInstance().getActiveDashInterface() != "ANY" && !AppStateDashboard::getInstance().getActiveDashInterface().empty()) {
                requestedInPorts.insert(AppStateDashboard::getInstance().getActiveDashInterface());
            }
        } else {
            logger.log("[ERROR] Failed to load Dashboard JSON.");
        }

        // 2. Load Transformer rules
        if (engine) {
            auto rules = ConfigParser::loadTransformerJson(transPath, logger);
            engine->setTransformerRules(rules);
            engine->updateDashboardInterfaces(AppStateDashboard::getInstance().getActiveDashInterface(), AppStateDashboard::getInstance().getActiveDashChannel());
            engine->updateTuningGlobals(nextDashboard.systemTuning);
            engine->selectActiveHardwareInterface(AppStateDashboard::getInstance().getActiveDashInterface());
            
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
            std::string targetFile = MainWindow::searchValidWebPath(AppConstants::GtrConfigs[i]);
            GtrSlotConfig gtrConfig;
            if (ConfigParser::loadGtrJsonConfig(i, targetFile, gtrConfig, logger)) {
                AppStateGTR::getInstance().setGtrSlotConfig(i, gtrConfig);
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
        HWND hwnd = AppStateDashboard::getInstance().getHWnd();
        if(hwnd) {
            MainWindow::SetupConsoleScrollbar(hwnd);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        logger.log("[INFO] --- Configuration Reload Complete ---");
    }
}
