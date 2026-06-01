file(GLOB_RECURSE CVOX_SANDBOX_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

# debugging purposes
message("src files:")
foreach(file ${CVOX_SANDBOX_SRCS})
    message(STATUS ${file})
endforeach()

add_executable(cvox_sandbox ${CVOX_SANDBOX_SRCS})
target_link_libraries(cvox_sandbox cvox::engine)