#ifndef PRESET_MANAGER_H
#define PRESET_MANAGER_H

#include "Core/Preset.h"
#include "Core/GtrWorkspaceConfig.h"

void activatePreset(int slotIndex, const Preset& preset, GtrWorkspaceConfig& config);
void manualForceSendLights(int slotIndex);
void manualForceSendMidi(int slotIndex);
void manualStepProgramChange(int slotIndex, int direction);

#endif // PRESET_MANAGER_H
