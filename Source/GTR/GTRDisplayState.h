#ifndef GTR_DISPLAY_STATE_H
#define GTR_DISPLAY_STATE_H

#include <string>

struct DisplayState {
    std::string text = "---";
    bool isActive = false;
    bool isBright = false;
};

void mapAsciiToGtrSegments(char c, unsigned char& outerByte, unsigned char& innerByte);
#endif // GTR_DISPLAY_STATE_H
