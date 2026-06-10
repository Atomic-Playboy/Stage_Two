#pragma once
#include <string>
#include <vector>
#include <map>
#include <windows.h>

struct DualColorNode {
    COLORREF dim;
    COLORREF bright;
};

struct DashboardBlock {
    std::string effect;
    int cc = 0;
    std::string colorName;
    std::string textColorName;
    std::string loadState;
    bool transientState = false;
    int ccOnOverride = -1;
    int ccOffOverride = -1;
};

struct PresetMatrixNode {
    bool active = false;
    std::string songTitle;
    std::vector<DashboardBlock> interactiveTiles;
};

struct SystemTuning {
    std::string midiInterface = "ANY";
    int defaultChannel = -1;
    int ccOnDefaultValue = 127;
    int ccOffDefaultValue = 0;
    int debugHistoryLimit = 50000;
    int crawlSpeed = 1;
    int orbBlinkDuration = 12;
    bool filterMidiClock = true;
    bool filterActiveSensing = true;
    bool verboseRawCrawl = false;
};

struct WorkspaceConfig {
    SystemTuning systemTuning;
    std::map<std::string, std::string> webCreatorThemeData;
    std::map<std::string, DualColorNode> themeColors;
    std::map<int, PresetMatrixNode> programChangeDashboardMatrix;
};

struct RawMidiMessage {
    std::string interfaceName;
    std::string type;
    int channel = 0;
    int ccNumber = 0;
    int ccValue = 0;
    int programNumber = 0;
};

struct Transform {
    std::string interfaceName;
    std::string channel;
    std::string type;
    std::vector<std::string> details;
};

struct MIDITrigger {
    std::string priority;
    std::string comment;
    struct {
        std::string interfaceName;
        std::string channel;
        std::string type;
        std::vector<std::string> details;
    } trigger;
    std::vector<Transform> transforms;
};

