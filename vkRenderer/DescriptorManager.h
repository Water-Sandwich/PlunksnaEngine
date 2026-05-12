//
// Created by d on 2/20/26.
//

#ifndef DESCRIPTORMANAGER_H
#define DESCRIPTORMANAGER_H
#include <limits>
#include <vulkan/vulkan_core.h>
#include <vector>

#include "Buffer.h"
#include "Context.h"
#include "assetHandler/Texture.h"
#include "utils/Types.h"

namespace Plunksna {

using DescriptorSet = u32;
using Descriptor = u32;
constexpr DescriptorSet NULL_DESCRIPTOR = std::numeric_limits<DescriptorSet>::max();

enum ShareType
{
    eSHARED,
    eEXCLUSIVE
};

class DescriptorManager {
private:
    enum BindType
    {
        eNONE = -1,
        eBUFFER = 0,
        eIMAGE,
    };

    struct StageLayout
    {
        VkDescriptorType type;
        VkShaderStageFlags stages;
        ShareType shareType;
        u32 descriptorCount;
        VkDescriptorBindingFlags bindingFlags;
        u32 bindPoint;
    };

    struct BufferInfo
    {
        VkDescriptorBufferInfo vkBufferInfo;
        Buffer buffer;
        void* map = nullptr;
    };

    struct ImageInfo
    {
        VkDescriptorImageInfo vkImageInfo;
        //likely to add more here in the future
    };

    struct StageDescriptor
    {
        StageDescriptor(BindType typ, u32 size)
        {
            type = typ;

            if (typ == eIMAGE)
                imageInfos.resize(size);
            else
                bufferInfos.resize(size);
        }

        BindType type;

        std::vector<BufferInfo> bufferInfos;
        std::vector<ImageInfo> imageInfos;
    };

    struct DescriptorPack
    {
        VkDescriptorPool pool = VK_NULL_HANDLE;
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> sets;

        u32 totalDescriptorSets;

        std::vector<StageLayout> layoutStages;

        //hold all the descriptor info for a particular stage,
        //setstages.size = layoutstages.size
        //buffer/image infos.size = layoutstage.descriptorCount * (if exclusive) totalDescriptorSets
        std::vector<StageDescriptor> descriptors;

        std::vector<VkWriteDescriptorSet> writeQueue;

        bool isVariable = false;
    };

public:
    DescriptorManager() = default;

    VkDescriptorPool getPool(DescriptorSet desc) const;
    VkDescriptorSetLayout getLayout(DescriptorSet desc) const;
    VkDescriptorSet getSet(DescriptorSet desc, i32 index) const;

    VkDescriptorPool* getPoolPtr(DescriptorSet desc);
    VkDescriptorSetLayout* getLayoutPtr(DescriptorSet desc);
    VkDescriptorSet* getSetPtr(DescriptorSet desc, i32 index);

    //start to build a descriptor pack, returns a handle to the current build queue
    DescriptorSet beginBuild(u32 maxSets);
    //add a binding, returns index of binding/stage
    Descriptor pushBinding(DescriptorSet desc, ShareType shareType, VkDescriptorType type, VkShaderStageFlags stages,
        u32 bindPoint = UINT32_MAX, u32 descriptorCount = 1, VkDescriptorBindingFlags bindingFlags = 0);
    //submit the queue and build the pool and layout and allocates descriptor sets, returns finished layout
    VkDescriptorSetLayout submitBuild(const Context& context, DescriptorSet desc);

    void allocateDescriptorBuffers(const Context& context, DescriptorSet desc, Descriptor buf, VkDeviceSize size,
        VmaAllocationCreateFlagBits access = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);

    void updateWriteQueue(const Context& context, DescriptorSet desc);

    //TODO: descriptor index tba latter whenever i need multiple descriptors for images
    void pushImageWrite(DescriptorSet desc, Descriptor buf, Texture* texture, VkSampler sampler, u32 arrayIndex = 0);

    void* getBufferWrite(DescriptorSet desc, Descriptor buf, u32 index);

    void clean(const Context& context);
    void cleanDescriptor(const Context& context, DescriptorSet descriptor, Descriptor descBuf);
private:
    void createPool(const Context& context, DescriptorSet desc);
    void createLayout(const Context& context, DescriptorSet desc);
    void allocateSets(const Context& context, DescriptorSet desc);

    static constexpr bool isBufferDescriptor(VkDescriptorType type);
    static constexpr bool isImageDescriptor(VkDescriptorType type);

    static constexpr BindType vkTypeToBindType(VkDescriptorType type);
    static constexpr u32 shareMult(ShareType type, u32 descriptorCount, u32 maxSets);

    static constexpr VkBufferUsageFlagBits bufferTypeToUsage(VkDescriptorType type);

private:
    //1 pool per layout, N sets
    std::vector<DescriptorPack> m_descriptors;
};

} // Plunksna

#endif //DESCRIPTORMANAGER_H
