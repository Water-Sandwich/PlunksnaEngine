//
// Created by d on 5/11/25.
//


#include "Engine.h"
#include "Components.h"
#include "Keyboard.h"
#include "Log.h"
#include "Random.h"
#include "Mouse.h"
#include "vkRenderer/Renderer.h"

#include <iostream>
#include <thread>
#include <vulkan/vulkan.h>
#include <tracy/Tracy.hpp>

namespace Plunksna {

void Engine::tick(f32 delta_ms)
{
    rotateCamera();
    moveCamera(delta_ms);
}

void Engine::render()
{
    m_renderer.draw(m_window);
}

void Engine::handleEvents()
{
    ZoneScopedN("Event handling");
    SDL_Event event;
    g_keyboard.swap();
    g_mouse.swap();

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
        case SDL_EVENT_MOUSE_WHEEL: {
            g_mouse.setScroll(event.wheel.y);
            break;
        }
        case SDL_EVENT_WINDOW_RESIZED:{
            m_renderer.resizeNotif();
            break;
        }
        }
    }
}

void Engine::moveCamera(f32 delta_ms)
{
    Camera* camera = m_renderer.getCamera();

    f32 speed = 0.02;
    if (g_keyboard.get(SDL_SCANCODE_LALT))
        speed = 1;

    glm::vec3 inputDir(0);

    if (g_keyboard.get(SDL_SCANCODE_W))
        inputDir.x += 1;
    if (g_keyboard.get(SDL_SCANCODE_S))
        inputDir.x -= 1;
    if (g_keyboard.get(SDL_SCANCODE_A))
        inputDir.y += 1;
    if (g_keyboard.get(SDL_SCANCODE_D))
        inputDir.y -= 1;
    if (g_keyboard.get(SDL_SCANCODE_SPACE))
        inputDir.z += 1;
    if (g_keyboard.get(SDL_SCANCODE_LCTRL))
        inputDir.z -= 1;

    if (glm::length(inputDir) == 0)
        return;

    inputDir = glm::normalize(inputDir);

    glm::vec3 forward = camera->m_direction;
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,0,1)));
    glm::vec3 up = glm::normalize(glm::cross(right, forward));

    inputDir = (forward * inputDir.x) + (-right * inputDir.y) + (up * inputDir.z);

    camera->m_position += inputDir * delta_ms * speed;
}

void Engine::rotateCamera()
{
    if (!g_mouse.get(SDL_BUTTON_RIGHT)) {
        SDL_SetWindowRelativeMouseMode(m_window.getWindow(), false);
        return;
    }

    SDL_SetWindowRelativeMouseMode(m_window.getWindow(), true);

    Camera* camera = m_renderer.getCamera();
    f32 sensitivity = 0.005;

    glm::vec2 delta = g_mouse.getMouseDelta();

    glm::vec3 forward = camera->m_direction;
    glm::vec3 up(0,0,1);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));

    glm::quat yaw = glm::angleAxis(-delta.x * sensitivity, up);
    glm::quat pitch = glm::angleAxis(-delta.y * sensitivity, right);

    glm::quat finalRot = pitch * yaw;
    forward = finalRot * forward;

    camera->m_direction = forward;
}

Engine::Engine(const std::string& title, const glm::uvec2& size, SDL_WindowFlags flags)
    : m_window(title, size, flags), m_renderer(m_assetHandler)
{
    m_renderer.init(m_window);

    m_maxFPS = 144.f;
    m_maxFrameTime_ms = 1000.f / m_maxFPS;
    m_deltaTime_ms = m_maxFrameTime_ms;

    init();
}

void Engine::init()
{
    LOG("Engine init");
}

void Engine::run()
{
    while (m_isRunning) {
        ZoneScopedN("Main loop");
        m_startTime = std::chrono::system_clock::now();

        handleEvents();
        tick(m_deltaTime_ms);
        render();

        m_lastTime = std::chrono::system_clock::now();
        m_deltaTime_ms = std::chrono::duration<f32, std::milli>(m_lastTime - m_startTime).count();

        if (m_deltaTime_ms < m_maxFrameTime_ms) {
            std::chrono::duration<f32, std::milli> waitTime_ms(m_maxFrameTime_ms - m_deltaTime_ms);
            std::this_thread::sleep_for(waitTime_ms);
            //LOG("Frame time: " << m_deltaTime_ms);
            m_deltaTime_ms += waitTime_ms.count();
        }
        else {
            LOG_S(eWARNING, "High frame time: " << m_deltaTime_ms << "ms");
        }
    }
}

Engine::~Engine()
{
    LOG("Destroying engine")
    m_renderer.clean();
}

} //Plunksna