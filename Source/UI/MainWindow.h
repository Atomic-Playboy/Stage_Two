#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <windows.h>
#include <string>
#include "Core/AppState.h"
#include "Core/AppConstants.h"
#include "UI/UIConstants.h"

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
    static HWND hLoadBtn[AppConstants::MAX_HARDWARE_SLOTS];
    static HWND hFileLabel[AppConstants::MAX_HARDWARE_SLOTS];
    static HWND hPcDownBtn[AppConstants::MAX_HARDWARE_SLOTS];
    static HWND hPcUpBtn[AppConstants::MAX_HARDWARE_SLOTS];
    static HWND hUpperLcd[AppConstants::MAX_HARDWARE_SLOTS];
    static HWND hButtonLcd[AppConstants::MAX_HARDWARE_SLOTS][6];
    static HWND hAbLcd[AppConstants::MAX_HARDWARE_SLOTS];
    static HWND hSendLightsBtn[AppConstants::MAX_HARDWARE_SLOTS];
    static HWND hSendMidiBtn[AppConstants::MAX_HARDWARE_SLOTS];

    static HFONT hLcdFont;
    static HBRUSH hBrushSoftBg;
    static HBRUSH hBrushLcdPanel;
    static COLORREF rgbNeonOrange;
    static COLORREF rgbDimOrange;
    static COLORREF rgbChassisText;
};

#endif // MAINWINDOW_H
