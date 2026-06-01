#include <cvox/cvox.h>
#include <memory>

class SandboxApp : public cvox::Application
{
    void
    on_init() override
    {
    }
};

namespace cvox
{
std::unique_ptr<Application>
create_application()
{
    return std::make_unique<SandboxApp>();
}
}; // namespace cvox
