[![Build Status](https://travis-ci.org/mmaldacker/Vortex2D.svg?branch=master)](https://travis-ci.org/mmaldacker/Vortex2D)
[![Build status](https://ci.appveyor.com/api/projects/status/p7q9aple11yhs1ck?svg=true)](https://ci.appveyor.com/project/mmaldacker/vortex2d)
[![Code Coverage](http://codecov.io/github/mmaldacker/Vortex2D/coverage.svg?branch=master)](http://codecov.io/github/mmaldacker/Vortex2D?branch=master)

# Vortex2D

A C++ real time fluid simulation engine based on the incompressible Navier-Stokes equation.
This implementation is grid based as opposed to particle based (such as the smoothed-particle hyddrodynamics a.k.a. SPH implementations).
All computing is done on the GPU with OpenGL using simple vertex/fragment shaders and thus requires only OpenGL 3.2+ support.

## Compiling/Dependencies

The project uses CMake to compile, the engine relies on GLM and GLAD for the Engine, GLFW3 for the examples, and GMock/GTest for the tests. Glad is included in the project and all the other dependencies are downloaded by CMake. This means getting started is a simple matter of:

```
mkdir build && cd build
cmake .. -DENABLE_EXAMPLES=ON
make -j 4
```

## Introduction

This engine is a work in progress and there might be changes to its API in the future. The fundamental piece of the engine is a texture which represents the velocity field of the fluid. This means we cannot simulate an inifinite size world, but a world exactly the size of said texture. Boundaries, boundary velocities and forces can be applied to the field. The field is then evolved using the incompressible Navier-Stokes equations. Evolving this field is divided in three steps:
* Advection: moving the velocity field (or any other field) using the velocity field with an integrator (currently Runge-Kutta 3)
* Adding external forces: gravity, forces applied by bodies in the fluid, etc
* Projection: calculating a pressure field, ensuring it is incompressible and applying it to the velocity to make it divergence free and ensuring boundary conditions. This done by solving a system of linear equations, using either the successive-over-relaxation iterative solver or the preconditioned conjugate gradient solver.

## Fluid engine

The class *World* is the core of the simulation and handles the boundaries, forces, velocity field, solving the incompressible equations and advections.

### World

This is the main class that handles the core of the simulation. It is constructed with a size and time-step.

The class has methods to draw boundaries, either once at the beginning or each time the boundary changes (e.g. with a moving object). Note that the boundaries need to cleared before hand. Boundary velocities can be drawn as well (e.g. with a moving object). Then forces can be applied to the fluid (e.g. rising smoke) and the *Solve* method is called to apply one step to the simulation. This will solve the incompressible equations and do the advection.

Visualisation of the fluid with either the *Density* or *World* classes need to be updated after the *Solve* method by advecting their fluid. This is done with the *Advect* method.

#### Boundaries

There are two main functions to define boundaries: *DrawLiquid* and *DrawSolid*. The default type of boundary for the simulation is known as *Dirichlet boundaries*. This defines a boundary of pressure 0, in other words nothing happens to the fluid when moving against it. So *DrawLiquid* defines the area where liquid simulation happens. *DrawSolid* then defines the solid boundaries, i.e. where the fluid get deflected.

### Density

This class is used to represent dye or smoke in the fluid. The object is initialised with the same size as the *Engine* and some section can then be marked to contain values to be advected (by drawing a shape in it). The *Advect* method has to be called after calling *Solve* on the engine so the dye/smoke can be evolved. The whole *Density* class can then be rendered (it is after all, simply a texture).

### Water

To simulate water, it is a simple matter of moving the fluid region itself using the *Advect* function in the *World* class.

## Example

A simple example for a smoke simulation:

```cpp
Dimensions dimensions(glm::vec2(500), 1.0);
World world(dimensions, 0.033);
Density density(dimensions);

Rectangle source(20.0)
Rectangle force(20.0)
Rectangle obstacle({100.0, 50.0})

source.Position = {200.0, 400.0};
source.Colour = glm::vec4(1.0);

force.Position = (glm::vec2)source.Position;
force.Colour = {0.0, -5.0, 0.0, 0.0};

obstacle.Position = {200.0, 200.0};
obstacle.Rotation = 45.0;
obstacle.Colour = glm::vec4(1.0);

auto boundaries = world.DrawBoundaries();
boundaries.DrawSolid(obstacle);

void Update(Renderer::RenderTarget & target)
{
    engine.RenderForce(force);

    density.Render(source);

    world.Solve();
    density.Advect(world);
    target.Render(density);
}
```
## Documentation

Doxygen generated documentation can be found [here](http://mmaldacker.github.io/Vortex2D/html). Those are automatically generated after each commit.

## Demo

[![Demo](http://img.youtube.com/vi/c8Idjf03bI8/0.jpg)](http://www.youtube.com/watch?v=c8Idjf03bI8)
