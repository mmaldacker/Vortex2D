=========
Rendering
=========

Initialization
==============

The rendering API is very basic and supports only the most basic functionality.

Create an instance of :cpp:class:`Vortex2D::Renderer::Instance` which is then used to create an instance of :cpp:class:`Vortex2D::Renderer::Device`.

The device is then used to create any other object. The main one is the :cpp:class:`Vortex2D::Renderer::RenderWindow` which is a window where to render sprites and polygons.
The function :cpp:func:`Vortex2D::Fluid::RenderWindow::Display()` is then used to present the result to the screen.

.. code-block:: cpp

	Vortex2D::Renderer::Instance instance("Application name", extensions); // pass list of required extensions
	Vortex2D::Renderer::Device device(instance.GetPhysicalDevice(), surface);

	Vortex2D::Renderer::RenderWindow window(device, surface, width, height);

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

Rendering shapes
================

We are now ready to draw things on the screen. Let's start with some shapes like rectangles and circles:

.. code-block:: cpp

    Vortex2D::Renderer::Rectangle rectangle(device, {100.0f, 100.0f});
    Vortex2D::Renderer::Ellipse circle(device, {50.0f, 50.0f});

    rectangle.Colour = {1.0f, 0.0f, 0.0f, 1.0f}; 
    rectangle.Position = {200.0f, 100.0f};

    circle.Colour = {0.0f, 0.0f, 1.0f, 1.0f};
    circle.Position = {500.0f, 400.0f};

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

Rendering textures
==================

Of course we can also render textures, using sprites. 

.. code-block:: cpp

	Vortex2D::Renderer::Texture texture(device, 100, 100, vk::Format::eR8G8B8A8Unorm);
	Vortex2D::Renderer::Sprite sprite(device, texture);