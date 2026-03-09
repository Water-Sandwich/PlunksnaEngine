//
// Created by d on 6/29/25.
//

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <bitset>
#include <SDL3/SDL_keycode.h>

namespace Plunksna {

//dual buffered keyboard helper
class Keyboard {
public:
    Keyboard() noexcept;

    //swap key and mod buffers, updates mod state on call
    void swap() noexcept;

    //set key value
    void set(SDL_Scancode key, bool value);
    void set(SDL_Keycode key, bool value);

    //get current key value
    bool get(SDL_Scancode key) const;
    bool get(SDL_Keycode key) const;

    //get previous key value
    bool getPrevious(SDL_Scancode key) const;
    bool getPrevious(SDL_Keycode key) const;

    //get a key that was JUST pressed, last frame off, this frame on
    bool getPressed(SDL_Scancode key) const;
    bool getPressed(SDL_Keycode key) const;

    //get a key that was JUST released, last frame on, this frame off
    bool getReleased(SDL_Scancode key) const;
    bool getReleased(SDL_Keycode key) const;

private:
    unsigned char getOtherIndex() const;
    SDL_Scancode getScanCode(SDL_Keycode key) const;
    const std::bitset<512>& getKeys() const;

private:
    std::bitset<512> m_keyBuffer[2]; //internal keyboard state buffers

    SDL_Keymod m_modBuffer[2];

    unsigned char m_stateIndex = 0;
};

inline static Keyboard g_keyboard{};

} // Plunksna

#endif //KEYBOARD_H
