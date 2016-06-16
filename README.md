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
Renderer::Sprite sprite;

output.begin();
program.use();
input.bind();
sprite.render();
output.end();
```

There are two classes that simplify and make this clearer: *Buffer* and *Operator*. By using operator overloading, the above can be written as:

```cpp
Buffer output, input;
Operator op;

output = op(input);
```

Writting information to a Buffer is a matter of rendering a shape to the Buffer. For examples to add gravity, we can draw a rectangle covering the whole texture with the colour being the gravity force. 
 
## Fluid engine

The class Engine is the core of the simulation and handles the boundaries, forces, velocity field, solving the incompressible equations and advections.

In addition, there are classes to help to common cases we want to simulate:
* *Density*: handles drawing a smoke
* *Water*: handles drawing water

### Engine

This is the main class that handles the core of the simulation. It is constructed with a size, time step and linear solver. There are currently two available linear solvers: an iterative preconditioned conjugate gradient solver and an iterative red-black successive over relaxation solver.

It is simply a matter then a of setting your boundaries, either once at the beginning or each time the boundary changes (e.g. with a moving object). Note that the boundaries need to cleared before hand. Then forces can be applied to the fluid (e.g. rising smoke) and the Solve method is called to apply one step to the simulation.

Visualisation of the fluid with either the Density or Water classes need to be updated after the Solver method by advecting their fluid. This is done with the advect method.

#### Boundaries

There are two types of boundaries possible which require some explanation. When solving the incompressible equations, we need to know what pressure to use at the boundaries and we support two types:

##### Dirichlet 

Dirichlet boundaries simply set the pressure at boundaries to 0. This means that any movement towards a dirichlet boundary will be unchanged. This is useful for simulating water/air interaction or to simulate a continuation of the fluid without showing it (e.g. smoke disapearing from the side of the screen).

##### Neumann 

Neumann boundaries set the pressure to be the negate of the fluid pressure. This represents solid objects and the fluid will respond by moving away from it. It is then used for solid objects immersed in the fluid.

### Density

This class is used to represent dye or smoke in the fluid. The object is initialised with the same size as the Engine and some section can then be marked to contain values to be advected. The Advect method has to be called after calling Solve on the engine. The whole Density class can then be rendered.

### Water

This class is used to simulate water. This is more complex than the Density class as the interesting part of simulating water, is simulating the boundary between water and air. For simplicity, the air is simulated as having pressure 0 (i.e. dirichlet boundaries). This boundary needs to be updated with the movement of the water. The LevelSet method is used here to achieve this. The LevelSet represents the boundary of the water, it's a grid where each cell gives the distance to the water/air boundary. This needs to be advected after each Solve like for Density. In addition, the values need to be renormalised after each advection using the Redistance method as advection doesn't preserve the distance values correctly. Finally, the engine must know of the new boundaries after each advection, this is done by setting dirichlet boundaries with the GetBoundaries method. 

## Example

```cpp
Dimensions dimensions(glm::vec2(500));
ConjugateGradient solver(dimensions.Size);
Engine engine(dimensions, &solver, 0.033);
Density density(dimensions, 0.033);

Rectangle source(20.0)
Rectangle force(20.0)
Rectangle obstacle({100.0, 50.0})
Rectangle top({500,1}), bottom({500,1}), left({1,500}), right({1,500});

source.Position = {200.0, 400.0};
source.Colour = glm::vec4(1.0);

force.Position = (glm::vec2)source.Position;
force.Colour = {0.0, -5.0, 0.0, 0.0};

obstacle.Position = {200.0, 200.0};
obstacle.Rotation = 45.0;
obstacle.Colour = glm::vec4(1.0);

top.Colour = bottom.Colour = left.Colour = right.Colour = glm::vec4(1.0);

top.Position = {0.0, 0.0};
bottom.Position = {0.0, 499.0};
left.Position = {0.0, 0.0};
right.Position = {499.0, 0.0};

void Update(Renderer::RenderTarget & target)
{
    engine.RenderDirichlet(top);
    engine.RenderDirichlet(bottom);
    engine.RenderDirichlet(left);
    engine.RenderDirichlet(right);

    engine.RenderNeumann(obstacle);

    engine.RenderForce(force);

    density.Render(source);

    engine.Solve();
    density.Advect(engine);

	target.Render(density);
}
```

## Documentation

Doxygen generated documentation can be found [here](http://mmaldacker.github.io/Vortex2D/).

## Demo

[![Demo](http://img.youtube.com/vi/c8Idjf03bI8/0.jpg)](http://www.youtube.com/watch?v=c8Idjf03bI8)
