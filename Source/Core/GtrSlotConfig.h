#ifndef GTR_SLOT_CONFIG_H
#define GTR_SLOT_CONFIG_H

#include "Core/Preset.h"
#include <string>
#include <vector>

struct GtrSlotConfig {
    bool loaded = false;
    std::string filepath;
    std::string filename = "[No Config Loaded]";
    std::string inPort1, inPort2;
    std::string outPort1, outPort2;
    std::string inChan = "omni";
    std::string outChan = "omni";
    std::string inChan2 = "omni";
    std::string outChan2 = "omni";
    std::vector<Preset> presets;
    int activeAbState = 0;
    int activePcNum = -1;
    std::string activePresetName = "";
};

#endif // GTR_SLOT_CONFIG_H
