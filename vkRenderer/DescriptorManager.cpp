//
// Created by d on 2/20/26.
//

#include "DescriptorManager.h"

#include <glm/common.hpp>

#include "RendererUtils.h"
#include "engine/Exception.h"

namespace Plunksna {
VkDescriptorPool DescriptorManager::getPool(Descriptor desc) const
{
    return m_descriptors[desc].pool;
}

VkDescriptorSetLayout DescriptorManager::getLayout(Descriptor desc) const
{
    return m_descriptors[desc].layout;
}

VkDescriptorSet DescriptorManager::getSet(Descriptor desc, i32 index) const
{
    return m_descriptors[desc].sets[index];
}

VkDescriptorPool* DescriptorManager::getPoolPtr(Descriptor desc)
{
    return &m_descriptors[desc].pool;
}

VkDescriptorSetLayout* DescriptorManager::getLayoutPtr(Descriptor desc)
{
    return &m_descriptors[desc].layout;
}

VkDescriptorSet* DescriptorManager::getSetPtr(Descriptor desc, i32 index)
{
    return &m_descriptors[desc].sets[index];
}

Descriptor DescriptorManager::beginBuild(u32 maxSets)
{
    m_descriptors.emplace_back();
    m_descriptors.back().totalDescriptorSets = maxSets;
    return m_descriptors.size() - 1;
}

DescriptorBuf DescriptorManager::pushBinding(Descriptor desc, ShareType shareType, VkDescriptorType type, VkShaderStageFlags stages,
    u32 bindPoint, u32 descriptorCount, VkDescriptorBindingFlags bindingFlags)
{
    auto& pack = m_descriptors[desc];
    ASSERT(!pack.isVariable,
        "Descriptor [" << desc << "]" << " is already variable, attempting to add another binding at the end!");

    pack.layoutStages.push_back(StageLayout(type, stages, shareType, descriptorCount, bindingFlags, bindPoint));

    if (bindingFlags != 0) {
        pack.isVariable = true;
    }

    pack.descriptors.emplace_back(vkTypeToBindType(type), shareMult(shareType, descriptorCount, pack.totalDescriptorSets));

    return pack.layoutStages.size() - 1;
}

VkDescriptorSetLayout DescriptorManager::submitBuild(const Context& context, Descriptor desc)
{
    ASSERT(desc < m_descriptors.size(),
        "Trying to access an uninitialized descriptor pack! desc: " << desc <<
        " size: " << m_descriptors.size());

    createPool(context, desc);

    createLayout(context, desc);

    allocateSets(context, desc);

    return getLayout(desc);
}

void DescriptorManager::allocateDescriptorBuffers(const Context& context, Descriptor desc, DescriptorBuf buf,
    VkDeviceSize size, VmaAllocationCreateFlagBits access)
{
    auto& pack = m_descriptors[desc];
    auto& descriptor = pack.descriptors[buf];

    ASSERT(descriptor.type == eBUFFER,
        "Trying to update a buffer on an image descriptor!")

    VkBufferUsageFlagBits usage = bufferTypeToUsage(pack.layoutStages[buf].type);

    //TODO: put all of these buffers into 1 megabuffer
    for (int i = 0; i < descriptor.bufferInfos.size(); i++) {
        auto& bufferInfo = descriptor.bufferInfos[i];
        RenderUtils::createBuffer(context, bufferInfo.buffer, size, usage, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            access);

        vmaMapMemory(context.allocator, bufferInfo.buffer.allocation, &bufferInfo.map);

        bufferInfo.vkBufferInfo.buffer = bufferInfo.buffer.buffer;
        bufferInfo.vkBufferInfo.range = size;
        bufferInfo.vkBufferInfo.offset = 0;

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = getSet(desc, i);
        write.dstArrayElement = 0;
        write.dstBinding = pack.layoutStages[buf].bindPoint;
        write.descriptorType = pack.layoutStages[buf].type;

        write.descriptorCount = 1;
        write.pBufferInfo = &bufferInfo.vkBufferInfo;

        pack.writeQueue.push_back(write);
    }
}

void DescriptorManager::updateWriteQueue(const Context& context, Descriptor desc)
{
    auto& pack = m_descriptors[desc];

    vkUpdateDescriptorSets(context.device, pack.writeQueue.size(), pack.writeQueue.data(), 0, nullptr);
    pack.writeQueue.clear();
}

void DescriptorManager::pushImageWrite(Descriptor desc, DescriptorBuf buf, Texture* texture, VkSampler sampler, u32 index)
{
    auto& pack = m_descriptors[desc];
    auto& descriptor = pack.descriptors[buf];

    ASSERT(descriptor.type == eIMAGE,
        "Attempting to push image info on a buffer descriptor!")

    //TODO: figure out multiple descriptors on 1 stage

    auto& imageInfo = descriptor.imageInfos[index];

    imageInfo.vkImageInfo.imageLayout = texture->layout;
    imageInfo.vkImageInfo.imageView = texture->fullView;
    imageInfo.vkImageInfo.sampler = sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = getSet(desc, index);
    write.dstArrayElement = 0;
    write.dstBinding = pack.layoutStages[buf].bindPoint;
    write.descriptorType = pack.layoutStages[buf].type;

    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo.vkImageInfo;

    pack.writeQueue.push_back(write);
}

void* DescriptorManager::getBufferWrite(Descriptor desc, DescriptorBuf buf, u32 index)
{
    auto& pack = m_descriptors[desc];
    auto& setStage = pack.descriptors[buf];

    ASSERT(setStage.type == eBUFFER,
        "Attempting to access a buffer on a non buffer object!")

    index = glm::min(index, (u32)setStage.bufferInfos.size() - 1);
    return setStage.bufferInfos[index].map;
}

void DescriptorManager::clean(const Context& context)
{
    for (auto& pack : m_descriptors) {
        VK_DESTROY(pack.pool, context.device, vkDestroyDescriptorPool)
        VK_DESTROY(pack.layout, context.device, vkDestroyDescriptorSetLayout)
    }

    m_descriptors.clear();
}

void DescriptorManager::createPool(const Context& context, Descriptor desc)
{
    auto& pack = m_descriptors[desc];
    auto& buildQueue = pack.layoutStages;

    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.reserve(buildQueue.size());

    for (const StageLayout& build : buildQueue) {
        VkDescriptorPoolSize size = {
            .type = build.type,
            .descriptorCount = build.descriptorCount * pack.totalDescriptorSets
        };

        poolSizes.push_back(size);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<u32>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = pack.totalDescriptorSets;

    ASSERT_V(vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &pack.pool),
        "failed to create descriptor pool!")
}

void DescriptorManager::createLayout(const Context& context, Descriptor desc)
{
    auto& buildQueue = m_descriptors[desc].layoutStages;

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(buildQueue.size());

    std::vector<VkDescriptorBindingFlags> flags(buildQueue.size(), 0);

    u32 currentBinding = 0;
    for (u32 i = 0; i < buildQueue.size(); i++) {
        auto& layoutBuild = m_descriptors[desc].layoutStages[i];
        flags[i] = buildQueue[i].bindingFlags;

        if (layoutBuild.bindPoint != UINT32_MAX) {
            ASSERT(layoutBuild.bindPoint >= currentBinding,
                "Improperly formed bind point, currentBinding: " << currentBinding <<
                ", layoutBuildStage[" << i << "].bindPoint: " << layoutBuild.bindPoint)

            currentBinding = layoutBuild.bindPoint;
        }
        else {
            layoutBuild.bindPoint = currentBinding;
        }

        VkDescriptorSetLayoutBinding bind {
            .binding = currentBinding,
            .descriptorType = buildQueue[i].type,
            .descriptorCount = buildQueue[i].descriptorCount,
            .stageFlags = buildQueue[i].stages,
            .pImmutableSamplers = nullptr
        };

        bindings.push_back(bind);
        currentBinding++;
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
    flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsInfo.bindingCount = flags.size();
    flagsInfo.pBindingFlags = flags.data();

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<u32>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    layoutInfo.pNext = &flagsInfo;

    ASSERT_V(vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &m_descriptors[desc].layout),
        "failed to create descriptor set layout!")
}

void DescriptorManager::allocateSets(const Context& context, Descriptor desc)
{
    auto& pack = m_descriptors[desc];
    std::vector layouts(pack.totalDescriptorSets, pack.layout);

    pack.sets.resize(pack.totalDescriptorSets);

    const auto& last = pack.layoutStages.back();

    std::vector<u32> sizes(pack.totalDescriptorSets,
        shareMult(last.shareType, last.descriptorCount, pack.totalDescriptorSets));

    VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};
    countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    countInfo.descriptorSetCount = sizes.size();
    countInfo.pDescriptorCounts = sizes.data();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pack.pool;
    allocInfo.descriptorSetCount = pack.totalDescriptorSets;
    allocInfo.pSetLayouts = layouts.data();

    if (pack.isVariable)
        allocInfo.pNext = &countInfo;
    else
        allocInfo.pNext = nullptr;

    ASSERT_V(vkAllocateDescriptorSets(context.device, &allocInfo, pack.sets.data()),
        "failed to allocate descriptor sets!")
}

constexpr bool DescriptorManager::isBufferDescriptor(VkDescriptorType type)
{
    switch (type)
    {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            return true;
        default:
            return false;
    }
}

constexpr bool DescriptorManager::isImageDescriptor(VkDescriptorType type)
{
    switch (type)
    {
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            return true;
        default:
            return false;
    }
}

constexpr DescriptorManager::BindType DescriptorManager::vkTypeToBindType(VkDescriptorType type)
{
    return isImageDescriptor(type) ? eIMAGE : eBUFFER;
}

constexpr u32 DescriptorManager::shareMult(ShareType type, u32 descriptorCount, u32 maxSets)
{
    return type == eEXCLUSIVE ? descriptorCount * maxSets : descriptorCount;
}

constexpr VkBufferUsageFlagBits DescriptorManager::bufferTypeToUsage(VkDescriptorType type)
{
    switch (type) {
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        return VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;

    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;

    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
    case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
        return VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;

    default:
        LOG_S(eWARNING, "Hit an unexpected buffer type! :" << type);
        return static_cast<VkBufferUsageFlagBits>(0);
    }
}

} // Plunksna