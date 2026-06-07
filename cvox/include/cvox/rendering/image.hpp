#pragma once
#include "cvox/core.h"
#include <vulkan/vulkan.h>

namespace cvox
{
class CVOX_API Image
{
public:
    friend class CommandBuffer;
    struct CVOX_API CreateInfo
    {
        VkImage img{VK_NULL_HANDLE};
        VkFormat format{VK_FORMAT_UNDEFINED};
        VkExtent2D extent; /// Must be valid extent
    };

    friend class VulkanContext;
    [[nodiscard]] VkImage
    image() noexcept
    {
        return m_image;
    }

    [[nodiscard]] VkImageView
    view() noexcept
    {
        return m_view;
    }
    [[nodiscard]] VkImageLayout
    layout() noexcept
    {
        return m_layout;
    }
    [[nodiscard]] VkExtent2D
    extent() noexcept
    {
        return m_extent;
    }

private:
    Image() = default;

    VkExtent2D m_extent;
    VkImage m_image{VK_NULL_HANDLE};
    VkImageView m_view{VK_NULL_HANDLE};
    VkImageLayout m_layout{VK_IMAGE_LAYOUT_UNDEFINED};

    bool m_owns_image{false};
};
} // namespace cvox