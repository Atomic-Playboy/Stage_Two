#ifndef EVALUATION_ENGINE_H
#define EVALUATION_ENGINE_H

#include <RtMidi.h>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <memory>
#include <functional>
#include <mutex>
#include "Core/MidiData.h"
#include "Core/Logger.h"

class EvaluationEngine {
public:
    EvaluationEngine(
        ThreadSafeLogger& systemLogger,
        std::function<void(const std::string&)> onPatchUpdate,
        std::function<void(const std::string&)> onRawMidiCrawl,
        std::function<void(int, int)> onStateChange
    );
    ~EvaluationEngine();

    void setTransformerRules(const std::vector<MIDITrigger>& rules) {
        std::lock_guard<std::mutex> lock(engineMutex);
        transformerRules = rules;
    }

    void updateDashboardInterfaces(const std::string& dashInt, int dashChan) {
        std::lock_guard<std::mutex> lock(engineMutex);
        dashInterface = dashInt;
        dashChannel = dashChan;
    }

    void updateTuningGlobals(const SystemTuning& globals) {
        std::lock_guard<std::mutex> lock(engineMutex);
        liveGlobals = globals;
        logger.setHistoryLimit(globals.debugHistoryLimit);
    }

    void selectActiveHardwareInterface(const std::string& interfaceName);
    std::vector<std::string> getDiscoveredInputs() const;

    // Core functionality upgrades for GTR Integration
    void rebuildPorts(const std::set<std::string>& requestedInPorts, const std::set<std::string>& requestedOutPorts);
    void transmitMidi(const std::string& portName, const std::vector<unsigned char>& message);

    ThreadSafeLogger& getLogger() { return logger; }
    
    std::vector<MIDITrigger> transformerRules;
    SystemTuning liveGlobals;
    std::string dashInterface;
    int dashChannel;

private:
    void queryAvailableHardware();

    ThreadSafeLogger& logger;
    std::function<void(const std::string&)> patchCallback;
    std::function<void(const std::string&)> crawlCallback;
    std::function<void(int, int)> stateCallback;

    std::unique_ptr<RtMidiIn> midiInScanner;
    std::vector<std::string> discoveredInputNames;

    std::map<std::string, std::unique_ptr<RtMidiIn>> activeInputPorts;
    std::map<std::string, std::unique_ptr<RtMidiOut>> activeOutputPorts;
    std::mutex engineMutex;
};

#endif // EVALUATION_ENGINE_H
