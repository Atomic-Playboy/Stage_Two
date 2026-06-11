Stage Two: MIDI Production Dashboard

Stage Two is a high-performance Win32 C++ production environment designed for real-time MIDI signal transformation and hardware management. It features a unified dashboard UI, 4-slot GTR hardware simulation, and dynamic MIDI rule processing.
🛠 Features

    Multi-Threaded MIDI Routing: Low-latency signal processing via RtMidi.

    Unified Win32 Dashboard: A single-window chassis for monitoring and control.

    Dynamic Configuration: Real-time workspace updates via JSON-based rule sets.

    Hardware Simulation: 4-slot GTR pedalboard emulation with SysEx support.

🚀 Getting Started
Prerequisites

    IDE: CLion (or MSVC compatible environment).

    Toolchain: CMake 3.x+ and a C++17 compatible compiler (MSVC 2022+ recommended).

    Dependencies: This project utilizes nlohmann/json and RtMidi, which are managed automatically via CMake's FetchContent.

Build Instructions

    Clone the repository: git clone https://github.com/Atomic-Playboy/Stage_Two

    Open the project in CLion.

    Configure your CMake profile to use your local MSVC toolchain.

    Build the Stage_Two target.

Usage

    Configuration: Place your dashboard.json and transformer.json files in the same directory as the executable.

    Key Bindings:

        F4: View Dashboard

        F5: View GTR Simulator

        F6: Debug Console

    Logs: The engine generates gtr_engine.log in the execution directory for real-time diagnostics.

📂 Architecture

    App/: Entry point and WinMain initialization.

    Core/: Global application state, MIDI data structures, and logging.

    Config/: JSON parsing logic and file system watchers.

    Logic/: MIDI transformation and preset management.

    UI/: Win32 rendering, LCD simulation, and event handling.

⚖️ License

Distributed under the MIT License. See LICENSE for more information.