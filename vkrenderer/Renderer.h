//
// Created by d on 1/5/26.
//

#ifndef VKRENDERER_H
#define VKRENDERER_H
#include <optional>
#include <vector>
#include <SDL3/SDL_stdinc.h>
#include <vulkan/vulkan_core.h>
#include <filesystem>

#include "Window.h"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <bits/chrono.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Plunksna {
#ifdef NDEBUG
static const bool s_enableValidationLayers = false;
#else
static const bool s_enableValidationLayers = true;
#endif

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const
    {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

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
    void createSwapChain(const Window& window);
    void createSurface(const Window& window);
    void createImageViews();

    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createRenderPass();
    void createFrameBuffers();

    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    void createDepthBuffers();

    void createTextureImage();
    void createTextureImageView();
    void createTextureSampler();

    void loadModel();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPools();
    void createDescriptorSets();

    void cleanSwapChain();
    void recreateSwapChain(const Window& window);

    void updateUniformBuffer(uint32_t currentImage);

    std::vector<VkLayerProperties> getLayers();
    std::vector<VkExtensionProperties> getExtensions();
    std::vector<const char*> getRequiredExtensions();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool checkValidationLayers();

    void selectDevice(const Window& window);
    std::vector<VkPhysicalDevice> getPhysicalDevices();
    bool isDeviceSuitable(VkPhysicalDevice device, const Window& window);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, const Window& window);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, const Window& window);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window);

    static std::vector<char> readFile(const std::string& filename);
    VkShaderModule createShaderModule(const std::vector<char>& code);

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                      VkDeviceMemory& bufferMemory);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t mipLevels);
    VkImageView createImageView(VkImage image, VkFormat format, uint32_t mipLevels,
                                VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT);
    void generateMipMaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

    float getMaxAnisotropy();

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    bool hasStencil(VkFormat format);

public:
    VkDebugUtilsMessengerEXT m_debugger = VK_NULL_HANDLE;
    VkInstance m_instance = VK_NULL_HANDLE;

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    QueueFamilyIndices m_familyIndices;

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    VkCommandPool m_transientCommandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;

    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;

    std::vector<VkSemaphore> m_vsImageAvailable; //TODO: semaphores should belong to image, not frame
    std::vector<VkSemaphore> m_vsRenderFinished;
    std::vector<VkFence> m_vfInFlight;

    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer m_indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;

    VkImage m_textureImage;
    uint32_t m_mipLevels;
    VkDeviceMemory m_textureImageMemory;
    VkImageView m_textureImageView;
    VkSampler m_textureSampler;

    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;

    VkImage m_depthImage = VK_NULL_HANDLE;
    VkDeviceMemory m_depthImageMemory = VK_NULL_HANDLE;
    VkImageView m_depthImageView = VK_NULL_HANDLE;

    float m_queuePriority = 1.f;
    bool m_forceVSync = true;

    int m_maxInFlightFrames = 2;
    int m_currentFrame = 0;

    bool m_hasResized = false;

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
