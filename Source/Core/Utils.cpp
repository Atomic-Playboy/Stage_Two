#include "Core/Utils.h"
#include "Core/PathConstants.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <mutex>
#include <cctype>

static std::mutex utilLogMutex;

int safeStoi(const std::string& str, int defaultValue) {
    try {
        if (str.empty()) return defaultValue;
        return std::stoi(str);
    } catch (...) {
        return defaultValue;
    }
}

std::string toLowerCase(std::string str) {
    for (char& c : str) {
        c = static_cast<char>(std::tolower(std::tolower(static_cast<unsigned char>(c))));
    }
    return str;
}

bool containsSubstringIgnoreCase(std::string haystack, std::string needle) {
    haystack = toLowerCase(haystack);
    needle = toLowerCase(needle);
    return haystack.find(needle) != std::string::npos;
}

bool parseChannelMatch(const std::string& confChan, unsigned char midiStatusByte) {
    std::string chan = toLowerCase(confChan);
    if (chan == "omni" || chan.empty()) return true;
    int expectedChan = safeStoi(chan, 1) - 1;
    int actualChan = midiStatusByte & 0x0F;
    return expectedChan == actualChan;
}

unsigned char getOutputChannelByte(unsigned char statusBase, const std::string& confChan) {
    std::string chan = toLowerCase(confChan);
    if (chan == "omni" || chan.empty()) {
        return statusBase;
    }
    int expectedChan = safeStoi(chan, 1);
    if (expectedChan < 1 || expectedChan > 16) expectedChan = 1;
    return (statusBase & 0xF0) | (static_cast<unsigned char>(expectedChan - 1) & 0x0F);
}

void appendToLogFile(const std::string& line) {
    std::lock_guard<std::mutex> lock(utilLogMutex);
    std::ofstream outFile("gtr_engine.log", std::ios_base::app);
    if (outFile.is_open()) {
        outFile << line << "\n";
    }
}
