=====
Setup
=====

Vortex2D is multi-platform and currently supports the following:
 * Windows
 * Linux
 * macOS
 * iOS

CMake is used to generate the appropriate build scripts for each platform.
The dependencies, which a fetched when calling cmake are:
 * glm
 * SPIRV-cross

for the tests:
 * gtest

for the examples:
 * glfw

Windows
=======

.. code-block::
  cmake ..

Linux
=====

.. code-block::
  cmake ..

macOS
=====

.. code-block::
  cmake .. -DMOLTENVK_DIR=../../MoltenVK/Package/Latest/MoltenVK/

iOS
===

Find code signing identity with: `/usr/bin/env xcrun security find-identity -v -p codesigning`

.. code-block::
  cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/ios.toolchain.cmake -DIOS_PLATFORM=OS -DIOS_ARCH=arm64 -DENABLE_VISIBILITY=true -DMOLTENVK_DIR=../../MoltenVK/Package/Latest/MoltenVK/ -DCODE_SIGN_IDENTITY="iPhone Developer" -DDEVELOPMENT_TEAM_ID=XXXXXX

Documentation
=============

* doxygen
* sphinx
