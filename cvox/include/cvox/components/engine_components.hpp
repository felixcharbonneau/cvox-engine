#pragma once
#include "cvox/core.h"
#include <cstdint>
#include <functional>

namespace cvox
{
namespace cmp
{
/// Window callbacks
struct CVOX_API WindowResizeCallback
{
    std::function<void(uint32_t width, uint32_t height)> on_resize;
};

} // namespace cmp
}; // namespace cvox