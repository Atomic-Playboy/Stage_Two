#ifndef APP_STATE_H
#define APP_STATE_H

#include <windows.h>
#include "Core/MidiData.h"
#include "Core/GtrWorkspaceConfig.h"
#include "Core/DisplayState.h"
#include <vector>
#include <string>
#include <map>
#include <mutex>
#include <chrono>
#include <functional>

typedef WorkspaceConfig DashWorkspaceConfig;

enum AppViewMode { MODE_DASHBOARD = 0, MODE_GTR = 1, MODE_CONSOLE = 2 };

class AppState {
public:
    static AppState& getInstance();

    DashWorkspaceConfig getDashWorkspace() const;
    void setDashWorkspace(const DashWorkspaceConfig& config);

    GtrWorkspaceConfig getGtrWorkspace(int slot) const;
    void setGtrWorkspace(int slot, const GtrWorkspaceConfig& config);

    DisplayState getGtrDisplayState(int slot, int btnIndex) const;
    void setGtrDisplayState(int slot, int btnIndex, const DisplayState& state);

    bool getPhysicalButtonState(int ccNum) const;
    void setPhysicalButtonState(int ccNum, bool val);

    std::vector<DashboardBlock> getBlockData() const;
    void setBlockData(const std::vector<DashboardBlock>& data);
    void updateBlockState(int ccNumber, bool toggle, bool& changed);

    std::vector<std::string> getDiscoveredInputs() const;
    void setDiscoveredInputs(const std::vector<std::string>& inputs);

    std::string getActiveDashInterface() const;
    void setActiveDashInterface(const std::string& iface);

    int getActiveDashChannel() const;
    void setActiveDashChannel(int chan);

    bool isInterfaceOverridden() const;
    void setInterfaceOverridden(bool val);

    bool isChannelOverridden() const;
    void setChannelOverridden(bool val);

    int getCurrentProgramChange() const;
    void setCurrentProgramChange(int pc);

    void loadPresetMatrixBlocks(int pcNumber);

    std::vector<std::string> getCrawlMessages() const;
    void pushCrawlMessage(const std::string& msg);
    void pruneStaleCrawlMessages(std::function<int(const std::string&)> measureWidthFunc);

    HWND getHWnd() const;
    void setHWnd(HWND hwnd);

    int getScrollOffset() const;
    void setScrollOffset(int val);
    void incrementScrollOffset(int amount);

    int getBannerScrollOffset() const;
    void setBannerScrollOffset(int val);
    void incrementBannerScrollOffset(int amount);

    int getBlinkState() const;
    void setBlinkState(int state);

    int getBlinkTimer() const;
    void setBlinkTimer(int timer);
    void decrementBlinkTimer();

    bool isDebugConsoleMode() const;
    void setDebugConsoleMode(bool mode);

    AppViewMode getAppViewMode() const;
    void setAppViewMode(AppViewMode mode);

    int getDynamicCrawlSpeed() const;
    void setDynamicCrawlSpeed(int speed);

    int getDynamicOrbBlinkDuration() const;
    void setDynamicOrbBlinkDuration(int duration);

private:
    AppState();
    ~AppState() = default;
    AppState(const AppState&) = delete;
    AppState& operator=(const AppState&) = delete;

    mutable std::recursive_mutex stateMutex;

    DashWorkspaceConfig dashWorkspace;
    GtrWorkspaceConfig gtrWorkspaces[5];
    DisplayState currentGuiLcds[5][7];
    std::map<int, bool> physicalButtonState;

    std::vector<DashboardBlock> blockData;
    std::vector<std::string> discoveredInputs;

    std::string activeDashInterface = "ANY";
    int activeDashChannel = -1;
    bool interfaceOverridden = false;
    bool channelOverridden = false;
    int currentProgramChange = 0;

    std::vector<std::string> crawlMessages;
    std::vector<std::chrono::steady_clock::time_point> crawlTimestamps;

    HWND hWnd = nullptr;
    int scrollOffset = 0;
    int bannerScrollOffset = 0;
    int blinkState = 0;
    int blinkTimer = 0;

    bool debugConsoleMode = false;
    AppViewMode viewMode = MODE_DASHBOARD;

    int dynamicCrawlSpeed = 2;
    int dynamicOrbBlinkDuration = 10;
};

#endif // APP_STATE_H
