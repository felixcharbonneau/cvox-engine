#include "cvox/rendering/swapchain.hpp"
#include "cvox/io/window_context.hpp"
#include "cvox/rendering/vulkan_context.hpp"

namespace cvox
{

Swapchain::SupportDetails
Swapchain::query_swapchain_support(VkPhysicalDevice pdev, VkSurfaceKHR surface)
{
    Swapchain::SupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pdev, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(pdev, surface, &format_count, nullptr);

    if (format_count != 0)
    {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(pdev, surface, &format_count, details.formats.data());
    }
    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pdev, surface, &present_mode_count, nullptr);

    if (present_mode_count != 0)
    {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            pdev, surface, &present_mode_count, details.present_modes.data());
    }

    return details;
}
void
Swapchain::init()
{
    create();
}
void
Swapchain::deinit()
{
    free();
}
void
Swapchain::recreate()
{
    m_engine.get_context<VulkanContext>()->wait_idle();
    free();
    create();
}

void
Swapchain::create()
{
    VulkanContext* context = m_engine.get_context<VulkanContext>();

    Swapchain::SupportDetails support_details =
        query_swapchain_support(context->physical_device(), context->window_surface());

    m_format = support_details.formats[0];
    for (auto format : support_details.formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            m_format = format;
            break;
        }
    }
    m_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (auto present_mode : support_details.present_modes)
    {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            m_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }
    /// Swapchain image extent.
    if (support_details.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        m_extent = support_details.capabilities.currentExtent;
    }
    else
    {
        int width, height;
        m_engine.get_context<WindowContext>()->get_framebuffer_size(&width, &height);

        m_extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height),
        };
        m_extent.width = std::clamp(m_extent.width,
                                    support_details.capabilities.minImageExtent.width,
                                    support_details.capabilities.maxImageExtent.width);
        m_extent.height = std::clamp(m_extent.height,
                                     support_details.capabilities.minImageExtent.height,
                                     support_details.capabilities.maxImageExtent.height);
    }
    uint32_t image_count = support_details.capabilities.minImageCount + 1;
    if (support_details.capabilities.maxImageCount > 0 &&
        image_count > support_details.capabilities.maxImageCount)
    {
        image_count = support_details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = context->window_surface(),
        .minImageCount = image_count,
        .imageFormat = m_format.format,
        .imageColorSpace = m_format.colorSpace,
        .imageExtent = m_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = support_details.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = m_present_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = nullptr,
    };

    Queue& graphics = context->graphics_queue();
    Queue& present = context->present_queue();
    if (graphics.family_index() == present.family_index())
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        uint32_t queue_family_indices[] = {graphics.family_index(), present.family_index()};
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    VkResult result = vkCreateSwapchainKHR(context->device(), &create_info, nullptr, &m_swapchain);
    /// TODO: check result

    m_dirty = false;

    std::vector<VkImage> imgs;
    uint32_t img_count;
    vkGetSwapchainImagesKHR(context->device(), m_swapchain, &img_count, nullptr);
    imgs.resize(img_count);
    vkGetSwapchainImagesKHR(context->device(), m_swapchain, &img_count, imgs.data());

    m_images.clear();
    m_images.reserve(img_count);
    for (auto img : imgs)
    {
        m_images.push_back(context->create_image(
            Image::CreateInfo{.img = img, .format = m_format.format, .extent = m_extent}));
    }
    m_render_finished_semaphores.clear();
    for (size_t i = 0; i < img_count; i++)
    {
        m_render_finished_semaphores.push_back(context->create_semaphore({}));
    }
}
void
Swapchain::free()
{
    VulkanContext* context = m_engine.get_context<VulkanContext>();
    vkDestroySwapchainKHR(context->device(), m_swapchain, nullptr);
}

SwapchainImage
Swapchain::next_image(Semaphore wait_semaphore)
{
    uint32_t image_index;
    VulkanContext* vk = m_engine.get_context<VulkanContext>();

    vkAcquireNextImageKHR(vk->device(),
                          m_swapchain,
                          UINT64_MAX,
                          wait_semaphore.semaphore(),
                          VK_NULL_HANDLE,
                          &image_index);
    return {
        .image = m_images[image_index],
        .image_index = image_index,
    };
}

}; // namespace cvox