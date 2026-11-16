# Gridiron Lite

Welcome to Gridiron Lite, a lightweight and high performance football game written entirely in C++. The goal of this was to make a highly performant football game that could introduce realistic gameplay into an ultra-lightweight executable.

## Build and Run

Builds can be run for the Debug, Release, or Profile targets. For example:

```bash
cmake --build --preset debug
./build/debug/GridironLite
```

For running config only, run without the build flag

```bash
cmake --preset debug
```

## Requirements

- SDL3 (version 3.4.18)