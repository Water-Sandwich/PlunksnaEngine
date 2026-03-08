//
// Created by d on 1/5/26.
//

#include "Renderer.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <cstring>
#include <vulkan/vk_enum_string_helper.h>
#include <set>

#include "Engine/Exception.h"
#include "Engine/Log.h"
#include "Engine/Window.h"

#include <fstream>
#include <chrono>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "PushConstant.h"
#include "RendererUtils.h"
#include "Engine/Random.h"

#define SIZE(type) alignedSize(sizeof(type), m_context.physicalDeviceProperties.limits.minUniformBufferOffsetAlignment)

using namespace Plunksna::RenderUtils;

namespace Plunksna {

Renderer::Renderer(AssetHandler& assetHandler) :
m_swapChain(m_context), m_assetHandler(assetHandler), m_camera({-2,0,0}, glm::normalize(glm::vec3(2,0,0)))
{
}

void Renderer::createInstance()
{
    ASSERT(checkValidationLayers(s_validationLayers),
        "Could not find validation layers")

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Watchamacallit";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Plunksna";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    auto extensions = getRequiredExtensions(s_enableValidationLayers);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (s_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<u32>(s_validationLayers.size());
        createInfo.ppEnabledLayerNames = s_validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    createInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    auto result = vkCreateInstance(&createInfo, nullptr, &m_context.instance);
    if (result != VK_SUCCESS) {
        THROW(string_VkResult(result))
    }
}

void Renderer::createAllocator()
{
    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorCreateInfo.physicalDevice = m_context.physicalDevice;
    allocatorCreateInfo.device = m_context.device;
    allocatorCreateInfo.instance = m_context.instance;

    vmaCreateAllocator(&allocatorCreateInfo, &m_context.allocator);
}

VkInstance Renderer::init(const Window& window)
{
    if (m_context.instance == VK_NULL_HANDLE)
        createInstance();

    if (s_enableValidationLayers)
        initDebugger();

    m_swapChain.createSurface(window);

    selectDevice(window);
    createLogicalDevice(window);
    createAllocator();
    m_swapChain.init(window, m_verticalSync);
    m_camera.resize((f32)m_swapChain.width() / (f32)m_swapChain.height());

    createRenderPass();
    initDescriptors();
    createGraphicsPipeline();
    createCommandPool();
    m_swapChain.initResources();

    std::vector<std::string> names{"sus1.png", "sus.png", "sus2.png"};
    createTextures(names);
    // createTextureImage();
    // createTextureImageView();
    createTextureSampler();

    loadModel();
    createVertexAndIndexBuffers();
    m_assetHandler.freeMeshHost(m_mesh);

    //frame resources
    createFrameResources();

    return m_context.instance;
}

void Renderer::draw(const Window& window)
{
    FrameResource& currentFrame = m_frameResources[m_currentFrame];
    vkWaitForFences(m_context.device, 1, &currentFrame.frameInFlightFence, VK_TRUE, UINT64_MAX);

    u32 imageIndex;
    ASSERT_V(m_swapChain.fetch(currentFrame, imageIndex),
        "Could not recreate swapchain");

    updateCameraBuffer(m_currentFrame);
    updateObjectsBuffer(m_currentFrame);

    vkResetFences(m_context.device, 1, &currentFrame.frameInFlightFence);
    vkResetCommandBuffer(currentFrame.commandBuffer, 0);
    recordCommandBuffer(currentFrame.commandBuffer, imageIndex);

    VkSemaphore waitSemaphores[] = {currentFrame.imageAvailableSem};
    VkSemaphore signalSemaphores[] = {m_swapChain.getRenderFinishedSemaphore(imageIndex)};

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentFrame.commandBuffer;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    ASSERT_V(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, currentFrame.frameInFlightFence),
        "failed to submit draw command buffer!")

    VkResult result = m_swapChain.present(m_presentQueue, imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_hasResized) {
        m_hasResized = false;
        m_swapChain.regenerate(window, m_verticalSync);
    }
    else if (result != VK_SUCCESS)
        THROW("Could not present swap chain image")

    m_currentFrame = ++m_currentFrame % m_maxInFlightFrames;
}

void Renderer::clean()
{
    m_swapChain.clean();
    m_swapChain.cleanSurface();

    for (auto& rec : m_frameResources)
        rec.destroyBuffers(m_context);

    VK_DESTROY(m_textureSampler, m_context.device, vkDestroySampler)
    //m_assetHandler.destroyTexture(m_context, m_textureAsset);
    for (auto tex : m_textures)
        m_assetHandler.destroyTexture(m_context, tex);

    VK_DESTROY(m_context.renderPass, m_context.device, vkDestroyRenderPass)

    VK_DESTROY(m_graphicsPipeline, m_context.device, vkDestroyPipeline)
    VK_DESTROY(m_pipelineLayout, m_context.device, vkDestroyPipelineLayout)

    VK_DESTROY(m_commandPool, m_context.device, vkDestroyCommandPool)
    VK_DESTROY(m_transientCommandPool, m_context.device, vkDestroyCommandPool)

    m_descriptors.clean(m_context);

    for (auto& rec : m_frameResources)
        rec.destroySync(m_context);

    m_assetHandler.destroyMesh(m_context, m_mesh);

    vmaDestroyAllocator(m_context.allocator);

    if (m_context.device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_context.device, nullptr);
        m_context.device = VK_NULL_HANDLE;
    }

    if (s_enableValidationLayers)
        VK_DESTROY(m_debugger, m_context.instance, DestroyDebugUtilsMessengerEXT)

    if (m_context.instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_context.instance, nullptr);
        m_context.instance = VK_NULL_HANDLE;
    }
}

Camera* Renderer::getCamera()
{
    return &m_camera;
}

void Renderer::resizeNotif()
{
    m_hasResized = true;
}

void Renderer::selectDevice(const Window& window)
{
    auto devices = getPhysicalDevices(m_context);

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            m_context.physicalDevice = device;
            m_context.msaaSamples = getMaxMSAA(device);

            vkGetPhysicalDeviceProperties(m_context.physicalDevice, &m_context.physicalDeviceProperties);
            break;
        }
    }

    ASSERT(m_context.physicalDevice != VK_NULL_HANDLE,
        "Failed to find suitable GPU")
}

void Renderer::initDescriptors()
{
    m_descriptor = m_descriptors.beginBuild();

    //camera
    m_descriptors.pushBinding(m_descriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    //per object data
    m_descriptors.pushBinding(m_descriptor, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    //texture
    m_descriptors.pushBinding(m_descriptor, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT,
        MAX_TEXTURES, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT);

    m_descriptors.submitBuild(m_context, m_descriptor, m_maxInFlightFrames, MAX_TEXTURES);
}

void Renderer::initDescriptorSets()
{
    for (i32 i = 0; i < m_maxInFlightFrames; i++) {
        VkDescriptorBufferInfo cameraUBOInfo{};
        cameraUBOInfo.buffer = m_frameResources[i].uniformBuffer.buffer;
        cameraUBOInfo.offset = 0;
        cameraUBOInfo.range = sizeof(CameraSO);

        m_descriptors.pushBufferInfo(m_descriptor, cameraUBOInfo);

        VkDescriptorBufferInfo SSBOInfo{};
        SSBOInfo.buffer = m_frameResources[i].storageBuffer.buffer;
        SSBOInfo.offset = 0;
        SSBOInfo.range = sizeof(PerObjectSO) * MAX_OBJECTS_SSBO;

        m_descriptors.pushBufferInfo(m_descriptor, SSBOInfo);

        std::vector<VkDescriptorImageInfo> images(m_textures.size());

        for (int j = 0; j < images.size(); j++) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_assetHandler.getTexture(m_textures[j])->fullView;
            imageInfo.sampler = m_textureSampler;

            images[j] = imageInfo;
        }

        m_descriptors.pushImageInfos(m_descriptor, images);

        m_frameResources[i].descriptorSet = m_descriptors.pushSetWrite(m_descriptor, i);
    }

    m_descriptors.createDescriptorSets(m_context, m_descriptor);
}

void Renderer::createGraphicsPipeline()
{
    Asset vertShaderHnd = m_assetHandler.loadShader(m_context, "vertShader.spv");
    Asset fragShaderHnd = m_assetHandler.loadShader(m_context, "fragShader.spv");

    ShaderModule* vertShader = m_assetHandler.getShader(vertShaderHnd);
    ShaderModule* fragShader = m_assetHandler.getShader(fragShaderHnd);

    m_assetHandler.freeShaderHost(vertShaderHnd);
    m_assetHandler.freeShaderHost(fragShaderHnd);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShader->shaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShader->shaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingsDesc = Vertex::getBindingDescription();
    auto attribDesc = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingsDesc; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<u32>(attribDesc.size());
    vertexInputInfo.pVertexAttributeDescriptions = attribDesc.data(); // Optional

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<f32>(m_swapChain.width());
    viewport.height = static_cast<f32>(m_swapChain.height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChain.extent();

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<u32>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;

    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = m_context.sampleShading;
    multisampling.rasterizationSamples = m_context.msaaSamples;
    multisampling.minSampleShading = 0.2f;
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                                        | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPushConstantRange pushConstant{};
    pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstant.offset = 0;
    pushConstant.size = sizeof(PushConstant);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstant;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = m_descriptors.getLayoutPtr(m_descriptor);

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional

    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    ASSERT_V(vkCreatePipelineLayout(m_context.device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout),
        "failed to create pipeline layout!")

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_context.renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    pipelineInfo.pDepthStencilState = &depthStencil;

    ASSERT_V(vkCreateGraphicsPipelines(m_context.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline),
        "failed to create graphics pipeline!")

    m_assetHandler.destroyShaderModule(m_context, vertShaderHnd);
    m_assetHandler.destroyShaderModule(m_context, fragShaderHnd);
}

void Renderer::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_swapChain.format();
    colorAttachment.samples = m_context.msaaSamples;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = m_swapChain.format();
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat(m_context);
    depthAttachment.samples = m_context.msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    subpass.pResolveAttachments = &colorAttachmentResolveRef;

    std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<u32>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    ASSERT_V(vkCreateRenderPass(m_context.device, &renderPassInfo, nullptr, &m_context.renderPass),
        "failed to create render pass!")
}

void Renderer::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_context.familyIndices.graphicsFamily.value();

    ASSERT_V(vkCreateCommandPool(m_context.device, &poolInfo, nullptr, &m_commandPool),
        "failed to create command pool!")

    VkCommandPoolCreateInfo transientCommandInfo{};
    transientCommandInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = m_context.familyIndices.graphicsFamily.value();

    ASSERT_V(vkCreateCommandPool(m_context.device, &poolInfo, nullptr, &m_transientCommandPool),
        "failed to create command pool!")
}

void Renderer::createCommandBuffers()
{
    std::vector<VkCommandBuffer> tempBuffer(m_maxInFlightFrames);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<u32>(tempBuffer.size());

    ASSERT_V(vkAllocateCommandBuffers(m_context.device, &allocInfo, tempBuffer.data()),
        "failed to allocate command buffers!")

    for (i32 i = 0; i < m_maxInFlightFrames; i++)
        m_frameResources[i].commandBuffer = tempBuffer[i];
}

void Renderer::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (i32 i = 0; i < m_maxInFlightFrames; i++) {
        ASSERT_V(vkCreateSemaphore(m_context.device, &semaphoreInfo, nullptr, &m_frameResources[i].imageAvailableSem),
            "Could not create semaphore")

        ASSERT_V(vkCreateFence(m_context.device, &fenceInfo, nullptr, &m_frameResources[i].frameInFlightFence),
            "Could not create fence")
    }
}

void Renderer::createTextures(std::vector<std::string> textures)
{
    m_textures.reserve(textures.size());

    for (const auto& name : textures) {
        Asset tex = createTextureImage(name);
        createTextureImageView(tex);
        m_textures.push_back(tex);
    }
}

Asset Renderer::createTextureImage(const std::string& file)
{
    Asset textureAsset = m_assetHandler.loadTexture(file);
    Texture* texture = m_assetHandler.getTexture(textureAsset);

    u32 texWidth = texture->width();
    u32 texHeight = texture->height();
    VkDeviceSize imageSize = texture->getSize() * 4;

    texture->mipLevels = getMipLevels(texWidth, texHeight);

    void* data;
    Buffer stagingBuffer = beginStagingBuffer(imageSize, &data);

    std::memcpy(data, texture->pixels, imageSize);

    vmaUnmapMemory(m_context.allocator, stagingBuffer.allocation);

    m_assetHandler.freeTextureHost(textureAsset);

    //image layout is undefined
    createImage(m_context, texture->image, texWidth, texHeight, texture->mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    //image layout to source
    transitionImageLayout(texture->image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texture->mipLevels);

    copyBufferToImage(stagingBuffer.buffer, texture->image.image, texWidth, texHeight);

    stagingBuffer.destroy(m_context);

    //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
    generateMipMaps(texture->image.image, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, texture->mipLevels);

    return textureAsset;
}

void Renderer::createTextureImageView(Asset textureAsset) const
{
    Texture* texture = m_assetHandler.getTexture(textureAsset);
    texture->fullView = createImageView(m_context, texture->image.image, VK_FORMAT_R8G8B8A8_SRGB, texture->mipLevels);
}

void Renderer::createTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = getMaxAnisotropy(m_context);
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

    ASSERT_V(vkCreateSampler(m_context.device, &samplerInfo, nullptr, &m_textureSampler),
        "failed to create texture sampler!");
}

void Renderer::loadModel()
{
    m_mesh = m_assetHandler.loadMesh("sus.obj");
}

void Renderer::createVertexAndIndexBuffers()
{
    Mesh* mesh = m_assetHandler.getMesh(m_mesh);
    VkDeviceSize bufferSize = mesh->verticesSize + mesh->indicesSize;

    void* data;
    Buffer stagingBuffer = beginStagingBuffer(bufferSize, &data);

    memcpy(data, mesh->vertices.data(), mesh->verticesSize);
    memcpy(static_cast<u8*>(data) + mesh->verticesSize, mesh->indices.data(), mesh->indicesSize);

    createBuffer(mesh->combinedBuffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT
        | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

    endAndCopyStagingBuffer(stagingBuffer, mesh->combinedBuffer, bufferSize);
}

void Renderer::createUniformBuffers()
{
    //TODO, suballocate UBOs
    VkDeviceSize bufferSize = SIZE(CameraSO);

    for (size_t i = 0; i < m_maxInFlightFrames; i++) {
        createBuffer(m_frameResources[i].uniformBuffer, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

        vmaMapMemory(m_context.allocator, m_frameResources[i].uniformBuffer.allocation, &m_frameResources[i].uniformBufferMapped);
    }
}

void Renderer::createSSBOs()
{
    //TODO, suballocate UBOs
    VkDeviceSize bufferSize = SIZE(PerObjectSO) * MAX_OBJECTS_SSBO;

    for (size_t i = 0; i < m_maxInFlightFrames; i++) {
        //TODO: Could be sequential if using memcpy
        createBuffer(m_frameResources[i].storageBuffer, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT);

        vmaMapMemory(m_context.allocator, m_frameResources[i].storageBuffer.allocation, &m_frameResources[i].storageBufferMapped);
    }
}

void Renderer::createModelUBOs()
{
    for (i32 i = 0; i < m_objectSpawnCount; i++) {

        f32 radius = 10.f;

        glm::vec3 pos = g_random.randomVector<3, f32>() * (radius * static_cast<f32>(std::cbrt(g_random.randomReal(0.0, 1.0))));


        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model, pos);

        model = glm::rotate(
            model,
            g_random.randomReal(-180.f, 180.f),
            g_random.randomVector<3, f32>()
        );

        PerObjectSO obj(model);
        obj.textureIndex = g_random.randomInt(0, m_textures.size() - 1);

        m_objects.push_back(obj);
    }
}

void Renderer::createFrameResources()
{
    m_frameResources.resize(m_maxInFlightFrames);
    m_objects.reserve(MAX_OBJECTS_SSBO);

    createUniformBuffers();
    createSSBOs();
    createCommandBuffers();
    initDescriptorSets();
    createSyncObjects();
    createModelUBOs();
}

void Renderer::updateCameraBuffer(u32 currentImage)
{
    m_camera.resize((f32)m_swapChain.width() / (f32)m_swapChain.height());

    CameraSO camUBO{
        .view = m_camera.getView(),
        .proj = m_camera.getPerspective()
    };

    auto* buffer = static_cast<std::byte*>(
        m_frameResources[currentImage].uniformBufferMapped
    );

    memcpy(buffer, &camUBO, sizeof(CameraSO));
    //memcpy(buffer + SIZE(CameraSO), m_modelUBOs.data(), sizeof(PerObjectSO) * m_modelUBOs.size());
}

//TODO: eeventually only update those that change
void Renderer::updateObjectsBuffer(u32 currentImage)
{
    auto* buffer = static_cast<std::byte*>(
        m_frameResources[currentImage].storageBufferMapped
    );

    memcpy(buffer, m_objects.data(), sizeof(PerObjectSO) * m_objects.size());
}


void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex) const
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    ASSERT_V(vkBeginCommandBuffer(commandBuffer, &beginInfo),
        "failed to begin recording command buffer! (begin)")

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_context.renderPass;
    renderPassInfo.framebuffer = m_swapChain.getFrameBuffer(imageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapChain.extent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = static_cast<u32>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

    Mesh* mesh = m_assetHandler.getMesh(m_mesh);

    VkBuffer vertexBuffers[] = {mesh->combinedBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    //indices need to be aligned to 4B if UINT32
    VkDeviceSize indexOffset = (mesh->verticesSize + 3) & ~3;

    vkCmdBindIndexBuffer(commandBuffer, mesh->combinedBuffer.buffer, indexOffset, VK_INDEX_TYPE_UINT32);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<f32>(m_swapChain.width());
    viewport.height = static_cast<f32>(m_swapChain.height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChain.extent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    for (u32 i = 0; i < m_objects.size(); i++) {
        PushConstant push{i};
        vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &push);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_pipelineLayout, 0, 1, &m_frameResources[m_currentFrame].descriptorSet, 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, (mesh->indicesCount), 1, 0, 0, 0);
    }

    // PushConstant push{0};
    // vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &push);
    // vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
    //                     m_pipelineLayout, 0, 1, &m_frameResources[m_currentFrame].descriptorSet, 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, (mesh->indicesCount), m_objects.size(), 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    ASSERT_V(vkEndCommandBuffer(commandBuffer),
        "failed to record command buffer! (end)")
}

void Renderer::createBuffer(Buffer& buffer, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
    VmaAllocationCreateFlags flags) const
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memoryUsage;
    allocInfo.flags = flags;

    ASSERT_V(
        vmaCreateBuffer(
            m_context.allocator,
            &bufferInfo,
            &allocInfo,
            &buffer.buffer,
            &buffer.allocation,
            nullptr
        ),
        "failed to create buffer!"
    );
}

VkCommandBuffer Renderer::beginSingleTimeCommands() const
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_transientCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_context.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer) const
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_context.device, m_transientCommandPool, 1, &commandBuffer);
}

void Renderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    endSingleTimeCommands(commandBuffer);
}

void Renderer::copyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height) const
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    endSingleTimeCommands(commandBuffer);
}

void Renderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                                     u32 mipLevels) const
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0; // TODO
    barrier.dstAccessMask = 0; // TODO

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else {
        THROW("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    endSingleTimeCommands(commandBuffer);
}

void Renderer::generateMipMaps(VkImage image, VkFormat imageFormat, i32 texWidth, i32 texHeight,
                               u32 mipLevels) const
{
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m_context.physicalDevice, imageFormat, &formatProperties);


    ASSERT(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT,
        "texture image format does not support linear blitting!")

    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    i32 mipWidth = texWidth;
    i32 mipHeight = texHeight;

    for (u32 i = 1; i < mipLevels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                             0, nullptr,
                             0, nullptr,
                             1, &barrier);

        if (mipWidth > 1) mipWidth /= 2;
        if (mipHeight > 1) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    endSingleTimeCommands(commandBuffer);
}

Buffer Renderer::beginStagingBuffer(VkDeviceSize bufferSize, void** data) const
{
    Buffer stagingBuffer;
    createBuffer(stagingBuffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    vmaMapMemory(m_context.allocator, stagingBuffer.allocation, data);

    return stagingBuffer;
}

void Renderer::endAndCopyStagingBuffer(Buffer& stagingBuffer, const Buffer& dst, VkDeviceSize bufferSize) const
{
    copyBuffer(stagingBuffer.buffer, dst.buffer, bufferSize);
    vmaUnmapMemory(m_context.allocator, stagingBuffer.allocation);
    stagingBuffer.destroy(m_context);
}

bool Renderer::isDeviceSuitable(VkPhysicalDevice device) const
{
    auto indices = m_swapChain.getQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device, s_deviceExtensions);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = m_swapChain.getSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

void Renderer::createLogicalDevice(const Window& window)
{
    m_context.familyIndices = m_swapChain.getQueueFamilies(m_context.physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<u32> uniqueQueueFamilies = {
        m_context.familyIndices.graphicsFamily.value(), m_context.familyIndices.presentFamily.value()
    };

    for (u32 queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &m_queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    deviceFeatures.sampleRateShading = m_context.sampleShading;

    VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
    indexingFeatures.runtimeDescriptorArray = VK_TRUE;
    indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
    indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.pNext = &indexingFeatures;

    createInfo.enabledExtensionCount = static_cast<u32>(s_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = s_deviceExtensions.data();

    if (s_enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<u32>(s_validationLayers.size());
        createInfo.ppEnabledLayerNames = s_validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    ASSERT_V(vkCreateDevice(m_context.physicalDevice, &createInfo, nullptr, &m_context.device),
        "Could not create logical device")

    vkGetDeviceQueue(m_context.device, m_context.familyIndices.graphicsFamily.value(), 0, &m_graphicsQueue);
    vkGetDeviceQueue(m_context.device, m_context.familyIndices.presentFamily.value(), 0, &m_presentQueue);
}

} // Plunksna
