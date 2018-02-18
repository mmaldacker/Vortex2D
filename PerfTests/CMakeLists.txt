enable_testing()
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
download_project(PROJ                benchmark
                 GIT_REPOSITORY      https://github.com/google/benchmark.git
                 GIT_TAG             v1.2.0
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE})
add_subdirectory(${benchmark_SOURCE_DIR} ${benchmark_BINARY_DIR})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(vortex2d_perftests "main.cpp" )

target_compile_features(vortex2d_perftests PRIVATE cxx_variadic_templates)
target_link_libraries(vortex2d_perftests vortex2d fluidrigidcoupling2d_lib benchmark gtest gmock Threads::Threads)

copy_shader(TARGET vortex2d_perftests SOURCES vortex2d)
