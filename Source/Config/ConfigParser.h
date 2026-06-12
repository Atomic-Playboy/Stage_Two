#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "Core/MidiData.h"
#include "GTR/GtrSlotConfig.h"
#include "Core/Logger.h"
#include <string>
#include <vector>

class ConfigParser {
public:
    static bool loadDashboardJson(const std::string& filePath, DashboardConfig& dashboard, ThreadSafeLogger& logger);
    static std::vector<MIDITrigger> loadTransformerJson(const std::string& filePath, ThreadSafeLogger& logger);
    static bool loadGtrJsonConfig(int slotIndex, const std::string& filePath, GtrSlotConfig& config, ThreadSafeLogger& logger);
};
#endif // CONFIG_PARSER_H
