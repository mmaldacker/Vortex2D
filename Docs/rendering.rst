=========
Rendering
=========

The rendering API is very basic and supports only the most basic functionality.

Create an instance of ``Vortex2D::Renderer::Instance``` which is then used to create an instance of ``Vortex2D::Renderer::Device``.

The device is then used to create any other object. The main one is the ``Vortex2D::Renderer::RenderWindow`` which is a target where to render sprites and polygons.
The function ``Display()`` is then used to present the result to the screen.

.. doxygenclass:: Vortex2D::Renderer::Instance
.. doxygenclass:: Vortex2D::Renderer::Device