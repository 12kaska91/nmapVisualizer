#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>

class Device {
    public:
        float x, y;
        std::string assetPath;
        SDL_Texture* texture;
        SDL_Renderer* renderer;

        Device(SDL_Renderer* renderer) {
            this->renderer = renderer;
            x = 0;
            y = 0;
            assetPath = "../assets/pc.svg";
            texture = loadTexture(assetPath, renderer);
        }

        ~Device() {
            if (texture) {
                SDL_DestroyTexture(texture);
            }
        }

        void setDimensions(float x, float y) {
            this->x = x;
            this->y = y;
        }

        void setAssetPath(const std::string& path, SDL_Renderer* renderer) {
            assetPath = path;
            texture = loadTexture(assetPath, renderer);
        }

        void render(SDL_Renderer* renderer) {
            SDL_FRect* rect = new SDL_FRect{ x, y, texture->h, texture->w };
            SDL_RenderTexture(renderer, texture, nullptr, rect);
            delete rect;
        }
    private:
        SDL_Texture* loadTexture(const std::string& filePath, SDL_Renderer* renderer) {
            SDL_Surface* surface = IMG_Load(filePath.c_str());
            if (!surface) {
                SDL_Log("Failed to load image: %s", SDL_GetError());
                return nullptr;
            }
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (!texture) {
                SDL_Log("Failed to create texture: %s", SDL_GetError());
            }
            SDL_DestroySurface(surface);
            return texture;
        }
};

#endif // GRAPHICS_HPP