#pragma once
#include <string>
#include <mutex>
#include <fstream>

class ThreadSafeLogger {
public:
    ThreadSafeLogger();
    ~ThreadSafeLogger();
    void log(const std::string& message);
    void setHistoryLimit(int limit);
    std::string getHistory();

private:
    std::mutex logMutex;
    std::string stringBuffer;
    std::ofstream logFile;
    int historyLimit;
};
