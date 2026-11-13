#include "window.hpp"

#include <SDL3/SDL.h>

#include <string>

namespace platform {

Context::Context() {}

Context::~Context() noexcept {
  SDL_Quit();
}

bool Context::init(SDL_InitFlags flags) {
  if (!SDL_Init(flags)) {
    return false;
  }
  return true;
}

Window::Window(std::string_view title, int width, int height, SDL_WindowFlags flags) noexcept {
  std::string title_str{title};
  SDL_Window* raw_window = SDL_CreateWindow(title_str.c_str(), width, height, flags);
  window_.reset(raw_window);
}

bool Window::valid() noexcept {
  if (!window_) {
    return false;
  }

  return true;
}

SDL_Window* Window::native_handle() const noexcept {
  return window_.get();
}

std::pair<int, int> Window::size() const noexcept {
  int w = 0;
  int h = 0;
  SDL_GetWindowSize(window_.get(), &w, &h);
  return {w, h};
}
} // namespace platform
