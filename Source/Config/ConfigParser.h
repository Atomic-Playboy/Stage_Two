#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "Core/MidiData.h"
#include "Core/GtrWorkspaceConfig.h"
#include "Core/Logger.h"
#include <string>
#include <vector>

class ConfigParser {
public:
    static bool loadWorkspaceJson(const std::string& filePath, WorkspaceConfig& workspace, ThreadSafeLogger& logger);
    static std::vector<MIDITrigger> loadTransformerJson(const std::string& filePath, ThreadSafeLogger& logger);
    static bool loadGtrJsonConfig(int slotIndex, const std::string& filePath, GtrWorkspaceConfig& config, ThreadSafeLogger& logger);
};

#endif // CONFIG_PARSER_H
