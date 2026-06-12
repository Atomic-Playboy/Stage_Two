/**
 * Logger.cpp
 * Thread-safe logging mechanism preventing buffer overflows in UI mode.
 */

#include "Logger.h"
#include "Core/AppConstants.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

ThreadSafeLogger::ThreadSafeLogger() {
    historyLimit = 50000;
    stringBuffer = "";
    logFile.open(AppConstants::LogFile, std::ios::out | std::ios::app);
}

ThreadSafeLogger::~ThreadSafeLogger() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) logFile.close();
}

// [STUDY GUIDE: Williams, Chapter 3.2 - "Protecting shared data with mutexes"]
// std::lock_guard ensures that if multiple threads try to log simultaneously (e.g. rapid MIDI input + UI event),
// only one thread can modify the stringBuffer and write to the file at a time.
void ThreadSafeLogger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    struct tm buf;
    localtime_s(&buf, &time_t_now);
    std::stringstream ss;

    ss << "[" << std::put_time(&buf, "%H:%M:%S") << "." << std::setw(3) << std::setfill('0') << ms.count() << "] " << message << "\r\n";
    
    std::string formatted = ss.str();
    stringBuffer.append(formatted);
    if (logFile.is_open()) {
        logFile << formatted;
        logFile.flush();
    }
    if (stringBuffer.length() > (size_t)historyLimit) {
        stringBuffer = stringBuffer.substr(stringBuffer.length() - 10000);
    }
    std::cout << message << std::endl;
}

void ThreadSafeLogger::setHistoryLimit(int limit) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (limit > 0) historyLimit = limit;
}

std::string ThreadSafeLogger::getHistory() {
    std::lock_guard<std::mutex> lock(logMutex);
    return stringBuffer;
}
