#include <iostream>
#include <SDL3/SDL.h>
#include "graphics.hpp"
#include "utils.hpp"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_Window *window = SDL_CreateWindow("nmapVisualizer",
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          0);

    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
    
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    Device deviceList[] = {
        Device(renderer),
    };

    deviceList[0].setDimensions(WINDOW_HEIGHT / 4.0f, WINDOW_HEIGHT / 4.0f);

    std::string result = win_run_nmap_xml("scanme.nmap.org");
    printf("%s\n", result.c_str());

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        for (auto &device : deviceList) {
            device.render(renderer);
        }

        SDL_RenderPresent(renderer);
    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
