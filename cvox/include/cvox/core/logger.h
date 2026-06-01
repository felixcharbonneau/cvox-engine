#pragma once
#include <print>

namespace cvox
{
namespace log
{
enum Level
{
    ERROR = 1,
    WARNING = 2,
    INFO = 3,
    DEBUG = 4,
    TRACE = 5
};
constexpr Level g_level = TRACE;

namespace internal
{
template<typename... Args>
inline void
println(std::format_string<Args...> fmt, Args&&... args)
{
    std::println(fmt, std::forward<Args>(args)...);
}
}; // namespace internal

template<typename... Args>
inline void
error(std::format_string<Args...> fmt, Args&&... args)
{
    if constexpr (g_level >= ERROR)
    {
        internal::println(fmt, std::forward<Args>(args)...);
    }
}
template<typename... Args>
inline void
warning(std::format_string<Args...> fmt, Args&&... args)
{
    if constexpr (g_level >= WARNING)
    {
        internal::println(fmt, std::forward<Args>(args)...);
    }
}
template<typename... Args>
inline void
info(std::format_string<Args...> fmt, Args&&... args)
{
    if constexpr (g_level >= INFO)
    {
        internal::println(fmt, std::forward<Args>(args)...);
    }
}
template<typename... Args>
inline void
debug(std::format_string<Args...> fmt, Args&&... args)
{
    if constexpr (g_level >= DEBUG)
    {
        internal::println(fmt, std::forward<Args>(args)...);
    }
}
template<typename... Args>
inline void
trace(std::format_string<Args...> fmt, Args&&... args)
{
    if constexpr (g_level >= TRACE)
    {
        internal::println(fmt, std::forward<Args>(args)...);
    }
}
} // namespace log

} // namespace cvox