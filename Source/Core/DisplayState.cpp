#include "Core/DisplayState.h"
#include <cctype>

void mapAsciiToGtrSegments(char c, unsigned char& outerByte, unsigned char& innerByte) {
    outerByte = 0x00;
    innerByte = 0x00;
    c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    switch (c) {
        case 'A': outerByte = 0x77; innerByte = 0x01; break;
        case 'B': outerByte = 0x0F; innerByte = 0x25; break;
        case 'C': outerByte = 0x39; innerByte = 0x00; break;
        case 'D': outerByte = 0x0F; innerByte = 0x24; break;
        case 'E': outerByte = 0x79; innerByte = 0x01; break;
        case 'F': outerByte = 0x71; innerByte = 0x01; break;
        case 'G': outerByte = 0x3D; innerByte = 0x01; break;
        case 'H': outerByte = 0x76; innerByte = 0x01; break;
        case 'I': outerByte = 0x09; innerByte = 0x24; break;
        case 'J': outerByte = 0x1E; innerByte = 0x00; break;
        case 'K': outerByte = 0x70; innerByte = 0x18; break;
        case 'L': outerByte = 0x38; innerByte = 0x00; break;
        case 'M': outerByte = 0x36; innerByte = 0x0A; break;
        case 'N': outerByte = 0x36; innerByte = 0x12; break;
        case 'O': outerByte = 0x3F; innerByte = 0x00; break;
        case 'P': outerByte = 0x73; innerByte = 0x01; break;
        case 'Q': outerByte = 0x3F; innerByte = 0x10; break;
        case 'R': outerByte = 0x73; innerByte = 0x11; break;
        case 'S': outerByte = 0x6D; innerByte = 0x01; break;
        case 'T': outerByte = 0x01; innerByte = 0x24; break;
        case 'U': outerByte = 0x3E; innerByte = 0x00; break;
        case 'V': outerByte = 0x30; innerByte = 0x48; break;
        case 'W': outerByte = 0x36; innerByte = 0x50; break;
        case 'X': outerByte = 0x00; innerByte = 0x5A; break;
        case 'Y': outerByte = 0x00; innerByte = 0x2A; break;
        case 'Z': outerByte = 0x09; innerByte = 0x48; break;
        case '1': outerByte = 0x06; innerByte = 0x00; break;
        case '2': outerByte = 0x5B; innerByte = 0x01; break;
        case '3': outerByte = 0x4F; innerByte = 0x01; break;
        case '4': outerByte = 0x66; innerByte = 0x01; break;
        case '5': outerByte = 0x6D; innerByte = 0x01; break;
        case '6': outerByte = 0x7D; innerByte = 0x01; break;
        case '7': outerByte = 0x27; innerByte = 0x00; break;
        case '8': outerByte = 0x7F; innerByte = 0x01; break;
        case '9': outerByte = 0x67; innerByte = 0x01; break;
        case '0': outerByte = 0x3F; innerByte = 0x48; break;
        case '-': outerByte = 0x40; innerByte = 0x01; break;
        case '_': outerByte = 0x08; innerByte = 0x00; break;
        case '*': outerByte = 0x40; innerByte = 0xFF; break;
        case ' ': outerByte = 0x00; innerByte = 0x00; break;
        default:  outerByte = 0x00; innerByte = 0x00; break;
    }
}
