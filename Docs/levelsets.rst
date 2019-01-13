.. _levelsets:

==========
Level sets
==========

A level set is a signed distance field. It's a field containing positive or negative value, where the values are 0 represent a contour, or border.
This is used to represent shapes, the numbers give you the distance to the shape border. 
It's the fundamental way that we represent the area of a fluid and the area of the obstacles, i.e. the boundaries.

The level set is represented simply as a float texture. To set the level set, we simply render on that texture. This means that the class :cpp:class:`Vortex2D::Fluid::LevelSet` inherits :cpp:class:`Vortex2D::Renderer::RenderTexture`.

Basic shapes
============

There is a list of basic shapes that can be used to render on a level set:

 * :cpp:class:`Vortex2D::Fluid::Rectangle`
 * :cpp:class:`Vortex2D::Fluid::Polygon`
 * :cpp:class:`Vortex2D::Fluid::Circle`

They are used the same way as regular drawable shapes, i.e. 

.. code-block:: cpp

	Vortex2D::Fluid::Rectangle rectangle(device, {100.0f, 100.0f});
	rectangle.Position = {40.0f, 60.0f};

	Vortex2D::Fluid::LevelSet levelSet(device, {400, 400});
	auto renderCmd = levelSet.Record({rectangle});
	renderCmd.Submit(); // note that renderCmd and rectangle have to be alive untill the rendering is done

Combining shapes
================

Multiple shapes can be combined together to build the level set. You can either take the union or the intersection when rendering. This happens by using certain blend states which are:

 * :cpp:enumerator:`Vortex2D::Renderer::IntersectionBlend`
 * :cpp:enumerator:`Vortex2D::Renderer::UnionBlend`

 After combining several shapes, the resulting float texture is not a signed distance field. It needs to be reinitialised which is simply done by calling :cpp:func:`Vortex2D::Fluid::LevelSet::Reinitialise`.