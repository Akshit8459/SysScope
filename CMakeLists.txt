cmake_minimum_required(VERSION 3.20)

project(SysScope
    VERSION 0.3.0
    DESCRIPTION "Unified C++20 Linux System Observability & Performance Diagnostics Platform"
    LANGUAGES C CXX
)

# C++ Standard Configuration
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(Sanitizers)

# Options
option(BUILD_TESTS "Build SysScope unit and integration tests" ON)
option(ENABLE_SANITY_CHECKS "Enable ASan/UBSan sanitizers" OFF)

# Generate version header
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/sysscope/core/version.hpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/include/sysscope/core/version.hpp"
)

# Include Directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

# Core Sources (Pure C++20 / POSIX, strictly Qt-Free)
file(GLOB_RECURSE SYSSCOPE_SOURCES
    "src/core/*.cpp"
    "src/platform/*.cpp"
    "src/util/*.cpp"
    "src/collectors/*.cpp"
    "src/telemetry/*.cpp"
    "src/analytics/*.cpp"
    "src/ui/tui/*.cpp"
    "src/ui/*.cpp"
    "src/storage/*.cpp"
    "src/ipc/*.cpp"
)
list(FILTER SYSSCOPE_SOURCES EXCLUDE REGEX "src/ui/qt/.*")

# Core Static Library (100% Qt-Free)
add_library(sysscope_core STATIC ${SYSSCOPE_SOURCES})
target_include_directories(sysscope_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_compiler_warnings(sysscope_core)

if(ENABLE_SANITY_CHECKS)
    enable_sanitizers(sysscope_core)
endif()

# TUI & CLI Main Binary
add_executable(syscope src/main.cpp)
target_link_libraries(syscope PRIVATE sysscope_core)
set_target_compiler_warnings(syscope)

# Optional Qt6 Desktop Telemetry Visualizer Target (syscope_gui)
find_package(Qt6 COMPONENTS Widgets Charts QUIET)
if(Qt6_FOUND)
    message(STATUS "-- Qt6 found: building syscope_gui desktop visualizer target")
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    file(GLOB_RECURSE QT_UI_SOURCES "src/ui/qt/*.cpp")
    set(QT_UI_HEADERS
        "include/sysscope/ui/qt/telemetry_bridge.hpp"
        "include/sysscope/ui/qt/telemetry_model.hpp"
        "include/sysscope/ui/qt/diagnosis_model.hpp"
        "include/sysscope/ui/qt/process_tree_model.hpp"
        "include/sysscope/ui/qt/history_service.hpp"
        "include/sysscope/ui/qt/playback_controller.hpp"
        "include/sysscope/ui/qt/main_window.hpp"
        "include/sysscope/ui/qt/chart/telemetry_chart.hpp"
        "include/sysscope/ui/qt/pages/dashboard_page.hpp"
        "include/sysscope/ui/qt/pages/cpu_page.hpp"
        "include/sysscope/ui/qt/pages/memory_page.hpp"
        "include/sysscope/ui/qt/pages/psi_page.hpp"
        "include/sysscope/ui/qt/pages/process_page.hpp"
        "include/sysscope/ui/qt/pages/diagnostics_page.hpp"
        "include/sysscope/ui/qt/pages/history_page.hpp"
    )
    add_executable(syscope_gui ${QT_UI_SOURCES} ${QT_UI_HEADERS})
    target_compile_definitions(syscope_gui PRIVATE SYSSCOPE_HAS_QT)
    target_link_libraries(syscope_gui PRIVATE sysscope_core Qt6::Widgets Qt6::Charts)
    target_include_directories(syscope_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
else()
    message(STATUS "-- Qt6 not found: skipping syscope_gui desktop visualizer target")
endif()

# Stressor Tools
add_executable(synthetic_load tools/synthetic_load.cpp)
add_executable(memory_stressor tools/memory_stressor.cpp)
add_executable(io_stressor tools/io_stressor.cpp)

# Testing Suite
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
