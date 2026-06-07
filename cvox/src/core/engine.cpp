#include "cvox/core/engine.hpp"
#include "cvox/io/window_context.hpp"
#include "cvox/rendering/render_context.hpp"
#include "cvox/rendering/vulkan_context.hpp"

namespace cvox
{
void
Engine::init(std::unique_ptr<Application> application)
{
    m_application = std::move(application);
    m_application->m_engine = this;

    /// Contexts(subsystems)
    register_context<WindowContext>();
    register_context<VulkanContext>();
    register_context<RenderContext>();

    m_application->on_init();
}

void
Engine::deinit()
{
    m_application->on_deinit();
    /// Reverse-order deinitialisation of contexts.
    for (auto reverse_iter = m_contexts.rbegin(); reverse_iter != m_contexts.rend(); ++reverse_iter)
    {
        reverse_iter->get()->on_deinit();
    }
}

void
Engine::run()
{
    WindowContext* window_context = get_context<WindowContext>();
    RenderContext* render_context = get_context<RenderContext>();

    m_running = true;
    while (m_running)
    {
        window_context->poll_events();
        if (window_context->should_close())
        {
            m_running = false;
        }
        render_context->draw_frame();
    }
    deinit();
}

} // namespace cvox