find_package(glfw3 REQUIRED)

add_library(vortex2d_window STATIC
        "RenderWindow.h"
        "RenderWindow.cpp"
        "GLFW.h"
        "GLFW.cpp")

target_compile_features(vortex2d_window PRIVATE cxx_nullptr)
target_link_libraries(vortex2d_window PUBLIC vortex2d glfw)
target_include_directories(vortex2d_window PUBLIC .)

if(APPLE)
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(CORE_VIDEO_FRAMEWORK CoreVideo)

    target_link_libraries(vortex2d_window ${COCOA_FRAMEWORK} ${IOKIT_FRAMEWORK} ${CORE_VIDEO_FRAMEWORK})
endif()

source_group(Examples REGULAR_EXPRESSION ".*(h|cpp)")
