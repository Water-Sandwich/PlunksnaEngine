//
// Created by d on 2/20/26.
//

#include "DescriptorManager.h"

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

VkDescriptorSet DescriptorManager::getSet(Descriptor desc, int index) const
{
    return m_descriptors[desc].sets[index];
}

Descriptor DescriptorManager::beginBuild()
{
    m_descriptors.emplace_back();
    return m_descriptors.size() - 1;
}

uint32_t DescriptorManager::pushBinding(Descriptor desc, VkDescriptorType type, VkShaderStageFlags stages)
{
    m_descriptors[desc].layoutBuildStages.push_back(DescriptorBindingBuild(type, stages));
    return m_descriptors[desc].layoutBuildStages.size() - 1;
}

VkDescriptorSetLayout DescriptorManager::submitBuild(const Context& context, Descriptor desc, uint32_t maxSets)
{
    ASSERT(desc < m_descriptors.size(),
        "Trying to access an uninitialized descriptor pack! desc: " << desc <<
        " size: " << m_descriptors.size());

    createPool(context, desc, maxSets);

    createLayout(context, desc);

    allocateSets(context, desc, maxSets);

    return getLayout(desc);
}

uint32_t DescriptorManager::pushBufferInfo(Descriptor desc, VkDescriptorBufferInfo info)
{
    auto& pack = m_descriptors[desc];
    auto index = pack.setBuildStages.size();

    ASSERT(index < pack.layoutBuildStages.size(),
        "Attempting to build a set larger than its layout! size:" << index)

    ASSERT(isBufferDescriptor(pack.layoutBuildStages[index].type),
        "Trying to bind an invalid type to a buffer layout binding! expectedType: "
        << pack.layoutBuildStages[index].type);

    DescriptorSetBuild build{
        .bufferInfo = info
    };

    pack.setBuildStages.push_back(build);
    return index;
}

uint32_t DescriptorManager::pushImageInfo(Descriptor desc, VkDescriptorImageInfo info)
{
    auto& pack = m_descriptors[desc];
    auto index = pack.setBuildStages.size();

    ASSERT(index < pack.layoutBuildStages.size(),
        "Attempting to build a set larger than its layout! size:" << index)

    ASSERT(isImageDescriptor(pack.layoutBuildStages[index].type),
        "Trying to bind an invalid type to a image layout binding! expectedType: "
        << pack.layoutBuildStages[index].type);

    DescriptorSetBuild build{
        .imageInfo = info
    };

    pack.setBuildStages.push_back(build);
    return index;
}

VkDescriptorSet DescriptorManager::pushSetWrite(const Context& context, Descriptor desc, int setNum)
{
    auto& pack = m_descriptors[desc];

    ASSERT(pack.layoutBuildStages.size() == pack.setBuildStages.size(),
        "Trying to build an incomplete set!\n"
        << " setSize:" << pack.setBuildStages.size()
        << " layoutSize: " << pack.layoutBuildStages.size());

    ASSERT(setNum < pack.sets.size(),
        "Trying to access a set out of bounds! setNum: " << setNum << "setsSize: " << pack.sets.size());

    pack.setWrites.reserve(pack.setBuildStages.size());

    for (uint32_t i = 0; i < pack.setBuildStages.size(); i++) {
        auto& layoutBuild = pack.layoutBuildStages[i];
        auto& setBuild = pack.setBuildStages[i];

        VkWriteDescriptorSet write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = getSet(desc, setNum),
            .dstBinding = i,
            .dstArrayElement = 0,
            .descriptorType = layoutBuild.type,
            .descriptorCount = 1,
            .pBufferInfo = &setBuild.bufferInfo
        };

        if (isBufferDescriptor(layoutBuild.type))
            write.pBufferInfo = &setBuild.bufferInfo;
        else
            write.pImageInfo = &setBuild.imageInfo;


        pack.setWrites.push_back(write);
    }

    return pack.sets[setNum];
}

void DescriptorManager::createDescriptorSets(const Context& context, Descriptor desc)
{
    auto& pack = m_descriptors[desc];

    vkUpdateDescriptorSets(context.device, pack.setWrites.size(), pack.setWrites.data(), 0, nullptr);
}

void DescriptorManager::clean(const Context& context)
{
    for (auto& pack : m_descriptors) {
        VK_DESTROY(pack.pool, context.device, vkDestroyDescriptorPool)
        VK_DESTROY(pack.layout, context.device, vkDestroyDescriptorSetLayout)
    }

    m_descriptors.clear();
}

void DescriptorManager::createPool(const Context& context, Descriptor desc, uint32_t maxSets)
{
    auto& buildQueue = m_descriptors[desc].layoutBuildStages;

    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.reserve(buildQueue.size());

    for (const DescriptorBindingBuild& build : buildQueue) {
        VkDescriptorPoolSize size = {
            .type = build.type,
            .descriptorCount = maxSets
        };

        poolSizes.push_back(size);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxSets;

    ASSERT_V(vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &m_descriptors[desc].pool),
        "failed to create descriptor pool!")
}

void DescriptorManager::createLayout(const Context& context, Descriptor desc)
{
    auto& buildQueue = m_descriptors[desc].layoutBuildStages;

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(buildQueue.size());

    for (uint32_t i = 0; i < buildQueue.size(); i++) {
        VkDescriptorSetLayoutBinding bind {
            .binding = i,
            .descriptorType = buildQueue[i].type,
            .descriptorCount = 1,
            .stageFlags = buildQueue[i].stages,
            .pImmutableSamplers = nullptr
        };

        bindings.push_back(bind);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    ASSERT_V(vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &m_descriptors[desc].layout),
        "failed to create descriptor set layout!")
}

void DescriptorManager::allocateSets(const Context& context, Descriptor desc, uint32_t maxSets)
{
    auto& pack = m_descriptors[desc];
    pack.sets.resize(maxSets);
    std::vector layouts(maxSets, pack.layout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pack.pool;
    allocInfo.descriptorSetCount = maxSets;
    allocInfo.pSetLayouts = layouts.data();

    ASSERT_V(vkAllocateDescriptorSets(context.device, &allocInfo, pack.sets.data()),
        "failed to allocate descriptor sets!")
}

} // Plunksna