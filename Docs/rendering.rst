=========
Rendering
=========

Initialization
==============

The rendering API is very basic and supports only the most basic functionality.

Create an instance of :cpp:class:`Vortex::Renderer::Instance` which is then used to create an instance of :cpp:class:`Vortex::Renderer::Device`.

The device is then used to create any other object. The main one is the :cpp:class:`Vortex::Renderer::RenderWindow` which is a window where to render sprites and polygons.
The function :cpp:func:`Vortex::Fluid::RenderWindow::Display()` is then used to present the result to the screen.

.. code-block:: cpp

  Vortex::Renderer::Instance instance("Application name", extensions); // pass list of required extensions
  Vortex::Renderer::Device device(instance.GetPhysicalDevice(), surface);

  Vortex::Renderer::RenderWindow window(device, surface, width, height);

Note that the instance requires a list of extensions necessary to create a window. With GLFW they can be retrived as:

.. code-block:: cpp

	std::vector<const char*> GetGLFWExtensions()
	{
	    std::vector<const char*> extensions;
	    unsigned int glfwExtensionCount = 0;
	    const char** glfwExtensions;

	    // get the required extensions from GLFW
	    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	    for (unsigned int i = 0; i < glfwExtensionCount; i++)
	    {
	        extensions.push_back(glfwExtensions[i]);
	    }

	    return extensions;
	}

In addition, you also need to create a surface which can be also done with the help of GLFW:

.. code-block:: cpp

	vk::UniqueSurfaceKHR GetGLFWSurface(GLFWwindow* window, vk::Instance instance)
	{
	    // create surface
	    VkSurfaceKHR surface;
	    if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), window, nullptr, &surface) != VK_SUCCESS)
	    {
	        throw std::runtime_error("failed to create window surface!");
	    }

	    return vk::UniqueSurfaceKHR(surface, vk::SurfaceKHRDeleter{instance});
	}

Render Targets
==============

To be able to render, we need to record :cpp:class:`Vortex::Renderer::RenderCommand` on a :cpp:class:`Vortex::Renderer::RenderTarget`. There are two implementations of it:

 * :cpp:class:`Vortex::Renderer::RenderWindow`
 * :cpp:class:`Vortex::Renderer::RenderTexture`

You can render implementations of the abstract class :cpp:class:`Vortex::Renderer::Drawable`, which get recorder in the render command. To actually render it on the render target, the submit function needs to be called. Note, it can be called repeatedly (e.g. over several frames).

In addition, the blend state needs to be passed in, see :cpp:class:`Vortex::Renderer::ColorBlendState`.

Shapes
======

We are now ready to draw things on the screen. Let's start with some shapes like rectangles and circles:

.. code-block:: cpp

    Vortex::Renderer::Rectangle rectangle(device, {100.0f, 100.0f});
    Vortex::Renderer::Ellipse circle(device, {50.0f, 50.0f});

    auto blendMode = vk::PipelineColorBlendAttachmentState()
        .setBlendEnable(true)
        .setAlphaBlendOp(vk::BlendOp::eAdd)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

    // note that rectangle, circle and render need to be alive for the duration of the rendering
    auto render = renderTarget.Record({rectangle, circle}, blendMode);
    render.Submit();

Textures
========

Of course we can also render textures, using sprites. 

.. code-block:: cpp

  Vortex::Renderer::Texture texture(device, 100, 100, vk::Format::eR8G8B8A8Unorm);
  Vortex::Renderer::Sprite sprite(device, texture);

Transformations
===============

The shapes and textures can be positioned, i.e. are transformable. You can set the following properties on them:

* Position
* Scale
* Rotation
* Anchor

As an example:

.. code-block:: cpp

    Vortex::Renderer::Ellipse circle(device, {50.0f, 50.0f});
    circle.Colour = {0.0f, 0.0f, 1.0f, 1.0f};
    circle.Position = {500.0f, 400.0f};
