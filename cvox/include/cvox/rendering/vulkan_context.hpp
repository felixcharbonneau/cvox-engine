#pragma once
#include "cvox/core/context.hpp"
#include <vulkan/vulkan.h>

namespace cvox
{
struct QueueFamilyIndices;
class CVOX_API VulkanContext : public Context
{
public:
    VulkanContext(Engine& engine);

    void on_init() override;
    void on_deinit() override;

private:
    constexpr static bool layers_enabled{true};

    void create_instance();
    void create_debug_messenger();
    void pick_physical_device();
    void create_logical_device();
    QueueFamilyIndices get_queue_indices();

    VkInstance m_instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debbuger{VK_NULL_HANDLE};
    VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
    VkDevice m_device{VK_NULL_HANDLE};
};
}; // namespace cvox