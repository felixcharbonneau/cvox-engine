#include "cvox/io/window_context.hpp"
#define GLFW_INCLUDE_VULKAN
#include "cvox/components/engine_components.hpp"
#include "cvox/core/engine.hpp"
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
    glfwSetWindowSizeCallback(
        m_window,
        [](GLFWwindow* window, int width, int height)
        {
            WindowContext* window_context = (WindowContext*)glfwGetWindowUserPointer(window);
            auto resize_callbacks =
                window_context->engine().registry().view<cmp::WindowResizeCallback>();
            for (auto [e, callback_component] : resize_callbacks->each())
            {
                if (callback_component.on_resize)
                {
                    callback_component.on_resize(width, height);
                }
            }
        });
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
void
WindowContext::get_framebuffer_size(int* width, int* height) const noexcept
{
    glfwGetFramebufferSize(m_window, width, height);
}
}; // namespace cvox