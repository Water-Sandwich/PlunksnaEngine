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

#include "Window.h"

#include <bits/chrono.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Context.h"
#include "FrameResource.h"
#include "SwapChain.h"
#include "Vertex.h"
#include "Buffer.h"

namespace Plunksna {
#ifdef NDEBUG
static const bool s_enableValidationLayers = false;
#else
static const bool s_enableValidationLayers = false;
#endif


struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};


//===========================================
//====================RENDERER===============
//===========================================

const std::filesystem::path g_workingPath = std::filesystem::current_path().parent_path();
const std::filesystem::path g_modelPath = g_workingPath / "models";
const std::filesystem::path g_texturePath = g_workingPath / "textures";

class Renderer
{
public:
    //=========BASE========
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;

    VkInstance init(const Window& window);
    void draw(const Window& window);
    void clean();

    void resizeNotif();

    //======LAYERS AND EXTENSIONS========

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

    //textures
    void createTextureImage();
    void createTextureImageView();
    void createTextureSampler();

    //3d
    void loadModel();
    void createVertexBuffer();
    void createIndexBuffer();

    //init device
    void selectDevice(const Window& window);
    bool isDeviceSuitable(VkPhysicalDevice device);

    //buffers and commands
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    //[[deprecated]]
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                      VkDeviceMemory& bufferMemory);

    void createBuffer(Buffer& buffer, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = {});

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t mipLevels);
    void generateMipMaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

public:
    VkDebugUtilsMessengerEXT m_debugger = VK_NULL_HANDLE;

private:
    Context m_context;
    SwapChain m_swapChain;

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandPool m_transientCommandPool = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    VkDescriptorPool m_descriptorPool;
    std::vector<FrameResource> m_frameResources;

    //texture
    VkImage m_textureImage;
    uint32_t m_mipLevels;
    VkDeviceMemory m_textureImageMemory;
    VkImageView m_textureImageView;
    VkSampler m_textureSampler;

    float m_queuePriority = 1.f;
    bool m_verticalSync = true;

    int m_maxInFlightFrames = 2;
    int m_currentFrame = 0;

    bool m_hasResized = false;

    //model
    //VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    //VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer m_indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

    Buffer m_vertexBuffer;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
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
