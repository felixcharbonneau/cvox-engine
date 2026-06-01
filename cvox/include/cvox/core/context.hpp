#pragma once
#include "cvox/core.h"
#include <concepts>
#include <cstddef>

namespace cvox
{
class Engine;
class CVOX_API Context
{
public:
    friend class Engine;
    Context(Engine& engine) : m_engine(engine){};

    virtual void on_init() {};
    virtual void on_deinit() {};

    template<std::derived_from<Context> T>
    static std::size_t
    type_id()
    {
        static const std::size_t id = next_type_id();
        return id;
    }

private:
    inline static std::size_t
    next_type_id()
    {
        static std::size_t counter = 0;
        return counter++;
    }

    Engine& m_engine;
};
}; // namespace cvox