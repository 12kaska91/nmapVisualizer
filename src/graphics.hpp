#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <SDL3/SDL.h>

class Device {
    public:
        void render(SDL_Renderer* renderer) {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderRect(renderer, nullptr);
        }
    private:

};

#endif // GRAPHICS_HPP