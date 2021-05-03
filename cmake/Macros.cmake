# Find the requested package
function(vortex2d_find_package)
    list(GET ARGN 0 target)
    list(REMOVE_AT ARGN 0)

    if (IOS_PLATFORM)
        find_host_package(${target} REQUIRED)
    else()
        find_package(${target} REQUIRED)
    endif()
endfunction()

# Find the requested program
function(vortex2d_find_program)
    if (IOS_PLATFORM)
        find_host_program(${ARGN})
    else()
        find_program(${ARGN})
    endif()
endfunction()

set(vortex2d_macro__internal_dir ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

# Function to compile the shaders and generate a C++ source file to include
function(compile_shader)
    cmake_parse_arguments(SHADER "" "OUTPUT;VERSION" "SOURCES" ${ARGN})

    if (NOT DEFINED GLSL_VALIDATOR)
      vortex2d_find_program(GLSL_VALIDATOR glslangValidator hints "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin")
    elseif(NOT IS_ABSOLUTE ${GLSL_VALIDATOR})
      set(GLSL_VALIDATOR "${CMAKE_CURRENT_LIST_DIR}/${GLSL_VALIDATOR}")
    endif()
    message("Using compiler: ${GLSL_VALIDATOR}")

    file(REMOVE "${SHADER_OUTPUT}.h" "${SHADER_OUTPUT}.cpp")

    set(COMPILE_SCRIPT ${vortex2d_macro__internal_dir}/../Scripts/GenerateSPIRV.py)
    add_custom_command(
       OUTPUT "${SHADER_OUTPUT}.h" "${SHADER_OUTPUT}.cpp"
       COMMAND ${PYTHON_EXECUTABLE} ${COMPILE_SCRIPT} --compiler ${GLSL_VALIDATOR} --vulkan_version ${SHADER_VERSION} --output ${SHADER_OUTPUT} ${SHADER_SOURCES}
       DEPENDS ${SHADER_SOURCES} ${COMPILE_SCRIPT})
endfunction()

# Copy dlls
function(vortex2d_copy_dll)
    list(GET ARGN 0 target)
    list(REMOVE_AT ARGN 0)

    add_custom_command(
    TARGET ${target} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:vortex2d>
        $<TARGET_FILE_DIR:${target}>)
endfunction()

# Find vulkan or MoltenVK on macOS/iOS
function(vortex2d_find_vulkan)
  if(APPLE)
    if(NOT DEFINED MOLTENVK_DIR)
      message(FATAL_ERROR "Must specify MOLTENVK_DIR")
    endif()
    if(NOT IS_ABSOLUTE ${MOLTENVK_DIR})
      set(MOLTENVK_DIR "${CMAKE_CURRENT_LIST_DIR}/${MOLTENVK_DIR}")
    endif()

    if(IOS_PLATFORM)
      set(VORTEX2D_OS "iOS")
    else()
      set(VORTEX2D_OS "macOS")
    endif()

  	set(VULKAN_LIBRARIES "-L${MOLTENVK_DIR}/${VORTEX2D_OS}/static -lMoltenVK -framework Metal -framework IOSurface -framework QuartzCore -framework IOKit -framework Foundation -framework Cocoa" PARENT_SCOPE)
  	set(VULKAN_INCLUDES "${MOLTENVK_DIR}/include" PARENT_SCOPE)
  else()
  	find_package(Vulkan REQUIRED)
  	set(VULKAN_LIBRARIES Vulkan::Vulkan PARENT_SCOPE)
    if (NOT Vulkan_FOUND)
      message(FATAL_ERROR "Cannot find vulkan")
    endif()
  endif()
endfunction()

# Generate a Vortex2DConfig.cmake file (and associated files) from the targets registered against
# the EXPORT name "Vortex2DConfigExport" (EXPORT parameter of install(TARGETS))
function(vortex2d_export_targets)
    # CMAKE_CURRENT_LIST_DIR or CMAKE_CURRENT_SOURCE_DIR not usable for files that are to be included like this one
    set(CURRENT_DIR "${PROJECT_SOURCE_DIR}/cmake")

    include(CMakePackageConfigHelpers)
    write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/Vortex2DConfigVersion.cmake"
                                     VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
                                     COMPATIBILITY SameMajorVersion)

    set(targets_config_filename "Vortex2DTargets.cmake")

    export(EXPORT Vortex2DConfigExport
           FILE "${CMAKE_CURRENT_BINARY_DIR}/${targets_config_filename}")

    set(config_package_location lib/cmake/Vortex2D)
    configure_package_config_file("${CURRENT_DIR}/Vortex2DConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/Vortex2DConfig.cmake"
      INSTALL_DESTINATION "${config_package_location}")

    install(EXPORT Vortex2DConfigExport
            FILE ${targets_config_filename}
            DESTINATION ${config_package_location})

    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Vortex2DConfig.cmake"
                  "${CMAKE_CURRENT_BINARY_DIR}/Vortex2DConfigVersion.cmake"
            DESTINATION ${config_package_location}
            COMPONENT devel)
endfunction()

function(copy_vortex2d_framework target)
  # Create Frameworks directory in app bundle
  add_custom_command(
      TARGET
      ${target}
      POST_BUILD COMMAND /bin/sh -c
      \"COMMAND_DONE=0 \;
      if ${CMAKE_COMMAND} -E make_directory
          ${CMAKE_CURRENT_BINARY_DIR}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${target}.app/Frameworks
          \&\>/dev/null \; then
          COMMAND_DONE=1 \;
      fi \;
      if [ \\$$COMMAND_DONE -eq 0 ] \; then
          echo Failed to create Frameworks directory in app bundle \;
          exit 1 \;
      fi\"
  )

  # Copy the framework into the app bundle
  add_custom_command(
      TARGET
      ${target}
      POST_BUILD COMMAND /bin/sh -c
      \"COMMAND_DONE=0 \;
      if ${CMAKE_COMMAND} -E copy_directory
          ${PROJECT_BINARY_DIR}/Vortex2D/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/vortex2d.framework
          ${CMAKE_CURRENT_BINARY_DIR}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${target}.app/Frameworks/vortex2d.framework
          \&\>/dev/null \; then
          COMMAND_DONE=1 \;
      fi \;
      if [ \\$$COMMAND_DONE -eq 0 ] \; then
          echo Failed to copy the framework into the app bundle \;
          exit 1 \;
      fi\"
  )

  if (DEFINED CODE_SIGN_IDENTITY)
    # Codesign the framework in it's new spot
    add_custom_command(
        TARGET
        ${target}
        POST_BUILD COMMAND /bin/sh -c
        \"COMMAND_DONE=0 \;
        if codesign --force --verbose
            ${CMAKE_CURRENT_BINARY_DIR}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${target}.app/Frameworks/vortex2d.framework
            --sign ${CODE_SIGN_IDENTITY}
            \&\>/dev/null \; then
            COMMAND_DONE=1 \;
        fi \;
        if [ \\$$COMMAND_DONE -eq 0 ] \; then
            echo Framework codesign failed \;
            exit 1 \;
        fi\"
    )
  endif()
endfunction()
