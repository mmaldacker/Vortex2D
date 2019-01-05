.. _levelsets:

==========
Level sets
==========

A level set is a signed distance field. It's a field containing positive or negative value, where the values are 0 represent a contour, or border.
This is used to represent shapes, the numbers give you the distance to the shape border. 
It's the fundamental way that we represent the area of a fluid and the area of the obstacles, i.e. the boundaries.

Signed distance field
=====================


Basic shapes
============

There are several shapes that will set a level set, we draw them like we draw any shape. The format of the :cpp:func:`Vortex2D::Renderer::RenderTarget` must be ``vk::Format::eR32Sfloat``.
The class :cpp:func:`Vortex2D::Fluid::LevelSet` fullfills this. 

.. code-block:: cpp

	Vortex2D::Fluid::Rectangle rectangle(device, {100.0f, 100.0f});
	rectangle.Position = {40.0f, 60.0f};

	Vortex2D::Fluid::LevelSet levelSet(device, {400, 400});
	auto renderCmd = levelSet.Record({rectangle});
	renderCmd.Submit(); // note that renderCmd and rectangle have to be alive untill the rendering is done

Re-initialization
=================

We can also draw any other shape, and then transform the data into a correct level set. This operation is call re-initialization.