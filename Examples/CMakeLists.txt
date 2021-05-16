set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
if (APPLE)
    set(GLFW_VULKAN_STATIC ON  CACHE BOOL "" FORCE)
endif()
FetchContent_Declare(glfw3
                     GIT_REPOSITORY      https://github.com/glfw/glfw.git
                     GIT_TAG             3.3)
FetchContent_GetProperties(glfw3)
if(NOT glfw3_POPULATED)
  FetchContent_Populate(glfw3)
  add_subdirectory(${glfw3_SOURCE_DIR} ${glfw3_BINARY_DIR})
endif()

set(BOX2D_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BOX2D_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(box2d
                     GIT_REPOSITORY      https://github.com/erincatto/Box2D.git
                     GIT_TAG             v2.3.1)

FetchContent_GetProperties(box2d)
if(NOT box2d_POPULATED)
  FetchContent_Populate(box2d)
  add_subdirectory(${box2d_SOURCE_DIR}/Box2D ${box2d_BINARY_DIR})
endif()

file(GLOB EXAMPLES_SOURCES
        "main.cpp"
        "RenderExample.h"
        "ObstacleSmokeExample.h"
        "SmokeExample.h"
        "SmokeVelocityExample.h"
        "WaterExample.h"
        "BuoyancyWaterExample.h"
        "WaterFallExample.h"
        "WatermillExample.h"
        "Runner.h"
        "Rigidbody.h"
        "Rigidbody.cpp")

add_executable(vortex2d_examples ${EXAMPLES_SOURCES})
target_link_libraries(vortex2d_examples vortex2d glfw Box2D glm)
target_include_directories(vortex2d_examples PRIVATE
    $<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
    ${box2d_SOURCE_DIR}/Box2D)

if (WIN32)
    vortex_copy_dll(vortex2d_examples)
endif()
