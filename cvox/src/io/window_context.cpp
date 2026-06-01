#include "cvox/io/window_context.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <print>

namespace cvox
{
WindowContext::WindowContext(Engine& engine) : cvox::Context(engine){};
void
WindowContext::on_init()
{
    if (!glfwInit())
    {
        /// TODO: handle error;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(m_width, m_height, "cvox", NULL, NULL);

    if (!m_window)
    {
        glfwTerminate();
        /// TODO: handle this;
    }

    glfwSetWindowUserPointer(m_window, this);
};
void
WindowContext::on_deinit()
{
    glfwTerminate();
};
bool
WindowContext::should_close()
{
    return glfwWindowShouldClose(m_window);
}
void
WindowContext::poll_events()
{
    glfwPollEvents();
}
}; // namespace cvox