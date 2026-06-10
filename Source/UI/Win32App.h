#ifndef WIN32_APP_H
#define WIN32_APP_H

#include <windows.h>
#include <vector>
#include <string>

class Win32App {
public:
    static void SelectAndOpenTargetFile(HWND hwnd, int fileKindIndex);
    static void RebuildMasterMenu(HWND hwnd, const std::vector<std::string>& inputs);
    static bool InitializeAndRun(HINSTANCE hInst, int nShow);
};

#endif // WIN32_APP_H
