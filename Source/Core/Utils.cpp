#include "Core/Utils.h"
#include <algorithm>
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
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return str;
}

bool containsSubstringIgnoreCase(std::string haystack, std::string needle) {
    haystack = toLowerCase(haystack);
    needle = toLowerCase(needle);
    return haystack.find(needle) != std::string::npos;
}

void appendToLogFile(const std::string& line) {
    std::lock_guard<std::mutex> lock(utilLogMutex);
    std::ofstream outFile("gtr_engine.log", std::ios_base::app);
    if (outFile.is_open()) {
        outFile << line << "\n";
    }
}
