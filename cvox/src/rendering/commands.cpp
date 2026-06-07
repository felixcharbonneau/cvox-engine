#include "cvox/rendering/commands.hpp"

#include "cvox/rendering/vulkan_context.hpp"

namespace cvox
{
CommandBuffer
CommandPool::create_command_buffer(const CommandBuffer::CreateInfo& create_info)
{
    CommandBuffer cmd;
    cmd.m_queue = &create_info.queue;
    VkCommandBufferAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    vkAllocateCommandBuffers(m_vulkan_context->device(), &allocInfo, &cmd.m_buffer);
    return cmd;
}
void
CommandBuffer::begin()
{
    VkCommandBufferBeginInfo begin_info = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(m_buffer, &begin_info);
}
void
CommandBuffer::end()
{
    vkEndCommandBuffer(m_buffer);
}
void
CommandBuffer::reset()
{
    vkResetCommandBuffer(m_buffer, 0);
}
void
CommandBuffer::transition_image(Image& img, VkImageLayout new_layout)
{
    VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
                                         ? VK_IMAGE_ASPECT_DEPTH_BIT
                                         : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageMemoryBarrier2 image_barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,

        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
        .oldLayout = img.layout(),
        .newLayout = new_layout,
        .image = img.image(),
        .subresourceRange =
            {
                .aspectMask = aspect_mask,
                .baseMipLevel = 0,
                .levelCount = VK_REMAINING_MIP_LEVELS,
                .baseArrayLayer = 0,
                .layerCount = VK_REMAINING_ARRAY_LAYERS,
            },
    };

    VkDependencyInfo dep_info = {.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                                 .imageMemoryBarrierCount = 1,
                                 .pImageMemoryBarriers = &image_barrier};
    vkCmdPipelineBarrier2(m_buffer, &dep_info);
    img.m_layout = new_layout;
}

void
CommandBuffer::begin_rendering(const RenderBeginInfo& begin_info)
{
    VkRenderingAttachmentInfo color_attachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = begin_info.color_image.m_view,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = {.color = {{0.0f, 1.0f, 0.0f, 1.0f}}},
    };

    VkRenderingInfo rendering_info = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = {.offset = {0, 0}, .extent = begin_info.color_image.m_extent},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
    };
    vkCmdBeginRendering(m_buffer, &rendering_info);
}
void
CommandBuffer::end_rendering()
{
    vkCmdEndRendering(m_buffer);
}
void
CommandBuffer::submit(std::span<Semaphore> wait_semaphores,
                      std::span<Semaphore> signal_semaphores,
                      Fence fence)
{
    std::vector<VkSemaphore> wait_handles;
    wait_handles.reserve(wait_semaphores.size());
    for (auto& s : wait_semaphores)
        wait_handles.push_back(s.semaphore());

    std::vector<VkSemaphore> signal_handles;
    signal_handles.reserve(signal_semaphores.size());
    for (auto& s : signal_semaphores)
        signal_handles.push_back(s.semaphore());

    std::vector<VkPipelineStageFlags> wait_stages(wait_handles.size(),
                                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = static_cast<uint32_t>(wait_handles.size()),
        .pWaitSemaphores = wait_handles.data(),
        .pWaitDstStageMask = wait_stages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &m_buffer,
        .signalSemaphoreCount = static_cast<uint32_t>(signal_handles.size()),
        .pSignalSemaphores = signal_handles.data(),
    };

    vkQueueSubmit(m_queue->queue(), 1, &submit_info, fence.fence());
}

} // namespace cvox