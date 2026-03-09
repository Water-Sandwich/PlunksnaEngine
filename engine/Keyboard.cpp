//
// Created by d on 6/29/25.
//
#include "Keyboard.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>

#include "Log.h"

namespace Plunksna {

Keyboard::Keyboard() noexcept
    :   m_keyBuffer{std::bitset<512>(), std::bitset<512>()},
        m_modBuffer{0, 0}
{

}

void Keyboard::swap() noexcept
{
    m_stateIndex ^= 1; //toggle 1/0

    //old buffer into new buffer
    m_keyBuffer[m_stateIndex] = m_keyBuffer[getOtherIndex()];
    m_modBuffer[m_stateIndex] = SDL_GetModState();

    //SDL_PumpEvents();
}

void Keyboard::set(SDL_Scancode key, bool value)
{
    m_keyBuffer[m_stateIndex][key] = value;
}

void Keyboard::set(SDL_Keycode key, bool value)
{
    set(getScanCode(key), value);
}

bool Keyboard::get(SDL_Scancode key) const
{
    return getKeys()[key];
}

bool Keyboard::get(SDL_Keycode key) const
{
    return get(getScanCode(key));
}

bool Keyboard::getPrevious(SDL_Scancode key) const
{
    return m_keyBuffer[getOtherIndex()][key];
}

bool Keyboard::getPrevious(SDL_Keycode key) const
{
    return getPrevious(getScanCode(key));
}

bool Keyboard::getPressed(SDL_Scancode key) const
{
    const auto& prevKeys = m_keyBuffer[getOtherIndex()];
    return (!prevKeys[key] && getKeys()[key]);
}

bool Keyboard::getPressed(SDL_Keycode key) const
{
    return getPressed(getScanCode(key));
}

bool Keyboard::getReleased(SDL_Scancode key) const
{
    const auto& prevKeys = m_keyBuffer[getOtherIndex()];
    return (prevKeys[key] && !getKeys()[key]);
}

bool Keyboard::getReleased(SDL_Keycode key) const
{
    return getReleased(getScanCode(key));
}

unsigned char Keyboard::getOtherIndex() const
{
    return m_stateIndex ^ 1;
}

SDL_Scancode Keyboard::getScanCode(SDL_Keycode key) const
{
    //copied due to function being const and SDL_GetScancodeFromKey dont like it
    auto mod = m_modBuffer[m_stateIndex];
    return SDL_GetScancodeFromKey(key, &mod);
}

const std::bitset<512>& Keyboard::getKeys() const
{
    return m_keyBuffer[m_stateIndex];
}
}
