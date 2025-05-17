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
private:
    static constexpr void deleteWindow(SDL_Window* window);
    static constexpr void deleteRenderer(SDL_Renderer* renderer);

public:
    Window() = delete;
    Window(const std::string& title, const glm::uvec2& size, SDL_WindowFlags flags);
    ~Window();

    std::shared_ptr<SDL_Window> getWindow();
    std::shared_ptr<SDL_Renderer> getRenderer();

private:
    glm::uvec2 m_size;
    std::string m_title;
    std::shared_ptr<SDL_Window> m_window;
    std::shared_ptr<SDL_Renderer> m_renderer;
};
}

#endif //PLUNKSNAWINDOW_H
