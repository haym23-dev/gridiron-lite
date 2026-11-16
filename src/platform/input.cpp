#include "input.hpp"

#include <algorithm>
#include <cmath>

namespace platform {

GamepadHandle::GamepadHandle(SDL_JoystickID id) : id_{id} {
  SDL_Gamepad* raw_pad = SDL_OpenGamepad(id);
  pad_.reset(raw_pad);
}

bool GamepadHandle::valid() const noexcept {
  if (!pad_) {
    return false;
  }

  return true;
}

SDL_JoystickID GamepadHandle::getId() const noexcept {
  return id_;
}

SDL_Gamepad* GamepadHandle::native_gamepad() const noexcept {
  return pad_.get();
}

bool Input::init() {
  int count{0};
  SDL_JoystickID* ids = SDL_GetGamepads(&count);
  if (ids == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_INPUT, "SDL_GetGamepads failed: %s", SDL_GetError());
    return false;
  }

  int slot = 0;
  for (int i = 0; i < count && slot < static_cast<int>(kMaxGamepads); ++i) {
    GamepadHandle handle{ids[i]};
    if (!handle.valid()) {
      SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Failed to open gamepad id %u: %s", 
        static_cast<unsigned int>(ids[i]), SDL_GetError());
      continue;
    }
    
    pads_[slot++] = std::move(handle);
  }

  SDL_free(ids);
  activeSlot_ = slot > 0 ? 0 : -1;
  return true;
}

namespace {

// SDL Stick/trigger range top
constexpr float kAxisMax = 32767.0f;

float normalizeAxis(std::int16_t raw, float deadzone) {
  const float v = static_cast<float>(raw);
  const float mag = std::fabs(v);
  if (mag <= deadzone) {
    return 0.0f;
  }
  const float scaled = (mag - deadzone) / (kAxisMax - deadzone);
  return std::copysign(std::min(scaled, 1.0f), v);
}

} // namespace

int Input::findSlot(SDL_JoystickID id) const noexcept {
  for (std::size_t i = 0; i < pads_.size(); ++i) {
    if (pads_[i] && pads_[i]->getId() == id) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

int Input::freeSlot() const noexcept {
  for (std::size_t i = 0; i < pads_.size(); ++i) {
    if (!pads_[i]) {
      return static_cast<int>(i);
    }
  }
  return -1;
}

void Input::clearPadState() noexcept {
  padDown_.fill(false);
  axisRaw_.fill(0);
}

void Input::pump(const SDL_Event& event) {
  switch (event.type) {
    // ---Keyboard---
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP: {
      const bool down = (event.type == SDL_EVENT_KEY_DOWN);
      for (std::size_t i = 0; i < kKeyBindingsCount; ++i) {
        if (kKeyBindings[i].key == event.key.scancode) {
          keyDown_[i] = down;
        }
      }
      break;
    }

    // ---Gamepad plugged in---
    case SDL_EVENT_GAMEPAD_ADDED: {
      if (findSlot(event.gdevice.which) >= 0) {
        // Already open
        break;
      }
      const int slot = freeSlot();
      if (slot < 0) {
        // No capacity
        break;
      }

      GamepadHandle handle{event.gdevice.which};
      if (!handle.valid()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_INPUT, "Failed to open gamepad id %u: %s", 
                    static_cast<unsigned int>(event.gdevice.which), SDL_GetError());
        break;
      }
      pads_[slot] = std::move(handle);
      if (activeSlot_ < 0) {
        // First pad drives
        activeSlot_ = slot;
      }

      break;
    }

    // ---Gamepad unplugged---
    case SDL_EVENT_GAMEPAD_REMOVED: {
      const int slot = findSlot(event.gdevice.which);
      if (slot < 0) {
        break;
      }

      pads_[slot].reset();
      if (slot == activeSlot_) {
        // Don't leave stuck stick
        clearPadState();
        activeSlot_ = -1;
        for (std::size_t i = 0; i < kMaxGamepads; ++i) {
          if (pads_[i]) {
            activeSlot_ = static_cast<int>(i);
            break;
          }
        }
      }

      break;
    }

    // ---Gamepad button---
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
      if (activeSlot_ < 0) {
        // No active gamepad, ignore input
        break;
      }
      if (event.gbutton.which != pads_[activeSlot_]->getId()) {
        // Button press not from active gamepad
        break;
      }

      const bool down = (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
      for (std::size_t i = 0; i < kPadButtonBindings.size(); ++i) {
        if (kPadButtonBindings[i].button == event.gbutton.button) {
          padDown_[i] = down;
        }
      }

      break;
    }

    // ---Gamepad stick/trigger---
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
      if (activeSlot_ < 0) {
        break;
      }
      if (event.gaxis.which != pads_[activeSlot_]->getId()) {
        break;
      }

      if (event.gaxis.axis < SDL_GAMEPAD_AXIS_COUNT) {
        axisRaw_[event.gaxis.axis] = event.gaxis.value;
      }

      break;
    }

    // ---Window lost focus, clear everything---
    case SDL_EVENT_WINDOW_FOCUS_LOST: {
      keyDown_.fill(false);
      clearPadState();
      break;
    }

    default: {
      break;
    }
  }
}

void Input::newFrame() {
  // Per-frame accumulators
  std::array<float, actionCount> value{};
  std::array<bool, actionCount> engaged{};

  // Keyboard contributions: key press adds to signed value
  for (std::size_t i = 0; i < kKeyBindingsCount; ++i) {
    if (!keyDown_[i]) {
      continue;
    }

    const std::size_t a = idx(kKeyBindings[i].action);
    value[a] += kKeyBindings[i].value;
    engaged[a] = true;
  }

  // Gamepad button contributions (only active pad)
  for (std::size_t i = 0; i < kPadButtonBindingsCount; ++i) {
    if (!padDown_[i]) {
      continue;
    }

    const std::size_t a = idx(kPadButtonBindings[i].action);
    value[a] += kPadButtonBindings[i].value;
    engaged[a] = true;
  }

  // Gamepad axis contributions, dead-zoned and scaled
  if (activeSlot_ >= 0) {
    for (const auto& b : kPadAxisBindings) {
      float v = normalizeAxis(axisRaw_[b.axis], b.deadzone);
      if (b.invert) {
        v = -v;
      }

      v *= b.scale;
      if (v != 0.0f) {
        const std::size_t a = idx(b.action);
        value[a] += v;
        engaged[a] = true;
      }
    }
  }

  // Clamp each action to -1..1
  for (std::size_t a = 0; a < actionCount; ++a) {
    value[a] = std::clamp(value[a], -1.0f, 1.0f);
  }

  // Normalize the movement vector so diagonals aren't faster
  {
    const std::size_t x = idx(ActionId::MoveX);
    const std::size_t y = idx(ActionId::MoveY);
    const float mag = std::sqrt((value[x] * value[x]) + (value[y] * value[y]));
    if (mag > 1.0f) {
      value[x] /= mag;
      value[y] /= mag;
    }
  }

  // Build the snapshot and compute this frames edges
  for (std::size_t a = 0; a < actionCount; ++a) {
    curr_[a].value = value[a];
    curr_[a].held = engaged[a];
    curr_[a].pressed = curr_[a].held && !prev_[a].held;
    curr_[a].released = !curr_[a].held && prev_[a].held;
  }

  // Latch for next frame's edge detection
  prev_ = curr_;
}

} // namespace platform 