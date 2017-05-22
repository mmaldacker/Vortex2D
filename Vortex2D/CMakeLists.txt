file(GLOB LIB_SOURCES
    "Vortex2D.h"
#    "Engine/*.cpp"
#    "Engine/*.h"
#    "Engine/LinearSolver/*.h"
#    "Engine/LinearSolver/*.cpp"
    "Renderer/*.cpp"
    "Renderer/*.h")

file(GLOB SHADER_SOURCES
    "Renderer/Kernels/*.vert"
    "Renderer/Kernels/*.frag")

find_package(Vulkan)

# TODO this only works for Win32
find_program(GLSL_VALIDATOR glslangValidator hints "$ENV{VULKAN_SDK}/Bin")

# TODO extract this in a function
foreach(SHADER_SOURCE ${SHADER_SOURCES})
    get_filename_component(SHADER_FILENAME ${SHADER_SOURCE} NAME)
    set(SHADER "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_FILENAME}.spv")
    add_custom_command(
        OUTPUT ${SHADER}
        COMMAND ${GLSL_VALIDATOR} -V ${SHADER_SOURCE} -o ${SHADER}
        DEPENDS ${SHADER_SOURCE})
    list(APPEND SHADERS ${SHADER})
endforeach(SHADER_SOURCE)

add_library(vortex2d STATIC ${LIB_SOURCES} ${SHADERS} ${SHADER_SOURCES})
target_link_libraries(vortex2d PUBLIC glm ${Vulkan_LIBRARIES})
target_include_directories(vortex2d PUBLIC ../ ${Vulkan_INCLUDE_DIRS})

target_compile_features(vortex2d PRIVATE cxx_variadic_templates)
