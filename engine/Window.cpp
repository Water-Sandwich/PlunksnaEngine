//
// Created by d on 5/11/25.
//

#include "Window.h"

#include <iostream>
#include <vector>

#include "Exception.h"

#include <vulkan/vulkan.h>

namespace Plunksna {

constexpr void Window::deleteWindow(SDL_Window* window)
{
    if (window)
        SDL_DestroyWindow(window);
}

Window::Window(const std::string& title, const glm::uvec2& size, SDL_WindowFlags flags) :
    m_size(size), m_window(nullptr, deleteWindow)
{
    m_title = title;

    SDL_Window* window = SDL_CreateWindow(title.c_str(), size.x, size.y, flags);
    m_window = {window, deleteWindow};
    THROW_IF_NULL(m_window, "No window")

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


} //Plunksna