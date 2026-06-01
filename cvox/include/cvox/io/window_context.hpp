#pragma once
#include "cvox/core/context.hpp"
#include <cstdint>

class GLFWwindow;

namespace cvox
{
class CVOX_API WindowContext : public Context
{
public:
    WindowContext(Engine& engine);
    void on_init() override;
    void on_deinit() override;

    bool should_close();

    void poll_events();

private:
    GLFWwindow* m_window;

    uint32_t m_width = 800, m_height = 800;
};
} // namespace cvox