========
Vortex2D
========

--------------------------
2D real-time fluid engine
--------------------------

Introduction
============

This is a 2D engine with the goal of being used in real-time scenarios, e.g. video games, in the same manner as a Box2D.

It is a hybrid engine that uses particles for the advection part (moving the particles given their velocities) and a grid to resolve the incompressible constrains.
However, the particles are not visible to the user and you only work with the grid directly.

The engine runs directly on the GPU using Vulkan compute shaders. The rendering is then also done with Vulkan. The grids are represented by textures and operations by the user are all done by drawing shapes on the textures. The engine itself is written in C++ and it provides a simple wrapper around Vulkan and the basic rendering functionalities: shapes, textures, sprites, blending, render textures, etc.


Table of content
================

.. toctree::
 	setup
	rendering
	levelsets
	world
	rigidbody
	rendererapiref
	fluidapirref
