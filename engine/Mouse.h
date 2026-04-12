//
// Created by ws on 7/16/25.
//

#ifndef MOUSE_H
#define MOUSE_H

#include <glm/vec2.hpp>
#include <SDL3/SDL_mouse.h>
#include <utils/Types.h>

namespace Plunksna {

class Mouse {
public:
    Mouse() noexcept;

    //swap mouse state buffers
    void swap() noexcept;

    //return true if mouse button is down
    bool get(u8 button) const;

    //returns if down last frame
    bool getPrevious(u8 button) const;

    //returns if button was JUST down
    bool getPressed(u8 button) const;

    //returns if button was JUST up
    bool getReleased(u8 button) const;

    //get change in mouse from last frame
    glm::vec2 getMouseDelta() const;

    //get position of mouse within the window
    glm::vec2 getMouseWindow() const;

    //get position of mouse within desktop
    glm::vec2 getMouseGlobal() const;

    void setScroll(f32 scroll);

    f32 getScroll() const;

    f32 getScrollFrame() const;

    f32 getScrollTotal() const;

private:
    unsigned char getOtherIndex() const;

private:
    glm::vec2 m_deltaMouse, m_globalMouse, m_windowMouse;
    SDL_MouseButtonFlags m_states[2];
    f32 m_scroll;
    f32 m_scrollTotal;
    f32 m_scrollFrame;
    unsigned char m_stateIndex = 0;
};

extern Mouse g_mouse;

} // Plunksna

#endif //MOUSE_H
