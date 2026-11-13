#pragma once

#include <SDL3/SDL.h>

#include <memory>
#include <string_view>
#include <utility>

namespace platform {

class Context {

public:

  /**
   * Constructor
   * 
   * @param flags - Video flag on main thread
   */
  explicit Context();

  /**
   * Destructor - Safely quit SDL video
   */
  ~Context() noexcept;

  /**
   * init - Initialize and return false if errors
   */
  bool init(SDL_InitFlags flags);

  // Copy and move - Non-copyable and non-movable (represents global state)
  Context(const Context&) = delete;
  Context& operator=(const Context &) = delete;
  Context(Context&&) = delete;
  Context& operator=(Context&&) = delete;
};

struct WindowDeleter {
  void operator()(SDL_Window* window) const noexcept {
    if (window) {
      SDL_DestroyWindow(window);
    }
  }
};

class Window {
public:
  /**
   * Constructor
   * 
   * @param title - Title of the window
   * @param width - Width of window in pixels
   * @param height - Height of window in pixels
   * @param flags - SDL Window Flags (default to 0)
   */
  Window(std::string_view title, int width, int height, SDL_WindowFlags flags = 0) noexcept;

  // Destructor and move semantics
  ~Window() noexcept = default;
  Window(Window&&) noexcept = default;
  Window& operator=(Window&&) noexcept = default;

  Window(const Window&) noexcept = delete;
  Window& operator=(const Window&) noexcept = delete;

  /**
   * valid - Check if initialized correctly
   */
  [[nodiscard]] bool valid() noexcept;

  /**
   * native_handle - Return the window pointer
   */
  [[nodiscard]] SDL_Window* native_handle() const noexcept;

  /**
   * size - Getter for window size from height/width
   */
  [[nodiscard]] std::pair<int, int> size() const noexcept;

private:

  // Window instance
  std::unique_ptr<SDL_Window, WindowDeleter> window_{nullptr};
};
} // namespace platform
