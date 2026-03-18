//
// Created by d on 3/18/26.
//

#ifndef GUI_H
#define GUI_H
#include <imgui.h>

#include "vkRenderer/Context.h"
#include "engine/Window.h"

namespace Plunksna {

class GUI {
public:
    void init(const Context& context, const Window& window, u32 framesInFlight);
    void clean(const Context& context);

    void render();

    ImDrawData* getDrawData();

private:
    void draw();

private:
    VkDescriptorPool m_imguiPool = VK_NULL_HANDLE;
    ImDrawData* m_drawData = nullptr;
};

} // Plunksna

#endif //GUI_H
