file(GLOB_RECURSE CVOX_ENGINE_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)


add_library(cvox_engine 
    STATIC 
    ${CVOX_ENGINE_SRCS}
)
add_library(cvox::engine ALIAS cvox_engine)
target_link_libraries(
    cvox_engine 
    EnTT::EnTT 
    glfw
)
target_include_directories(
    cvox_engine 
    PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
