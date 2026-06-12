#ifndef GTR_PRESET_H
#define GTR_PRESET_H

#include "GTR/GTRDisplayState.h"
#include <string>

struct Preset {
    std::string name = "Clean";
    int pcTrigger = -1;
    bool sendPc = false;
    bool sendConfig = false;
    int abOffState = 0;
    int abOnState = 0;
    int abInitialState = 0;
    DisplayState displays[7];
};

#endif // GTR_PRESET_H
