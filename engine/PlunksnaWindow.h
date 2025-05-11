//
// Created by d on 5/11/25.
//

#ifndef PLUNKSNAWINDOW_H
#define PLUNKSNAWINDOW_H

#include <SDL3/SDL.h>
#include <memory>

class PlunksnaWindow
{
private:
    SDL_Point m_size;
    std::string m_title;
    std::shared_ptr<SDL_Window> m_window;
    std::shared_ptr<SDL_Renderer> m_renderer;

private:
    static constexpr void deleteWindow(SDL_Window* window);
    static constexpr void deleteRenderer(SDL_Renderer* renderer);

public:
    PlunksnaWindow() = delete;
    PlunksnaWindow(const std::string& title, SDL_Point size, SDL_WindowFlags flags);
    ~PlunksnaWindow();

    std::shared_ptr<SDL_Window> getWindow();
    std::shared_ptr<SDL_Renderer> getRenderer();
};


#endif //PLUNKSNAWINDOW_H
