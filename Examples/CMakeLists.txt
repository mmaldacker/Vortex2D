file(GLOB EXAMPLES_SOURCES "*.h" "*.cpp")

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})
link_directories(${GLFW_LIBRARY_DIRS})

add_executable(vortex2d_examples ${EXAMPLES_SOURCES})
target_link_libraries(vortex2d_examples vortex2d ${GLFW_LIBRARIES})

if(APPLE)
	find_library(COCOA_FRAMEWORK Cocoa)
	find_library(IOKIT_FRAMEWORK IOKit)
	find_library(CORE_VIDEO_FRAMEWORK CoreVideo)

	target_link_libraries(vortex2d_examples ${COCOA_FRAMEWORK} ${IOKIT_FRAMEWORK} ${CORE_VIDEO_FRAMEWORK})
endif()

source_group(Examples REGULAR_EXPRESSION ".*(h|cpp)")
