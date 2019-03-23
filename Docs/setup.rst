=====
Setup
=====

Vortex2D is multi-platform and currently supports the following:

 * Windows
 * Linux
 * macOS
 * iOS

CMake is used to generate the appropriate build scripts for each platform.
The dependencies, which are fetched when calling cmake, are **glm** and **SPIRV-cross**. The tests use **gtest** and the examples use **glfw**.

The only dependency required is python.
There a several variables that can be used to configure:

+-------------------------+-------------------------+
| CMake                   | Builds                  |
+=========================+=========================+
|VORTEX2D_ENABLE_TESTS    |builds the tests         |
+-------------------------+-------------------------+
|VORTEX2D_ENABLE_EXAMPLES |builds the examples      |
+-------------------------+-------------------------+
|VORTEX2D_ENABLE_DOCS     |builds the documentation |
+-------------------------+-------------------------+

The main library is built as a dll on windows, shared library on linux and (dynamic) framework on macOS/iOS.

Prerequisite
============

Following dependencies are necessary:

 * Python
 * glslangValidator (comes with Vulkan SDK)

Following minimum compilers are necessary:

  * GCC 5.4 or later
  * MSVC 2015 or later
  * Clang 3.4 or later

Windows
=======

To build on windows, `cmake-gui` is the easiest to use. Only the variables specified above should be changed.

Linux
=====

The package `xorg-dev` might need to first be installed.
Again, regular cmake commands should be use to configure cmake:

.. code-block:: bash

  cmake .. 

macOS
=====

In addition to the normal variables, we need to specify the location of MoltenVK and the glslang compiler:

.. code-block:: bash

  cmake .. -DMOLTENVK_DIR=path_to/MoltenVK/Package/Latest/MoltenVK/ -DGLSL_VALIDATOR=path_to/bin/glslangValidator

iOS
===

The framework needs to signed on iOS, so the following variables need to be defined:

+---------------------+--------------------------------------------+
| Variable            | Value                                      |
+---------------------+--------------------------------------------+
| CODE_SIGN_IDENTITY  | "iPhone Developer"                         |
+---------------------+--------------------------------------------+
| DEVELOPMENT_TEAM_ID | set to the team id,                        |
|                     | can be found on the apple developer portal |
+---------------------+--------------------------------------------+

In addition, the MoltenVK location has to be specified, and the toolchain:

.. code-block:: bash

  cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake -DIOS_PLATFORM=OS -DIOS_ARCH=arm64 -DENABLE_VISIBILITY=true -DGLSL_VALIDATOR=path_to/bin/glslangValidator -DMOLTENVK_DIR=path_to/MoltenVK/Package/Latest/MoltenVK/ -DCODE_SIGN_IDENTITY="iPhone Developer" -DDEVELOPMENT_TEAM_ID=XXXXXX

Documentation
=============

To build the documentation the following is required:

* doxygen
* sphinx
* sphinx_rtd_theme
* sphinx breathe
