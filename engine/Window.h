//
// Created by d on 5/11/25.
//

#ifndef PLUNKSNAWINDOW_H
#define PLUNKSNAWINDOW_H

#include <SDL3/SDL.h>
#include <memory>
#include <glm/vec2.hpp>

namespace Plunksna {
class Window
{
public:
    Window() = delete;
    Window(const std::string& title, const glm::uvec2& size, SDL_WindowFlags flags);
    ~Window();

    SDL_Window* getWindow() const;
    SDL_Renderer* getRenderer() const;

private:
    static constexpr void deleteWindow(SDL_Window* window);
    static constexpr void deleteRenderer(SDL_Renderer* renderer);

private:
    glm::uvec2 m_size;
    std::string m_title;
    std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> m_window;
    std::unique_ptr<SDL_Renderer, void(*)(SDL_Renderer*)> m_renderer;
};
}

#endif //PLUNKSNAWINDOW_H
