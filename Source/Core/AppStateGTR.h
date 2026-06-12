#ifndef APP_STATE_GTR_H
#define APP_STATE_GTR_H

#include "GTR/GtrSlotConfig.h"
#include "GTR/GTRDisplayState.h"
#include "Core/AppConstants.h"
#include <map>
#include <mutex>

class AppStateGTR {
public:
    static AppStateGTR& getInstance();
    GtrSlotConfig getGtrSlotConfig(int slot) const;
    void setGtrSlotConfig(int slot, const GtrSlotConfig& config);
    DisplayState getGtrDisplayState(int slot, int btnIndex) const;
    void setGtrDisplayState(int slot, int btnIndex, const DisplayState& state);
    bool getPhysicalButtonState(int ccNum) const;
    void setPhysicalButtonState(int ccNum, bool val);

private:
    AppStateGTR();
    ~AppStateGTR() = default;
    AppStateGTR(const AppStateGTR&) = delete;
    AppStateGTR& operator=(const AppStateGTR&) = delete;

    mutable std::recursive_mutex stateMutex;
    GtrSlotConfig gtrSlots[AppConstants::MAX_HARDWARE_SLOTS];
    DisplayState currentGuiLcds[AppConstants::MAX_HARDWARE_SLOTS][7];
    std::map<int, bool> physicalButtonState;
};

#endif // APP_STATE_GTR_H
