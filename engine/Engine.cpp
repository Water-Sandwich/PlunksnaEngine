//
// Created by d on 5/11/25.
//

#include "Engine.h"
#include "Log.h"

#include <iostream>

using namespace Plunksna;

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
{}

struct Pos
{
    int x,y,z;
};

void Engine::init()
{
    m_maxFPS = 60;
    m_maxFrameTime = 1000.f / static_cast<float>(m_maxFPS);

    LOG("PsnaEngine: init");


    m_registry.add<Pos>(0, 55, 55, 55);
    m_registry.add<Pos>(01, 5, 5, 5);
    m_registry.add<Pos>(02, 5, 5, 5);
    m_registry.add<Pos>(3000, 5, 5, 5);
    m_registry.add<Pos>(04, 5, 5, 5);
    m_registry.add<Pos>(05, 5, 5, 5);
    auto* comp = m_registry.get<Pos>(0);
    comp->x = 6;
    m_registry.remove<Pos>(0);


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
        unsigned int deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(m_lastTime - m_startTime).
            count();

        if (deltaTime < m_maxFrameTime) {
            auto val = m_maxFrameTime - deltaTime;
            SDL_Delay(val);
        }
        else {
            //LOG_S(Logs::eLETHAL, "High frame time: " << deltaTime << "ms");
        }
    }
}

void Engine::clean()
{
    LOG("PsnaEngine: clean")
}

Engine::~Engine()
{
    LOG("PsnaEngine: delete")
}
