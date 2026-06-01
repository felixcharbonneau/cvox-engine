#pragma once
#include "cvox/core.h"

namespace cvox
{

class Engine;

class CVOX_API Application
{
public:
    friend class Engine;
    Application() = default;

    /// Getters
    Engine&
    engine()
    {
        return *m_engine;
    }

protected:
    virtual void
    on_init()
    {
    }
    virtual void
    on_deinit()
    {
    }

private:
    Engine* m_engine;
};

} // namespace cvox
