/**
 * MidiUtils.cpp
 * Dedicated low-level bitwise operations for the MIDI 1.0 Specification.
 */

#include "MIDI/MidiUtils.h"
#include "Core/Utils.h"
#include <cctype>

// [STUDY GUIDE: MIDI 1.0 Spec - "Channel Voice Messages"]
// The bitwise AND (& 0x0F) masks out the top 4 bits of the status byte, leaving only the channel.
bool parseChannelMatch(const std::string& confChan, unsigned char midiStatusByte) {
    std::string chan = toLowerCase(confChan);
    if (chan == "omni" || chan.empty()) return true;
    int expectedChan = safeStoi(chan, 1) - 1;
    int actualChan = midiStatusByte & 0x0F;
    return expectedChan == actualChan;
}

// Reconstructs a full MIDI status byte (e.g., 0xB0 for CC) by masking the command 
// into the upper nibble (0xF0) and the requested channel into the lower nibble (0x0F).
unsigned char getOutputChannelByte(unsigned char statusBase, const std::string& confChan) {
    std::string chan = toLowerCase(confChan);
    if (chan == "omni" || chan.empty()) {
        return statusBase;
    }
    int expectedChan = safeStoi(chan, 1);
    if (expectedChan < 1 || expectedChan > 16) expectedChan = 1;
    return (statusBase & 0xF0) | (static_cast<unsigned char>(expectedChan - 1) & 0x0F);
}
