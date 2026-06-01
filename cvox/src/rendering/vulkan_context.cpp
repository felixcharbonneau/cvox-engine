#include "cvox/rendering/vulkan_context.hpp"
#include "cvox/core/logger.h"
#include <cassert>
#include <optional>
#include <print>
#include <vector>

namespace cvox
{

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphics_index;
    std::optional<uint32_t> present_index;
};

VulkanContext::VulkanContext(Engine& engine) : Context(engine)
{
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

    VkInstanceCreateInfo create_info = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                        .pApplicationInfo = &app_info,
                                        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
                                        .ppEnabledLayerNames = layers.data(),
                                        .enabledExtensionCount =
                                            static_cast<uint32_t>(extensions.size()),
                                        .ppEnabledExtensionNames = extensions.data()};
    VkResult result = vkCreateInstance(&create_info, NULL, &m_instance);
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

    for (auto pdev : pdevs)
    {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(pdev, &props);
        log::debug("device found: {}", props.deviceName);
    }

    /// TODO: make sure the device suits the application.
    assert(pdev_count > 0);
    m_physical_device = pdevs[0];
}

QueueFamilyIndices
VulkanContext::get_queue_indices()
{
}

void
VulkanContext::create_logical_device()
{
    VkDeviceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    };

    VkResult result = vkCreateDevice(m_physical_device, &create_info, NULL, &m_device);
}

void
VulkanContext::on_init()
{
    create_instance();
    create_debug_messenger();
    pick_physical_device();
}

void
VulkanContext::on_deinit()
{
    if (m_debbuger != VK_NULL_HANDLE)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT fn =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                m_instance, "vkDestroyDebugUtilsMessengerEXT");
        fn(m_instance, m_debbuger, NULL);
    }
    if (m_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_instance, NULL);
    }
}

} // namespace cvox