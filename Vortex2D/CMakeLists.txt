file(GLOB LIB_SOURCES
    "Vortex2D.h"
    "Engine/LevelSet.h"
    "Engine/LevelSet.cpp"
    "Engine/Pressure.h"
    "Engine/Pressure.cpp"
    "Engine/Advection.h"
    "Engine/Advection.cpp"
    "Engine/Extrapolation.h"
    "Engine/Extrapolation.cpp"
    "Engine/World.h"
    "Engine/World.cpp"
    "Engine/Size.h"
    "Engine/Size.cpp"
    "Engine/Boundaries.h"
    "Engine/Boundaries.cpp"
    "Engine/PrefixScan.h"
    "Engine/PrefixScan.cpp"
    "Engine/Particles.h"
    "Engine/Particles.cpp"
    "Engine/LinearSolver/LinearSolver.h"
    "Engine/LinearSolver/LinearSolver.cpp"
    "Engine/LinearSolver/Preconditioner.h"
    "Engine/LinearSolver/Reduce.h"
    "Engine/LinearSolver/Reduce.cpp"
    "Engine/LinearSolver/GaussSeidel.h"
    "Engine/LinearSolver/GaussSeidel.cpp"
    "Engine/LinearSolver/ConjugateGradient.h"
    "Engine/LinearSolver/ConjugateGradient.cpp"
    "Engine/LinearSolver/Diagonal.h"
    "Engine/LinearSolver/Diagonal.cpp"
    "Engine/LinearSolver/IncompletePoisson.h"
    "Engine/LinearSolver/IncompletePoisson.cpp"
    "Engine/LinearSolver/Transfer.h"
    "Engine/LinearSolver/Transfer.cpp"
    "Engine/LinearSolver/Multigrid.h"
    "Engine/LinearSolver/Multigrid.cpp"        
    "Renderer/*.cpp"
    "Renderer/*.h"
    "Utils/variant.hpp")

file(GLOB SHADER_SOURCES
    "Renderer/Kernels/*.vert"
    "Renderer/Kernels/*.frag"
    "Engine/Kernels/Advect.comp"
    "Engine/Kernels/AdvectVelocity.comp"
    "Engine/Kernels/BuildDiv.comp"
    "Engine/Kernels/BuildMatrix.comp"
    "Engine/Kernels/Extrapolate.comp"
    "Engine/Kernels/Project.comp"
    "Engine/Kernels/Redistance.comp"
    "Engine/Kernels/ConstrainVelocity.comp"
    "Engine/Kernels/ExtrapolateVelocity.comp"
    "Engine/Kernels/PolygonMin.comp"
    "Engine/Kernels/PolygonMax.comp"
    "Engine/Kernels/UpdateVertices.comp"
    "Engine/Kernels/DistanceField.frag"
    "Engine/Kernels/PreScanAdd.comp"
    "Engine/Kernels/PreScan.comp"
    "Engine/Kernels/PreScanStoreSum.comp"
    "Engine/Kernels/ParticleCount.comp"
    "Engine/Kernels/ParticleSpawn.comp"
    "Engine/Kernels/ParticleBucket.comp"
    "Engine/Kernels/ParticleCloud.vert"
    "Engine/Kernels/ParticleCloud.frag"
    "Engine/Kernels/ParticlePhi.comp"
    "Engine/Kernels/ParticleToGrid.comp"
    "Engine/Kernels/ParticleFromGrid.comp"
    "Engine/Kernels/AdvectParticles.comp"
    "Engine/LinearSolver/Kernels/*.comp")

download_project(PROJ                SPIRV_Cross
                 GIT_REPOSITORY      https://github.com/KhronosGroup/SPIRV-Cross.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${SPIRV_Cross_SOURCE_DIR} ${SPIRV_Cross_BINARY_DIR})

find_package(Vulkan)

function(compile_shader)
    set(options "")
    set(oneValueArgs TARGET)
    set(multiValueArgs SOURCES)

    cmake_parse_arguments(SHADER "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # TODO this only works for Win32
    find_program(GLSL_VALIDATOR glslc hints "$ENV{VULKAN_SDK}/Bin")
    message("Using compiler: ${GLSL_VALIDATOR}")

    foreach(SHADER_SOURCE ${SHADER_SOURCES})
        get_filename_component(SHADER_FILENAME ${SHADER_SOURCE} NAME)
        set(SHADER "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_FILENAME}.spv")
        add_custom_command(
            OUTPUT ${SHADER}
            COMMAND ${GLSL_VALIDATOR} -c ${SHADER_SOURCE} -o ${SHADER}
            DEPENDS ${SHADER_SOURCE})
        list(APPEND SHADERS ${SHADER})
    endforeach(SHADER_SOURCE)

    add_custom_target("${SHADER_TARGET}_SHADERS" DEPENDS ${SHADERS})
    set_target_properties("${SHADER_TARGET}_SHADERS" PROPERTIES SHADERS_OUTPUT "${SHADERS}")
    add_dependencies(${SHADER_TARGET} "${SHADER_TARGET}_SHADERS")
endfunction()

function(copy_shader)
  set(options "")
  set(oneValueArgs TARGET)
  set(multiValueArgs SOURCES)

  cmake_parse_arguments(SHADER "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  foreach(SHADER_SOURCE ${SHADER_SOURCES})
    get_target_property(SHADER_FILES "${SHADER_SOURCE}_SHADERS" SHADERS_OUTPUT)
    foreach(SHADER_FILE ${SHADER_FILES})
      get_filename_component(SHADER_FILENAME ${SHADER_FILE} NAME)
      add_custom_command(
        OUTPUT ${SHADER_FILENAME}
        COMMAND ${CMAKE_COMMAND} -E copy ${SHADER_FILE} ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${SHADER_FILE})
      list(APPEND SHADERS "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_FILENAME}")
    endforeach()
  endforeach(SHADER_SOURCE)

  add_custom_target("${SHADER_TARGET}_SHADERS_COPY" DEPENDS ${SHADERS})
  add_dependencies(${SHADER_TARGET} "${SHADER_TARGET}_SHADERS_COPY")
endfunction()

add_library(vortex2d STATIC ${LIB_SOURCES} ${SHADERS} ${SHADER_SOURCES} "Engine/Kernels/CommonAdvect.comp" "Engine/Kernels/CommonProject.comp" "Engine/Kernels/CommonPreScan.comp")
compile_shader(TARGET vortex2d SOURCES ${SHADER_SOURCES})
target_link_libraries(vortex2d PUBLIC glm Vulkan::Vulkan spirv-cross-core)
target_include_directories(vortex2d PUBLIC ../)

target_compile_features(vortex2d PRIVATE cxx_variadic_templates cxx_generic_lambdas)
