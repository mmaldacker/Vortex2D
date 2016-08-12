file(GLOB EXAMPLES_SOURCES "*.h" "*.cpp")

find_package(glfw3 REQUIRED)

add_executable(vortex2d_examples ${EXAMPLES_SOURCES})
target_link_libraries(vortex2d_examples vortex2d glfw)

if(APPLE)
	find_library(COCOA_FRAMEWORK Cocoa)
	find_library(IOKIT_FRAMEWORK IOKit)
	find_library(CORE_VIDEO_FRAMEWORK CoreVideo)

	target_link_libraries(vortex2d_examples ${COCOA_FRAMEWORK} ${IOKIT_FRAMEWORK} ${CORE_VIDEO_FRAMEWORK})
endif()

source_group(Examples REGULAR_EXPRESSION ".*(h|cpp)")
