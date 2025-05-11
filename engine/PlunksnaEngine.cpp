//
// Created by d on 5/11/25.
//

#include "PlunksnaEngine.h"
#include "PlunksnaLog.h"

#include <iostream>

void PlunksnaEngine::tick(float dt) {}

void PlunksnaEngine::handleEvents()
{
    while (SDL_PollEvent(&m_event)) {
        switch (m_event.type) {
        case SDL_EVENT_QUIT: {
            m_isRunning = false;
        }
        }
    }
}

PlunksnaEngine::PlunksnaEngine(const std::string& title, SDL_Point size, SDL_WindowFlags flags) :
    m_window(title, size, flags)
{}

void PlunksnaEngine::init()
{
    m_maxFPS = 60;
    m_maxFrameTime = 1000.f / static_cast<float>(m_maxFPS);

    LOG("PsnaEngine: init");
}

void PlunksnaEngine::run()
{
    //m_isRunning = false;
    m_lastTime = std::chrono::system_clock::now();

    while (m_isRunning) {
        m_startTime = std::chrono::system_clock::now();

        //work
        SDL_Delay(17);

        //do events
        handleEvents();
        //update
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

void PlunksnaEngine::clean()
{
    LOG("PsnaEngine: clean")
}

PlunksnaEngine::~PlunksnaEngine()
{
    LOG("PsnaEngine: delete")
}
