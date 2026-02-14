//
// Created by d on 5/11/25.
//

#ifndef GAME_H
#define GAME_H

#include <chrono>
#include <string>
#include <glm/vec2.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_core.h>

#include "Components.h"
#include "vkrenderer/Renderer.h"
#include "Window.h"
#include "assethandler/AssetHandler.h"
#include "ecs/Registry.h"

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

private:
    void tick(float delta_ms);
    void render();
    void handleEvents();

    void moveCamera(float delta_ms);
    void rotateCamera();

private:
    Registry m_registry;

    Window m_window;
    Renderer m_renderer;
    AssetHandler m_assetHandler;

    TimePoint m_startTime;
    TimePoint m_lastTime;

    float m_maxFPS;
    float m_maxFrameTime_ms;
    float m_deltaTime_ms;

public:
    bool m_isRunning = true;
};

}

#endif //GAME_H
