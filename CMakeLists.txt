cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(cvox
    VERSION 1.0
)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/deps.cmake")

add_subdirectory(cvox)
add_subdirectory(sandbox)
