#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <windows.h>
#include <string>
#include "Core/AppState.h"

class MainWindow {
public:
    static void pushCrawlMessage(const std::string& message);
    static std::string getAbsoluteExecutionPath(const std::string& relativeFilename);
    static std::string searchValidWebPath(const std::string& filename);
    static void SetupConsoleScrollbar(HWND hwnd);
    static void HandleScrollMessage(HWND hwnd, WPARAM wp);
    static void DrawTopBanner(HDC hdcMem, RECT rect, int topSectionHeight);
    static void DrawDashboardGrid(HDC hdcMem, RECT mainArea, int rowHeight, int gridRows);
    static void DrawBottomTicker(HDC hdcMem, RECT rect, int viewWidth);
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    static void switchAppView(AppViewMode mode, HWND hwnd);
    static void repositionGtrControls(int width);

private:
    static HWND hLoadBtn[4];
    static HWND hFileLabel[4];
    static HWND hPcDownBtn[4];
    static HWND hPcUpBtn[4];
    static HWND hUpperLcd[4];
    static HWND hButtonLcd[4][6];
    static HWND hAbLcd[4];
    static HWND hSendLightsBtn[4];
    static HWND hSendMidiBtn[4];

    static HFONT hLcdFont;
    static HBRUSH hBrushSoftBg;
    static HBRUSH hBrushLcdPanel;
    static COLORREF rgbNeonOrange;
    static COLORREF rgbDimOrange;
    static COLORREF rgbChassisText;
};

#endif // MAINWINDOW_H
