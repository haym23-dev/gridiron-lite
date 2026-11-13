#include "platform/window.hpp"

#include <SDL3/SDL.h>

int main() {
  // Destruction order is guaranteed reverse order of construction
  // window destructed first, ctx destructed second
  platform::Context ctx;
  bool ctxValid = ctx.init(SDL_INIT_VIDEO);
  if (!ctxValid) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init failed: %s", SDL_GetError());
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
    }

    SDL_Delay(16);  // Basic frame pacing (60 fps)
  }

  return 0;
}