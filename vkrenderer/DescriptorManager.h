//
// Created by d on 2/20/26.
//

#ifndef DESCRIPTORMANAGER_H
#define DESCRIPTORMANAGER_H
#include <limits>
#include <vulkan/vulkan_core.h>
#include <vector>

#include "Context.h"
#include "utils/Types.h"

namespace Plunksna {

using Descriptor = u32;
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

        u32 maxSets;
        u32 maxTotalBuildStages;

        std::vector<DescriptorBindingBuild> layoutBuildStages;
        std::vector<DescriptorSetBuild> setBuildStages;
        std::vector<VkWriteDescriptorSet> setWrites;
    };

public:
    DescriptorManager() = default;

    VkDescriptorPool getPool(Descriptor desc) const;
    VkDescriptorSetLayout getLayout(Descriptor desc) const;
    VkDescriptorSet getSet(Descriptor desc, i32 index) const;

    VkDescriptorPool* getPoolPtr(Descriptor desc);
    VkDescriptorSetLayout* getLayoutPtr(Descriptor desc);
    VkDescriptorSet* getSetPtr(Descriptor desc, i32 index);

    //start to build a descriptor pack, returns a handle to the current build queue
    Descriptor beginBuild();
    //add a binding, returns bind point
    u32 pushBinding(Descriptor desc, VkDescriptorType type, VkShaderStageFlags stages);
    //submit the queue and build the pool and layout and allocates descriptor sets, returns finished layout
    VkDescriptorSetLayout submitBuild(const Context& context, Descriptor desc, u32 maxSets);

    //push a buffer to the descriptor set queue build, returns the binding point
    u32 pushBufferInfo(Descriptor desc, VkDescriptorBufferInfo info);
    //push an image to the descriptor set queue build, returns the binding point
    u32 pushImageInfo(Descriptor desc, VkDescriptorImageInfo info);
    //prepare descriptor set writes
    VkDescriptorSet pushSetWrite(Descriptor desc, i32 setNum);
    //initialize all descriptor sets
    void createDescriptorSets(const Context& context, Descriptor desc);

    void clean(const Context& context);
private:
    void createPool(const Context& context, Descriptor desc, u32 maxSets);
    void createLayout(const Context& context, Descriptor desc);
    void allocateSets(const Context& context, Descriptor desc, u32 maxSets);

    static constexpr bool isBufferDescriptor(VkDescriptorType type);
    static constexpr bool isImageDescriptor(VkDescriptorType type);

private:
    //1 pool per layout, N sets
    std::vector<DescriptorPack> m_descriptors;
};

} // Plunksna

#endif //DESCRIPTORMANAGER_H
