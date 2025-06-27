//
// Created by d on 5/11/25.
//

#include "Engine.h"
#include "Log.h"
#include "Random.h"

#include <iostream>
#include <thread>

namespace Plunksna {

void Engine::tick(float delta_ms)
{
    // m_filter->foreach([](auto& a)
    // {
    //     ++a.x;
    // });

    //Entity num = rand() % m_registry.totalCount();
    Entity num = g_Random.randomInt(0, m_registry.totalCount());
    m_registry.removeEntity(num);
    LOG(num)
}

void Engine::handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT: {
            m_isRunning = false;
        }
        }
    }
}

Engine::Engine(const std::string& title, const glm::uvec2& size, SDL_WindowFlags flags) :
    m_window(title, size, flags)
{
    m_maxFPS = 60.f;
    m_maxFrameTime_ms = 1000.f / m_maxFPS;
    m_deltaTime_ms = m_maxFrameTime_ms;
    srand(time(0));
}

void Engine::init()
{
    LOG("init");

    auto def = [](Pos& a)
    {
        std::cout << a.x << "," << a.y << "," << a.z << std::endl;
    };

    m_filter = m_registry.makeFilter<Pos>(def);
    m_registry.makeFilter<int>(nullptr, -1);

    for (int i = 0; i < 262144; i++) {
        auto e = m_registry.makeEntity();
        m_registry.add<Pos>(e, i, i, i);
    }

    m_filter->foreachDefault();

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
        tick(m_deltaTime_ms);
        //render

        m_lastTime = std::chrono::system_clock::now();
        m_deltaTime_ms = std::chrono::duration<float, std::milli>(m_lastTime - m_startTime).count();

        if (m_deltaTime_ms < m_maxFrameTime_ms) {
            const std::chrono::duration<float, std::milli> waitTime_ms(m_maxFrameTime_ms - m_deltaTime_ms);
            std::this_thread::sleep_for(waitTime_ms);
            LOG(waitTime_ms);
        }
        else {
            LOG_S(eWARNING, "High frame time: " << m_deltaTime_ms << "ms");
        }
    }
}

void Engine::clean()
{
    LOG("clean")
    SDL_Quit();
}

Engine::~Engine()
{
    LOG("delete")
}

} //Plunksna