#pragma once
#include "commands.hpp"
#include "cvox/core/context.hpp"
#include "image.hpp"
#include "swapchain.hpp"
#include "sync.hpp"
#include <vulkan/vulkan.h>

namespace cvox
{
class CVOX_API Queue
{
public:
    friend class VulkanContext;
    [[nodiscard]] VkQueue
    queue() noexcept
    {
        return m_queue;
    }
    [[nodiscard]] uint32_t
    family_index() noexcept
    {
        return m_family_index;
    }
    [[nodiscard]] uint32_t
    queue_index() noexcept
    {
        return m_queue_index;
    }

private:
    uint32_t m_family_index{0};
    uint32_t m_queue_index{0};
    VkQueue m_queue{VK_NULL_HANDLE};
};

struct QueueFamilyIndices;
class CVOX_API VulkanContext : public Context
{
public:
    VulkanContext(Engine& engine);

    void on_init() override;
    void on_deinit() override;

    /// @brief Calls vkDeviceWait idle, which makes the cpu wait for the gpu to complete its tasks.
    void wait_idle();
    void wait_for_fence(Fence fence);
    void reset_fence(Fence fence);
    bool present(SwapchainImage& img, std::span<Semaphore> wait_semaphores = {});

    Image create_image(const Image::CreateInfo& create_info);
    Semaphore create_semaphore(const Semaphore::CreateInfo& create_info);
    Fence create_fence(const Fence::CreateInfo& create_info);
    CommandPool create_command_pool(const CommandPool::CreateInfo& create_info);
    [[nodiscard]] Semaphore
    render_finished(uint32_t image_index)
    {
        return m_swapchain.m_render_finished_semaphores[image_index];
    }

    void recreate_swapchain_if_needed();
    void
    swapchain_needs_recreation()
    {
        m_swapchain.m_dirty = true;
    }

    /// Getters
    [[nodiscard]] VkDevice&
    device() noexcept
    {
        return m_device;
    }
    [[nodiscard]] VkPhysicalDevice&
    physical_device() noexcept
    {
        return m_physical_device;
    }
    [[nodiscard]] VkInstance&
    instance() noexcept
    {
        return m_instance;
    }
    [[nodiscard]] Queue&
    graphics_queue() noexcept
    {
        return m_graphics_queue;
    }
    [[nodiscard]] Queue&
    present_queue() noexcept
    {
        return m_present_queue;
    }
    [[nodiscard]] VkSurfaceKHR&
    window_surface() noexcept
    {
        return m_surface;
    }

    [[nodiscard]] SwapchainImage
    next_image(Semaphore wait_semaphore) noexcept
    {
        return m_swapchain.next_image(wait_semaphore);
    }

    [[nodiscard]] uint32_t
    graphics_family_index() noexcept
    {
        return m_graphics_queue.m_family_index;
    }

private:
    constexpr static bool layers_enabled{true};

    void create_instance();
    void create_debug_messenger();
    void pick_physical_device();
    void create_logical_device();
    [[nodiscard]] QueueFamilyIndices get_queue_indices(VkPhysicalDevice p_dev);

    VkInstance m_instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debbuger{VK_NULL_HANDLE};
    VkPhysicalDevice m_physical_device{VK_NULL_HANDLE};
    VkDevice m_device{VK_NULL_HANDLE};

    Swapchain m_swapchain;
    VkSurfaceKHR m_surface{VK_NULL_HANDLE};

    Queue m_graphics_queue, m_present_queue;
};
}; // namespace cvox