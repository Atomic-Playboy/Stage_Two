#ifndef DISPLAY_STATE_H
#define DISPLAY_STATE_H

#include <string>

struct DisplayState {
    std::string text = "---";
    bool isActive = false;
    bool isBright = false;
};

void mapAsciiToGtrSegments(char c, unsigned char& outerByte, unsigned char& innerByte);

#endif // DISPLAY_STATE_H
