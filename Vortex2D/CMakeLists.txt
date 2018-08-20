set(LIB_SOURCES
    "Engine/Density.cpp"
    "Engine/LevelSet.cpp"
    "Engine/Pressure.cpp"
    "Engine/Advection.cpp"
    "Engine/Extrapolation.cpp"
    "Engine/World.cpp"
    "Engine/Size.cpp"
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
    "Renderer/Buffer.cpp"
    "Renderer/CommandBuffer.cpp"
    "Renderer/DescriptorSet.cpp"
    "Renderer/Device.cpp"
    "Renderer/Instance.cpp"
    "Renderer/Pipeline.cpp"
    "Renderer/RenderState.cpp"
    "Renderer/RenderTexture.cpp"
    "Renderer/RenderWindow.cpp"
    "Renderer/RenderTarget.cpp"
    "Renderer/Shapes.cpp"
    "Renderer/Sprite.cpp"
    "Renderer/Texture.cpp"
    "Renderer/Timer.cpp"
    "Renderer/Transformable.cpp"
    "Renderer/Work.cpp"
    "SPIRV/Reflection.cpp")

set(LIB_HEADERS
    "Vortex2D.h"
    "Engine/Density.h"
    "Engine/LevelSet.h"
    "Engine/Pressure.h"
    "Engine/Advection.h"
    "Engine/Extrapolation.h"
    "Engine/World.h"
    "Engine/Size.h"
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
    "Renderer/Drawable.h"
    "Renderer/Buffer.h"
    "Renderer/CommandBuffer.h"
    "Renderer/DescriptorSet.h"
    "Renderer/Device.h"
    "Renderer/Instance.h"
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
    "Utils/mapbox/variant.hpp"
    "Utils/mapbox/variant_visitor.hpp"
    "Utils/mapbox/recursive_wrapper.hpp"
    "Utils/vk_mem_alloc.h")

file(GLOB SHADER_SOURCES
    "Renderer/Kernels/*.vert"
    "Renderer/Kernels/*.frag"
    "Engine/Kernels/Advect.comp"
    "Engine/Kernels/AdvectVelocity.comp"
    "Engine/Kernels/BuildDiv.comp"
    "Engine/Kernels/BuildRigidbodyDiv.comp"
    "Engine/Kernels/BuildMatrix.comp"
    "Engine/Kernels/Extrapolate.comp"
    "Engine/Kernels/Project.comp"
    "Engine/Kernels/RigidbodyPressure.comp"
    "Engine/Kernels/RigidbodyForce.comp"
    "Engine/Kernels/RigidbodyTrans.comp"
    "Engine/Kernels/Redistance.comp"
    "Engine/Kernels/ConstrainVelocity.comp"
    "Engine/Kernels/ConstrainRigidbodyVelocity.comp"
    "Engine/Kernels/ExtrapolateVelocity.comp"
    "Engine/Kernels/PolygonDist.frag"
    "Engine/Kernels/CircleDist.frag"
    "Engine/Kernels/UpdateVertices.comp"
    "Engine/Kernels/DistanceField.frag"
    "Engine/Kernels/PreScanAdd.comp"
    "Engine/Kernels/PreScan.comp"
    "Engine/Kernels/PreScanStoreSum.comp"
    "Engine/Kernels/ParticleCount.comp"
    "Engine/Kernels/ParticleClamp.comp"
    "Engine/Kernels/ParticleSpawn.comp"
    "Engine/Kernels/ParticleBucket.comp"
    "Engine/Kernels/ParticlePhi.comp"
    "Engine/Kernels/ParticleToGrid.comp"
    "Engine/Kernels/ParticleFromGrid.comp"
    "Engine/Kernels/AdvectParticles.comp"
    "Engine/Kernels/VelocityDifference.comp"
    "Engine/Kernels/VelocityMax.comp"
    "Engine/LinearSolver/Kernels/*.comp")

download_project(PROJ                glm
                 GIT_REPOSITORY      https://github.com/g-truc/glm.git
                 GIT_TAG             0.9.9.0
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${glm_SOURCE_DIR} ${glm_BINARY_DIR})

download_project(PROJ                SPIRV_Cross
                 GIT_REPOSITORY      https://github.com/KhronosGroup/SPIRV-Cross.git
                 GIT_TAG             master
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${SPIRV_Cross_SOURCE_DIR} ${SPIRV_Cross_BINARY_DIR})
set_property(TARGET spirv-cross-core PROPERTY POSITION_INDEPENDENT_CODE ON)

vortex2d_find_package(PythonInterp REQUIRED)
vortex2d_find_vulkan()

compile_shader(SOURCES ${SHADER_SOURCES} OUTPUT "vortex2d_generated_spirv")

add_library(vortex2d
  SHARED
    ${LIB_SOURCES}
    ${LIB_HEADERS}
    ${SHADER_SOURCES}
    "Engine/Kernels/CommonAdvect.comp"
    "Engine/Kernels/CommonProject.comp"
    "Engine/Kernels/CommonPreScan.comp"
    "Engine/Kernels/CommonParticles.comp"
    vortex2d_generated_spirv.cpp
    vortex2d_generated_spirv.h)

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
      PUBLIC_HEADER "Vortex2D.h"
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

  add_custom_command(TARGET vortex2d POST_BUILD COMMAND cp -r ${CMAKE_SOURCE_DIR}/Vortex2D/ $<TARGET_FILE_DIR:vortex2d>/Headers/)

  add_custom_command(
      TARGET vortex2d
      POST_BUILD
    COMMAND /bin/bash -c "${CMAKE_DIR}/install_name.sh $<TARGET_FILE_DIR:vortex2d>/vortex2d")

  add_custom_command(
      TARGET vortex2d
      POST_BUILD
      COMMAND install_name_tool -id \"@rpath/vortex2d.framework/vortex2d\" $<TARGET_FILE_DIR:vortex2d>/vortex2d)

endif()

target_link_libraries(vortex2d PUBLIC glm ${VULKAN_LIBRARIES} PRIVATE spirv-cross-core)

target_include_directories(vortex2d
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
        $<BUILD_INTERFACE:${VULKAN_INCLUDES}>
        $<INSTALL_INTERFACE:include/Vortex2D>
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

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
install(DIRECTORY . DESTINATION include/Vortex2D
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

vortex2d_export_targets()
