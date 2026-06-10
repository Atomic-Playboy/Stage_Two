#include <windows.h>
#include <memory>
#include <vector>
#include <string>
#include <fstream>

#include "Core/MidiData.h"
#include "Core/AppState.h"
#include "Core/CoreGlobals.h"
#include "Core/PathConstants.h"
#include "MIDI/EvaluationEngine.h"
#include "Config/FileWatcher.h"
#include "Config/ConfigParser.h"
#include "UI/MainWindow.h"
#include "UI/Win32App.h"

std::string getValidConfigPath(const std::string& filename) {
    std::vector<std::string> pathsToTry = {
        MainWindow::getAbsoluteExecutionPath(filename),
        MainWindow::getAbsoluteExecutionPath(PathConstants::ParentDir + filename),
        MainWindow::getAbsoluteExecutionPath(PathConstants::GrandparentDir + filename)
    };
    for(const auto& p : pathsToTry) {
        std::ifstream f(p); if(f.good()) return p;
    }
    return filename;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    (void)hPrev;
    (void)lpCmd;

    std::string localJson = getValidConfigPath(PathConstants::DashboardJson);
    std::string transJson = getValidConfigPath(PathConstants::TransformerJson);

    CoreGlobals::watcher = std::make_unique<FileWatcher>();
    CoreGlobals::watcher->initializePaths(localJson, transJson);
    CoreGlobals::watcher->startWatching(CoreGlobals::reloadConfiguration);
    CoreGlobals::engine = std::make_unique<EvaluationEngine>(CoreGlobals::logger,
        [](const std::string& patch) { (void)patch; },
        [](const std::string& crawl) { MainWindow::pushCrawlMessage(crawl); },
        [](int stateKind, int pcValue) {
            HWND hwnd = AppState::getInstance().getHWnd();
            if (hwnd) {
                PostMessage(hwnd, WM_USER + 100, (WPARAM)stateKind, 0);
                if (pcValue != -1) {
                    PostMessage(hwnd, WM_USER + 101, (WPARAM)pcValue, 0);
                }
            }
        }
    );
    
    auto inputs = CoreGlobals::engine->getDiscoveredInputs();
    AppState::getInstance().setDiscoveredInputs(inputs);

    CoreGlobals::reloadConfiguration();
    Win32App::InitializeAndRun(hInst, nShow);

    CoreGlobals::watcher->stopWatching();
    return 0;
}
