#pragma once
#include <string>
#include <mutex>
#include <thread>
#include <functional>
#include <windows.h>

class FileWatcher {
public:
    FileWatcher();
    ~FileWatcher();
    void initializePaths(const std::string& dashboardJsonFile, const std::string& transformerJsonFile);
    void updateDashboardJsonPath(const std::string& newPath);
    void updateTransformerJsonPath(const std::string& newPath);
    std::string getTargetDashboardJson() const {
        std::lock_guard<std::mutex> lock(pathMutex);
        return targetDashboardJson;
    }
    std::string getTargetTransformerJson() const {
        std::lock_guard<std::mutex> lock(pathMutex);
        return targetTransformerJson;
    }

    void startWatching(std::function<void()> onReloadCallback);
    void stopWatching();

private:
    void runWatcherLoop();
    FILETIME getFileLastWriteTime(const std::string& path);
    std::string extractDirectoryPath(const std::string& fullPath);

    std::string targetDashboardJson;
    std::string targetTransformerJson;
    mutable std::mutex pathMutex;

    std::thread watchThread;
    bool keepRunning = false;
    std::function<void()> reloadCallback;
};
