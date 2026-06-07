#pragma once
#include "cvox/core/context.hpp"
#include "sync.hpp"
#include "vulkan_context.hpp"
#include <array>

namespace cvox
{
class CVOX_API RenderContext : public Context
{
    struct FrameResource
    {
        Semaphore image_available_semaphore;
        Fence draw_fence;
        CommandBuffer cmd;
    };

public:
    void on_init() override;
    void on_deinit() override;

    [[nodiscard]] uint32_t
    frames_in_flight() const noexcept
    {
        return m_frames_in_flight;
    }

    RenderContext(Engine& engine);
    void draw_frame();

private:
    constexpr static uint32_t m_frames_in_flight{3};
    std::array<FrameResource, m_frames_in_flight> m_frame_resources;
    uint32_t m_current_frame_in_flight{0};

    CommandPool m_command_pool;
};
} // namespace cvox