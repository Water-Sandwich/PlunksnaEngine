//
// Created by d on 5/11/25.
//

#ifndef GAME_H
#define GAME_H

#include <chrono>
#include <string>
#include <SDL3/SDL.h>

#include "Window.h"
namespace Plunksna
{
class Engine
{
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

private:
    void tick(float dt);
    void handleEvents();

public:
    Engine(const std::string& title, SDL_Point size, SDL_WindowFlags flags);

    Engine() = delete;
    Engine(const Engine& game) = delete;
    Engine(Engine&& game) = delete;

    void init();

    void run();

    void clean();

    ~Engine();

private:
    SDL_Event m_event;

    TimePoint m_startTime;
    TimePoint m_lastTime;
    unsigned int m_maxFPS;
    unsigned int m_maxFrameTime;

public:
    Window m_window;
    bool m_isRunning = true;
};
}

#endif //GAME_H
