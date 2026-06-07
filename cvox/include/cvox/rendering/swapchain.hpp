#pragma once
#include "cvox/core.h"
#include "cvox/core/engine.hpp"
#include "image.hpp"
#include "sync.hpp"
#include <vulkan/vulkan.h>

namespace cvox
{
struct CVOX_API SwapchainImage
{
    Image& image;
    uint32_t image_index;
};
class CVOX_API Swapchain
{
    struct CVOX_API SupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

public:
    friend class VulkanContext;
    /// this must not do any vulkan tasks, as the VkContext isnt initialized yet.
    Swapchain(Engine& engine) : m_engine(engine){};

    SwapchainImage next_image(Semaphore wait_semaphore);

    void init();
    void deinit();
    void recreate();

private:
    void create();
    void free();
    VkSwapchainKHR m_swapchain;
    std::vector<Image> m_images;
    std::vector<Semaphore> m_render_finished_semaphores;

    VkSurfaceFormatKHR m_format;
    VkExtent2D m_extent;
    VkPresentModeKHR m_present_mode;

    static SupportDetails query_swapchain_support(VkPhysicalDevice pdev, VkSurfaceKHR surface);

    Engine& m_engine;
    bool m_dirty = false;
};
} // namespace cvox