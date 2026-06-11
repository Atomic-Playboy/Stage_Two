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

#include "Core/MidiData.h"
#include "Core/AppState.h"
#include "Core/CoreGlobals.h"
#include "Core/AppConstants.h"
#include "MIDI/EvaluationEngine.h"
#include "Config/FileWatcher.h"
#include "Config/ConfigParser.h"
#include "UI/MainWindow.h"
#include "UI/Win32App.h"
#include "UI/UIConstants.h"

// Scans standard execution paths to ensure JSON configurations are found 
// regardless of whether the app is launched via CLion, desktop shortcut, or terminal.
std::string getValidConfigPath(const std::string& filename) {
    std::vector<std::string> pathsToTry = {
        MainWindow::getAbsoluteExecutionPath(filename),
        MainWindow::getAbsoluteExecutionPath(AppConstants::ParentDir + filename),
        MainWindow::getAbsoluteExecutionPath(AppConstants::GrandparentDir + filename)
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
            HWND hwnd = AppState::getInstance().getHWnd();
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
    AppState::getInstance().setDiscoveredInputs(inputs);

    // Force an initial parse of all JSON files before launching UI
    CoreGlobals::reloadConfiguration();
    
    // Launch the blocking Win32 UI loop
    Win32App::InitializeAndRun(hInst, nShow);

    CoreGlobals::watcher->stopWatching();
    return 0;
}
