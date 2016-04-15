# Vortex2D

A fluid simulation engine based on the incompressible Navier-Stokes equation. 
This implementation is grid based as opposed to particle based (such as the smoothed-particle hyddrodynamics a.k.a. SPH implementations). 
All computing is done on the GPU using simple vertex/fragment shaders and thus requires only OpenGL 3.2+ support. 

# Dependencies

 * OpenGL 3.2+
 * GLM (simple math library mirroring the math functionality of GLSL)
 * GLFW3 for the examples
 * CMake 
 
# Functionalities

* Neumann and Dirichlet boundaries
* Level set 
* Solid velocities
