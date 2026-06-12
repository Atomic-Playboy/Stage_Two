#include "Core/AppStateGTR.h"

AppStateGTR& AppStateGTR::getInstance() {
    static AppStateGTR instance;
    return instance;
}

AppStateGTR::AppStateGTR() {}

GtrSlotConfig AppStateGTR::getGtrSlotConfig(int slot) const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    if (slot >= 0 && slot < AppConstants::MAX_HARDWARE_SLOTS) return gtrSlots[slot];
    return GtrSlotConfig();
}

void AppStateGTR::setGtrSlotConfig(int slot, const GtrSlotConfig& config) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    if (slot >= 0 && slot < AppConstants::MAX_HARDWARE_SLOTS) gtrSlots[slot] = config;
}

DisplayState AppStateGTR::getGtrDisplayState(int slot, int btnIndex) const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    if (slot >= 0 && slot < AppConstants::MAX_HARDWARE_SLOTS && btnIndex >= 0 && btnIndex < 7) {
        return currentGuiLcds[slot][btnIndex];
    }
    return DisplayState();
}

void AppStateGTR::setGtrDisplayState(int slot, int btnIndex, const DisplayState& state) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    if (slot >= 0 && slot < AppConstants::MAX_HARDWARE_SLOTS && btnIndex >= 0 && btnIndex < 7) {
        currentGuiLcds[slot][btnIndex] = state;
    }
}

bool AppStateGTR::getPhysicalButtonState(int ccNum) const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    auto it = physicalButtonState.find(ccNum);
    if (it != physicalButtonState.end()) return it->second;
    return false;
}

void AppStateGTR::setPhysicalButtonState(int ccNum, bool val) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    physicalButtonState[ccNum] = val;
}
