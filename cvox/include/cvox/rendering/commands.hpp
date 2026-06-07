#pragma once
#include "cvox/core.h"
#include "image.hpp"
#include "sync.hpp"
#include <span>
#include <vulkan/vulkan.h>

namespace cvox
{
struct RenderBeginInfo
{
    Image& color_image;
};
class Queue;
class VulkanContext;
class CVOX_API CommandBuffer
{
public:
    friend class VulkanContext;
    friend class RenderContext;
    friend class CommandPool;

    struct CreateInfo
    {
        Queue& queue;
    };
    CommandBuffer() = default;

    void begin();
    void end();
    void submit(std::span<Semaphore> wait_semaphores,
                std::span<Semaphore> signal_semaphores,
                Fence fence);
    void reset();
    void transition_image(Image& img, VkImageLayout new_layout);
    void begin_rendering(const RenderBeginInfo& begin_info);
    void end_rendering();

private:
    VkCommandBuffer m_buffer;
    Queue* m_queue;
};

class CVOX_API CommandPool
{
public:
    friend class VulkanContext;
    friend class RenderContext;
    struct CreateInfo
    {
        uint32_t queue_family_index;
    };
    CommandBuffer create_command_buffer(const CommandBuffer::CreateInfo& create_info);

    CommandPool(VulkanContext* context) : m_vulkan_context(context){};

private:
    VkCommandPool m_pool;
    VulkanContext* m_vulkan_context;
};
} // namespace cvox