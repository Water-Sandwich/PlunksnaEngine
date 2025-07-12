//
// Created by d on 5/11/25.
//

#include "Engine.h"
#include "Components.h"
#include "Keyboard.h"
#include "Log.h"
#include "Random.h"

#include <iostream>
#include <thread>

namespace Plunksna {

void renderSolidRect(SDL_Renderer* renderer, const Transform2& transform, const RColorRGBA& color)
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

void updatePlayer(float delta_ms, Transform2& transform)
{
    LOG(delta_ms)

    if (g_keyboard.get(SDL_SCANCODE_W))
        transform.position.y -= (1 * delta_ms);
    if (g_keyboard.get(SDL_SCANCODE_S))
        transform.position.y += (1 * delta_ms);
    if (g_keyboard.get(SDL_SCANCODE_A))
        transform.position.x -= (1 * delta_ms);
    if (g_keyboard.get(SDL_SCANCODE_D))
        transform.position.x += (1 * delta_ms);
}

void Engine::tick(float delta_ms)
{
    m_player->foreach([&](Transform2& transform2, Player& p)
    {
        updatePlayer(delta_ms, transform2);
    });
}

void Engine::render()
{
    SDL_Renderer* renderer = m_window.getRenderer().get();

    SDL_SetRenderDrawColor(renderer, 0,10,10,255);
    SDL_RenderClear(renderer);

    m_renderFilter->foreach([&](const Transform2& a, const RColorRGBA& b)
    {
        renderSolidRect(renderer, a, b);
    });

    SDL_RenderPresent(renderer);
}

void Engine::handleEvents()
{
    SDL_Event event;
    g_keyboard.swap();

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT: {
            m_isRunning = false;
            break;
        }
        case SDL_EVENT_KEY_DOWN:{
            if (event.key.repeat == 1)
                break;

            g_keyboard.set(event.key.scancode, true);
            //LOG("DOWN: " << SDL_GetKeyName(event.key.key))
            break;
        }
        case SDL_EVENT_KEY_UP:{
            if (event.key.repeat == 1)
                break;

            g_keyboard.set(event.key.scancode, false);
            //LOG("UP: "<< SDL_GetKeyName(event.key.key))
            break;
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

    m_renderFilter = m_registry.makeFilter<Transform2, RColorRGBA>();
    m_player = m_registry.makeFilter<Transform2, Player>();

    for (int i = 0 ; i < 1025; i++) {
        if (i == 512) {
            LOG("lausad");
        }
        auto e = m_registry.makeEntity();
        m_registry.add<Transform2>(e, glm::vec2(350,50), glm::vec2(10,10));
        m_registry.add<RColorRGBA>(e, 255,255,255,255);
        m_registry.add<Player>(e);
    }

}

void Engine::run()
{
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
            m_deltaTime_ms += waitTime_ms.count();
        }
        else {
            LOG_S(eWARNING, "High frame time: " << m_deltaTime_ms << "ms");
        }
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