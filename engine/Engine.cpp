//
// Created by d on 5/11/25.
//

#include "Engine.h"
#include "Log.h"

#include <iostream>

namespace Plunksna {

void Engine::tick(float dt) {}

void Engine::handleEvents()
{
    while (SDL_PollEvent(&m_event)) {
        switch (m_event.type) {
        case SDL_EVENT_QUIT: {
            m_isRunning = false;
        }
        }
    }
}

Engine::Engine(const std::string& title, const glm::uvec2& size, SDL_WindowFlags flags) :
    m_window(title, size, flags)
{
    m_maxFPS = 60;
    m_maxFrameTime_ms = 1000.f / static_cast<float>(m_maxFPS);
}

struct Pos
{
    int x,y,z;
};

void Engine::init()
{
    LOG("init");

    auto e = m_registry.makeEntity();
    m_registry.add<Pos>(e, 5,5,5);
    m_registry.add<int>(e, 1);
    //m_registry.removeEntity(e);

    auto e1 = m_registry.makeEntity();
    m_registry.add<double>(e1, -1.5);
    m_registry.add<Pos>(e1, 1,1,1);
    m_registry.add<float>(e1, 1.2f);

    //m_registry.remove<Pos>(e1);
    m_registry.removeEntity(e);

    auto e2 = m_registry.makeEntity();
    m_registry.add<Pos>(e2, 0,0,0);

    LOG("Yipee!");
}

void Engine::run()
{
    //m_isRunning = false;
    m_lastTime = std::chrono::system_clock::now();

    while (m_isRunning) {
        m_startTime = std::chrono::system_clock::now();

        //do events
        handleEvents();
        //update
        SDL_Delay(17);
        //render

        m_lastTime = std::chrono::system_clock::now();
        unsigned int deltaTime_ms = std::chrono::duration_cast<std::chrono::milliseconds>(m_lastTime - m_startTime).
            count();

        if (deltaTime_ms < m_maxFrameTime_ms) {
            auto val = m_maxFrameTime_ms - deltaTime_ms;
            SDL_Delay(val);
        }
        // else {
        //     LOG_S(eLETHAL, "High frame time: " << deltaTime << "ms");
        // }
    }
}

void Engine::clean()
{
    LOG("clean")
}

Engine::~Engine()
{
    LOG("delete")
}

} //Plunksna