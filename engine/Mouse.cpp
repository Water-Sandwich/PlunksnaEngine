//
// Created by ws on 7/16/25.
//

#include "Mouse.h"

namespace Plunksna {
Mouse::Mouse() noexcept
    : m_deltaMouse(0,0), m_globalMouse(0,0), m_windowMouse(0,0), m_states(0,0), m_scroll(0) {

}

void Mouse::swap() noexcept {
    m_stateIndex = getOtherIndex();

    m_states[m_stateIndex] = SDL_GetGlobalMouseState(&m_globalMouse.x, &m_globalMouse.y);
    SDL_GetRelativeMouseState(&m_deltaMouse.x, &m_deltaMouse.y);
    SDL_GetMouseState(&m_windowMouse.x, &m_windowMouse.y);

    m_scrollFrame = 0;
}

bool Mouse::get(Uint8 button) const {
    return m_states[m_stateIndex] & SDL_BUTTON_MASK(button);
}

bool Mouse::getPrevious(Uint8 button) const {
    return m_states[getOtherIndex()] & SDL_BUTTON_MASK(button);
}

bool Mouse::getPressed(Uint8 button) const {
    return !getPrevious(button) && get(button);
}

bool Mouse::getReleased(Uint8 button) const {
    return getPrevious(button) && !get(button);
}

glm::vec2 Mouse::getMouseDelta() const {
    return m_deltaMouse;
}

glm::vec2 Mouse::getMouseWindow() const {
    return m_windowMouse;
}

glm::vec2 Mouse::getMouseGlobal() const {
    return m_globalMouse;
}

void Mouse::setScroll(float scroll) {
    m_scroll = scroll;
    m_scrollTotal += m_scroll;
    m_scrollFrame += m_scroll;
}

float Mouse::getScroll() const {
    return m_scroll;
}

float Mouse::getScrollFrame() const {
    return m_scrollFrame;
}

float Mouse::getScrollTotal() const {
    return m_scrollTotal;
}

unsigned char Mouse::getOtherIndex() const {
    return m_stateIndex ^ 1;
}

} // Plunksna