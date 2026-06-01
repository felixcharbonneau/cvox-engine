#pragma once
#include "application.hpp"
#include "context.hpp"
#include "cvox/core.h"
#include <cassert>
#include <memory>
#include <vector>

namespace cvox
{
class CVOX_API Engine
{
public:
    explicit Engine() = default;

    Engine(const Engine&) = delete;
    Engine operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine operator=(Engine&&) = delete;

    void init(std::unique_ptr<Application> application);

    void run();

    template<std::derived_from<Context> T, typename... Args> void register_context(Args&&... args);
    template<std::derived_from<Context> T> [[nodiscard]] T* get_context();

private:
    void deinit();
    std::unique_ptr<Application> m_application;

    std::vector<std::unique_ptr<Context>> m_contexts;
    bool m_running = false;
};

template<std::derived_from<Context> T, typename... Args>
void
Engine::register_context(Args&&... args)
{
    const std::size_t id = Context::type_id<T>();
    if (id >= m_contexts.size())
    {
        m_contexts.resize(id + 1);
    }
    std::unique_ptr<Context> ptr = std::make_unique<T>(*this, std::forward<Args>(args)...);

    ptr->on_init();
    m_contexts[id] = std::move(ptr);
}

template<std::derived_from<Context> T>
T*
Engine::get_context()
{
    const std::size_t id = Context::type_id<T>();
    assert(id <= m_contexts.size());
    return static_cast<T*>(m_contexts[id].get());
}

} // namespace cvox