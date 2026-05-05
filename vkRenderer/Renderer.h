//
// Created by d on 1/5/26.
//

#ifndef VKRENDERER_H
#define VKRENDERER_H

#include <vk_mem_alloc.h>
#include <vector>
#include <SDL3/SDL_stdinc.h>
#include <vulkan/vulkan_core.h>
#include <filesystem>
#include <tracy/TracyVulkan.hpp>

#include "engine/Window.h"

#include "Context.h"
#include "FrameResource.h"
#include "SwapChain.h"
#include "Vertex.h"
#include "Buffer.h"
#include "Camera.h"
#include "DescriptorManager.h"
#include "DrawSorter.h"
#include "assetHandler/AssetHandler.h"
#include "utils/Types.h"
#include "ShaderObjects.h"
#include "gui/Gui.h"

namespace Plunksna {
#ifdef NDEBUG
static const bool s_enableValidationLayers = false;
#else
static const bool s_enableValidationLayers = true;
#endif

//===========================================
//====================RENDERER===============
//===========================================


class Renderer
{
public:
    Renderer() = delete;
    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    explicit Renderer(AssetHandler& assetHandler);
    ~Renderer() = default;

    VkInstance init(const Window& window);
    void uploadTextures(const std::vector<Asset>& textures);
    void uploadMeshes(const std::vector<Asset>& meshes);
    void initFrameResources();

    void pushDrawCommand(const DrawMeshCommand& command);
    void draw(const Window& window);

    void clean();

    Camera* getCamera();

    void resizeNotif();
    void setVSync(const Window& window, bool vsync);
    bool getVSync() const;

private:
    inline static const std::vector<const char*> s_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    inline static const std::vector<const char*> s_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME //profiling
    };

    void createInstance();
    void createLogicalDevice(const Window& window);
    void createAllocator();

    void createGraphicsPipeline();
    void createRenderPass();

    void createCommandPool();

    void initDescriptors();
    void initDescriptorSets();

    //frame resources
    void createFrameResources();
    void createCommandBuffers();
    void createSyncObjects();
    void createUniformBuffers();
    void createSSBOs();
    void createProfilers();

    void updateCameraBuffer(u32 currentImage);
    void updateObjectsBuffer(u32 currentImage);

    //textures
    Asset createTextureImage(Asset tex);
    void createTextureImageView(Asset textureAsset) const;
    void createTextureSampler();

    //asset
    void createVertexAndIndexBuffers(Asset meshHnd);

    //init device
    void selectDevice(const Window& window);
    bool isDeviceSuitable(VkPhysicalDevice device) const;

    //draw
    void recordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex);
    void beginRenderPass(VkCommandBuffer commandBuffer, u32 imageIndex);
    void setViewPort(VkCommandBuffer commandBuffer);
    void drawMeshes(VkCommandBuffer commandBuffer);
    void endRenderPass(VkCommandBuffer commandBuffer);
    void bindMesh(Mesh* mesh, VkCommandBuffer commandBuffer) const;


    void createBuffer(Buffer& buffer, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
        VmaAllocationCreateFlags flags = {}) const;

    VkCommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void copyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height) const;

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               u32 mipLevels) const;
    //asset
    void generateMipMaps(VkImage image, VkFormat imageFormat, i32 texWidth, i32 texHeight, u32 mipLevels) const;

    Buffer beginStagingBuffer(VkDeviceSize bufferSize, void** data) const;

    void endAndCopyStagingBuffer(Buffer& stagingBuffer, const Buffer& dst, VkDeviceSize bufferSize) const;

private:
    Context m_context;
    SwapChain m_swapChain;
    AssetHandler& m_assetHandler;
    DrawSorter m_drawSorter;
    DescriptorManager m_descriptors;
    GUI m_gui;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandPool m_transientCommandPool = VK_NULL_HANDLE;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    std::vector<FrameResource> m_frameResources;
    Descriptor m_descriptor;

    Camera m_camera;

    VkSampler m_textureSampler;

    //instances
    const u32 MAX_OBJECTS_SSBO = 1000000;
    const u32 MAX_TEXTURES = 64;

    f32 m_queuePriority = 1.f;

    i32 m_maxInFlightFrames = 2;
    i32 m_currentFrame = 0;

    bool m_hasResized = false;
private:
    DescriptorBuf m_camBuf = 0;
    DescriptorBuf m_objBuf = 0;
    DescriptorBuf m_texBuf = 0;

private:
    //=======DEBUG=========
    VkDebugUtilsMessengerEXT m_debugger = VK_NULL_HANDLE;
    TracyVkCtx m_profiler = VK_NULL_HANDLE;

    void initDebugger();
    TracyVkCtx initProfiler(const Context& contex, VkQueue queue, VkCommandBuffer cmdBuf);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};
} // Plunksna

#endif //VKRENDERER_H
