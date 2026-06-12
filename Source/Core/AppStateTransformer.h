#ifndef APP_STATE_TRANSFORMER_H
#define APP_STATE_TRANSFORMER_H

#include <vector>
#include <string>
#include <chrono>
#include <mutex>
#include <functional>

class AppStateTransformer {
public:
    static AppStateTransformer& getInstance();
    std::vector<std::string> getDiscoveredInputs() const;
    void setDiscoveredInputs(const std::vector<std::string>& inputs);
    std::vector<std::string> getCrawlMessages() const;
    void pushCrawlMessage(const std::string& msg);
    void pruneStaleCrawlMessages(std::function<int(const std::string&)> measureWidthFunc);
    int getScrollOffset() const;
    void setScrollOffset(int val);
    void incrementScrollOffset(int amount);
    int getBlinkState() const;
    void setBlinkState(int state);
    int getBlinkTimer() const;
    void setBlinkTimer(int timer);
    void decrementBlinkTimer();
    int getDynamicCrawlSpeed() const;
    void setDynamicCrawlSpeed(int speed);
    int getDynamicOrbBlinkDuration() const;
    void setDynamicOrbBlinkDuration(int duration);

private:
    AppStateTransformer();
    ~AppStateTransformer() = default;
    AppStateTransformer(const AppStateTransformer&) = delete;
    AppStateTransformer& operator=(const AppStateTransformer&) = delete;

    mutable std::recursive_mutex stateMutex;
    std::vector<std::string> discoveredInputs;
    std::vector<std::string> crawlMessages;
    std::vector<std::chrono::steady_clock::time_point> crawlTimestamps;
    int scrollOffset = 0;
    int blinkState = 0;
    int blinkTimer = 0;
    int dynamicCrawlSpeed = 2;
    int dynamicOrbBlinkDuration = 10;
};

#endif // APP_STATE_TRANSFORMER_H
