//
// Created by d on 5/11/25.
//

#ifndef GAME_H
#define GAME_H

#include <chrono>
#include <string>
#include <glm/vec2.hpp>
#include <SDL3/SDL.h>

#include "Components.h"
#include "vkRenderer/Renderer.h"
#include "Window.h"
#include "assetHandler/AssetHandler.h"
#include "ecs/Registry.h"
#include <utils/Types.h>

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

    void run();

private:
    void tick(f32 delta_ms);
    void render();
    void handleEvents();

    void moveCamera(f32 delta_ms);
    void rotateCamera();

    void loadAssets();
    void addObjects();

private:
    Registry m_registry;

    Window m_window;
    Renderer m_renderer;
    AssetHandler m_assetHandler;

    f32 m_maxFPS;
    f32 m_maxFrameTime_ms;
    f32 m_deltaTime_ms;

    Filter<Model, Transform3D>* m_renderMeshes;
public:
    bool m_isRunning = true;
};

}

#endif //GAME_H
