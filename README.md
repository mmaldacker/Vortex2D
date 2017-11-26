[![Build Status](http://vortex2d.strangled.net:6500/badge/1)](http://vortex2d.strangled.net:6500/repo/1)
[![Build Status](https://travis-ci.org/mmaldacker/Vortex2D.svg?branch=master)](https://travis-ci.org/mmaldacker/Vortex2D)
[![Build Status](https://ci.appveyor.com/api/projects/status/p7q9aple11yhs1ck?svg=true)](https://ci.appveyor.com/project/mmaldacker/vortex2d)
[![Code Coverage](http://codecov.io/github/mmaldacker/Vortex2D/coverage.svg?branch=master)](http://codecov.io/github/mmaldacker/Vortex2D?branch=master)

# Vortex2D

A C++ real time fluid simulation engine based on the incompressible Navier-Stokes equation.
This implementation is a hybrid method using particles for advection and a grid for solving the incompressible equations.
All computing is done on the GPU with Vulkan using compute shaders.

## Compiling/Dependencies

CMake is used to get any required dependency. The only dependency needs is the glslc compiler and the vulkan library
* On windows, both come with the LunarG SDK.
* On Linux, get the LunarG SDK and run the `build_tools.sh` command. You also need to set some env variables with `source setup-env.sh`.

Any other dependencies is downloader by Cmake. For reference they are:
* GLM
* vkLoader
* gtest
* benchmark
* GLFW3
* Box2D

To compile the project, standard cmake is used:

```
mkdir build && cd build
cmake .. -DVORTEX2D_ENABLE_EXAMPLES=On -DVORTEX2D_ENABLE_TESTS=On -DVORTEX2D_ENABLE_PERFTESTS=On
make -j 4
```

## Documentation

Doxygen generated documentation can be found [here](http://mmaldacker.github.io/Vortex2D/html). Those are automatically generated after each commit.