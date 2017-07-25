file(GLOB LIB_SOURCES
    "Vortex2D.h"
    "Engine/LevelSet.h"
    "Engine/LevelSet.cpp"
    "Engine/Pressure.h"
    "Engine/Pressure.cpp"
    "Engine/LinearSolver/LinearSolver.h"
    "Engine/LinearSolver/LinearSolver.cpp"
    "Engine/LinearSolver/Reduce.h"
    "Engine/LinearSolver/Reduce.cpp"
    "Renderer/*.cpp"
    "Renderer/*.h")

file(GLOB SHADER_SOURCES
    "Renderer/Kernels/*.vert"
    "Renderer/Kernels/*.frag"
    "Engine/Kernels/*.comp"
    "Engine/LinearSolver/Kernels/*.comp")

find_package(Vulkan)

function(compile_shader)
    set(options "")
    set(oneValueArgs TARGET)
    set(multiValueArgs SOURCES)

    cmake_parse_arguments(SHADER "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    message("Compiling sources: ${SHADER_SOURCES}")

    # TODO this only works for Win32
    find_program(GLSL_VALIDATOR glslangValidator hints "$ENV{VULKAN_SDK}/Bin")
    message("Using compiler: ${GLSL_VALIDATOR}")

    foreach(SHADER_SOURCE ${SHADER_SOURCES})
        get_filename_component(SHADER_FILENAME ${SHADER_SOURCE} NAME)
        set(SHADER "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_FILENAME}.spv")
        add_custom_command(
            OUTPUT ${SHADER}
            COMMAND ${GLSL_VALIDATOR} -V ${SHADER_SOURCE} -o ${SHADER}
            DEPENDS ${SHADER_SOURCE})
        list(APPEND SHADERS ${SHADER})
    endforeach(SHADER_SOURCE)

    add_custom_target("${SHADER_TARGET}_SHADERS" DEPENDS ${SHADERS})
    add_dependencies(${SHADER_TARGET} "${SHADER_TARGET}_SHADERS")
endfunction()

add_library(vortex2d STATIC ${LIB_SOURCES} ${SHADERS} ${SHADER_SOURCES})
compile_shader(TARGET vortex2d SOURCES ${SHADER_SOURCES})
target_link_libraries(vortex2d PUBLIC glm ${Vulkan_LIBRARIES})
target_include_directories(vortex2d PUBLIC ../ ${Vulkan_INCLUDE_DIRS})

target_compile_features(vortex2d PRIVATE cxx_variadic_templates)
