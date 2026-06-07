#include "cvox/rendering/render_context.hpp"
#include "cvox/core/engine.hpp"
#include "cvox/rendering/vulkan_context.hpp"

namespace cvox
{
RenderContext::RenderContext(Engine& engine)
    : Context(engine), m_command_pool(engine.get_context<VulkanContext>())
{
}

void
RenderContext::on_init()
{
    VulkanContext* vk = engine().get_context<VulkanContext>();

    m_command_pool = vk->create_command_pool({
        .queue_family_index = vk->graphics_family_index(),
    });
    for (auto& frame_resource : m_frame_resources)
    {
        frame_resource.draw_fence = vk->create_fence({});
        frame_resource.image_available_semaphore = vk->create_semaphore({});
        frame_resource.cmd = m_command_pool.create_command_buffer({.queue = vk->graphics_queue()});
    }
}
void
RenderContext::on_deinit()
{
}

void
RenderContext::draw_frame()
{
    VulkanContext* vk = engine().get_context<VulkanContext>();
    vk->recreate_swapchain_if_needed();

    FrameResource& resource = m_frame_resources[m_current_frame_in_flight];
    CommandBuffer& cmd = resource.cmd;

    vk->wait_for_fence(resource.draw_fence);
    vk->reset_fence(resource.draw_fence);

    cmd.reset();
    SwapchainImage image = vk->next_image(resource.image_available_semaphore);

    cmd.begin();

    cmd.transition_image(image.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    cmd.begin_rendering({.color_image = image.image});
    cmd.end_rendering();
    cmd.transition_image(image.image, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    cmd.end();

    std::array wait{resource.image_available_semaphore};
    std::array signal{vk->render_finished(image.image_index)};
    cmd.submit(wait, signal, resource.draw_fence);

    if (!vk->present(image))
    {
        vk->swapchain_needs_recreation();
    }

    m_current_frame_in_flight = (m_current_frame_in_flight + 1) % m_frames_in_flight;
}

} // namespace cvox