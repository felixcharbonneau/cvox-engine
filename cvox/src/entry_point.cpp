#include "cvox/core/application.hpp"
#include "cvox/core/engine.hpp"
#include <memory>

namespace cvox
{
extern std::unique_ptr<Application> create_application();

int
entry()
{
    std::unique_ptr<Application> app = create_application();
    Engine engine = Engine();
    engine.init(std::move(app));
    engine.run();

    return 0;
}

} // namespace cvox

int
main()
{
    return cvox::entry();
}