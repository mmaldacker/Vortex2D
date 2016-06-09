# Vortex2D

A real time fluid simulation engine based on the incompressible Navier-Stokes equation. 
This implementation is grid based as opposed to particle based (such as the smoothed-particle hyddrodynamics a.k.a. SPH implementations). 
All computing is done on the GPU using simple vertex/fragment shaders and thus requires only OpenGL 3.2+ support. 

## Dependencies

 * OpenGL 3.2+
 * GLM 
 * GLFW3
 * CMake

## Introduction

This engine is a work in progress and there might be many changes to its API in the future. The fundamental piece of the engine is a grid which represents the velocity field of the fluid. Evolving this field is divided in three steps:
* Advection: moving the velocity field (or any other field) using the velocity field using an integrator (currently back & fort error correction & compensation)
* Adding external forces: gravity, forces applied by bodies in the fluid, etc
* Projection: calculating and applying a pressure field and applying it to the velocity to make divergence free and ensuring boundary conditions. This done by solver a system of linear equations, using either the successive-over-relaxation iterative solver or the preconditioned conjugate gradient solver. 

### Code structure

The code is divided in two sections: Renderer and Engine. 

The renderer section provides some simple wrapping around OpenGL functionality that makes coding the engine easier. This includes loading/compiling shaders, setting up textures and render textures, basic shape drawing, transformations and debugging content of textures. 

The engine builds on the tools provided by creating functionality to treat OpenGL as a generic computing platform. This is then used to build the linear solvers, the projection, advection and facilities to simulate smoke or water.

### General programming on the GPU

Since the engine works on a grid and most of the algorithms are trivially parallelisable, using the GPU with OpenGL is quite straightforward. Floating textures are used to represent grids, computations are done with the fragment shader and rendered on another texture by drawing a rectangle that covers the whole texture. While we can't update textures in place, we can use two textures and ping pong between them to get the same effect. 

So an operation looks like this:

```cpp
// constructions ellided for simplicity
Renderer::RenderTexture input, output;
Renderer::Program program;
Renderer::Quad quad;

output.begin();
program.use();
input.bind();
quand.render();
output.end();
```

There are two classes that simplify and make this clearer: *Buffer* and *Operator*. By using operator overloading, the above can be written as:

```cpp
Buffer output, input;
Operator operator;

output = operator(input);
```

Writting information to a Buffer is a matter of rendering a shape to the Buffer. For examples to add gravity, we can draw a rectangle covering the whole texture with the colour being the gravity force. 
 
## Fluid engine

Currently, each part of the engine is seperated in its constituents: 
 * *Boundary*: handles the setting of boundaries 
 * *Engine*: takes a linear solver, velocity grid (in the *Advection* class) and *Boundary* object to apply the projection step
 * *Advection*: contains the velocity grid, handles advection and adding of external forces
 * *LevelSet*: a special grid to handle the case of simulating water (more on this later)

In addition, there are classes to help to common cases we want to simulate:
* *Density*: handles drawing a smoke
* *Water*: handles drawing water

### Advection

### Projection

### Boundary conditions

### Level set method

## Demo

[![Demo](http://img.youtube.com/vi/c8Idjf03bI8/0.jpg)](http://www.youtube.com/watch?v=c8Idjf03bI8)
