#include "cvox/rendering/vulkan_context.hpp"
#include "cvox/core/logger.h"
#include <cassert>
#include <optional>
#include <print>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "cvox/components/engine_components.hpp"
#include "cvox/core/engine.hpp"
#include "cvox/io/window_context.hpp"

namespace cvox
{

struct QueueFamilyIndices
{
public:
    std::optional<uint32_t> graphics_index;
    std::optional<uint32_t> present_index;

    bool
    complete()
    {
        return graphics_index.has_value() && present_index.has_value();
    }
};

VulkanContext::VulkanContext(Engine& engine) : Context(engine), m_swapchain(engine)
{
}

Image
VulkanContext::create_image(const Image::CreateInfo& create_info)
{
    VkImage img;
    Image output;
    output.m_extent = create_info.extent;

    if (create_info.img == VK_NULL_HANDLE)
    {
        output.m_owns_image = true;
        /// TODO: create image
    }
    else
    {
        output.m_owns_image = false;
        img = create_info.img;
    }
    output.m_image = img;

    VkImageViewCreateInfo view_create_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = img,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = create_info.format,
        .components =
            {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };
    VkResult result = vkCreateImageView(m_device, &view_create_info, NULL, &output.m_view);
    assert(result == VK_SUCCESS);

    return output;
}

Semaphore
VulkanContext::create_semaphore(const Semaphore::CreateInfo& create_info)
{
    Semaphore semaphore;
    VkSemaphoreCreateInfo semaphore_info = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    vkCreateSemaphore(m_device, &semaphore_info, NULL, &semaphore.m_semaphore);
    return semaphore;
}

Fence
VulkanContext::create_fence(const Fence::CreateInfo& create_info)
{
    Fence fence;
    VkFenceCreateFlags flags = 0;
    if (create_info.signaled)
    {
        flags |= VK_FENCE_CREATE_SIGNALED_BIT;
    }
    VkFenceCreateInfo fence_info = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = flags,
    };
    vkCreateFence(m_device, &fence_info, NULL, &fence.m_fence);
    return fence;
}
CommandPool
VulkanContext::create_command_pool(const CommandPool::CreateInfo& create_info)
{
    CommandPool command_pool = CommandPool(this);
    VkCommandPoolCreateInfo poolInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                                     .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                     .queueFamilyIndex = create_info.queue_family_index};
    vkCreateCommandPool(m_device, &poolInfo, NULL, &command_pool.m_pool);
    return command_pool;
}

void
VulkanContext::create_instance()
{
    VkApplicationInfo app_info = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                  .apiVersion = VK_API_VERSION_1_3};
    std::vector<const char*> layers;
    std::vector<const char*> extensions;
    if constexpr (layers_enabled)
    {
        layers.push_back("VK_LAYER_KHRONOS_validation");
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    for (size_t i = 0; i < glfw_extension_count; i++)
    {
        extensions.push_back(glfw_extensions[i]);
    }

    VkInstanceCreateInfo create_info = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                        .pApplicationInfo = &app_info,
                                        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
                                        .ppEnabledLayerNames = layers.data(),
                                        .enabledExtensionCount =
                                            static_cast<uint32_t>(extensions.size()),
                                        .ppEnabledExtensionNames = extensions.data()};
    VkResult result = vkCreateInstance(&create_info, NULL, &m_instance);
    /// TODO: handle result
    result = glfwCreateWindowSurface(
        m_instance, engine().get_context<WindowContext>()->get_glfw_window(), NULL, &m_surface);
    /// TODO: handle result
}

static VkBool32
msg_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
             VkDebugUtilsMessageTypeFlagsEXT messageTypes,
             const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
             void* pUserData)
{
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        log::trace("[VALIDATION]{}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        log::info("[VALIDATION]{}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        log::warning("[VALIDATION]{}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        log::error("[VALIDATION]{}", pCallbackData->pMessage);
        break;
    default:
        break;
    };
    return VK_FALSE;
}

void
VulkanContext::create_debug_messenger()
{
    if constexpr (!layers_enabled)
    {
        return;
    }
    PFN_vkCreateDebugUtilsMessengerEXT fn =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance,
                                                                  "vkCreateDebugUtilsMessengerEXT");
    assert(fn);
    VkDebugUtilsMessengerCreateInfoEXT create_info{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = msg_callback};
    VkResult result = fn(m_instance, &create_info, NULL, &m_debbuger);
    /// TODO: handle result
}

void
VulkanContext::pick_physical_device()
{
    uint32_t pdev_count = 0;
    vkEnumeratePhysicalDevices(m_instance, &pdev_count, NULL);
    std::vector<VkPhysicalDevice> pdevs;
    pdevs.resize(pdev_count);
    vkEnumeratePhysicalDevices(m_instance, &pdev_count, pdevs.data());

    assert(pdev_count > 0);
    for (auto pdev : pdevs)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(pdev, &props);
        QueueFamilyIndices queue_indices = get_queue_indices(pdev);
        if (queue_indices.complete())
        {
            m_physical_device = pdev;
            break;
        }
    }

    /// TODO: make sure the device suits the application.
}

[[nodiscard]] QueueFamilyIndices
VulkanContext::get_queue_indices(VkPhysicalDevice p_dev)
{
    QueueFamilyIndices indices = {};

    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(p_dev, &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_props;
    queue_props.resize(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(p_dev, &queue_count, queue_props.data());
    for (auto [i, queue] : std::views::enumerate(queue_props))
    {
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_index = i;
            break;
        }
    }
    if (!indices.graphics_index.has_value())
    {
        return indices;
    }
    VkBool32 present_support = VK_FALSE;
    vkGetPhysicalDeviceSurfaceSupportKHR(
        p_dev, indices.graphics_index.value(), m_surface, &present_support);
    if (present_support == VK_TRUE)
    {
        indices.present_index = indices.graphics_index;
    }
    else
    {
        for (auto [i, queue] : std::views::enumerate(queue_props))
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(p_dev, i, m_surface, &present_support);
            if (present_support == VK_TRUE)
            {
                indices.present_index = i;
                break;
            }
        }
    }

    return indices;
}

void
VulkanContext::create_logical_device()
{
    float priority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    QueueFamilyIndices queue_family_indices = get_queue_indices(m_physical_device);
    queue_create_infos.push_back({
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queue_family_indices.graphics_index.value(),
        .queueCount = 1,
        .pQueuePriorities = &priority,

    });
    if (queue_family_indices.present_index.value() != queue_family_indices.graphics_index.value())
    {
        queue_create_infos.push_back({
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queue_family_indices.present_index.value(),
            .queueCount = 1,
            .pQueuePriorities = &priority,
        });
    }

    VkPhysicalDeviceVulkan12Features vulkan_12_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
        .bufferDeviceAddress = VK_TRUE,
    };

    VkPhysicalDeviceVulkan13Features vulkan_13_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &vulkan_12_features,
        .synchronization2 = VK_TRUE,

        .dynamicRendering = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 features_2 = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                                            .pNext = &vulkan_13_features};

    std::vector<const char*> extensions;
    extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features_2,
        .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledLayerCount = 0,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()};

    VkResult result = vkCreateDevice(m_physical_device, &create_info, NULL, &m_device);

    m_graphics_queue.m_queue_index = queue_family_indices.graphics_index.value();
    vkGetDeviceQueue(m_device, m_graphics_queue.m_queue_index, 0, &m_graphics_queue.m_queue);
    m_present_queue.m_queue_index = queue_family_indices.present_index.value();
    vkGetDeviceQueue(m_device, m_present_queue.m_queue_index, 0, &m_present_queue.m_queue);
}

void
VulkanContext::on_init()
{
    create_instance();
    create_debug_messenger();
    pick_physical_device();
    create_logical_device();
    m_swapchain.init();

    engine().registry().emplace<cmp::WindowResizeCallback>(
        entity(), [&](uint32_t, uint32_t) { this->m_swapchain.m_dirty = true; });
}

void
VulkanContext::wait_idle()
{
    vkDeviceWaitIdle(m_device);
}
void
VulkanContext::wait_for_fence(Fence fence)
{
    vkWaitForFences(m_device, 1, &fence.m_fence, VK_TRUE, UINT64_MAX);
}
void
VulkanContext::reset_fence(Fence fence)
{
    vkResetFences(m_device, 1, &fence.m_fence);
}

void
VulkanContext::on_deinit()
{
    m_swapchain.deinit();
    if (m_device)
    {
        wait_idle();
        vkDestroyDevice(m_device, NULL);
    }
    if (m_debbuger != VK_NULL_HANDLE)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT fn =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                m_instance, "vkDestroyDebugUtilsMessengerEXT");
        fn(m_instance, m_debbuger, NULL);
    }
    if (m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(m_instance, m_surface, NULL);
    }
    if (m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_instance, NULL);
    }
}

bool
VulkanContext::present(SwapchainImage& img, std::span<Semaphore> wait_semaphores)
{
    auto handles = wait_semaphores | std::views::transform(&Semaphore::m_semaphore);
    std::vector<VkSemaphore> semaphores(handles.begin(), handles.end());
    semaphores.push_back(m_swapchain.m_render_finished_semaphores[img.image_index].m_semaphore);

    VkPresentInfoKHR present_info = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = static_cast<uint32_t>(semaphores.size()),
        .pWaitSemaphores = semaphores.data(),
        .swapchainCount = 1,
        .pSwapchains = &m_swapchain.m_swapchain,
        .pImageIndices = &img.image_index,
    };
    vkQueuePresentKHR(m_present_queue.m_queue, &present_info);

    return true;
}
void
VulkanContext::recreate_swapchain_if_needed()
{
    if (m_swapchain.m_dirty)
    {
        m_swapchain.recreate();
        m_swapchain.m_dirty = false;
    }
}

} // namespace cvox