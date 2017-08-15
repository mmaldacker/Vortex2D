set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(vortex2d_perftests "main.cpp" )

target_compile_features(vortex2d_perftests PRIVATE cxx_variadic_templates)
target_link_libraries(vortex2d_perftests vortex2d vortex2d_tests_lib benchmark Threads::Threads)
