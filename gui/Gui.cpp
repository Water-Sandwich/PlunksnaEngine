//
// Created by d on 3/18/26.
//

#include "Gui.h"

#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>
#include <tracy/Tracy.hpp>

#include "vkRenderer/RendererUtils.h"
#include "engine/Exception.h"

namespace Plunksna {
void GUI::init(const Context& context, const Window& window, u32 framesInFlight)
{
    VkDescriptorPoolSize pool_sizes[] =
    {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    ASSERT_V(vkCreateDescriptorPool(context.device, &pool_info, nullptr, &m_imguiPool),
        "failed to init IMGui buffer");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    //createImGUIRenderPass();

    ImGui_ImplSDL3_InitForVulkan(window.getWindow());

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.ApiVersion = VK_API_VERSION_1_2;
    initInfo.Instance = context.instance;
    initInfo.PhysicalDevice = context.physicalDevice;
    initInfo.Device = context.device;
    initInfo.Queue = context.graphicsQueue;
    initInfo.DescriptorPool = m_imguiPool;
    initInfo.MinImageCount = framesInFlight;
    initInfo.ImageCount = framesInFlight;
    initInfo.QueueFamily = context.familyIndices.graphicsFamily.value();
    initInfo.PipelineInfoMain.RenderPass = context.renderPass;
    initInfo.PipelineInfoMain.MSAASamples = context.msaaSamples;

    ImGui_ImplVulkan_Init(&initInfo);
}

void GUI::clean(const Context& context)
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    VK_DESTROY(m_imguiPool, context.device, vkDestroyDescriptorPool)
}

void GUI::render()
{
    ZoneScopedN("Draw ImGui")
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    draw();

    ImGui::Render();
    m_drawData = ImGui::GetDrawData();
    ImGui::EndFrame();
}

ImDrawData* GUI::getDrawData()
{
    return m_drawData;
}

void GUI::draw()
{
    bool open = true;
    ImGui::ShowDemoWindow(&open);
}
} // Plunksna