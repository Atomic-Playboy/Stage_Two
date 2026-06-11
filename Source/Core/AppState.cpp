/**
 * AppState.cpp
 * Thread-safe global singleton containing runtime UI and hardware cache state.
 */

#include "Core/AppState.h"

AppState& AppState::getInstance() {
    static AppState instance;
    return instance;
}

AppState::AppState() {
    // Architectural Decision:
    // Human-readable stage setlists map program changes starting at 1. 
    // If we initialize at 0, the application launches into a blank, unmapped dashboard.
    currentProgramChange = 1;
}

DashboardConfig AppState::getDashboardConfig() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return dashboardConfig;
}

void AppState::setDashboardConfig(const DashboardConfig& config) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    dashboardConfig = config;
}

GtrSlotConfig AppState::getGtrSlotConfig(int slot) const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    if (slot >= 0 && slot < AppConstants::MAX_HARDWARE_SLOTS) return gtrSlots[slot];
    return GtrSlotConfig();
}

void AppState::setGtrSlotConfig(int slot, const GtrSlotConfig& config) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    if (slot >= 0 && slot < AppConstants::MAX_HARDWARE_SLOTS) gtrSlots[slot] = config;
}

DisplayState AppState::getGtrDisplayState(int slot, int btnIndex) const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    if (slot >= 0 && slot < AppConstants::MAX_HARDWARE_SLOTS && btnIndex >= 0 && btnIndex < 7) {
        return currentGuiLcds[slot][btnIndex];
    }
    return DisplayState();
}

void AppState::setGtrDisplayState(int slot, int btnIndex, const DisplayState& state) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    if (slot >= 0 && slot < AppConstants::MAX_HARDWARE_SLOTS && btnIndex >= 0 && btnIndex < 7) {
        currentGuiLcds[slot][btnIndex] = state;
    }
}

bool AppState::getPhysicalButtonState(int ccNum) const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    auto it = physicalButtonState.find(ccNum);
    if (it != physicalButtonState.end()) return it->second;
    return false;
}

void AppState::setPhysicalButtonState(int ccNum, bool val) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    physicalButtonState[ccNum] = val;
}

std::vector<DashboardBlock> AppState::getBlockData() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return blockData;
}

void AppState::setBlockData(const std::vector<DashboardBlock>& data) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    blockData = data;
}

void AppState::updateBlockState(int ccNumber, bool toggle, bool& changed) {
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

std::vector<std::string> AppState::getDiscoveredInputs() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return discoveredInputs;
}

void AppState::setDiscoveredInputs(const std::vector<std::string>& inputs) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    discoveredInputs = inputs;
}

std::string AppState::getActiveDashInterface() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return activeDashInterface;
}

void AppState::setActiveDashInterface(const std::string& iface) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    activeDashInterface = iface;
}

int AppState::getActiveDashChannel() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return activeDashChannel;
}

void AppState::setActiveDashChannel(int chan) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    activeDashChannel = chan;
}

bool AppState::isInterfaceOverridden() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return interfaceOverridden;
}

void AppState::setInterfaceOverridden(bool val) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    interfaceOverridden = val;
}

bool AppState::isChannelOverridden() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return channelOverridden;
}

void AppState::setChannelOverridden(bool val) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    channelOverridden = val;
}

int AppState::getCurrentProgramChange() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return currentProgramChange;
}

void AppState::setCurrentProgramChange(int pc) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    currentProgramChange = pc;
}

void AppState::loadPresetMatrixBlocks(int pcNumber) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    currentProgramChange = pcNumber;
    blockData.clear();
    auto it = dashboardConfig.programChangeDashboardMatrix.find(pcNumber);
    if (it != dashboardConfig.programChangeDashboardMatrix.end() && it->second.active) {
        blockData = it->second.interactiveTiles;
    }
    bannerScrollOffset = 0;
}

std::vector<std::string> AppState::getCrawlMessages() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return crawlMessages;
}

void AppState::pushCrawlMessage(const std::string& msg) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    crawlMessages.push_back(msg);
    crawlTimestamps.push_back(std::chrono::steady_clock::now());
}

void AppState::pruneStaleCrawlMessages(std::function<int(const std::string&)> measureWidthFunc) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    auto now = std::chrono::steady_clock::now();
    size_t pruneCount = 0;
    int totalPrunedWidth = 0;
    while (pruneCount < crawlTimestamps.size()) {
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - crawlTimestamps[pruneCount]).count();
        if (diff < 3000) {
            break;
        }
        std::string fullMsg = crawlMessages[pruneCount] + "   *** ";
        totalPrunedWidth += measureWidthFunc(fullMsg);
        pruneCount++;
    }
    if (pruneCount > 0) {
        crawlMessages.erase(crawlMessages.begin(), crawlMessages.begin() + pruneCount);
        crawlTimestamps.erase(crawlTimestamps.begin(), crawlTimestamps.begin() + pruneCount);
        scrollOffset -= totalPrunedWidth;
        if (scrollOffset < 0 || crawlMessages.empty()) {
            scrollOffset = 0;
        }
    }
}

HWND AppState::getHWnd() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return hWnd;
}

void AppState::setHWnd(HWND hwnd) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    hWnd = hwnd;
}

int AppState::getScrollOffset() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return scrollOffset;
}

void AppState::setScrollOffset(int val) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    scrollOffset = val;
}

void AppState::incrementScrollOffset(int amount) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    scrollOffset += amount;
}

int AppState::getBannerScrollOffset() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return bannerScrollOffset;
}

void AppState::setBannerScrollOffset(int val) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    bannerScrollOffset = val;
}

void AppState::incrementBannerScrollOffset(int amount) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    bannerScrollOffset += amount;
}

int AppState::getBlinkState() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return blinkState;
}

void AppState::setBlinkState(int state) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    blinkState = state;
}

int AppState::getBlinkTimer() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return blinkTimer;
}

void AppState::setBlinkTimer(int timer) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    blinkTimer = timer;
}

void AppState::decrementBlinkTimer() {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    if (blinkTimer > 0) {
        blinkTimer--;
        if (blinkTimer == 0) {
            blinkState = 0;
        }
    }
}

bool AppState::isDebugConsoleMode() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return debugConsoleMode;
}

void AppState::setDebugConsoleMode(bool mode) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    debugConsoleMode = mode;
    if (mode) viewMode = MODE_CONSOLE;
    else if (viewMode == MODE_CONSOLE) viewMode = MODE_DASHBOARD;
}

AppViewMode AppState::getAppViewMode() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return viewMode;
}

void AppState::setAppViewMode(AppViewMode mode) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    viewMode = mode;
    debugConsoleMode = (mode == MODE_CONSOLE);
}

int AppState::getDynamicCrawlSpeed() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return dynamicCrawlSpeed;
}

void AppState::setDynamicCrawlSpeed(int speed) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    dynamicCrawlSpeed = speed;
}

int AppState::getDynamicOrbBlinkDuration() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return dynamicOrbBlinkDuration;
}

void AppState::setDynamicOrbBlinkDuration(int duration) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    dynamicOrbBlinkDuration = duration;
}
