#include "Core/AppStateTransformer.h"

AppStateTransformer& AppStateTransformer::getInstance() {
    static AppStateTransformer instance;
    return instance;
}

AppStateTransformer::AppStateTransformer() {}

std::vector<std::string> AppStateTransformer::getDiscoveredInputs() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return discoveredInputs;
}

void AppStateTransformer::setDiscoveredInputs(const std::vector<std::string>& inputs) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    discoveredInputs = inputs;
}

std::vector<std::string> AppStateTransformer::getCrawlMessages() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return crawlMessages;
}

void AppStateTransformer::pushCrawlMessage(const std::string& msg) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    crawlMessages.push_back(msg);
    crawlTimestamps.push_back(std::chrono::steady_clock::now());
}

void AppStateTransformer::pruneStaleCrawlMessages(std::function<int(const std::string&)> measureWidthFunc) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    auto now = std::chrono::steady_clock::now();
    size_t pruneCount = 0;
    int totalPrunedWidth = 0;
    while (pruneCount < crawlTimestamps.size()) {
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - crawlTimestamps[pruneCount]).count();
        if (diff < 3000) {
            break;
        }
        std::string fullMsg = crawlMessages[pruneCount] + "   *** ";
        totalPrunedWidth += measureWidthFunc(fullMsg);
        pruneCount++;
    }
    if (pruneCount > 0) {
        crawlMessages.erase(crawlMessages.begin(), crawlMessages.begin() + pruneCount);
        crawlTimestamps.erase(crawlTimestamps.begin(), crawlTimestamps.begin() + pruneCount);
        scrollOffset -= totalPrunedWidth;
        if (scrollOffset < 0 || crawlMessages.empty()) {
            scrollOffset = 0;
        }
    }
}

int AppStateTransformer::getScrollOffset() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return scrollOffset;
}

void AppStateTransformer::setScrollOffset(int val) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    scrollOffset = val;
}

void AppStateTransformer::incrementScrollOffset(int amount) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    scrollOffset += amount;
}

int AppStateTransformer::getBlinkState() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return blinkState;
}

void AppStateTransformer::setBlinkState(int state) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    blinkState = state;
}

int AppStateTransformer::getBlinkTimer() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return blinkTimer;
}

void AppStateTransformer::setBlinkTimer(int timer) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    blinkTimer = timer;
}

void AppStateTransformer::decrementBlinkTimer() {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    if (blinkTimer > 0) {
        blinkTimer--;
        if (blinkTimer == 0) {
            blinkState = 0;
        }
    }
}

int AppStateTransformer::getDynamicCrawlSpeed() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return dynamicCrawlSpeed;
}

void AppStateTransformer::setDynamicCrawlSpeed(int speed) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    dynamicCrawlSpeed = speed;
}

int AppStateTransformer::getDynamicOrbBlinkDuration() const {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    return dynamicOrbBlinkDuration;
}

void AppStateTransformer::setDynamicOrbBlinkDuration(int duration) {
    std::lock_guard<std::recursive_mutex> lock(stateMutex);
    dynamicOrbBlinkDuration = duration;
}
