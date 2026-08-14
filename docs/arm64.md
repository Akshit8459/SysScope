# SysScope ARM64 & Heterogeneous SoC Design

## Target Architecture
SysScope is designed for embedded Linux, ARM64 SoCs, and heterogeneous acceleration targets (CPU, GPU, NPU, DSP).

```
                 SysScope Architecture
                            │
        ┌───────────────────┼───────────────────┐
        ▼                   ▼                   ▼
     CPU Cores         GPU Backend          NPU / DSP
  (/proc/stat)     (Qualcomm/NVIDIA)    (AcceleratorBackend)
```

## Cross-Compilation Verification

SysScope includes a dedicated CMake toolchain file (`cmake/toolchain-aarch64-linux-gnu.cmake`) for cross-compiling ARM64 AArch64 Linux binaries:

```bash
cmake -B build_arm64 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-aarch64-linux-gnu.cmake
cmake --build build_arm64
```

Binary Architecture Output:
`./build_arm64/syscope: ELF 64-bit LSB executable, ARM aarch64 ...`

*Note: ARM64 cross-compilation is verified; physical target hardware on-device deployment remains pending target hardware.*
