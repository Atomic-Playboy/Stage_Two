#ifndef CORE_GLOBALS_H
#define CORE_GLOBALS_H

#include <memory>
#include "Config/FileWatcher.h"
#include "MIDI/EvaluationEngine.h"
#include "Core/Logger.h"

namespace CoreGlobals {
    extern std::unique_ptr<FileWatcher> watcher;
    extern std::unique_ptr<EvaluationEngine> engine;
    extern ThreadSafeLogger logger;

    void reloadConfiguration();
}

#endif // CORE_GLOBALS_H
