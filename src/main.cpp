#include "platform/window.hpp"
#include "platform/input.hpp"

#include <SDL3/SDL.h>

int main() {
  // Destruction order is guaranteed reverse order of construction
  platform::Context ctx;
  bool ctxValid = ctx.init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
  if (!ctxValid) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init failed: %s", SDL_GetError());
    return 1;
  }

  platform::Input input;
  bool inputValid = input.init();
  if (!inputValid) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not set up SDL Inputs: %s", SDL_GetError());
    return 1;
  }

  platform::Window window{"GridironLite - New Game", 800, 600, SDL_WINDOW_RESIZABLE};
  if (!window.valid()) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateWindow failed: %s", SDL_GetError());
    return 1;
  }

  bool running = true;

  SDL_Event event{};
  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }

      input.pump(event);
    }

    input.newFrame();

    // Edges: these fire for exactly one frame, so they log cleanly.
    if (input.pressed(platform::ActionId::Confirm)) {
      SDL_Log("Confirm pressed");
    }
    if (input.released(platform::ActionId::Confirm)) {
      SDL_Log("Confirm released");
    }
    if (input.pressed(platform::ActionId::Cancel)) {
      SDL_Log("Cancel pressed");
    }

    // Axes: log only when the value changes, not every held frame.
    static float lastX = 0.0f;
    static float lastY = 0.0f;
    const float mx = input.axis(platform::ActionId::MoveX);
    const float my = input.axis(platform::ActionId::MoveY);
    if (mx != lastX || my != lastY) {
      SDL_Log("Move x=%.2f y=%.2f", mx, my);
      lastX = mx;
      lastY = my;
    }

    SDL_Delay(16);  // Basic frame pacing (60 fps)
  }

  return 0;
}