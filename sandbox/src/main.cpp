#include <cvox/cvox.h>
#include <memory>

class SandboxApp : public cvox::Application
{
    void
    on_init() override
    {
        entity = engine().registry().create();
        engine().registry().emplace<cvox::cmp::WindowResizeCallback>(
            entity,
            (cvox::cmp::WindowResizeCallback){.on_resize = [](uint32_t width, uint32_t height) {
                cvox::log::debug("w: {}, h: {}", width, height);
            }});
    }
    entt::entity entity;
};

namespace cvox
{
std::unique_ptr<Application>
create_application()
{
    return std::make_unique<SandboxApp>();
}
}; // namespace cvox
