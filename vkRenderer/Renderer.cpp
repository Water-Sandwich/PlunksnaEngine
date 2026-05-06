//
// Created by d on 1/5/26.
//

#include "Renderer.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <cstring>
#include <vulkan/vk_enum_string_helper.h>
#include <set>

#include "engine/Exception.h"
#include "engine/Log.h"
#include "engine/Window.h"

#include <fstream>
#include <chrono>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui_impl_vulkan.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "PushConstant.h"
#include "RendererUtils.h"
#include "assetHandler/Assets.h"
#include "engine/Random.h"
#include "tracy/Tracy.hpp"

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
    appInfo.pApplicationName = "Test";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
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
    m_swapChain.init(window);
    m_camera.resize((f32)m_swapChain.width() / (f32)m_swapChain.height());

    createRenderPass();
    initDescriptors();
    createGraphicsPipeline();
    createCommandPool();
    m_swapChain.initResources();
    m_drawSorter.setAssets(&m_assetHandler);

    m_gui.init(m_context, window, m_maxInFlightFrames);

    return m_context.instance;
}

void Renderer::uploadTextures(const std::vector<Asset>& textures)
{
    for (i32 i = 0; i < textures.size(); i++) {
        createTextureImage(m_context, m_assetHandler.getTexture(textures[i]));
        m_assetHandler.freeTextureHost(textures[i]);
        m_assetHandler.setTextureID(textures[i], i);
    }

    createTextureSampler();
}

void Renderer::uploadMeshes(const std::vector<Asset>& meshes)
{
    for (auto mesh : meshes) {
        createVertexAndIndexBuffers(mesh);
    }
}

void Renderer::initFrameResources()
{
    createFrameResources();
}

void Renderer::pushDrawCommand(const DrawMeshCommand& command)
{
    m_drawSorter.drawMesh(command);
}

void Renderer::draw(const Window& window)
{
    ZoneScopedN("Renderer::draw")
    m_gui.render();

    FrameResource& currentFrame = m_frameResources[m_currentFrame];

    vkWaitForFences(m_context.device, 1, &currentFrame.frameInFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(m_context.device, 1, &currentFrame.frameInFlightFence);

    u32 imageIndex;
    ASSERT_V(m_swapChain.fetch(currentFrame, imageIndex),
        "Could not fetch from swapchain");

    vkResetCommandBuffer(currentFrame.commandBuffer, 0);

    updateCameraBuffer();
    updateObjectsBuffer();

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

    ASSERT_V(vkQueueSubmit(m_context.graphicsQueue, 1, &submitInfo, currentFrame.frameInFlightFence),
        "failed to submit draw command buffer!")

    VkResult result = m_swapChain.present(m_context.presentQueue, imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_hasResized) {
        m_hasResized = false;
        m_swapChain.regenerate(window, m_swapChain.getVSync());
    }
    else if (result != VK_SUCCESS)
        THROW("Could not present swap chain image")

    m_currentFrame = ++m_currentFrame % m_maxInFlightFrames;
    m_drawSorter.clearAll();
}

void Renderer::clean()
{
    vkDeviceWaitIdle(m_context.device);

    m_gui.clean(m_context);

    m_swapChain.clean();
    m_swapChain.cleanSurface();

    VK_DESTROY(m_textureSampler, m_context.device, vkDestroySampler)

    m_assetHandler.clean(m_context);

    VK_DESTROY(m_context.renderPass, m_context.device, vkDestroyRenderPass)

    VK_DESTROY(m_graphicsPipeline, m_context.device, vkDestroyPipeline)
    VK_DESTROY(m_pipelineLayout, m_context.device, vkDestroyPipelineLayout)

    VK_DESTROY(m_commandPool, m_context.device, vkDestroyCommandPool)
    VK_DESTROY(m_context.m_transientCommandPool, m_context.device, vkDestroyCommandPool)

    m_descriptors.clean(m_context);

    for (auto& rec : m_frameResources)
        rec.destroySync(m_context);

    vmaDestroyAllocator(m_context.allocator);
    TracyVkDestroy(m_profiler)

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

void Renderer::setVSync(const Window& window, bool vsync)
{
    if (vsync == getVSync())
        return;

    m_swapChain.regenerate(window, vsync);
}

bool Renderer::getVSync() const
{
    return m_swapChain.getVSync();
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
    m_descriptor = m_descriptors.beginBuild(m_maxInFlightFrames);

    //RETURN BUFFER HANDLES PER BINDING
    //camera
    m_camBuf = m_descriptors.pushBinding(m_descriptor, eEXCLUSIVE, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0);
    //per object data
    m_objBuf = m_descriptors.pushBinding(m_descriptor, eEXCLUSIVE, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    //texture
    m_texBuf = m_descriptors.pushBinding(m_descriptor, eSHARED, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_SHADER_STAGE_FRAGMENT_BIT, 10,
        MAX_TEXTURES, VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT);

    //ALLOCATE BUFFERS HERE
    m_descriptors.submitBuild(m_context, m_descriptor);

    m_descriptors.allocateDescriptorBuffers(m_context, m_descriptor, m_camBuf, SIZE(CameraSO));
    m_descriptors.allocateDescriptorBuffers(m_context, m_descriptor, m_objBuf, SIZE(PerObjectSO) * MAX_OBJECTS_SSBO);

    m_descriptors.updateWriteQueue(m_context, m_descriptor);
}

void Renderer::initDescriptorSets()
{
    for (int i = 0; i < m_maxInFlightFrames; i++) {
        m_frameResources[i].descriptorSet = m_descriptors.getSet(m_descriptor, i);
    }
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
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = m_context.familyIndices.graphicsFamily.value();

    ASSERT_V(vkCreateCommandPool(m_context.device, &poolInfo, nullptr, &m_context.m_transientCommandPool),
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

void Renderer::createVertexAndIndexBuffers(Asset meshHnd)
{
    Mesh* mesh = m_assetHandler.getMesh(meshHnd);
    VkDeviceSize bufferSize = mesh->verticesSize + mesh->indicesSize;

    void* data;
    Buffer stagingBuffer = beginStagingBuffer(m_context, bufferSize, &data);

    memcpy(data, mesh->vertices.data(), mesh->verticesSize);
    memcpy(static_cast<u8*>(data) + mesh->verticesSize, mesh->indices.data(), mesh->indicesSize);

    createBuffer(m_context, mesh->combinedBuffer, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT
        | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
         VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

    copyBuffer(m_context, stagingBuffer.buffer, mesh->combinedBuffer.buffer, bufferSize);
    endStagingBuffer(m_context, stagingBuffer);

    m_assetHandler.freeMeshHost(meshHnd);
}

void Renderer::createProfilers()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_context.m_transientCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(m_context.device, &allocInfo, &cmd);

    m_profiler = initProfiler(m_context, m_context.graphicsQueue, cmd);

    vkFreeCommandBuffers(m_context.device, m_context.m_transientCommandPool, 1, &cmd);
}

void Renderer::createFrameResources()
{
    m_frameResources.resize(m_maxInFlightFrames);

    createCommandBuffers();
    initDescriptorSets();
    createSyncObjects();
    createProfilers();
}

void Renderer::updateCameraBuffer()
{
    m_camera.resize((f32)m_swapChain.width() / (f32)m_swapChain.height());

    CameraSO camUBO{
        .view = m_camera.getView(),
        .proj = m_camera.getPerspective(),
        .pos = m_camera.m_position
    };

    auto* buffer = static_cast<u8*>(
        m_descriptors.getBufferWrite(m_descriptor, m_camBuf, m_currentFrame)
    );

    memcpy(buffer, &camUBO, sizeof(CameraSO));
}

//TODO: eventually only update those that change
void Renderer::updateObjectsBuffer()
{
    ZoneScopedN("Update SSBO")

    auto* buffer = static_cast<u8*>(
        m_descriptors.getBufferWrite(m_descriptor, m_objBuf, m_currentFrame)
    );

    //m_drawSorter.cullFrustum(&m_camera);
    const auto& objs = m_drawSorter.getFinalObjects(&m_camera);

    ASSERT(objs.size() < MAX_OBJECTS_SSBO, "Max rendered object count reached!")

    memcpy(buffer, objs.data(), sizeof(PerObjectSO) * objs.size());
}


void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, u32 imageIndex)
{
    ZoneScopedN("Record command buffer");
    FrameResource& frame = m_frameResources[m_currentFrame];
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    ASSERT_V(vkBeginCommandBuffer(commandBuffer, &beginInfo),
        "failed to begin recording command buffer! (begin)")

    {
        TracyVkZone(m_profiler, commandBuffer, "Record command buffer")

        beginRenderPass(commandBuffer, imageIndex);

        setViewPort(commandBuffer);

        drawMeshes(commandBuffer);

        ImGui_ImplVulkan_RenderDrawData(m_gui.getDrawData(), commandBuffer);

        endRenderPass(commandBuffer);
    }

    TracyVkCollect(m_profiler, commandBuffer);

    ASSERT_V(vkEndCommandBuffer(commandBuffer),
        "failed to record command buffer! (end)")
}

void Renderer::beginRenderPass(VkCommandBuffer commandBuffer, u32 imageIndex)
{
    TracyVkZone(m_profiler, commandBuffer, "Begin render pass");

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
}

void Renderer::setViewPort(VkCommandBuffer commandBuffer)
{
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
}

void Renderer::drawMeshes(VkCommandBuffer commandBuffer)
{
    auto& drawCommands = m_drawSorter.getDrawCommands();
    u32 offset = 0;

    TracyVkZone(m_profiler, commandBuffer, "Render meshes");
    for (const auto& [meshHnd, objects] : drawCommands) {
        Mesh* mesh = m_assetHandler.getMesh(meshHnd);

        bindMesh(mesh, commandBuffer);

        PushConstant push{offset};
        vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstant), &push);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipelineLayout, 0, 1, &m_frameResources[m_currentFrame].descriptorSet, 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, (mesh->indicesCount), objects.size(), 0, 0, 0);
        offset += objects.size();
    }
}

void Renderer::endRenderPass(VkCommandBuffer commandBuffer)
{
    TracyVkZone(m_profiler, commandBuffer, "End render pass");
    vkCmdEndRenderPass(commandBuffer);
}

void Renderer::bindMesh(Mesh* mesh, VkCommandBuffer commandBuffer) const
{
    VkBuffer vertexBuffers[] = {mesh->combinedBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    //indices need to be aligned to 4B if UINT32
    VkDeviceSize indexOffset = (mesh->verticesSize + 3) & ~3;

    vkCmdBindIndexBuffer(commandBuffer, mesh->combinedBuffer.buffer, indexOffset, VK_INDEX_TYPE_UINT32);
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

    vkGetDeviceQueue(m_context.device, m_context.familyIndices.graphicsFamily.value(), 0, &m_context.graphicsQueue);
    vkGetDeviceQueue(m_context.device, m_context.familyIndices.presentFamily.value(), 0, &m_context.presentQueue);
}

} // Plunksna
