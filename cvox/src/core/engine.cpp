#include "cvox/core/engine.hpp"
#include "cvox/io/window_context.hpp"

namespace cvox
{
void
Engine::init(std::unique_ptr<Application> application)
{
    m_application = std::move(application);

    register_context<WindowContext>();

    m_application->on_init();
}

void
Engine::deinit()
{
    m_application->on_deinit();
    for (auto reverse_iter = m_contexts.rbegin(); reverse_iter != m_contexts.rend(); ++reverse_iter)
    {
        reverse_iter->get()->on_deinit();
    }
}

void
Engine::run()
{
    WindowContext* window_context = get_context<WindowContext>();

    m_running = true;
    while (m_running)
    {
        window_context->poll_events();
        if (window_context->should_close())
        {
            m_running = false;
        }
    }
    deinit();
}

} // namespace cvox