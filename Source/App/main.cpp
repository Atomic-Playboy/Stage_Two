/**
 * Stage Two Production Dashboard Engine
 * Entry Point (WinMain)
 * * Architecture Decision: 
 * We use a hidden console application wrapping a Win32 GUI window.
 * This bootstraps three main concurrent systems:
 * 1. The FileWatcher (Background Thread)
 * 2. The MIDI Evaluation Engine (Background Thread via RtMidi)
 * 3. The Win32 Message Loop (Main UI Thread)
 */

#include <windows.h>
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp>

#include "Core/MidiData.h"
#include "Core/AppStateDashboard.h"
#include "Core/AppStateTransformer.h"
#include "Core/CoreGlobals.h"
#include "Core/AppConstants.h"
#include "MIDI/EvaluationEngine.h"
#include "Config/FileWatcher.h"
#include "Config/ConfigParser.h"
#include "UI/MainWindow.h"
#include "UI/Win32App.h"
#include "UI/UIConstants.h"

using json = nlohmann::json;

void checkAndShowFirstBootWarning() {
    std::string exePath = MainWindow::getAbsoluteExecutionPath("");
    std::string flagPath = exePath + ".stage_two_initialized";
    std::ifstream f(flagPath);
    
    // If the file does not exist, it's the first time running the app
    if (!f.good()) {
        MessageBoxA(NULL,
            "Welcome to Stage Two!\n\n"
            "By default, this application looks for your configuration files in your standard Windows Documents folder:\n"
            "%USERPROFILE%\\Documents\\Stage_Two\\\n\n"
            "If you have custom-mapped your Documents folder to another drive or use OneDrive, you will need to either manually update the paths in 'stage_two.json' to point to your custom directory, or create a directory junction.\n\n"
            "(This notice will only appear once.)",
            "First Boot Notice - Directory Mapping",
            MB_OK | MB_ICONINFORMATION);

        // Create the flag file so this never triggers again
        std::ofstream out(flagPath);
        out << "Stage Two initialized.\n";
    }
}

std::string expandEnvironmentVariables(const std::string& text) {
    char buffer[MAX_PATH];
    DWORD result = ExpandEnvironmentStringsA(text.c_str(), buffer, MAX_PATH);
    if (result > 0 && result <= MAX_PATH) {
        return std::string(buffer);
    }
    return text;
}

void loadStageTwoConfig() {
    std::string configFilename = "stage_two.json";
    std::vector<std::string> pathsToTry = {
        MainWindow::getAbsoluteExecutionPath(configFilename),
        MainWindow::getAbsoluteExecutionPath(AppConstants::ParentDir + configFilename),
        MainWindow::getAbsoluteExecutionPath(AppConstants::GrandparentDir + configFilename)
    };
    
    std::string foundPath = "";
    for(const auto& p : pathsToTry) {
        std::ifstream f(p);
        if(f.good()) { foundPath = p; break; }
    }

    if (foundPath.empty()) {
        MessageBoxA(NULL, "Could not find stage_two.json in the executable or parent directories. Launching with default hardcoded paths.", "Startup Warning", MB_ICONWARNING);
        return;
    }

    std::ifstream file(foundPath);
    try {
        json j;
        file >> j;
        if (j.contains("paths")) {
            auto paths = j["paths"];
            if (paths.contains("Dashboard_Configuration")) AppConstants::DashboardJson = expandEnvironmentVariables(paths["Dashboard_Configuration"].get<std::string>());
            if (paths.contains("Transformer_Configuration")) AppConstants::TransformerJson = expandEnvironmentVariables(paths["Transformer_Configuration"].get<std::string>());
            if (paths.contains("GTR_1_Configuration")) AppConstants::GtrConfigs[0] = expandEnvironmentVariables(paths["GTR_1_Configuration"].get<std::string>());
            if (paths.contains("GTR_2_Configuration")) AppConstants::GtrConfigs[1] = expandEnvironmentVariables(paths["GTR_2_Configuration"].get<std::string>());
            if (paths.contains("GTR_3_Configuration")) AppConstants::GtrConfigs[2] = expandEnvironmentVariables(paths["GTR_3_Configuration"].get<std::string>());
            if (paths.contains("GTR_4_Configuration")) AppConstants::GtrConfigs[3] = expandEnvironmentVariables(paths["GTR_4_Configuration"].get<std::string>());
            if (paths.contains("Log_File")) AppConstants::LogFile = expandEnvironmentVariables(paths["Log_File"].get<std::string>());
            if (paths.contains("Dashboard_Configure_html")) AppConstants::HtmlDashboard = expandEnvironmentVariables(paths["Dashboard_Configure_html"].get<std::string>());
            if (paths.contains("Transformer_Configure_html")) AppConstants::HtmlTransformer = expandEnvironmentVariables(paths["Transformer_Configure_html"].get<std::string>());
            if (paths.contains("Dashboard_Blender")) AppConstants::HtmlDashboardBlender = expandEnvironmentVariables(paths["Dashboard_Blender"].get<std::string>());
            if (paths.contains("Transformer_Blender")) AppConstants::HtmlTransformerBlender = expandEnvironmentVariables(paths["Transformer_Blender"].get<std::string>());
            if (paths.contains("Heavy_Web")) AppConstants::HtmlConfiguratorIndex = expandEnvironmentVariables(paths["Heavy_Web"].get<std::string>());
            if (paths.contains("GTR_Configuration")) AppConstants::HtmlGtrConfig = expandEnvironmentVariables(paths["GTR_Configuration"].get<std::string>());
            if (paths.contains("Help_html")) AppConstants::HtmlHelpManual = expandEnvironmentVariables(paths["Help_html"].get<std::string>());
            if (paths.contains("Text_Editor")) AppConstants::TextEditor = expandEnvironmentVariables(paths["Text_Editor"].get<std::string>());
        }
        if (j.contains("web_creator_theme_data")) {
            DashboardConfig initialConfig = AppStateDashboard::getInstance().getDashboardConfig();
            for (auto& el : j["web_creator_theme_data"].items()) {
                initialConfig.webCreatorThemeData[el.key()] = el.value().get<std::string>();
            }
            AppStateDashboard::getInstance().setDashboardConfig(initialConfig);
        }
    } catch (const json::exception& e) {
        MessageBoxA(NULL, (std::string("JSON Parsing Error in stage_two.json:\n") + e.what()).c_str(), "Configuration Error", MB_ICONERROR);
    } catch (...) {
        MessageBoxA(NULL, "Unknown error reading stage_two.json", "Configuration Error", MB_ICONERROR);
    }
}

// Scans standard execution paths to ensure JSON configurations are found 
// regardless of whether the app is launched via CLion, desktop shortcut, or terminal.
std::string getValidConfigPath(const std::string& filename) {
    std::vector<std::string> pathsToTry = {
        MainWindow::getAbsoluteExecutionPath(filename),
        MainWindow::getAbsoluteExecutionPath(AppConstants::ParentDir + filename),
        MainWindow::getAbsoluteExecutionPath(AppConstants::GrandparentDir + filename),
        filename // Fallback to raw string in case the config provides an absolute path
    };
    for(const auto& p : pathsToTry) {
        std::ifstream f(p); if(f.good()) return p;
    }
    return filename;
}

// [STUDY GUIDE: Petzold, Chapter 1 - "Getting Started" -> The WinMain Entry Point]
// This is the historical entry point for all Windows applications, replacing the standard C++ main().
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    (void)hPrev;
    (void)lpCmd;

    // Trigger the warning dialog if this is the very first execution
    checkAndShowFirstBootWarning();

    // Bootstrap paths and theme values from stage_two.json before doing anything else
    loadStageTwoConfig();

    std::string localJson = getValidConfigPath(AppConstants::DashboardJson);
    std::string transJson = getValidConfigPath(AppConstants::TransformerJson);

    // Initialize background thread for hot-reloading configurations
    CoreGlobals::watcher = std::make_unique<FileWatcher>();
    CoreGlobals::watcher->initializePaths(localJson, transJson);
    CoreGlobals::watcher->startWatching(CoreGlobals::reloadConfiguration);

    // [STUDY GUIDE: Williams, Chapter 4 - "Synchronizing concurrent operations"]
    // Lambdas act as bridges passing background MIDI events to the main Win32 UI thread safely.
    CoreGlobals::engine = std::make_unique<EvaluationEngine>(CoreGlobals::logger,
        [](const std::string& patch) { (void)patch; },
        [](const std::string& crawl) { MainWindow::pushCrawlMessage(crawl); },
        [](int stateKind, int pcValue) {
            HWND hwnd = AppStateDashboard::getInstance().getHWnd();
            if (hwnd) {
                // [STUDY GUIDE: Petzold, Chapter 3 - "The Message Loop"]
                // PostMessage pushes a thread-safe update into the Win32 queue without blocking the MIDI engine.
                PostMessage(hwnd, WM_USER + 100, (WPARAM)stateKind, 0);
                if (pcValue != -1) {
                    PostMessage(hwnd, WM_USER + 101, (WPARAM)pcValue, 0);
                }
            }
        }
    );

    auto inputs = CoreGlobals::engine->getDiscoveredInputs();
    AppStateTransformer::getInstance().setDiscoveredInputs(inputs);

    // Force an initial parse of all JSON files before launching UI
    CoreGlobals::reloadConfiguration();

    // Launch the blocking Win32 UI loop
    Win32App::InitializeAndRun(hInst, nShow);

    CoreGlobals::watcher->stopWatching();
    return 0;
}
