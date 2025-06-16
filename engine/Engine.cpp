//
// Created by d on 5/11/25.
//

#include "Engine.h"
#include "Log.h"

#include <iostream>
#include <thread>

namespace Plunksna {

void Engine::tick(float delta_ms)
{
    SDL_Delay(17);
}

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
    m_maxFPS = 60.f;
    m_maxFrameTime_ms = 1000.f / m_maxFPS;
    m_deltaTime_ms = m_maxFrameTime_ms;
}

struct Pos
{
    int x,y,z;
};

void Engine::init()
{
    LOG("init");

    Filter<int, float> filter([](int& a, float& c)
    {
        std::cout << a << c << std::endl;
    });

    int a = 5;
    int b = 6;
    float c = 10;
    float d = 132.5f;

    //filter.add(0, &a, &c);
    filter.add(1, &a, &c);
    //filter.remove(0);

    filter.foreachDefault();
    filter.updateComponentAddress<int>(1, &b);
    filter.updateComponentAddress<float>(1, &d);
    //filter.updateComponentAddressFast<Pos>(1, nullptr);
    filter.foreachDefault();
    filter.remove(1);

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
    //Group<float, int, pos> thing = m_registry.makeGroup<float, int, pos>();

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
        }
        // else {
        //     LOG_S(eLETHAL, "High frame time: " << m_deltaTime_ms << "ms");
        // }
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