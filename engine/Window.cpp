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
    m_size(size), m_window(nullptr, deleteWindow), m_renderer(nullptr, deleteRenderer)
{
    SDL_Init(SDL_INIT_VIDEO);
    m_title = title;

    SDL_Window* window;
    SDL_Renderer* renderer;

    SDL_CreateWindowAndRenderer(title.c_str(), size.x, size.y, flags, &window, &renderer);

    m_window = {window, deleteWindow};
    m_renderer = {renderer, deleteRenderer};

    THROW_IF_NULL(m_window, "No window")
    THROW_IF_NULL(m_renderer, "No renderer")

    LOG("PsnaWindow made: " << title)
}

Window::~Window()
{
    LOG("deleting")
}

SDL_Window* Window::getWindow() const
{
    return m_window.get();
}

SDL_Renderer* Window::getRenderer() const
{
    return m_renderer.get();
}

} //Plunksna