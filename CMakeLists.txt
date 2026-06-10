cmake_minimum_required(VERSION 3.20)

project(GTRMapperDashboard
        VERSION 1.0
        LANGUAGES CXX
)

# ------------------------------------------------------------
# Global configuration
# ------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# IMPORTANT: ensures WinMain entry point + Windows subsystem
set(CMAKE_WIN32_EXECUTABLE TRUE)

# Prevent in-source builds (clean architecture hygiene)
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are not allowed.")
endif()

# ------------------------------------------------------------
# Output directories (clean separation)
# ------------------------------------------------------------
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ------------------------------------------------------------
# Project sources
# ------------------------------------------------------------
set(SOURCES
		Source/App/main.cpp
		Source/Config/ConfigParser.cpp
		Source/Config/FileWatcher.cpp
		Source/Core/AppState.cpp
		Source/Core/CoreGlobals.cpp
		Source/Core/DisplayState.cpp
		Source/Core/Logger.cpp
		Source/Core/Utils.cpp
		Source/Logic/PresetManager.cpp
		Source/MIDI/EvaluationEngine.cpp
		Source/UI/MainWindow.cpp
		Source/UI/Win32App.cpp		
)

add_executable(Stage_Two WIN32 ${SOURCES})

# ADD THIS LINE RIGHT HERE:
target_include_directories(Stage_Two PRIVATE Source)

# ------------------------------------------------------------
# Compile definitions (Windows hygiene)
# ------------------------------------------------------------
target_compile_definitions(Stage_Two PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _CRT_SECURE_NO_WARNINGS
)

# ------------------------------------------------------------
# Warning level (MSVC)
# ------------------------------------------------------------
if (MSVC)
    target_compile_options(Stage_Two PRIVATE
            /W4
            /permissive-
            /EHsc
            /Zi
    )
endif()

# ------------------------------------------------------------
# Threads
# ------------------------------------------------------------
find_package(Threads REQUIRED)

# ------------------------------------------------------------
# Dependencies via FetchContent (fully reproducible builds)
# ------------------------------------------------------------
include(FetchContent)

# ---- nlohmann/json ----
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# ---- RtMidi ----
FetchContent_Declare(
        rtmidi
        GIT_REPOSITORY https://github.com/thestk/rtmidi.git
        GIT_TAG master
)
FetchContent_MakeAvailable(rtmidi)

# ------------------------------------------------------------
# Link libraries
# ------------------------------------------------------------
target_link_libraries(Stage_Two PRIVATE
        rtmidi
        nlohmann_json::nlohmann_json
        Threads::Threads

        # Windows system libs
        winmm
        comdlg32
        shell32
)

# ------------------------------------------------------------
# IDE friendliness (CLion / VS)
# ------------------------------------------------------------
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Stage_Two)

# ------------------------------------------------------------
# Optional: better diagnostics in Debug
# ------------------------------------------------------------
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(Stage_Two PRIVATE DEBUG_BUILD)
endif()