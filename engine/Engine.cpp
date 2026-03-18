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
#include <tracy/Tracy.hpp>

#include "assetHandler/Assets.h"
#include <imgui_impl_sdl3.h>

namespace Plunksna {

static f32 timer = 0;
void Engine::tick(f32 delta_ms)
{
    ZoneScopedN("Tick")
    timer += delta_ms;
    rotateCamera();
    moveCamera(delta_ms);

    m_renderMeshes->foreach([&](Model& model, Transform3D& transform)
    {
        transform = glm::rotate(transform, delta_ms * 0.005f, glm::vec3(0,1,0));
    });

    if (g_keyboard.getPressed(SDL_SCANCODE_V)) {
        m_renderer.setVSync(m_window, !m_renderer.getVSync());
        LOG("Vsync: " << m_renderer.getVSync())
    }
}

void Engine::render()
{
    ZoneScopedN("Render")
    m_renderMeshes->foreach([&](Model& model, Transform3D& transform)
    {
        m_renderer.pushDrawCommand(DrawMeshCommand(model.mesh, transform, m_assetHandler.getTextureId(model.texture)));
    });

    m_renderer.draw(m_window);
}

void Engine::handleEvents()
{
    ZoneScopedN("Event handling");
    SDL_Event event;
    g_keyboard.swap();
    g_mouse.swap();

    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
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
        speed = .1;

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

void Engine::loadAssets()
{
    using namespace Assets;
    meshPyramid = m_assetHandler.loadMesh("obama_prism.obj");
    meshSus = m_assetHandler.loadMesh("sus.obj");

    texPyramid = m_assetHandler.loadTexture("obama_prism.jpg");
    texSus1 = m_assetHandler.loadTexture("sus.png");
    texSus2 = m_assetHandler.loadTexture("sus1.png");
    texSus3 = m_assetHandler.loadTexture("sus2.png");
}

void Engine::addObjects()
{
    for (i32 i = 0; i < 1000; i++) {
        Entity e = m_registry.makeEntity();

        f32 radius = 25.f;
        glm::vec3 pos = g_random.randomVector<3, f32>() * (radius * static_cast<f32>(std::cbrt(g_random.randomReal(0.0, 1.0))));
        glm::mat4 tx = glm::mat4(1.0f);

        tx = glm::translate(tx, pos);

        tx = glm::rotate(
            tx,
            g_random.randomReal(-180.f, 180.f),
            g_random.randomVector<3, f32>()
        );

        m_registry.add<Transform3D>(e, tx);

        Model model;

        if (g_random.randomInt(0,1) == 0) {
            model.mesh = Assets::meshSus;
            model.texture = Assets::texSus1;
        }
        else {
            model.mesh = Assets::meshPyramid;
            model.texture = Assets::texPyramid;
        }

        m_registry.add<Model>(e, model);
    }
}

Engine::Engine(const std::string& title, const glm::uvec2& size, SDL_WindowFlags flags)
    : m_window(title, size, flags), m_renderer(m_assetHandler)
{
    m_renderer.init(m_window);
    loadAssets();
    m_renderer.uploadTextures(m_assetHandler.getLoadedTextures());
    m_renderer.uploadMeshes(m_assetHandler.getLoadedMeshes());
    m_renderer.initFrameResources();

    m_maxFPS = 144.f;
    m_maxFrameTime_ms = 1000.f / m_maxFPS;
    m_deltaTime_ms = m_maxFrameTime_ms;

    m_renderMeshes = m_registry.makeFilter<Model, Transform3D>();

    addObjects();

    LOG("Engine init")
}

void Engine::run()
{
    auto startTime = std::chrono::system_clock::now();
    auto endTime = startTime;

    while (m_isRunning) {
        ZoneScopedN("Main loop");
        startTime = std::chrono::system_clock::now();

        handleEvents();
        tick(m_deltaTime_ms);
        render();

        endTime = std::chrono::system_clock::now();
        m_deltaTime_ms = std::chrono::duration<f32, std::milli>(endTime - startTime).count();

        if (m_deltaTime_ms < m_maxFrameTime_ms) {
            ZoneScopedN("Main wait")
            std::chrono::duration<f32, std::milli> waitTime_ms(m_maxFrameTime_ms - m_deltaTime_ms);
            std::this_thread::sleep_for(waitTime_ms);
            m_deltaTime_ms += waitTime_ms.count();
        }
    }
}

Engine::~Engine()
{
    LOG("Destroying engine")
    m_renderer.clean();
}

} //Plunksna