#ifndef MIDI_UTILS_H
#define MIDI_UTILS_H

#include <string>

bool parseChannelMatch(const std::string& confChan, unsigned char midiStatusByte);
unsigned char getOutputChannelByte(unsigned char statusBase, const std::string& confChan);

#endif // MIDI_UTILS_H
