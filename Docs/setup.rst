=====
Setup
=====

Vortex2D is multi-platform and currently supports the following:

 * Windows
 * Linux
 * macOS
 * iOS

CMake is used to generate the appropriate build scripts for each platform.
The dependencies, which are fetched when calling cmake are:

 * glm
 * SPIRV-cross

for the tests:

 * gtest

for the examples:

 * glfw

The only dependency required is python.
There a several variables that can be used to configure:

 *  `VORTEX2D_ENABLE_TESTS` builds the tests
 *  `VORTEX2D_ENABLE_EXAMPLES` builds the examples
 *  `VORTEX2D_ENABLE_DOCS` builds the documentation

The main library is built as a dll on windows, shared library on linux and (dynamic) framework on macOS/iOS.

Windows
=======

To build on windows, `cmake-gui` is the easiest to use. Only the variables specified above should be changed.

Linux
=====

Again, regular cmake commands should be use to configure cmake:

.. code-block:: bash

  cmake .. 

macOS
=====

In addition to the normal variables, we need to specify the location of MoltenVK:

.. code-block:: bash

  cmake .. -DMOLTENVK_DIR=../../MoltenVK/Package/Latest/MoltenVK/ -DVORTEX2D_ENABLE_EXAMPLES=On

iOS
===

The framework needs to signed on iOS, so the following variables need to be defined:

 * `CODE_SIGN_IDENTITY` should be set to `"iPhone Developer"`
 * `DDEVELOPMENT_TEAM_ID` should be set to the team id, can be found on the apple developer portal

.. code-block:: bash

  cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake -DIOS_PLATFORM=OS -DIOS_ARCH=arm64 -DENABLE_VISIBILITY=true -DMOLTENVK_DIR=../../MoltenVK/Package/Latest/MoltenVK/ -DCODE_SIGN_IDENTITY="iPhone Developer" -DDEVELOPMENT_TEAM_ID=XXXXXX

Documentation
=============

To build the documentation the following is required:

* doxygen
* sphinx
* breath sphinx theme
