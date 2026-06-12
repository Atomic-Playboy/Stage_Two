#include "Core/AppStateDashboard.h"

AppStateDashboard& AppStateDashboard::getInstance() {
    static AppStateDashboard instance;
    return instance;
}

AppStateDashboard::AppStateDashboard() {
    currentProgramChange = 1;
}

DashboardConfig AppStateDashboard::getDashboardConfig() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return dashboardConfig;
}

void AppStateDashboard::setDashboardConfig(const DashboardConfig& config) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    dashboardConfig = config;
}

std::vector<DashboardBlock> AppStateDashboard::getBlockData() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return blockData;
}

void AppStateDashboard::setBlockData(const std::vector<DashboardBlock>& data) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    blockData = data;
}

void AppStateDashboard::updateBlockState(int ccNumber, bool toggle, bool& changed) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    changed = false;
    for (auto& block : blockData) {
        if (block.cc == ccNumber) {
            if (block.transientState != toggle) {
                block.transientState = toggle;
                changed = true;
            }
        }
    }
}

std::string AppStateDashboard::getActiveDashInterface() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return activeDashInterface;
}

void AppStateDashboard::setActiveDashInterface(const std::string& iface) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    activeDashInterface = iface;
}

int AppStateDashboard::getActiveDashChannel() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return activeDashChannel;
}

void AppStateDashboard::setActiveDashChannel(int chan) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    activeDashChannel = chan;
}

bool AppStateDashboard::isInterfaceOverridden() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return interfaceOverridden;
}

void AppStateDashboard::setInterfaceOverridden(bool val) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    interfaceOverridden = val;
}

bool AppStateDashboard::isChannelOverridden() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return channelOverridden;
}

void AppStateDashboard::setChannelOverridden(bool val) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    channelOverridden = val;
}

int AppStateDashboard::getCurrentProgramChange() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return currentProgramChange;
}

void AppStateDashboard::setCurrentProgramChange(int pc) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    currentProgramChange = pc;
}

void AppStateDashboard::loadPresetMatrixBlocks(int pcNumber) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    currentProgramChange = pcNumber;
    blockData.clear();
    auto it = dashboardConfig.programChangeDashboardMatrix.find(pcNumber);
    if (it != dashboardConfig.programChangeDashboardMatrix.end() && it->second.active) {
        blockData = it->second.interactiveTiles;
    }
    bannerScrollOffset = 0;
}

int AppStateDashboard::getBannerScrollOffset() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return bannerScrollOffset;
}

void AppStateDashboard::setBannerScrollOffset(int val) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    bannerScrollOffset = val;
}

void AppStateDashboard::incrementBannerScrollOffset(int amount) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    bannerScrollOffset += amount;
}

HWND AppStateDashboard::getHWnd() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return hWnd;
}

void AppStateDashboard::setHWnd(HWND hwnd) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    hWnd = hwnd;
}

bool AppStateDashboard::isDebugConsoleMode() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return debugConsoleMode;
}

void AppStateDashboard::setDebugConsoleMode(bool mode) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    debugConsoleMode = mode;
    if (mode) viewMode = MODE_CONSOLE;
    else if (viewMode == MODE_CONSOLE) viewMode = MODE_DASHBOARD;
}

AppViewMode AppStateDashboard::getAppViewMode() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return viewMode;
}

void AppStateDashboard::setAppViewMode(AppViewMode mode) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    viewMode = mode;
    debugConsoleMode = (mode == MODE_CONSOLE);
}
