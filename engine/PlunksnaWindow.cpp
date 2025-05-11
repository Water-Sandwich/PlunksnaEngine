//
// Created by d on 5/11/25.
//

#include "PlunksnaWindow.h"

#include <iostream>

#include "PlunksnaException.h"

constexpr void PlunksnaWindow::deleteWindow(SDL_Window* window)
{
    if (window)
        SDL_DestroyWindow(window);
}

constexpr void PlunksnaWindow::deleteRenderer(SDL_Renderer* renderer)
{
    if (renderer)
        SDL_DestroyRenderer(renderer);
}

PlunksnaWindow::PlunksnaWindow(const std::string& title, SDL_Point size, SDL_WindowFlags flags) :
    m_size(size)
{
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

PlunksnaWindow::~PlunksnaWindow()
{
    LOG("PsnaWindow: deleting")
}

std::shared_ptr<SDL_Window> PlunksnaWindow::getWindow()
{
    return m_window;
}

std::shared_ptr<SDL_Renderer> PlunksnaWindow::getRenderer()
{
    return m_renderer;
}
