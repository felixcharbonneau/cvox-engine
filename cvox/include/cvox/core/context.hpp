#pragma once
#include "cvox/core.h"
#include <concepts>
#include <cstddef>
#include <entt/entt.hpp>

namespace cvox
{
class Engine;
class CVOX_API Context
{
public:
    friend class Engine;
    Context(Engine& engine) : m_engine(engine){};

    Context(Context&) = delete;
    Context operator=(Context&) = delete;
    Context(const Context&&) = delete;
    Context operator=(const Context&&) = delete;

    template<std::derived_from<Context> T>
    [[nodiscard]] static std::size_t
    type_id()
    {
        static const std::size_t id = next_type_id();
        return id;
    }

    [[nodiscard]] Engine&
    engine() noexcept
    {
        return m_engine;
    }

    [[nodiscard]] entt::entity
    entity() noexcept
    {
        return m_entity;
    }

protected:
    virtual void on_init() {};
    virtual void on_deinit() {};

private:
    [[nodiscard]] inline static std::size_t
    next_type_id()
    {
        static std::size_t counter = 0;
        return counter++;
    }

    entt::entity m_entity;
    Engine& m_engine;
};
}; // namespace cvox