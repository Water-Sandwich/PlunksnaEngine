//
// Created by d on 5/11/25.
//

#ifndef PLUNKSNAWINDOW_H
#define PLUNKSNAWINDOW_H

#include <SDL3/SDL.h>
#include <memory>
#include <glm/vec2.hpp>
#include <SDL3/SDL_vulkan.h>

namespace Plunksna {
class Window
{
public:
    Window(const std::string& title, const glm::uvec2& size, SDL_WindowFlags flags);
    ~Window();

    Window() = delete;
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    SDL_Window* getWindow() const;
    void createSurface(VkInstance instance);
    void destroySurface(VkInstance instance) const;

private:
    static constexpr void deleteWindow(SDL_Window* window);

private:
    glm::uvec2 m_size;
    std::string m_title;
    std::unique_ptr<SDL_Window, void(*)(SDL_Window*)> m_window;
    VkSurfaceKHR m_surface;

};
}

#endif //PLUNKSNAWINDOW_H
