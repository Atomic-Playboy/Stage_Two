#include "UI/Win32App.h"
#include "UI/MainWindow.h"
#include "UI/UIConstants.h"
#include "Core/AppState.h"
#include "Core/CoreGlobals.h"
#include "Core/AppConstants.h"
#include <commdlg.h>

void Win32App::SelectAndOpenTargetFile(HWND hwnd, int fileKindIndex) {
    OPENFILENAMEA ofn;
    char szFile[MAX_PATH] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileNameA(&ofn) == TRUE) {
        if (fileKindIndex == 1) {
            CoreGlobals::watcher->updateDashboardJsonPath(szFile);
        } else {
            CoreGlobals::watcher->updateTransformerJsonPath(szFile);
        }
        CoreGlobals::reloadConfiguration();
    }
}

void Win32App::RebuildMasterMenu(HWND hwnd, const std::vector<std::string>& inputs) {
    HMENU hMenu = CreateMenu();
    HMENU fileMenu = CreatePopupMenu();
    AppendMenuA(fileMenu, MF_STRING, UIConstants::CMD_OPEN_DASHBOARD, "Open Dashboard JSON...");
    AppendMenuA(fileMenu, MF_STRING, UIConstants::CMD_OPEN_TRANSFORMER, "Open Transformer JSON...");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)fileMenu, "File");
    HMENU viewMenu = CreatePopupMenu();
    AppendMenuA(viewMenu, MF_STRING, UIConstants::CMD_VIEW_DASHBOARD, "View Dashboard (F4)");
    AppendMenuA(viewMenu, MF_STRING, UIConstants::CMD_VIEW_GTR, "View GTR (F5)");
    AppendMenuA(viewMenu, MF_STRING, UIConstants::CMD_VIEW_CONSOLE, "View Console (F6)");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)viewMenu, "View");

    HMENU hSubMenu = CreatePopupMenu();
    AppendMenuA(hSubMenu, MF_STRING, UIConstants::CMD_HARDWARE_ANY, "ANY INTERFACE");
    for (size_t i = 0; i < inputs.size(); ++i) {
        AppendMenuA(hSubMenu, MF_STRING, UIConstants::CMD_HARDWARE_BASE + i, inputs[i].c_str());
    }
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "Hardware Interfaces");

    HMENU hChanMenu = CreatePopupMenu();
    AppendMenuA(hChanMenu, MF_STRING, UIConstants::CMD_CHANNEL_ANY, "ANY CHANNEL");
    for (int i = 1; i <= 16; ++i) {
        AppendMenuA(hChanMenu, MF_STRING, UIConstants::CMD_CHANNEL_BASE + i, ("Channel " + std::to_string(i)).c_str());
    }
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hChanMenu, "MIDI Channel");

    HMENU configMenu = CreatePopupMenu();
    AppendMenuA(configMenu, MF_STRING, UIConstants::CMD_WEB_DASHBOARD, "Web Dashboard Creator");
    AppendMenuA(configMenu, MF_STRING, UIConstants::CMD_WEB_TRANSFORMER, "Web Transformer Creator");
    AppendMenuA(configMenu, MF_STRING, UIConstants::CMD_DASH_BLENDER, "Dashboard Blender");
    AppendMenuA(configMenu, MF_STRING, UIConstants::CMD_TRANS_BLENDER, "Transformer Blender");
    AppendMenuA(configMenu, MF_STRING, UIConstants::CMD_HEAVY_WEB, "Heavy Web");
    AppendMenuA(configMenu, MF_STRING, UIConstants::CMD_EDIT_DASHBOARD, "Edit dashboard.json (Notepad)");
    AppendMenuA(configMenu, MF_STRING, UIConstants::CMD_EDIT_TRANSFORMER, "Edit transformer.json (Notepad)");
    AppendMenuA(configMenu, MF_STRING, UIConstants::CMD_GTR_CONFIG, "GTR Configuration");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)configMenu, "Configuration");

    HMENU helpMenu = CreatePopupMenu();
    AppendMenuA(helpMenu, MF_STRING, UIConstants::CMD_HELP_MANUAL, "Help / Manual");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)helpMenu, "Help");

    SetMenu(hwnd, hMenu);
}

// [STUDY GUIDE: Petzold, Chapter 1 - "Registering the Window Class"]
bool Win32App::InitializeAndRun(HINSTANCE hInst, int nShow) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, MainWindow::WndProc, 0, 0, hInst, NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, "StageOneClass", NULL };
    if(!RegisterClassEx(&wc)) return false;
    HWND hwnd = CreateWindowExA(0, "StageOneClass", "Stage One Production Dashboard Engine (Integrated GTR)", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1150, 840, NULL, NULL, hInst, NULL);
    if(!hwnd) return false;

    AppState::getInstance().setHWnd(hwnd);
    RebuildMasterMenu(hwnd, AppState::getInstance().getDiscoveredInputs());
    
    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    // [STUDY GUIDE: Petzold, Chapter 3 - "The Message Loop"]
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}
