# Release 1.6

* Improved API of command buffer and render command. All functions return `*this`.
* Moved `ExecuteCommand` as `Execute` in `Device`.
* Added buffer resize function.
* Added IndexBuffer.
* Better buffer copy methods.
* Fix buffer fill.
* Better texture copy methods.
* Added dynamic viewport and scissor to pipelines.
* Added shape interface
* Removed (emulated) properties of Transformable.
* Added clang-format.
* Fixes around dllexport/dllimport on windows.
* Fix layout for swapchain

# Release 1.5

* Bicubic interpolation
* Add velocity operator: add/set
* Overhauled rigidbody interface
* Correctly enforce sub-steps
* User setting of solver parameters
* Particle bounds fix
* Minor vulkan fixes

# Release 1.4

* Support for running simulation with multiple sub-steps per steps
* Rename world function "Solve" to "Step"
* Removed scaling functionality
* Fixed Timer class
* Added option to specify specialization constants in shaders
* CMake improvements when including in other projects
* Initialize to Vulkan 1.1 by default (with fallback to 1.0)
* Improve ComputeSize class

# Release 1.3

* Added strong rigidbody coupling
* Added support for blend constants
* Scaling of velocities when setting them in rigidbodies
* Using mapbox variant class
* Fix rigidbody div calculation
* Fix rigidbody centre calculation

# Release 1.2.1

* Fix broken particle unit-test from glm update
* Fix documentation on glsl compiler

# Release 1.2

* Faster linear solver
* Added calculation for the CFL number
* Updated glm version to 0.9.9.0
* Fixed initialization of glm variables
* Improved push constant API for compute shader
* Improved Conjugate Gradient exit condition

# Release 1.1

* macOS and iOS support via MoltenVK
* compiling shaders with glslangValidator instead of glslc
* improved signed distance field computation from polygons
* added Wait() function to RenderCommand
* correct synchronisation for present queue
* improved cmake configuration

# Release 1.0

* first release