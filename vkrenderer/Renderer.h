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

#include "engine/Window.h"


#include "Context.h"
#include "FrameResource.h"
#include "SwapChain.h"
#include "Vertex.h"
#include "Buffer.h"
#include "Camera.h"
#include "DescriptorManager.h"
#include "assethandler/AssetHandler.h"
#include "utils/Types.h"
#include "ShaderObjects.h"

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
    void draw(const Window& window);
    void clean();

    Camera* getCamera();

    void resizeNotif();

private:
    inline static const std::vector<const char*> s_validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    inline static const std::vector<const char*> s_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
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

    void createModelUBOs();
    void updateCameraBuffer(u32 currentImage);
    void updateObjectsBuffer(u32 currentImage);

    //asset
    //textures
    void createTextures(std::vector<std::string> textures);
    Asset createTextureImage(const std::string& file);
    void createTextureImageView(Asset textureAsse) const;
    void createTextureSampler();

    //asset
    //3d
    void loadModel();
    void createVertexAndIndexBuffers();

    //init device
    void selectDevice(const Window& window);
    bool isDeviceSuitable(VkPhysicalDevice device) const;

    //buffers and commands
    void recordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex) const;

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

public:
    VkDebugUtilsMessengerEXT m_debugger = VK_NULL_HANDLE;

private:
    Context m_context;
    SwapChain m_swapChain;
    AssetHandler& m_assetHandler;
    DescriptorManager m_descriptors;

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandPool m_transientCommandPool = VK_NULL_HANDLE;

    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    std::vector<FrameResource> m_frameResources;
    Descriptor m_descriptor;

    Camera m_camera;

    //model
    Asset m_mesh;

    //texture
    // Asset m_textureAsset = NULL_ASSET;
    std::vector<Asset> m_textures;
    VkSampler m_textureSampler;

    //instances
    const u32 MAX_OBJECTS_SSBO = 65536;
    const u32 MAX_TEXTURES = 64;
    u32 m_objectSpawnCount = 512;
    std::vector<PerObjectSO> m_objects;

    f32 m_queuePriority = 1.f;
    bool m_verticalSync = true;

    i32 m_maxInFlightFrames = 2;
    i32 m_currentFrame = 0;

    bool m_hasResized = false;
private:
    //=======DEBUG=========

    void initDebugger();
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
