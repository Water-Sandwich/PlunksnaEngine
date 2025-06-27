//
// Created by d on 5/11/25.
//

#include "Window.h"

#include <iostream>

#include "Exception.h"

namespace Plunksna {

constexpr void Window::deleteWindow(SDL_Window* window)
{
    if (window)
        SDL_DestroyWindow(window);
}

constexpr void Window::deleteRenderer(SDL_Renderer* renderer)
{
    if (renderer)
        SDL_DestroyRenderer(renderer);
}

Window::Window(const std::string& title, const glm::uvec2& size, SDL_WindowFlags flags) :
    m_size(size)
{
    SDL_Init(SDL_INIT_VIDEO);
    m_title = title;

    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_CreateWindowAndRenderer(title.c_str(), size.x, size.y, flags, &window, &renderer);

    m_window = std::shared_ptr<SDL_Window>(window, deleteWindow);
    m_renderer = std::shared_ptr<SDL_Renderer>(renderer, deleteRenderer);

    THROW_IF_NULL(m_window, "No window")
    THROW_IF_NULL(m_renderer, "No renderer")

    LOG("PsnaWindow made: " << title)
}

Window::~Window()
{
    LOG("deleting")
}

std::shared_ptr<SDL_Window> Window::getWindow()
{
    return m_window;
}

std::shared_ptr<SDL_Renderer> Window::getRenderer()
{
    return m_renderer;
}

} //Plunksna