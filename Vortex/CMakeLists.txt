set(LIB_VULKAN_HEADERS
    "Renderer/Vulkan/Vulkan.h"
    "Renderer/Vulkan/Instance.h"
    "Renderer/Vulkan/Device.h"
    "Renderer/Vulkan/RenderPass.h"
    )

set(LIB_VULKAN_SOURCES
    "Renderer/Vulkan/Device.cpp"
    "Renderer/Vulkan/Instance.cpp"
    "Renderer/Vulkan/RenderPass.cpp"
    "Renderer/Vulkan/CommandBuffer.cpp"
    "Renderer/Vulkan/Texture.cpp"
    "Renderer/Vulkan/Buffer.cpp"
    "Renderer/Vulkan/Timer.cpp"
    "Renderer/Vulkan/RenderTexture.cpp"
    "Renderer/Vulkan/RenderWindow.cpp"
    "Renderer/Vulkan/RenderTarget.cpp"
    "Renderer/Vulkan/BindGroup.cpp"
    "Renderer/Vulkan/vk_mem_alloc.h")

set(LIB_SOURCES
    "Engine/Density.cpp"
    "Engine/LevelSet.cpp"
    "Engine/Pressure.cpp"
    "Engine/Advection.cpp"
    "Engine/Extrapolation.cpp"
    "Engine/World.cpp"
    "Engine/Boundaries.cpp"
    "Engine/PrefixScan.cpp"
    "Engine/Particles.cpp"
    "Engine/Rigidbody.cpp"
    "Engine/Velocity.cpp"
    "Engine/Cfl.cpp"
    "Engine/LinearSolver/LinearSolver.cpp"
    "Engine/LinearSolver/Reduce.cpp"
    "Engine/LinearSolver/GaussSeidel.cpp"
    "Engine/LinearSolver/Jacobi.cpp"
    "Engine/LinearSolver/ConjugateGradient.cpp"
    "Engine/LinearSolver/Diagonal.cpp"
    "Engine/LinearSolver/IncompletePoisson.cpp"
    "Engine/LinearSolver/Transfer.cpp"
    "Engine/LinearSolver/Multigrid.cpp"
    "Renderer/Pipeline.cpp"
    "Renderer/RenderState.cpp"
    "Renderer/Shapes.cpp"
    "Renderer/Sprite.cpp"
    "Renderer/Transformable.cpp"
    "Renderer/Work.cpp"
    "SPIRV/Reflection.cpp"
    "SPIRV/ShaderLayout.cpp")

set(LIB_HEADERS
    "Vortex.h"
    "Engine/Density.h"
    "Engine/LevelSet.h"
    "Engine/Pressure.h"
    "Engine/Advection.h"
    "Engine/Extrapolation.h"
    "Engine/World.h"
    "Engine/Boundaries.h"
    "Engine/PrefixScan.h"
    "Engine/Particles.h"
    "Engine/Rigidbody.h"
    "Engine/Velocity.h"
    "Engine/Cfl.h"
    "Engine/LinearSolver/LinearSolver.h"
    "Engine/LinearSolver/Preconditioner.h"
    "Engine/LinearSolver/Reduce.h"
    "Engine/LinearSolver/GaussSeidel.h"
    "Engine/LinearSolver/Jacobi.h"
    "Engine/LinearSolver/ConjugateGradient.h"
    "Engine/LinearSolver/Diagonal.h"
    "Engine/LinearSolver/IncompletePoisson.h"
    "Engine/LinearSolver/Transfer.h"
    "Engine/LinearSolver/Multigrid.h"
    "Renderer/Common.h"
    "Renderer/Gpu.h"
    "Renderer/Drawable.h"
    "Renderer/Buffer.h"
    "Renderer/CommandBuffer.h"
    "Renderer/BindGroup.h"
    "Renderer/Device.h"
    "Renderer/Pipeline.h"
    "Renderer/RenderState.h"
    "Renderer/RenderTexture.h"
    "Renderer/RenderWindow.h"
    "Renderer/RenderTarget.h"
    "Renderer/Shapes.h"
    "Renderer/Sprite.h"
    "Renderer/Texture.h"
    "Renderer/Timer.h"
    "Renderer/Transformable.h"
    "Renderer/Work.h"
    "SPIRV/Reflection.h"
    "SPIRV/ShaderLayout.h"
    "Utils/mapbox/variant.hpp"
    "Utils/mapbox/variant_visitor.hpp"
    "Utils/mapbox/recursive_wrapper.hpp")

file(GLOB SHADER_SOURCES
    "Renderer/Kernels/*.vert"
    "Renderer/Kernels/*.frag"
    "Engine/Kernels/SDF/Redistance.comp"
    "Engine/Kernels/SDF/PolygonDist.frag"
    "Engine/Kernels/SDF/CircleDist.frag"
    "Engine/Kernels/SDF/DistanceField.frag"
    "Engine/Kernels/SDF/DualContour.comp"
    "Engine/Kernels/SDF/MeshReindexing.comp"
    "Engine/Kernels/Particles/ParticleCount.comp"
    "Engine/Kernels/Particles/ParticleClamp.comp"
    "Engine/Kernels/Particles/ParticleSpawn.comp"
    "Engine/Kernels/Particles/ParticleBucket.comp"
    "Engine/Kernels/Particles/ParticlePhi.comp"
    "Engine/Kernels/Particles/ParticleToGrid.comp"
    "Engine/Kernels/Particles/ParticleFromGrid.comp"
    "Engine/Kernels/PreScan/PreScanAdd.comp"
    "Engine/Kernels/PreScan/PreScan.comp"
    "Engine/Kernels/PreScan/PreScanStoreSum.comp"
    "Engine/Kernels/RigidBody/ConstrainRigidbodyVelocity.comp"
    "Engine/Kernels/RigidBody/RigidbodyPressure.comp"
    "Engine/Kernels/RigidBody/RigidbodyForce.comp"
    "Engine/Kernels/RigidBody/BuildRigidbodyDiv.comp"
    "Engine/Kernels/Advection/Advect.comp"
    "Engine/Kernels/Advection/AdvectVelocity.comp"
    "Engine/Kernels/Advection/AdvectParticles.comp"
    "Engine/Kernels/BuildDiv.comp"
    "Engine/Kernels/BuildMatrix.comp"
    "Engine/Kernels/Extrapolate.comp"
    "Engine/Kernels/Project.comp"
    "Engine/Kernels/ConstrainVelocity.comp"
    "Engine/Kernels/ExtrapolateVelocity.comp"
    "Engine/Kernels/VelocityDifference.comp"
    "Engine/Kernels/VelocityMax.comp"
    "Engine/LinearSolver/Kernels/*.comp")

set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_GLSL OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_CPP OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_REFLECT OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_C_API OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_UTIL OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL  ON CACHE BOOL "" FORCE)
FetchContent_Declare(spirv_cross
                     GIT_REPOSITORY      https://github.com/KhronosGroup/SPIRV-Cross.git
                     GIT_TAG             2019-11-01)
FetchContent_GetProperties(spirv_cross)
if(NOT spirv_cross_POPULATED)
    FetchContent_Populate(spirv_cross)
    add_subdirectory(${spirv_cross_SOURCE_DIR} ${spirv_cross_BINARY_DIR})
endif()
set_property(TARGET spirv-cross-core PROPERTY POSITION_INDEPENDENT_CODE ON)

vortex_find_package(PythonInterp REQUIRED)
vortex_find_vulkan()

compile_shader(SOURCES ${SHADER_SOURCES} OUTPUT "vortex_generated_spirv" VERSION 1.0)

add_library(vortex2d
  SHARED
    ${LIB_VULKAN_HEADERS}
    ${LIB_VULKAN_SOURCES}
    ${LIB_SOURCES}
    ${LIB_HEADERS}
    ${SHADER_SOURCES}
    "Engine/Kernels/Advection/CommonAdvect.comp"
    "Engine/Kernels/CommonProject.comp"
    "Engine/Kernels/PreScan/CommonPreScan.comp"
    "Engine/Kernels/Particles/CommonParticles.comp"
    "Engine/Kernels/RigidBody/CommonRigidbody.comp"
    "Engine/Kernels/CommonInterpolate.comp"
    "Engine/Kernels/SDF/QEF.comp"
    vortex_generated_spirv.cpp
    vortex_generated_spirv.h)

set(CMAKE_DIR "${PROJECT_SOURCE_DIR}/cmake")

# Create framework for macOS/iOS
if (APPLE)
  set_target_properties(vortex2d PROPERTIES
      FRAMEWORK TRUE
      FRAMEWORK_VERSION A
      MACOSX_FRAMEWORK_IDENTIFIER org.vortex2d
      # "current version" in semantic format in Mach-O binary file
      VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATH}
      # "compatibility version" in semantic format in Mach-O binary file
      SOVERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATH}
      PUBLIC_HEADER "Vortex.h"
    )

  # Only need signing on iOS
  if (IOS_PLATFORM)
    if (NOT DEFINED CODE_SIGN_IDENTITY)
      message(FATAL_ERROR "Need to define variables CODE_SIGN_IDENTITY and DEVELOPMENT_TEAM_ID")
    endif()
    set_target_properties(vortex2d PROPERTIES
      XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${CODE_SIGN_IDENTITY}
      XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${DEVELOPMENT_TEAM_ID}
    )
  endif()

  add_custom_command(TARGET vortex2d POST_BUILD COMMAND cp -r ${CMAKE_SOURCE_DIR}/Vortex/ $<TARGET_FILE_DIR:vortex2d>/Headers/)

  add_custom_command(
      TARGET vortex2d
      POST_BUILD
    COMMAND /bin/bash -c "${CMAKE_DIR}/install_name.sh $<TARGET_FILE_DIR:vortex2d>/vortex2d")

  add_custom_command(
      TARGET vortex2d
      POST_BUILD
      COMMAND install_name_tool -id \"@rpath/vortex2d.framework/vortex2d\" $<TARGET_FILE_DIR:vortex2d>/vortex2d)

  # On Apple, spirv-cross is already linked as part of MoltenVK
  target_link_options(vortex2d PUBLIC "LINKER:-force_load,$<TARGET_FILE:spirv-cross-core>")
endif()

target_link_libraries(vortex2d PUBLIC ${VULKAN_LIBRARIES} PRIVATE spirv-cross-core glm)

target_include_directories(vortex2d
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/..
        $<BUILD_INTERFACE:${VULKAN_INCLUDES}>
        $<INSTALL_INTERFACE:include/Vortex>
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR})

# windows definition to correctly generate the dll
if(WIN32)
    target_compile_definitions(vortex2d PRIVATE VORTEX2D_API_EXPORTS)
endif()

install(TARGETS vortex2d
        EXPORT Vortex2DConfigExport
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        FRAMEWORK DESTINATION frameworks)
install(DIRECTORY . DESTINATION include/Vortex
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

vortex_export_targets()
