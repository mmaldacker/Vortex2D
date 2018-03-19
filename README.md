[![Build Status](https://vortex2d.cachemiss.xyz:6500/badge/1)](https://vortex2d.cachemiss.xyz:6500/repo/1)
[![Build Status](https://travis-ci.org/mmaldacker/Vortex2D.svg?branch=master)](https://travis-ci.org/mmaldacker/Vortex2D)
[![Build Status](https://ci.appveyor.com/api/projects/status/p7q9aple11yhs1ck?svg=true)](https://ci.appveyor.com/project/mmaldacker/vortex2d)
[![codecov](https://codecov.io/gh/mmaldacker/Vortex2D/branch/master/graph/badge.svg)](https://codecov.io/gh/mmaldacker/Vortex2D)
[![Docs](https://readthedocs.org/projects/vortex2d/badge/?version=latest)](https://vortex2d.readthedocs.io)

# Vortex2D

A C++ real time fluid simulation engine based on the incompressible Navier-Stokes equation.
This implementation is a hybrid method using particles for advection and a grid for solving the incompressible equations.
All computing is done on the GPU with Vulkan using compute shaders.

Documentation can be found at https://vortex2d.readthedocs.io.

## Quick start

CMake is used to get any required dependency. The only dependencies needed are the glslc compiler, the vulkan library and python.
* On windows, glslc and  vulkan come with the LunarG SDK.
* On Linux, get the LunarG SDK and run the `build_tools.sh` command. You also need to set some env variables with `source setup-env.sh`.

Any other dependencies is downloader by Cmake. To compile the project, standard cmake is used. 

For example on linux:

```
mkdir build && cd build
cmake .. -DVORTEX2D_ENABLE_EXAMPLES=On -DVORTEX2D_ENABLE_TESTS=On
make -j 4
```


