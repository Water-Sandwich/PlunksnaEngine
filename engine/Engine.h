//
// Created by d on 5/11/25.
//

#ifndef GAME_H
#define GAME_H

#include <chrono>
#include <string>
#include <glm/vec2.hpp>
#include <SDL3/SDL.h>

#include "Window.h"
#include "../ecs/Registry.h"

namespace Plunksna {

class Engine
{
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

public:
    Engine(const std::string& title, const glm::uvec2& size, SDL_WindowFlags flags);
    Engine() = delete;
    Engine(const Engine& game) = delete;
    Engine(Engine&& game) = delete;

    ~Engine();

    void init();

    void run();

    void clean();

private:
    void tick(float delta_ms);
    void handleEvents();

private:
    Window m_window;
    SDL_Event m_event;

    TimePoint m_startTime;
    TimePoint m_lastTime;

    float m_maxFPS;
    float m_maxFrameTime_ms;
    float m_deltaTime_ms;

    Registry m_registry;
    struct Pos
    {
        int x,y,z;
    };
    Filter<Pos>* m_filter;

public:
    bool m_isRunning = true;
};

}

#endif //GAME_H
