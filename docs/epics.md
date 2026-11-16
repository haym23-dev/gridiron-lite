# Epics and Stories - Gridiron Lite

## E1 - Engine Foundation

- [x] **E1-1:** As a developer, I want a CMake project with Debug, Release and Profile configs, so that every build is reproducible. *AC* is one command builds on MacOS. Release uses LTO, `-fno-exceptions` and `fno-rtti`.
- [ ] **E1-2:** As a player, I want the game to open a window and respond to keyboard and gamepad, so that I can control it. *AC* is SDL3 window opening and input mapped to actions, not raw keys, and hot-plugging a pad works.
  - [x] `src/platform/window.*` owns SDL init/shutdown and creates/destroys the window; nothing else calls `SDL_Init`, `SDL_CreateWindow` or `SDL_Quit`.
  - [x] `src/platform/input.*` maps keyboard scancodes and gamepad buttons/axes to a neutral action set; callers read actions, never raw devices.
  - [ ] A per-frame snapshot exposes held / pressed / released per action plus dead-zoned analog axes, ready for E1-3.
  - [x] `SDL_EVENT_GAMEPAD_ADDED` / `_REMOVED` open and close pads in a fixed slot array, clearing that pad's input on removal.
  - [x] Window opens, resizes and closes cleanly in Debug, Release and Profile.
  - [ ] Every action is reachable from the keyboard.
  - [ ] No raw key or gamepad reads leak outside `src/platform/` (grep `SDL_EVENT_KEY`, `SDL_GetKeyboardState`, `SDL_Gamepad`).
  - [ ] Hot-plug sequence passes: no pad, plug in and control, unplug without a crash, replug and control again.
  - [ ] Two pads connected have defined, documented behaviour.
- [ ] **E1-3:** As a developer, I want a fixed 60 Hz sim step decoupled from rendering, so that the physics is deterministic. *AC* is the same inputs and seed give the same results every time, and render interpolates between sim states.

## E11 - Tooling, Performance and Release

- [ ] **E11-6:** As a developer, I want the build verified on Windows, Linux and macOS in CI, so that E1-1's portability claim stays true. *AC* is a CI matrix builds the Debug, Release and Profile configs from a clean checkout on all three platforms. *(Deferred from E1-1 — the MSVC and GNU-ld branches are written but only the macOS branch has been compiled.)*