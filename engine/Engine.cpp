//
// Created by d on 5/11/25.
//

#include "Engine.h"
#include "Log.h"
#include "Random.h"

#include <iostream>
#include <thread>

#include "Components.h"

namespace Plunksna {

void RenderSolidRect(SDL_Renderer* renderer, const Transform2& transform, const RColorRGBA& color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    glm::vec2 offset = transform.scale / 2.f;

    SDL_FRect r{
        transform.position.x - offset.x, transform.position.y - offset.y,
        transform.scale.x, transform.scale.y
    };

    SDL_RenderFillRect(renderer, &r);
    //SDL_RenderRect()
}

void Engine::tick(float delta_ms)
{
    auto* p = m_registry.get<Transform2>(0);
    p->scale.x = g_Random.randomReal(0.f,500.f);
}

void Engine::render()
{
    SDL_Renderer* renderer = m_window.getRenderer().get();

    SDL_SetRenderDrawColor(renderer, 0,10,10,255);
    SDL_RenderClear(renderer);

    m_renderFilter->foreach([&](const Transform2& a, const RColorRGBA& b)
    {
        RenderSolidRect(renderer, a, b);
    });

    SDL_RenderPresent(renderer);
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
}

void Engine::init()
{
    LOG("init");

    auto e = m_registry.makeEntity();
    m_renderFilter = m_registry.makeFilter<Transform2, RColorRGBA>();

    m_registry.add<Transform2>(e, glm::vec2(350,50), glm::vec2(10,10));
    m_registry.add<RColorRGBA>(e, 255,255,255,255);
}

void Engine::run()
{
    //m_isRunning = false;
    m_lastTime = std::chrono::system_clock::now();

    while (m_isRunning) {
        m_startTime = std::chrono::system_clock::now();

        handleEvents();
        tick(m_deltaTime_ms);
        render();

        m_lastTime = std::chrono::system_clock::now();
        m_deltaTime_ms = std::chrono::duration<float, std::milli>(m_lastTime - m_startTime).count();

        if (m_deltaTime_ms < m_maxFrameTime_ms) {
            const std::chrono::duration<float, std::milli> waitTime_ms(m_maxFrameTime_ms - m_deltaTime_ms);
            std::this_thread::sleep_for(waitTime_ms);
            //LOG(waitTime_ms);
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