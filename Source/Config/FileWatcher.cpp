/**
 * FileWatcher.cpp
 * * Architectural Decision:
 * We use Windows API FindFirstChangeNotification to block and wait for OS-level
 * file system interrupts rather than pegging the CPU with a constant while-loop.
 * A debounce sleep (100ms) allows IDEs/Notepad to finish writing the file lock 
 * before we trigger the parse reload.
 */

#include "FileWatcher.h"
#include <chrono>

FileWatcher::FileWatcher() {}
FileWatcher::~FileWatcher() { stopWatching(); }

void FileWatcher::initializePaths(const std::string& dashboardJsonFile, const std::string& transformerJsonFile) {
    std::lock_guard<std::mutex> lock(pathMutex);
    targetDashboardJson = dashboardJsonFile;
    targetTransformerJson = transformerJsonFile;
}

void FileWatcher::updateDashboardJsonPath(const std::string& newPath) {
    std::lock_guard<std::mutex> lock(pathMutex);
    targetDashboardJson = newPath;
}

void FileWatcher::updateTransformerJsonPath(const std::string& newPath) {
    std::lock_guard<std::mutex> lock(pathMutex);
    targetTransformerJson = newPath;
}

FILETIME FileWatcher::getFileLastWriteTime(const std::string& path) {
    FILETIME ftWrite = { 0, 0 };
    if(path.empty()) return ftWrite;
    HANDLE hFile = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile != INVALID_HANDLE_VALUE) {
        GetFileTime(hFile, NULL, NULL, &ftWrite);
        CloseHandle(hFile);
    }
    return ftWrite;
}

std::string FileWatcher::extractDirectoryPath(const std::string& fullPath) {
    std::string::size_type pos = fullPath.find_last_of("\\/");
    if (pos == std::string::npos) return ".";
    return fullPath.substr(0, pos);
}

// [STUDY GUIDE: Williams, Chapter 2 - "Managing Threads"]
// Spawns a detached background thread to poll the file system without freezing the main UI.
void FileWatcher::startWatching(std::function<void()> onReloadCallback) {
    stopWatching();
    keepRunning = true;
    reloadCallback = onReloadCallback;
    watchThread = std::thread(&FileWatcher::runWatcherLoop, this);
}

void FileWatcher::stopWatching() {
    if (keepRunning) {
        keepRunning = false;
        if (watchThread.joinable()) {
            watchThread.join();
        }
    }
}

void FileWatcher::runWatcherLoop() {
    std::string lastDash, lastTrans;
    {
        std::lock_guard<std::mutex> lock(pathMutex);
        lastDash = targetDashboardJson;
        lastTrans = targetTransformerJson;
    }

    std::string dashDir = extractDirectoryPath(lastDash);
    
    // [STUDY GUIDE: Petzold / Windows API Reference - Directory Management]
    // FindFirstChangeNotification is extremely efficient compared to basic while(true) loops.
    HANDLE hDirNotify = FindFirstChangeNotificationA(dashDir.c_str(), FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME);

    FILETIME lastDashTime = getFileLastWriteTime(lastDash);
    FILETIME lastTransTime = getFileLastWriteTime(lastTrans);

    while (keepRunning) {
        if (hDirNotify != INVALID_HANDLE_VALUE) {
            DWORD dwWait = WaitForSingleObject(hDirNotify, 800);
            if (dwWait == WAIT_OBJECT_0) {
                // Debounce sleep to prevent partial reads during IDE save operations
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::string currentDash, currentTrans;
                {
                    std::lock_guard<std::mutex> lock(pathMutex);
                    currentDash = targetDashboardJson;
                    currentTrans = targetTransformerJson;
                }

                FILETIME currentDashTime = getFileLastWriteTime(currentDash);
                FILETIME currentTransTime = getFileLastWriteTime(currentTrans);

                if (CompareFileTime(&lastDashTime, &currentDashTime) != 0 ||
                    CompareFileTime(&lastTransTime, &currentTransTime) != 0) {
                    lastDashTime = currentDashTime;
                    lastTransTime = currentTransTime;
                    if (reloadCallback) reloadCallback();
                }
                FindNextChangeNotification(hDirNotify);
            }
        } else {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    if (hDirNotify != INVALID_HANDLE_VALUE) FindCloseChangeNotification(hDirNotify);
}
