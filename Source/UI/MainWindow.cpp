#include "UI/MainWindow.h"
#include "Core/AppState.h"
#include "Core/CoreGlobals.h"
#include "Core/PathConstants.h"
#include "UI/Win32App.h"
#include "Logic/PresetManager.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <chrono>
#include <commdlg.h>
#include <shellapi.h>

#define IDC_LOAD_BASE 5000
#define IDC_LCD_UPPER_BASE 6000
#define IDC_SEND_LIGHTS_BASE 7000
#define IDC_SEND_MIDI_BASE 7500
#define IDC_PC_DOWN_BASE 8000
#define IDC_PC_UP_BASE 8500
#define IDC_LCD_AB_BASE 9000
#define IDC_LCD_BTN_BASE 10000

HWND MainWindow::hLoadBtn[4] = {NULL};
HWND MainWindow::hFileLabel[4] = {NULL};
HWND MainWindow::hPcDownBtn[4] = {NULL};
HWND MainWindow::hPcUpBtn[4] = {NULL};
HWND MainWindow::hUpperLcd[4] = {NULL};
HWND MainWindow::hButtonLcd[4][6] = {{NULL}};
HWND MainWindow::hAbLcd[4] = {NULL};
HWND MainWindow::hSendLightsBtn[4] = {NULL};
HWND MainWindow::hSendMidiBtn[4] = {NULL};

HFONT MainWindow::hLcdFont = NULL;
HBRUSH MainWindow::hBrushSoftBg = NULL;
HBRUSH MainWindow::hBrushLcdPanel = NULL;
COLORREF MainWindow::rgbNeonOrange = 0;
COLORREF MainWindow::rgbDimOrange = 0;
COLORREF MainWindow::rgbChassisText = 0;
static int g_ConsoleScrollPositionY = 0;
static int g_ConsoleTotalLinesHeight = 0;
static int g_ConsoleClientVisibleHeight = 0;

void MainWindow::pushCrawlMessage(const std::string& message) {
    AppState::getInstance().pushCrawlMessage(message);
}

std::string MainWindow::getAbsoluteExecutionPath(const std::string& relativeFilename) {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    if (pos == std::string::npos) return relativeFilename;
    return std::string(buffer).substr(0, pos) + "/" + relativeFilename;
}

std::string MainWindow::searchValidWebPath(const std::string& filename) {
    std::string pathDirect = getAbsoluteExecutionPath(filename);
    std::ifstream fDirect(pathDirect);
    if (fDirect.good()) return pathDirect;
    std::vector<std::string> pathsToTry = {
        getAbsoluteExecutionPath(PathConstants::ParentDir + filename),
        getAbsoluteExecutionPath(PathConstants::GrandparentDir + filename)
    };
    for(const auto& p : pathsToTry) {
        std::ifstream f(p); if(f.good()) return p;
    }
    return pathDirect;
}

void MainWindow::SetupConsoleScrollbar(HWND hwnd) {
    AppState& state = AppState::getInstance();
    if (state.getAppViewMode() != MODE_CONSOLE) {
        ShowScrollBar(hwnd, SB_VERT, FALSE);
        return;
    }
    std::string logHistory = CoreGlobals::logger.getHistory();
    int lineCount = 1;
    for (char c : logHistory) if (c == '\n') lineCount++;

    g_ConsoleTotalLinesHeight = lineCount * 16 + 40;
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    si.nMax = g_ConsoleTotalLinesHeight;
    si.nPage = g_ConsoleClientVisibleHeight > 0 ? g_ConsoleClientVisibleHeight : 100;

    if (g_ConsoleScrollPositionY > (g_ConsoleTotalLinesHeight - (int)si.nPage)) {
        g_ConsoleScrollPositionY = g_ConsoleTotalLinesHeight - (int)si.nPage;
    }
    if (g_ConsoleScrollPositionY < 0) g_ConsoleScrollPositionY = 0;
    si.nPos = g_ConsoleScrollPositionY;

    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
    ShowScrollBar(hwnd, SB_VERT, TRUE);
}

void MainWindow::HandleScrollMessage(HWND hwnd, WPARAM wp) {
    int action = LOWORD(wp);
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    GetScrollInfo(hwnd, SB_VERT, &si);

    int oldPos = si.nPos;
    switch (action) {
        case SB_LINEUP:        si.nPos -= 16; break;
        case SB_LINEDOWN:      si.nPos += 16; break;
        case SB_PAGEUP:        si.nPos -= si.nPage; break;
        case SB_PAGEDOWN:      si.nPos += si.nPage; break;
        case SB_THUMBTRACK:    si.nPos = si.nTrackPos; break;
        default: break;
    }

    if (si.nPos < 0) si.nPos = 0;
    if (si.nPos > (si.nMax - (int)si.nPage)) si.nPos = si.nMax - si.nPage;
    if (si.nPos != oldPos) {
        g_ConsoleScrollPositionY = si.nPos;
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void MainWindow::repositionGtrControls(int width) {
    int startX = (width - 1040) / 2;
    if (startX < 20) startX = 20;
    for(int i = 0; i < 4; i++) {
        int yOffset = 150 + (i * 130);
        if(hLoadBtn[i]) SetWindowPos(hLoadBtn[i], NULL, startX, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        if(hFileLabel[i]) SetWindowPos(hFileLabel[i], NULL, startX + 110, yOffset + 5, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        if(hPcDownBtn[i]) SetWindowPos(hPcDownBtn[i], NULL, startX + 470, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        if(hUpperLcd[i]) SetWindowPos(hUpperLcd[i], NULL, startX + 510, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        if(hPcUpBtn[i]) SetWindowPos(hPcUpBtn[i], NULL, startX + 580, yOffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        for(int btn=0; btn<6; btn++) {
            if(hButtonLcd[i][btn]) SetWindowPos(hButtonLcd[i][btn], NULL, startX + 330 + (btn * 70), yOffset + 40, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        if(hAbLcd[i]) SetWindowPos(hAbLcd[i], NULL, startX + 760, yOffset + 40, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        if(hSendLightsBtn[i]) SetWindowPos(hSendLightsBtn[i], NULL, startX + 830, yOffset + 40, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        if(hSendMidiBtn[i]) SetWindowPos(hSendMidiBtn[i], NULL, startX + 930, yOffset + 40, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    }
}

void MainWindow::switchAppView(AppViewMode mode, HWND hwnd) {
    AppState::getInstance().setAppViewMode(mode);
    int cmd = (mode == MODE_GTR) ? SW_SHOW : SW_HIDE;
    for(int i = 0; i < 4; i++) {
        if(hLoadBtn[i]) ShowWindow(hLoadBtn[i], cmd);
        if(hFileLabel[i]) ShowWindow(hFileLabel[i], cmd);
        if(hPcDownBtn[i]) ShowWindow(hPcDownBtn[i], cmd);
        if(hUpperLcd[i]) ShowWindow(hUpperLcd[i], cmd);
        if(hPcUpBtn[i]) ShowWindow(hPcUpBtn[i], cmd);
        for(int btn=0; btn<6; btn++) {
            if(hButtonLcd[i][btn]) ShowWindow(hButtonLcd[i][btn], cmd);
        }
        if(hAbLcd[i]) ShowWindow(hAbLcd[i], cmd);
        if(hSendLightsBtn[i]) ShowWindow(hSendLightsBtn[i], cmd);
        if(hSendMidiBtn[i]) ShowWindow(hSendMidiBtn[i], cmd);
    }
    if (mode == MODE_CONSOLE) {
        SetupConsoleScrollbar(hwnd);
    } else {
        ShowScrollBar(hwnd, SB_VERT, FALSE);
    }
    RECT r;
    GetClientRect(hwnd, &r);
    repositionGtrControls(r.right);
    InvalidateRect(hwnd, NULL, TRUE);
}

void MainWindow::DrawTopBanner(HDC hdcMem, RECT rect, int topSectionHeight) {
    AppState& state = AppState::getInstance();
    DashWorkspaceConfig workspace = state.getDashWorkspace();
    int currentPC = state.getCurrentProgramChange();
    int bannerScrollOffset = state.getBannerScrollOffset();

    std::string activeTitle = "[NO MAP ENTRIES MATCH CHANNELS]";
    auto itActive = workspace.programChangeDashboardMatrix.find(currentPC);
    if (itActive != workspace.programChangeDashboardMatrix.end() && itActive->second.active && !itActive->second.songTitle.empty()) {
        activeTitle = itActive->second.songTitle;
    }
    std::string activeText = "ACTIVE: " + std::to_string(currentPC) + " - " + activeTitle;
    std::string deckTitle = "[END OF SETLIST / UNMAPPED]";
    auto itDeck = workspace.programChangeDashboardMatrix.find(currentPC + 1);
    if (itDeck != workspace.programChangeDashboardMatrix.end() && itDeck->second.active && !itDeck->second.songTitle.empty()) {
        deckTitle = itDeck->second.songTitle;
    }
    if (deckTitle.length() > 40) deckTitle = deckTitle.substr(0, 40);
    std::string onDeckText = "NEXT: " + std::to_string(currentPC + 1) + " - " + deckTitle;
    std::string baselineString = "ACTIVE: 123 - [NO MAP ENTRIES MATCH CHANNELS]";
    int targetFontSize = static_cast<int>(topSectionHeight * 0.38);
    if (targetFontSize > 68) targetFontSize = 68;

    while (targetFontSize > 8) {
        HFONT testFont = CreateFontA(targetFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
        HGDIOBJ oldObj = SelectObject(hdcMem, testFont);
        SIZE ts;
        GetTextExtentPoint32A(hdcMem, baselineString.c_str(), (int)baselineString.length(), &ts);
        SelectObject(hdcMem, oldObj);
        DeleteObject(testFont);
        if (ts.cx < (rect.right - 80)) break;
        targetFontSize -= 2;
    }

    HFONT syncFont = CreateFontA(targetFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
    HGDIOBJ oldFont = SelectObject(hdcMem, syncFont);
    SetBkMode(hdcMem, TRANSPARENT);

    SIZE textMetricSize;
    GetTextExtentPoint32A(hdcMem, activeText.c_str(), (int)activeText.length(), &textMetricSize);

    int availableDrawWidth = rect.right - 50;
    RECT activeRect = { 25, 12, rect.right - 25, 12 + targetFontSize };

    SetTextColor(hdcMem, RGB(255, 0, 0));
    if (activeTitle.length() > 40 && textMetricSize.cx > availableDrawWidth) {
        HRGN hBannerClip = CreateRectRgn(activeRect.left, activeRect.top, activeRect.right, activeRect.bottom + 5);
        SelectClipRgn(hdcMem, hBannerClip);

        std::string paddedActiveText = activeText + "      *** ";
        SIZE loopSize;
        GetTextExtentPoint32A(hdcMem, paddedActiveText.c_str(), (int)paddedActiveText.length(), &loopSize);

        RECT textOffsetRect = { 25 - bannerScrollOffset, activeRect.top, 20000, activeRect.bottom };
        DrawTextA(hdcMem, paddedActiveText.c_str(), -1, &textOffsetRect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
        RECT secondaryOffsetRect = { 25 - bannerScrollOffset + loopSize.cx, activeRect.top, 20000, activeRect.bottom };
        DrawTextA(hdcMem, paddedActiveText.c_str(), -1, &secondaryOffsetRect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);

        if (bannerScrollOffset >= loopSize.cx) state.setBannerScrollOffset(0);
        SelectClipRgn(hdcMem, NULL);
        DeleteObject(hBannerClip);
    } else {
        DrawTextA(hdcMem, activeText.c_str(), -1, &activeRect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
    }

    RECT deckRect = { 25, 12 + targetFontSize + 10, rect.right - 25, topSectionHeight };
    DrawTextA(hdcMem, onDeckText.c_str(), -1, &deckRect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);

    SelectObject(hdcMem, oldFont);
    DeleteObject(syncFont);
}

void MainWindow::DrawDashboardGrid(HDC hdcMem, RECT mainArea, int rowHeight, int gridRows) {
    AppState& state = AppState::getInstance();
    std::vector<DashboardBlock> blockData = state.getBlockData();
    DashWorkspaceConfig workspace = state.getDashWorkspace();

    size_t numBlocks = blockData.size();
    if (numBlocks == 0) {
        RECT emptyRect = mainArea;
        SetTextColor(hdcMem, RGB(120, 120, 120));
        HFONT msgFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
        SelectObject(hdcMem, msgFont);
        DrawTextA(hdcMem, "Blank Dashboard Configuration - No Active Hardware Blocks Defined", -1, &emptyRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        DeleteObject(msgFont);
        return;
    }

    int totalWidth = mainArea.right - mainArea.left;
    size_t blocksProcessed = 0;
    SetBkMode(hdcMem, TRANSPARENT);
    std::string tileConstraintString(14, 'W');
    for (int r = 0; r < gridRows; ++r) {
        size_t remainingBlocks = numBlocks - blocksProcessed;
        size_t blocksInThisRow = (r == gridRows - 1) ? remainingBlocks : 4;
        if (blocksInThisRow > 4) blocksInThisRow = 4;
        if (blocksInThisRow == 0) break;

        int blockCellWidth = totalWidth / static_cast<int>(blocksInThisRow);
        int yTop = mainArea.top + (r * rowHeight);
        int yBottom = yTop + rowHeight;

        for (size_t c = 0; c < blocksInThisRow; ++c) {
            const auto& block = blockData[blocksProcessed++];
            int xLeft = mainArea.left + (static_cast<int>(c) * blockCellWidth);
            int xRight = xLeft + blockCellWidth;
            RECT cellRect = { xLeft + 4, yTop + 4, xRight - 4, yBottom - 4 };
            COLORREF targetBg = RGB(35, 35, 45);
            COLORREF targetText = RGB(245, 245, 245);

            auto itBg = workspace.themeColors.find(block.colorName);
            if (itBg != workspace.themeColors.end()) targetBg = block.transientState ? itBg->second.bright : itBg->second.dim;
            auto itFg = workspace.themeColors.find(block.textColorName);
            if (itFg != workspace.themeColors.end()) targetText = block.transientState ? itFg->second.bright : itFg->second.dim;

            HBRUSH cellBrush = CreateSolidBrush(targetBg);
            FillRect(hdcMem, &cellRect, cellBrush);
            DeleteObject(cellBrush);
            std::string rawName = block.effect;
            if (rawName.length() > 12) rawName = rawName.substr(0, 12);

            int maxAllowedFontHeight = (yBottom - yTop) - 40;
            if (maxAllowedFontHeight > 84) maxAllowedFontHeight = 84;
            if (maxAllowedFontHeight < 14) maxAllowedFontHeight = 14;

            int uniformGridFontSize = maxAllowedFontHeight;
            int cellWidthBounds = (xRight - xLeft) - 20;

            while (uniformGridFontSize > 8) {
                HFONT testFont = CreateFontA(uniformGridFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
                HGDIOBJ oldObj = SelectObject(hdcMem, testFont);
                SIZE ts;
                GetTextExtentPoint32A(hdcMem, tileConstraintString.c_str(), (int)tileConstraintString.length(), &ts);
                SelectObject(hdcMem, oldObj);
                DeleteObject(testFont);
                if (ts.cx < cellWidthBounds) break;
                uniformGridFontSize -= 1;
            }

            HFONT blockTextFont = CreateFontA(uniformGridFontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
            HGDIOBJ oldFont = SelectObject(hdcMem, blockTextFont);

            SetTextColor(hdcMem, targetText);
            DrawTextA(hdcMem, rawName.c_str(), -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            SelectObject(hdcMem, oldFont);
            DeleteObject(blockTextFont);
        }
    }
}

void MainWindow::DrawBottomTicker(HDC hdcMem, RECT rect, int viewWidth) {
    AppState& state = AppState::getInstance();
    HFONT scrollFont = CreateFontA(15, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
    HGDIOBJ oldFont = SelectObject(hdcMem, scrollFont);

    state.pruneStaleCrawlMessages([hdcMem](const std::string& str) -> int {
        SIZE sz;
        GetTextExtentPoint32A(hdcMem, str.c_str(), (int)str.length(), &sz);
        return sz.cx;
    });
    std::vector<std::string> crawlMessages = state.getCrawlMessages();
    int scrollOffset = state.getScrollOffset();

    HBRUSH crawlBg = CreateSolidBrush(RGB(10, 10, 10));
    FillRect(hdcMem, &rect, crawlBg);
    DeleteObject(crawlBg);
    SetTextColor(hdcMem, RGB(0, 255, 50));
    SetBkMode(hdcMem, TRANSPARENT);

    if (crawlMessages.empty()) {
        RECT readyRect = { 20, rect.top + 7, viewWidth, rect.bottom };
        DrawTextA(hdcMem, "Waiting for stream signals...", -1, &readyRect, DT_LEFT | DT_SINGLELINE);
    } else {
        std::string fullCrawl = "";
        for (const auto& m : crawlMessages) fullCrawl += m + "   *** ";
        SIZE textSize;
        GetTextExtentPoint32A(hdcMem, fullCrawl.c_str(), (int)fullCrawl.length(), &textSize);

        RECT textRect = { viewWidth - scrollOffset, rect.top + 7, 20000, rect.bottom };
        DrawTextA(hdcMem, fullCrawl.c_str(), -1, &textRect, DT_LEFT | DT_SINGLELINE);
        if (viewWidth - scrollOffset + textSize.cx < 0) state.setScrollOffset(0);
    }
    SelectObject(hdcMem, oldFont);
    DeleteObject(scrollFont);
}

LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    AppState& state = AppState::getInstance();
    switch (msg) {
        case WM_CREATE: {
            hLcdFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
            hBrushSoftBg = CreateSolidBrush(RGB(58, 66, 70));
            hBrushLcdPanel = CreateSolidBrush(RGB(0, 0, 0));
            rgbNeonOrange = RGB(255, 108, 0);
            rgbDimOrange = RGB(140, 55, 0);
            rgbChassisText = RGB(220, 220, 220);

            for (int i = 0; i < 4; ++i) {
                hLoadBtn[i] = CreateWindowA("BUTTON", ("Load Slot " + std::to_string(i+1)).c_str(), WS_CHILD | BS_PUSHBUTTON, 0, 0, 100, 30, hwnd, (HMENU)(INT_PTR)(IDC_LOAD_BASE + i), NULL, NULL);
                hFileLabel[i] = CreateWindowA("STATIC", "[No Config Loaded]", WS_CHILD, 0, 0, 300, 20, hwnd, NULL, NULL, NULL);
                hPcDownBtn[i] = CreateWindowA("BUTTON", "<", WS_CHILD | BS_PUSHBUTTON, 0, 0, 30, 30, hwnd, (HMENU)(INT_PTR)(IDC_PC_DOWN_BASE + i), NULL, NULL);
                hUpperLcd[i] = CreateWindowA("STATIC", "---", WS_CHILD | SS_CENTER, 0, 0, 60, 30, hwnd, (HMENU)(INT_PTR)(IDC_LCD_UPPER_BASE + i), NULL, NULL);
                SendMessageA(hUpperLcd[i], WM_SETFONT, (WPARAM)hLcdFont, TRUE);
                hPcUpBtn[i] = CreateWindowA("BUTTON", ">", WS_CHILD | BS_PUSHBUTTON, 0, 0, 30, 30, hwnd, (HMENU)(INT_PTR)(IDC_PC_UP_BASE + i), NULL, NULL);
                for (int btn = 0; btn < 6; ++btn) {
                    hButtonLcd[i][btn] = CreateWindowA("STATIC", "---", WS_CHILD | SS_CENTER | SS_NOTIFY, 0, 0, 60, 30, hwnd, (HMENU)(INT_PTR)(IDC_LCD_BTN_BASE + (i * 6) + btn), NULL, NULL);
                    SendMessageA(hButtonLcd[i][btn], WM_SETFONT, (WPARAM)hLcdFont, TRUE);
                }
                hAbLcd[i] = CreateWindowA("STATIC", "A", WS_CHILD | SS_CENTER | SS_NOTIFY, 0, 0, 50, 30, hwnd, (HMENU)(INT_PTR)(IDC_LCD_AB_BASE + i), NULL, NULL);
                SendMessageA(hAbLcd[i], WM_SETFONT, (WPARAM)hLcdFont, TRUE);
                hSendLightsBtn[i] = CreateWindowA("BUTTON", "Send Lights", WS_CHILD | BS_PUSHBUTTON, 0, 0, 90, 25, hwnd, (HMENU)(INT_PTR)(IDC_SEND_LIGHTS_BASE + i), NULL, NULL);
                hSendMidiBtn[i] = CreateWindowA("BUTTON", "Send MIDI", WS_CHILD | BS_PUSHBUTTON, 0, 0, 90, 25, hwnd, (HMENU)(INT_PTR)(IDC_SEND_MIDI_BASE + i), NULL, NULL);
            }

            SetTimer(hwnd, 1, 30, NULL);
            SetTimer(hwnd, 2, 1000, NULL);
            SetTimer(hwnd, 3, 33, NULL);
            break;
        }
        case WM_VSCROLL: HandleScrollMessage(hwnd, wp); break;
        case WM_MOUSEWHEEL: {
            if (state.getAppViewMode() != MODE_CONSOLE) break;
            short zDelta = (short)HIWORD(wp);
            int linesToScroll = (zDelta / 120) * 3;
            g_ConsoleScrollPositionY -= (linesToScroll * 16);
            int maxScroll = g_ConsoleTotalLinesHeight - g_ConsoleClientVisibleHeight;
            if (maxScroll < 0) maxScroll = 0;
            if (g_ConsoleScrollPositionY < 0) g_ConsoleScrollPositionY = 0;
            if (g_ConsoleScrollPositionY > maxScroll) g_ConsoleScrollPositionY = maxScroll;

            SCROLLINFO si; si.cbSize = sizeof(si); si.fMask = SIF_POS; si.nPos = g_ConsoleScrollPositionY;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_KEYDOWN: {
            if (wp == VK_F4) { switchAppView(MODE_DASHBOARD, hwnd); return 0; }
            if (wp == VK_F5) { switchAppView(MODE_GTR, hwnd); return 0; }
            if (wp == VK_F6) { switchAppView(MODE_CONSOLE, hwnd); return 0; }
            if (state.getAppViewMode() != MODE_CONSOLE) break;
            if (wp == VK_UP) SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
            else if (wp == VK_DOWN) SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
            else if (wp == VK_PRIOR) SendMessage(hwnd, WM_VSCROLL, SB_PAGEUP, 0);
            else if (wp == VK_NEXT) SendMessage(hwnd, WM_VSCROLL, SB_PAGEDOWN, 0);
            break;
        }
        case WM_TIMER: {
            if (wp == 1) {
                int speed = state.getDynamicCrawlSpeed();
                state.incrementScrollOffset(speed);
                state.incrementBannerScrollOffset(speed);
                state.decrementBlinkTimer();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            if (wp == 3 && state.getAppViewMode() == MODE_GTR) {
                static DisplayState lastLcd[4][7] = {};
                static int lastAb[4] = {-1, -1, -1, -1};
                static std::string lastFile[4] = {"", "", "", ""};
                for(int slot = 0; slot < 4; slot++) {
                    GtrWorkspaceConfig cfg = state.getGtrWorkspace(slot);
                    if (cfg.loaded) {
                        if (lastFile[slot] != cfg.filename) {
                            SetWindowTextA(hFileLabel[slot], cfg.filename.c_str());
                            lastFile[slot] = cfg.filename;
                        }

                        DisplayState upper = state.getGtrDisplayState(slot, 0);
                        if (lastLcd[slot][0].text != upper.text || lastLcd[slot][0].isActive != upper.isActive || lastLcd[slot][0].isBright != upper.isBright) {
                            SetWindowTextA(hUpperLcd[slot], upper.isActive ? upper.text.c_str() : "");
                            InvalidateRect(hUpperLcd[slot], NULL, TRUE);
                            lastLcd[slot][0] = upper;
                        }

                        for(int btn=0; btn<6; btn++) {
                            DisplayState bState = state.getGtrDisplayState(slot, btn+1);
                            if (lastLcd[slot][btn+1].text != bState.text || lastLcd[slot][btn+1].isActive != bState.isActive || lastLcd[slot][btn+1].isBright != bState.isBright) {
                                SetWindowTextA(hButtonLcd[slot][btn], bState.isActive ? bState.text.c_str() : "");
                                InvalidateRect(hButtonLcd[slot][btn], NULL, TRUE);
                                lastLcd[slot][btn+1] = bState;
                            }
                        }

                        int abVal = cfg.activeAbState;
                        if (lastAb[slot] != abVal) {
                            std::string abText = "---";
                            if (abVal == 0) abText = "OFF";
                            else if (abVal == 1) abText = "A";
                            else if (abVal == 2) abText = "B";
                            else if (abVal == 3) abText = "A/B";
                            SetWindowTextA(hAbLcd[slot], abText.c_str());
                            InvalidateRect(hAbLcd[slot], NULL, TRUE);
                            lastAb[slot] = abVal;
                        }
                    }
                }
            }
            break;
        }
        case WM_USER + 100: {
            int stateKind = (int)wp;
            if (stateKind == 1) state.setBlinkState(1);
            else if (stateKind == 2) state.setBlinkState(2);
            else state.setBlinkState(3);
            state.setBlinkTimer(state.getDynamicOrbBlinkDuration());
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_USER + 101: {
            state.loadPresetMatrixBlocks((int)wp);
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_COMMAND: {
            int wmId = LOWORD(wp);
            int wmEvent = HIWORD(wp);

            if (wmId == 3104) { switchAppView(MODE_DASHBOARD, hwnd); return 0; }
            if (wmId == 3105) { switchAppView(MODE_GTR, hwnd); return 0; }
            if (wmId == 3106) { switchAppView(MODE_CONSOLE, hwnd); return 0; }

            if (wmId == 3009) {
                std::string targetHtml = getAbsoluteExecutionPath("GTR_Config.html");
                ShellExecuteA(NULL, "open", targetHtml.c_str(), NULL, NULL, SW_SHOWNORMAL);
                return 0;
            }

            if (wmId >= IDC_LOAD_BASE && wmId < IDC_LOAD_BASE + 4) {
                int slot = wmId - IDC_LOAD_BASE;
                char filename[MAX_PATH] = "";
                OPENFILENAMEA ofn;
                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
                ofn.lpstrFile = filename;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
                if (GetOpenFileNameA(&ofn)) {
                    std::string target = "gtr" + std::to_string(slot+1) + ".json";
                    CopyFileA(filename, target.c_str(), FALSE);
                    CoreGlobals::reloadConfiguration();
                }
                return 0;
            }
            if (wmId >= IDC_SEND_LIGHTS_BASE && wmId < IDC_SEND_LIGHTS_BASE + 4) { manualForceSendLights(wmId - IDC_SEND_LIGHTS_BASE); return 0; }
            if (wmId >= IDC_SEND_MIDI_BASE && wmId < IDC_SEND_MIDI_BASE + 4) { manualForceSendMidi(wmId - IDC_SEND_MIDI_BASE); return 0; }
            if (wmId >= IDC_PC_DOWN_BASE && wmId < IDC_PC_DOWN_BASE + 4) { manualStepProgramChange(wmId - IDC_PC_DOWN_BASE, -1); return 0; }
            if (wmId >= IDC_PC_UP_BASE && wmId < IDC_PC_UP_BASE + 4) { manualStepProgramChange(wmId - IDC_PC_UP_BASE, 1); return 0; }

            if (wmEvent == STN_CLICKED) {
                if (wmId >= IDC_LCD_BTN_BASE && wmId < IDC_LCD_BTN_BASE + 24) {
                    int offset = wmId - IDC_LCD_BTN_BASE;
                    int slot = offset / 6;
                    int btnIndex = offset % 6;
                    GtrWorkspaceConfig cfg = state.getGtrWorkspace(slot);
                    if (cfg.loaded) {
                        for (auto& preset : cfg.presets) {
                            if (preset.pcTrigger == cfg.activePcNum) {
                                if (preset.displays[btnIndex + 1].isActive) {
                                   preset.displays[btnIndex + 1].isBright = !preset.displays[btnIndex + 1].isBright;
                                   activatePreset(slot, preset, cfg);
                                }
                                break;
                            }
                        }
                    }
                }
                if (wmId >= IDC_LCD_AB_BASE && wmId < IDC_LCD_AB_BASE + 4) {
                    int slot = wmId - IDC_LCD_AB_BASE;
                    GtrWorkspaceConfig cfg = state.getGtrWorkspace(slot);
                    if (cfg.loaded) {
                        for (auto& preset : cfg.presets) {
                            if (preset.pcTrigger == cfg.activePcNum) {
                                preset.abInitialState = (preset.abInitialState == preset.abOffState) ? preset.abOnState : preset.abOffState;
                                activatePreset(slot, preset, cfg);
                                break;
                            }
                        }
                    }
                }
            }

            if (wmId == 1001) Win32App::SelectAndOpenTargetFile(hwnd, 1);
            else if (wmId == 1002) Win32App::SelectAndOpenTargetFile(hwnd, 2);
            else if (wmId >= 2001 && wmId < 2100) {
                state.setInterfaceOverridden(true);
                if (wmId == 2001) state.setActiveDashInterface("ANY");
                else {
                    char menuTextBuf[256] = {0};
                    HMENU hMainMenu = GetMenu(hwnd);
                    if (hMainMenu) {
                        bool foundText = false;
                        for (int i = 0; i < GetMenuItemCount(hMainMenu); ++i) {
                            HMENU hSub = GetSubMenu(hMainMenu, i);
                            if (hSub && GetMenuStringA(hSub, wmId, menuTextBuf, sizeof(menuTextBuf), MF_BYCOMMAND)) {
                                if (strlen(menuTextBuf) > 0) {
                                    state.setActiveDashInterface(std::string(menuTextBuf));
                                    foundText = true; break;
                                }
                            }
                        }
                        if (!foundText || state.getActiveDashInterface().empty()) state.setActiveDashInterface("ANY");
                    }
                }
                state.loadPresetMatrixBlocks(state.getCurrentProgramChange());
                if (CoreGlobals::engine) {
                    CoreGlobals::engine->updateDashboardInterfaces(state.getActiveDashInterface(), state.getActiveDashChannel());
                    CoreGlobals::engine->selectActiveHardwareInterface(state.getActiveDashInterface());
                }
            } else if (wmId >= 2100 && (static_cast<int>(wmId) <= 2116)) {
                state.setChannelOverridden(true);
                if (wmId == 2100) state.setActiveDashChannel(-1);
                else state.setActiveDashChannel(wmId - 2100);
                if (CoreGlobals::engine) CoreGlobals::engine->updateDashboardInterfaces(state.getActiveDashInterface(), state.getActiveDashChannel());
            }
            break;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdcStatic = (HDC)wp;
            HWND hwndStatic = (HWND)lp;
            int id = GetDlgCtrlID(hwndStatic);
            if (id >= IDC_LCD_UPPER_BASE && id < IDC_LCD_UPPER_BASE + 4) {
                int slot = id - IDC_LCD_UPPER_BASE;
                DisplayState ds = state.getGtrDisplayState(slot, 0);
                SetTextColor(hdcStatic, ds.isBright ? rgbNeonOrange : rgbDimOrange);
                SetBkColor(hdcStatic, RGB(0,0,0));
                return (INT_PTR)hBrushLcdPanel;
            }
            if (id >= IDC_LCD_BTN_BASE && id < IDC_LCD_BTN_BASE + 24) {
                int offset = id - IDC_LCD_BTN_BASE;
                int slot = offset / 6;
                int btnIndex = (offset % 6) + 1;
                DisplayState ds = state.getGtrDisplayState(slot, btnIndex);
                SetTextColor(hdcStatic, ds.isBright ? rgbNeonOrange : rgbDimOrange);
                SetBkColor(hdcStatic, RGB(0,0,0));
                return (INT_PTR)hBrushLcdPanel;
            }
            if (id >= IDC_LCD_AB_BASE && id < IDC_LCD_AB_BASE + 4) {
                SetTextColor(hdcStatic, rgbNeonOrange);
                SetBkColor(hdcStatic, RGB(0,0,0));
                return (INT_PTR)hBrushLcdPanel;
            }
            SetTextColor(hdcStatic, rgbChassisText);
            SetBkColor(hdcStatic, RGB(58, 66, 70));
            return (INT_PTR)hBrushSoftBg;
        }
        case WM_SIZE: {
            RECT r;
            GetClientRect(hwnd, &r);
            g_ConsoleClientVisibleHeight = (r.bottom - 35) - 130;
            if (state.getAppViewMode() == MODE_CONSOLE) SetupConsoleScrollbar(hwnd);
            repositionGtrControls(r.right);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect; GetClientRect(hwnd, &rect);
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

            COLORREF frameBgColor = RGB(20, 20, 20);
            int blinkState = state.getBlinkState();
            if (blinkState == 1) frameBgColor = RGB(0, 191, 255);
            else if (blinkState == 2) frameBgColor = RGB(0, 220, 0);
            else if (blinkState == 3) frameBgColor = RGB(220, 0, 0);

            HBRUSH bgBrush = CreateSolidBrush(frameBgColor);
            FillRect(hdcMem, &rect, bgBrush);
            DeleteObject(bgBrush);
            size_t numBlocks = state.getBlockData().size();
            int gridRows = (numBlocks > 12) ? 4 : (numBlocks >= 9) ? 3 : (numBlocks >= 5) ? 2 : 1;
            int rowHeight = (rect.bottom - 35 - 130) / gridRows;
            DrawTopBanner(hdcMem, rect, 130);
            RECT mainArea = { 0, 130, rect.right, rect.bottom - 35 };

            AppViewMode mode = state.getAppViewMode();
            if (mode == MODE_CONSOLE) {
                HBRUSH consoleBg = CreateSolidBrush(RGB(15, 15, 20));
                FillRect(hdcMem, &mainArea, consoleBg);
                DeleteObject(consoleBg);
                SetBkMode(hdcMem, TRANSPARENT);
                SetTextColor(hdcMem, RGB(0, 240, 40));
                HFONT consoleFont = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, "Courier New");
                HGDIOBJ oldConsoleFont = SelectObject(hdcMem, consoleFont);

                HRGN hClipRgn = CreateRectRgn(mainArea.left, mainArea.top, mainArea.right, mainArea.bottom);
                SelectClipRgn(hdcMem, hClipRgn);
                std::string logHistory = CoreGlobals::logger.getHistory();
                RECT textPrintArea = { mainArea.left + 15, mainArea.top + 15 - g_ConsoleScrollPositionY, mainArea.right - 25, mainArea.top + 30000 };
                DrawTextA(hdcMem, logHistory.c_str(), -1, &textPrintArea, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_NOPREFIX);

                SelectClipRgn(hdcMem, NULL); DeleteObject(hClipRgn);
                SelectObject(hdcMem, oldConsoleFont); DeleteObject(consoleFont);
            } else if (mode == MODE_GTR) {
                HBRUSH gtrBrush = CreateSolidBrush(RGB(58, 66, 70));
                FillRect(hdcMem, &mainArea, gtrBrush);
                DeleteObject(gtrBrush);
            } else {
                DrawDashboardGrid(hdcMem, mainArea, rowHeight, gridRows);
            }

            RECT crawlRect = { 0, rect.bottom - 35, rect.right, rect.bottom };
            DrawBottomTicker(hdcMem, crawlRect, rect.right);

            BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);
            SelectObject(hdcMem, hbmOld); DeleteObject(hbmMem); DeleteDC(hdcMem);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY: {
            DeleteObject(hLcdFont);
            DeleteObject(hBrushSoftBg);
            DeleteObject(hBrushLcdPanel);
            PostQuitMessage(0);
            break;
        }
        default: return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}
