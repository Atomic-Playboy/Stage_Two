#include "UI/Win32App.h"
#include "UI/MainWindow.h"
#include "Core/AppState.h"
#include "Core/CoreGlobals.h"
#include "Core/PathConstants.h"
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
    AppendMenuA(fileMenu, MF_STRING, 1001, "Open Dashboard JSON...");
    AppendMenuA(fileMenu, MF_STRING, 1002, "Open Transformer JSON...");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)fileMenu, "File");
    HMENU viewMenu = CreatePopupMenu();
    AppendMenuA(viewMenu, MF_STRING, 3104, "View Dashboard (F4)");
    AppendMenuA(viewMenu, MF_STRING, 3105, "View GTR (F5)");
    AppendMenuA(viewMenu, MF_STRING, 3106, "View Console (F6)");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)viewMenu, "View");

    HMENU hSubMenu = CreatePopupMenu();
    AppendMenuA(hSubMenu, MF_STRING, 2001, "ANY INTERFACE");
    for (size_t i = 0; i < inputs.size(); ++i) {
        AppendMenuA(hSubMenu, MF_STRING, 2002 + i, inputs[i].c_str());
    }
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "Hardware Interfaces");

    HMENU hChanMenu = CreatePopupMenu();
    AppendMenuA(hChanMenu, MF_STRING, 2100, "ANY CHANNEL");
    for (int i = 1; i <= 16; ++i) {
        AppendMenuA(hChanMenu, MF_STRING, 2100 + i, ("Channel " + std::to_string(i)).c_str());
    }
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hChanMenu, "MIDI Channel");

    HMENU configMenu = CreatePopupMenu();
    AppendMenuA(configMenu, MF_STRING, 3001, "Web Dashboard Creator");
    AppendMenuA(configMenu, MF_STRING, 3002, "Web Transformer Creator");
    AppendMenuA(configMenu, MF_STRING, 3007, "Dashboard Blender");
    AppendMenuA(configMenu, MF_STRING, 3008, "Transformer Blender");
    AppendMenuA(configMenu, MF_STRING, 3006, "Heavy Web");
    AppendMenuA(configMenu, MF_STRING, 3004, "Edit dashboard.json (Notepad)");
    AppendMenuA(configMenu, MF_STRING, 3005, "Edit transformer.json (Notepad)");
    AppendMenuA(configMenu, MF_STRING, 3009, "GTR Configuration");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)configMenu, "Configuration");

    HMENU helpMenu = CreatePopupMenu();
    AppendMenuA(helpMenu, MF_STRING, 4001, "Help / Manual");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)helpMenu, "Help");

    SetMenu(hwnd, hMenu);
}

bool Win32App::InitializeAndRun(HINSTANCE hInst, int nShow) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, MainWindow::WndProc, 0, 0, hInst, NULL, LoadCursor(NULL, IDC_ARROW), NULL, NULL, "StageOneClass", NULL };
    if(!RegisterClassEx(&wc)) return false;
    
    HWND hwnd = CreateWindowExA(0, "StageOneClass", "Stage One Production Dashboard Engine (Integrated GTR)", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1150, 840, NULL, NULL, hInst, NULL);
    if(!hwnd) return false;

    AppState::getInstance().setHWnd(hwnd);
    RebuildMasterMenu(hwnd, AppState::getInstance().getDiscoveredInputs());
    
    ShowWindow(hwnd, nShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}
