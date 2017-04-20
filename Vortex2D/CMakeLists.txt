file(GLOB LIB_SOURCES
        "Vortex2D.h"
        "Engine/*.cpp"
        "Engine/*.h"
        "Engine/LinearSolver/*.h"
        "Engine/LinearSolver/*.cpp"
        "Renderer/*.cpp"
        "Renderer/*.h")

find_package(Vulkan)

add_library(vortex2d STATIC ${LIB_SOURCES})
target_link_libraries(vortex2d PUBLIC glm ${Vulkan_LIBRARIES})
target_include_directories(vortex2d PUBLIC ../ ${Vulkan_INCLUDE_DIRS})

target_compile_features(vortex2d PRIVATE cxx_variadic_templates)
