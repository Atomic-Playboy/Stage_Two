#ifndef GTR_SEND_LIGHTS_H
#define GTR_SEND_LIGHTS_H

#include "Core/Preset.h"
#include "Core/GtrSlotConfig.h"

void activatePreset(int slotIndex, const Preset& preset, GtrSlotConfig& config);
void manualForceSendLights(int slotIndex);
void manualForceSendMidi(int slotIndex);
void manualStepProgramChange(int slotIndex, int direction);

#endif // GTR_SEND_LIGHTS_H
