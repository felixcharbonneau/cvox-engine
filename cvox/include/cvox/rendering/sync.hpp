#pragma once
#include "cvox/core.h"
#include <vulkan/vulkan.h>

namespace cvox
{
class CVOX_API Semaphore
{
public:
    struct CreateInfo
    {
    };
    friend class VulkanContext;
    friend class RenderContext;
    [[nodiscard]] VkSemaphore
    semaphore() noexcept
    {
        return m_semaphore;
    }

private:
    Semaphore() = default;
    VkSemaphore m_semaphore{VK_NULL_HANDLE};
};

class CVOX_API Fence
{
public:
    struct CreateInfo
    {
        bool signaled{true}; //< Weither the fence is created in a signaled state, true by detfault.
    };
    friend class VulkanContext;
    friend class RenderContext;
    [[nodiscard]] VkFence
    fence() noexcept
    {
        return m_fence;
    }

private:
    Fence() = default;
    VkFence m_fence{VK_NULL_HANDLE};
};
} // namespace cvox