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

Descriptor DescriptorManager::beginBuild()
{
    m_descriptors.emplace_back();
    return m_descriptors.size() - 1;
}

u32 DescriptorManager::pushBinding(Descriptor desc, VkDescriptorType type, VkShaderStageFlags stages,
    u32 descriptorCount, VkDescriptorBindingFlags bindingFlags)
{
    m_descriptors[desc].layoutBuildStages.push_back(DescriptorBindingBuild(type, stages, descriptorCount, bindingFlags));

    if (bindingFlags != 0)
        m_descriptors[desc].isVariable = true;

    return m_descriptors[desc].layoutBuildStages.size() - 1;
}

VkDescriptorSetLayout DescriptorManager::submitBuild(const Context& context, Descriptor desc, u32 maxSets, u32 maxVariableDescriptors)
{
    ASSERT(desc < m_descriptors.size(),
        "Trying to access an uninitialized descriptor pack! desc: " << desc <<
        " size: " << m_descriptors.size());

    createPool(context, desc, maxSets);

    createLayout(context, desc);

    allocateSets(context, desc, maxSets, maxVariableDescriptors);

    return getLayout(desc);
}

u32 DescriptorManager::pushBufferInfo(Descriptor desc, VkDescriptorBufferInfo info)
{
    auto& pack = m_descriptors[desc];
    auto index = pack.setBuildStages.size() % pack.layoutBuildStages.size();

    ASSERT(pack.setBuildStages.size() <= pack.maxTotalBuildStages,
        "Attempting to build a set with too many buffers! size: " << index)

    ASSERT(isBufferDescriptor(pack.layoutBuildStages[index].type),
        "Trying to bind an invalid type to a buffer layout binding! expectedType: " <<
        pack.layoutBuildStages[index].type);

    if (pack.isVariable && !pack.setBuildStages.empty()) {
        ASSERT(pack.setBuildStages.back().type != eVARBUFFER || pack.setBuildStages.back().type != eVARIMAGE,
            "Trying to bind a regular descriptor at the end of a variable descriptor set!")
    }

    DescriptorSetBuild build{};
    build.type = eBUFFER;
    build.bufferInfo = info;

    pack.setBuildStages.push_back(build);
    return index;
}

u32 DescriptorManager::pushImageInfo(Descriptor desc, VkDescriptorImageInfo info)
{
    auto& pack = m_descriptors[desc];
    auto index = pack.setBuildStages.size() % pack.layoutBuildStages.size();;

    ASSERT(pack.setBuildStages.size() <= pack.maxTotalBuildStages,
        "Attempting to build a set with too many images! size:" << index)

    ASSERT(isImageDescriptor(pack.layoutBuildStages[index].type),
        "Trying to bind an invalid type to a image layout binding! expectedType: " <<
        pack.layoutBuildStages[index].type);

    if (pack.isVariable && !pack.setBuildStages.empty()) {
        ASSERT(pack.setBuildStages.back().type != eVARBUFFER || pack.setBuildStages.back().type != eVARIMAGE,
            "Trying to bind a regular descriptor at the end of a variable descriptor set!")
    }

    DescriptorSetBuild build{};
    build.type = eIMAGE;
    build.imageInfo = info;

    pack.setBuildStages.push_back(build);
    return index;
}

u32 DescriptorManager::pushImageInfos(Descriptor desc, const std::vector<VkDescriptorImageInfo>& info)
{
    auto& pack = m_descriptors[desc];
    auto index = pack.setBuildStages.size() % pack.layoutBuildStages.size();

    ASSERT(pack.isVariable,
        "Trying to push a vector object to non-variable descriptor set!")

    ASSERT(isImageDescriptor(pack.layoutBuildStages[index].type),
        "Trying to bind an invalid type to a image layout binding! expectedType: " <<
        pack.layoutBuildStages[index].type);

    ASSERT(pack.setBuildStages.size() <= pack.maxTotalBuildStages,
        "Attempting to build a set with too many images! size:" << index)

    ASSERT(index == pack.layoutBuildStages.size() - 1,
        "A variable descriptor set must have its variable binding be last!")

    ASSERT(info.size() + pack.variableDescriptors <= pack.maxVariableDescriptors,
        "Trying to add more descriptors to variable set than what was allocated for! " <<
        "max: " << pack.maxVariableDescriptors << "size: " << info.size());

    pack.variableDescriptors += info.size();

    DescriptorSetBuild build{};
    build.type = eVARIMAGE;
    build.descriptorCount = info.size(); //TODO: std::move here?
    build.imageInfos = info;

    pack.setBuildStages.push_back(build);
    return index;
}

VkDescriptorSet DescriptorManager::pushSetWrite(Descriptor desc, i32 setNum)
{
    auto& pack = m_descriptors[desc];

    ASSERT(pack.setBuildStages.size() <= pack.maxTotalBuildStages,
        "Trying to build a set higher than the max stage count! max: " << pack.maxTotalBuildStages <<
        "current: " << pack.setBuildStages.size());

    ASSERT((pack.setBuildStages.size() % pack.layoutBuildStages.size()) == 0,
        "Trying to build an incomplete set! " <<
        "setSize:" << pack.setBuildStages.size() <<
        " layoutSize: " << pack.layoutBuildStages.size());

    ASSERT(setNum < pack.sets.size(),
        "Trying to access a set out of bounds! setNum: " << setNum << "setsSize: " << pack.sets.size());

    ASSERT(pack.isVariable && pack.setBuildStages.back().type == eVARIMAGE,
        "A variable descriptor set must have the last binding be a vector object!")

    for (u32 i = 0; i < pack.layoutBuildStages.size(); i++) {
        auto& layoutBuild = pack.layoutBuildStages[i];
        auto& setBuild = pack.setBuildStages[setNum * pack.layoutBuildStages.size() + i];

        if (i < pack.layoutBuildStages.size() - 1) {
            ASSERT(pack.isVariable && (setBuild.type != eVARIMAGE || setBuild.type == eVARBUFFER),
                "Descriptor set of binding " << i << " cannot be a vector object, as it is not the last binding")
        }

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = getSet(desc, setNum);
        write.dstBinding = i;
        write.dstArrayElement = 0;
        write.descriptorType = layoutBuild.type;
        write.descriptorCount = setBuild.descriptorCount;

        switch (setBuild.type) {
        case eIMAGE:
            write.pImageInfo = &setBuild.imageInfo;
            break;
        case eBUFFER:
            write.pBufferInfo = &setBuild.bufferInfo;
            break;
        case eVARIMAGE:
            write.pImageInfo = setBuild.imageInfos.data();
            break;
        default:
            THROW("Unsupported type");
        }

        pack.setWrites.push_back(write);
    }

    return pack.sets[setNum];
}

void DescriptorManager::createDescriptorSets(const Context& context, Descriptor desc)
{
    auto& pack = m_descriptors[desc];

    vkUpdateDescriptorSets(context.device, pack.setWrites.size(), pack.setWrites.data(), 0, nullptr);

    pack.setWrites.clear();
}

void DescriptorManager::clean(const Context& context)
{
    for (auto& pack : m_descriptors) {
        VK_DESTROY(pack.pool, context.device, vkDestroyDescriptorPool)
        VK_DESTROY(pack.layout, context.device, vkDestroyDescriptorSetLayout)
    }

    m_descriptors.clear();
}

void DescriptorManager::createPool(const Context& context, Descriptor desc, u32 maxSets)
{
    auto& buildQueue = m_descriptors[desc].layoutBuildStages;
    m_descriptors[desc].maxSets = maxSets;

    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.reserve(buildQueue.size());

    for (const DescriptorBindingBuild& build : buildQueue) {
        VkDescriptorPoolSize size = {
            .type = build.type,
            .descriptorCount = build.descriptorCount * maxSets
        };

        poolSizes.push_back(size);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<u32>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = maxSets;

    ASSERT_V(vkCreateDescriptorPool(context.device, &poolInfo, nullptr, &m_descriptors[desc].pool),
        "failed to create descriptor pool!")
}

void DescriptorManager::createLayout(const Context& context, Descriptor desc)
{
    auto& buildQueue = m_descriptors[desc].layoutBuildStages;
    m_descriptors[desc].maxTotalBuildStages = buildQueue.size() * m_descriptors[desc].maxSets;

    std::vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(buildQueue.size());

    std::vector<VkDescriptorBindingFlags> flags(buildQueue.size(), 0);

    for (u32 i = 0; i < buildQueue.size(); i++) {
        flags[i] = buildQueue[i].bindingFlags;

        VkDescriptorSetLayoutBinding bind {
            .binding = i,
            .descriptorType = buildQueue[i].type,
            .descriptorCount = buildQueue[i].descriptorCount,
            .stageFlags = buildQueue[i].stages,
            .pImmutableSamplers = nullptr
        };

        bindings.push_back(bind);
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
    flagsInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
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

void DescriptorManager::allocateSets(const Context& context, Descriptor desc, u32 maxSets, u32 maxVariableDescriptors)
{
    auto& pack = m_descriptors[desc];
    std::vector layouts(maxSets, pack.layout);

    if (pack.isVariable) {
        ASSERT(maxVariableDescriptors != 0,
            "A variable descriptor set must have a valid maxVariableDescriptors value!")
    }
    else {
        ASSERT(maxVariableDescriptors == 0,
            "A static descriptor set must have a maxVariableDescriptors value of 0!")
    }

    pack.sets.resize(maxSets);
    pack.setWrites.reserve(pack.maxTotalBuildStages);
    pack.setBuildStages.reserve(pack.maxTotalBuildStages);
    pack.maxVariableDescriptors = maxVariableDescriptors;

    std::vector sizes(maxSets, maxVariableDescriptors);

    VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};
    countInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    countInfo.descriptorSetCount = sizes.size();
    countInfo.pDescriptorCounts = sizes.data();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pack.pool;
    allocInfo.descriptorSetCount = maxSets;
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

} // Plunksna