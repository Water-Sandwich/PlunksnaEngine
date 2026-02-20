//
// Created by d on 1/5/26.
//

#ifndef VKRENDERER_H
#define VKRENDERER_H

#include <vk_mem_alloc.h>
#include <optional>
#include <vector>
#include <SDL3/SDL_stdinc.h>
#include <vulkan/vulkan_core.h>
#include <filesystem>

#include "engine/Window.h"

#include <bits/chrono.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Context.h"
#include "FrameResource.h"
#include "SwapChain.h"
#include "Vertex.h"
#include "Buffer.h"
#include "Camera.h"
#include "assethandler/AssetHandler.h"

namespace Plunksna {
#ifdef NDEBUG
static const bool s_enableValidationLayers = false;
#else
static const bool s_enableValidationLayers = true;
#endif

struct CameraUBO
{
    glm::mat4 view;
    glm::mat4 proj;
};

struct ModelUBO
{
    glm::mat4 model;

    //camera
    // glm::mat4 view;
    // glm::mat4 proj;
};

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

    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createRenderPass();

    void createCommandPool();
    void createDescriptorPools();

    //frame resources
    void createFrameResources();
    void createCommandBuffers();
    void createSyncObjects();
    void createUniformBuffers();
    void createDescriptorSets();

    void updateUniformBuffer(uint32_t currentImage);

    //asset
    //textures
    void createTextureImage();
    void createTextureImageView() const;
    void createTextureSampler();

    //asset
    //3d
    void loadModel();
    void createVertexBuffer();
    void createIndexBuffer();

    //init device
    void selectDevice(const Window& window);
    bool isDeviceSuitable(VkPhysicalDevice device) const;

    //buffers and commands
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) const;

    void createBuffer(Buffer& buffer, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
        VmaAllocationCreateFlags flags = {}) const;

    VkCommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t mipLevels) const;
    //asset
    void generateMipMaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) const;

    Buffer beginStagingBuffer(VkDeviceSize bufferSize, void** data) const;

    void endAndCopyStagingBuffer(Buffer& stagingBuffer, const Buffer& dst, VkDeviceSize bufferSize) const;

public:
    VkDebugUtilsMessengerEXT m_debugger = VK_NULL_HANDLE;

private:
    Context m_context;
    SwapChain m_swapChain;
    AssetHandler& m_assetHandler;

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandPool m_transientCommandPool = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    VkDescriptorPool m_descriptorPool;
    std::vector<FrameResource> m_frameResources;

    Camera m_camera;

    //assets

    //model
    Asset m_mesh;

    //texture
    Asset m_textureAsset = NULL_ASSET;
    VkSampler m_textureSampler;


    float m_queuePriority = 1.f;
    bool m_verticalSync = true;

    int m_maxInFlightFrames = 2;
    int m_currentFrame = 0;

    bool m_hasResized = false;
    const int MAX_OBJECTS = 1;
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
