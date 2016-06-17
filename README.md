# Vortex2D

A real time fluid simulation engine based on the incompressible Navier-Stokes equation. 
This implementation is grid based as opposed to particle based (such as the smoothed-particle hyddrodynamics a.k.a. SPH implementations). 
All computing is done on the GPU with OpenGL using simple vertex/fragment shaders and thus requires only OpenGL 3.2+ support. 

## Dependencies

 * OpenGL 3.2+
 * GLM 
 * GLFW3
 * CMake

## Introduction

This engine is a work in progress and there might be changes to its API in the future. The fundamental piece of the engine is a texture which represents the velocity field of the fluid. This means we cannot simulate an inifinitely size world, but a world exactly the size of said texture. Boundaries, boundary velocities and forces can be applied to the field. The field is then evolved using the incompressible Navier-Stokes equations. Evolving this field is divided in three steps:
* Advection: moving the velocity field (or any other field) using the velocity field with an integrator (currently Runge-Kutta 3)
* Adding external forces: gravity, forces applied by bodies in the fluid, etc
* Projection: calculating a pressure field, ensuring it is incompressible and applying it to the velocity to make it divergence free and ensuring boundary conditions. This done by solving a system of linear equations, using either the successive-over-relaxation iterative solver or the preconditioned conjugate gradient solver. 

### Code structure

The code is divided in two sections: Renderer and Engine. 

The renderer section provides some simple wrapping around OpenGL functionality that makes coding the engine easier. This includes loading/compiling shaders, setting up textures and render textures, basic shape drawing, transformations and debugging content of textures. 

The engine builds on the tools provided by creating functionality to treat OpenGL as a generic computing platform. This is then used to build the linear solvers, the projection, advection and other facilities to simulate smoke or water.

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

The class *Engine* is the core of the simulation and handles the boundaries, forces, velocity field, solving the incompressible equations and advections.

In addition, there are classes to help two common cases we want to simulate:
* *Density*: handles drawing a smoke
* *Water*: handles drawing water

### Engine

This is the main class that handles the core of the simulation. It is constructed with a size, time-step and linear solver. There are currently two available linear solvers: an iterative preconditioned conjugate gradient solver and an iterative red-black successive over relaxation solver.

The class has methods to draw boundaries, either once at the beginning or each time the boundary changes (e.g. with a moving object). Note that the boundaries need to cleared before hand. Boundary velocities can be drawn as well (e.g. with a moving object). Then forces can be applied to the fluid (e.g. rising smoke) and the *Solve* method is called to apply one step to the simulation. This will solve the incompressible equations and do the advection.

Visualisation of the fluid with either the *Density* or *Water* classes need to be updated after the *Solve* method by advecting their fluid. This is done with the *Advect* method.

#### Boundaries

There are two types of boundaries possible which require some explanation. When solving the incompressible equations, we need to know what pressure to use at the boundaries and we support two types:

 * Dirichlet 

Dirichlet boundaries simply set the pressure at boundaries to 0. This means that any movement towards a dirichlet boundary will be unchanged. This is useful for simulating water/air interaction or to simulate a continuation of the fluid without showing it (e.g. smoke disapearing from the side of the screen).

 * Neumann 

Neumann boundaries set the pressure to be the negative of the fluid pressure. This represents solid objects and the fluid will respond by moving away from it. It is used for solid objects immersed in the fluid.

### Density

This class is used to represent dye or smoke in the fluid. The object is initialised with the same size as the *Engine* and some section can then be marked to contain values to be advected (by drawing a shape in it). The *Advect* method has to be called after calling *Solve* on the engine so the dye/smoke can be evolved. The whole *Density* class can then be rendered (it is after all, simply a texture).

### Water

This class is used to simulate water. This is more complex than the *Density* class as the interesting part of simulating water, is simulating the boundary between water and air. The velocity field is then seperated in two section: the water and air section.

For simplicity, the air is simulated as having pressure 0 (i.e. dirichlet boundaries). For the simulation to be interesting, the interface air/water needs to be evolved, so that our water can splash around. We use the level set method to achieve this. 

A level set is a grid where each grid cell has the signed distance to the nearest water/air boundary. The distance is positive in the water and negative in the air. The *Water* class has a level set grid the size of the *Engine*, we can then mark (i.e. draw) some sections as beeing water. The class will internally maintain the level set to contain an approximate distance field.

We can then evolve the level set by simply advecting it with the velocity field. Again, the class will ensure the distance field is preserved. After each advection, the engine's boundaries need to be updated, in other words we need to set a dirichlet boundaries wherever the distance is negative. The *GetBoundaries* method is used to update the dirichlet boundaries.

Finally, the level set can be rendered as water (again, it is simply a texture).

## Example

A simple example for a smoke simulation:

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

## Todo

* The velocity field is extrapolated in the boundaries so the level set can be properly advectated. However for Neumann boundaries, only the normal velocity on the surface should be set.
* Neumann boundaries can move the flui, it'd be interesting to have this work the other way around, e.g. objects moved by water 