#ifndef APP_STATE_DASHBOARD_H
#define APP_STATE_DASHBOARD_H

#include <windows.h>
#include "Core/MidiData.h"
#include "Core/AppConstants.h"
#include <vector>
#include <string>
#include <map>
#include <mutex>

enum AppViewMode { MODE_DASHBOARD = 0, MODE_GTR = 1, MODE_CONSOLE = 2 };

class AppStateDashboard {
public:
    static AppStateDashboard& getInstance();
    DashboardConfig getDashboardConfig() const;
    void setDashboardConfig(const DashboardConfig& config);
    std::vector<DashboardBlock> getBlockData() const;
    void setBlockData(const std::vector<DashboardBlock>& data);
    void updateBlockState(int ccNumber, bool toggle, bool& changed);
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
    int getBannerScrollOffset() const;
    void setBannerScrollOffset(int val);
    void incrementBannerScrollOffset(int amount);
    HWND getHWnd() const;
    void setHWnd(HWND hwnd);
    bool isDebugConsoleMode() const;
    void setDebugConsoleMode(bool mode);
    AppViewMode getAppViewMode() const;
    void setAppViewMode(AppViewMode mode);

private:
    AppStateDashboard();
    ~AppStateDashboard() = default;
    AppStateDashboard(const AppStateDashboard&) = delete;
    AppStateDashboard& operator=(const AppStateDashboard&) = delete;

    mutable std::recursive_mutex stateMutex;
    DashboardConfig dashboardConfig;
    std::vector<DashboardBlock> blockData;
    std::string activeDashInterface = "ANY";
    int activeDashChannel = -1;
    bool interfaceOverridden = false;
    bool channelOverridden = false;
    int currentProgramChange = 1;
    int bannerScrollOffset = 0;
    HWND hWnd = nullptr;
    bool debugConsoleMode = false;
    AppViewMode viewMode = MODE_DASHBOARD;
};

#endif // APP_STATE_DASHBOARD_H
