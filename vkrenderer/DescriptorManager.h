//
// Created by d on 2/20/26.
//

#ifndef DESCRIPTORMANAGER_H
#define DESCRIPTORMANAGER_H
#include <limits>
#include <vulkan/vulkan_core.h>
#include <vector>

#include "Context.h"

namespace Plunksna {

using Descriptor = uint32_t;
constexpr Descriptor NULL_DESCRIPTOR = std::numeric_limits<Descriptor>::max();

class DescriptorManager {
private:
    struct DescriptorBindingBuild
    {
        VkDescriptorType type;
        VkShaderStageFlags stages;
    };

    struct DescriptorSetBuild
    {
        union
        {
            VkDescriptorBufferInfo bufferInfo;
            VkDescriptorImageInfo imageInfo;
        };
    };

    struct DescriptorPack
    {
        VkDescriptorPool pool = VK_NULL_HANDLE;
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> sets;

        std::vector<DescriptorBindingBuild> layoutBuildStages;
        std::vector<DescriptorSetBuild> setBuildStages;
        std::vector<VkWriteDescriptorSet> setWrites;
    };

public:
    DescriptorManager() = default;

    VkDescriptorPool getPool(Descriptor desc) const;
    VkDescriptorSetLayout getLayout(Descriptor desc) const;
    VkDescriptorSet getSet(Descriptor desc, int index) const;

    //start to build a descriptor pack, returns a handle to the current build queue
    Descriptor beginBuild();
    //add a binding, returns bind point
    uint32_t pushBinding(Descriptor desc, VkDescriptorType type, VkShaderStageFlags stages);
    //submit the queue and build the pool and layout and allocates descriptor sets, returns finished layout
    VkDescriptorSetLayout submitBuild(const Context& context, Descriptor desc, uint32_t maxSets);

    //push a buffer to the descriptor set queue build, returns the binding point
    uint32_t pushBufferInfo(Descriptor desc, VkDescriptorBufferInfo info);
    //push an image to the descriptor set queue build, returns the binding point
    uint32_t pushImageInfo(Descriptor desc, VkDescriptorImageInfo info);
    //prepare descriptor set writes
    VkDescriptorSet pushSetWrite(const Context& context, Descriptor desc, int setNum);
    //initialize all descriptor sets
    void createDescriptorSets(const Context& context, Descriptor desc);

    void clean(const Context& context);
private:
    void createPool(const Context& context, Descriptor desc, uint32_t maxSets);
    void createLayout(const Context& context, Descriptor desc);
    void allocateSets(const Context& context, Descriptor desc, uint32_t maxSets);

    static constexpr bool isBufferDescriptor(VkDescriptorType type);
    static constexpr bool isImageDescriptor(VkDescriptorType type);

private:
    //1 pool per layout, N sets
    std::vector<DescriptorPack> m_descriptors;
};

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

#endif //DESCRIPTORMANAGER_H
