#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

namespace platform {

/**
 * @brief enum to define all options for keyboard and gamepad
 */
enum class ActionId : std::uint8_t {
  MoveX = 0u,   ///< X axis movement
  MoveY,        ///< Y axis movement
  Confirm,      ///< Confirm/OK
  Cancel,       ///< Cancel action
  Count         ///< Count helper for enum, must be last
};

/**
 * @brief data struct for button or axis (joystick) presses
 */
struct ActionState {
  bool held = false;
  bool pressed = false;
  bool released = false;
  float value = 0.0f;     // 0/1 for buttons, -1..1 for axes
};

/**
 * @brief for SDL Key <-> action, value
 */
struct KeyBinding {
  SDL_Scancode key;
  ActionId action;
  float value = 0.0f;
};

/**
 * @brief for Gamepad button <-> action, value
 */
struct PadButtonBinding {
  SDL_GamepadButton button;
  ActionId action;
  float value = 0.0f;
};

/**
 * @brief for Gamepad axis (joystick) <-> action, value
 */
struct PadAxisBinding {
  SDL_GamepadAxis axis;
  ActionId action;
  bool invert = false;
  float scale = 0.0f;
  float deadzone = 0.0f;
};

inline constexpr std::array<KeyBinding, 6> kKeyBindings{{
  {SDL_SCANCODE_D,      ActionId::MoveX,    +1.0f},
  {SDL_SCANCODE_A,      ActionId::MoveX,    -1.0f},
  {SDL_SCANCODE_W,      ActionId::MoveY,    +1.0f},
  {SDL_SCANCODE_S,      ActionId::MoveY,    -1.0f},
  {SDL_SCANCODE_RETURN, ActionId::Confirm,  +1.0f},
  {SDL_SCANCODE_ESCAPE, ActionId::Cancel,   +1.0f},
}};

inline constexpr std::array<PadButtonBinding, 2> kPadButtonBindings{{
  {SDL_GAMEPAD_BUTTON_SOUTH,  ActionId::Confirm,  +1.0f},
  {SDL_GAMEPAD_BUTTON_EAST,   ActionId::Cancel,   -1.0f}
}};

inline constexpr std::array<PadAxisBinding, 2> kPadAxisBindings{{
  {SDL_GAMEPAD_AXIS_LEFTX,  ActionId::MoveX,  false,  +1.0f, 8000.0f},
  {SDL_GAMEPAD_AXIS_LEFTY,  ActionId::MoveY,  false,  +1.0f, 8000.0f}
}};

inline constexpr std::size_t actionCount = static_cast<std::size_t>(ActionId::Count);
inline constexpr std::size_t kMaxGamepads = 2u;
inline constexpr std::size_t kKeyBindingsCount = kKeyBindings.size();
inline constexpr std::size_t kPadButtonBindingsCount = kPadButtonBindings.size();

/**
 * @brief Custom deleter for SDL_Gamepad smart pointer
 */
struct GamepadDeleter {
  void operator()(SDL_Gamepad* pad) const noexcept {
    if (pad) {
      SDL_CloseGamepad(pad);
    }
  }
};

class GamepadHandle {
public:

  /**
   * @brief Constructor
   */
  explicit GamepadHandle(SDL_JoystickID id);

  /**
   * @brief Destructor - Clean up gamepad instance
   */
  ~GamepadHandle() noexcept = default;

  /** @brief No copying */ 
  GamepadHandle(const GamepadHandle&) = delete;

  /** @brief No copy-assigning */
  GamepadHandle& operator=(const GamepadHandle &) = delete;

  /** @brief Move allowed */
  GamepadHandle(GamepadHandle&&) noexcept = default;

  /** @brief Move-assignment allowed */
  GamepadHandle& operator=(GamepadHandle&&) noexcept = default;

  /**
   * @brief Invalid if setup failed in constructor
   */
  [[nodiscard]] bool valid() const noexcept;

  /**
   * @brief Get the Joystick ID it was opened from for the manager to match with
   */
  [[nodiscard]] SDL_JoystickID getId() const noexcept;

  /**
   * @brief Get pointer to gamepad
   */
  [[nodiscard]] SDL_Gamepad* native_gamepad() const noexcept;

private:
  SDL_JoystickID id_;
  std::unique_ptr<SDL_Gamepad, GamepadDeleter> pad_;
};

class Input {
public:

  /**
   * @brief Open connection pads
   */
  bool init();

  /**
   * @brief Called per event from main
   */
  void pump(const SDL_Event& event);

  /**
   * @brief Compute pressed/released, latch prev
   */
  void newFrame();

  /**
   * @brief Check if button is held down
   */
  [[nodiscard]] bool held(ActionId a) const noexcept {
    return curr_[idx(a)].held;
  }

  /**
   * @brief Check if button is pressed down
   */
  [[nodiscard]] bool pressed(ActionId a) const noexcept {
    return curr_[idx(a)].pressed;
  }

  /**
   * @brief Check if button is released
   */
  [[nodiscard]] bool released(ActionId a) const noexcept {
    return curr_[idx(a)].released;
  }

  /**
   * @brief Determine value of the axis
   */
  [[nodiscard]] float axis(ActionId a) const noexcept {
    return curr_[idx(a)].value;
  }

private:
  static constexpr std::size_t idx(ActionId a) noexcept {
    return static_cast<std::size_t>(a);
  }

  std::array<std::optional<GamepadHandle>, kMaxGamepads> pads_{};
  std::array<ActionState, actionCount> curr_{};
  std::array<ActionState, actionCount> prev_{};

  std::array<bool, kKeyBindingsCount> keyDown_{};
  std::array<bool, kPadButtonBindingsCount> padDown_{};
  std::array<std::int16_t, SDL_GAMEPAD_AXIS_COUNT> axisRaw_{};
  int activeSlot_ = -1;

  /**
   * @brief Find slot the based on Joystick ID
   */
  int findSlot(SDL_JoystickID id) const noexcept;

  int freeSlot() const noexcept;

  void clearPadState() noexcept;
};

} // namespace platform