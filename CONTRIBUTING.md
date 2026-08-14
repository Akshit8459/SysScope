# Contributing to SysScope

Thank you for your interest in contributing to **SysScope**!

## Code Architecture & Constraints

1. **Zero Qt Dependency in Core Engine (`libsysscope_core.a`)**:
   - `libsysscope_core.a` must remain 100% pure C++20 / POSIX.
   - Do NOT include any Qt headers (`<QObject>`, `<QWidget>`, etc.) inside `include/sysscope/core/`, `collectors/`, `platform/`, `telemetry/`, `analytics/`, or `storage/`.
   - Presentation layers (`src/ui/qt/`) access core data models via `QtTelemetryBridge`.

2. **C++20 & Strict Compilation Flags**:
   - Code must compile cleanly under GCC and Clang with `-Wall -Wextra -Wpedantic -Werror`.
   - Use RAII, standard smart pointers (`std::unique_ptr`, `std::shared_ptr`), and non-throwing `sysscope::core::Result<T>` error handling.

3. **Testing Requirements**:
   - Every collector or analytics feature must include unit test coverage in `tests/unit/`.
   - Ensure all 28 core integration tests pass cleanly before submitting changes:
     ```bash
     ./build_linux/tests/syscope_test_suite
     ```
