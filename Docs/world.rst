=====
World
=====

The world classes are the centre of the engine, where the fluid gets animated. They contain essentially three fields:

 * The velocity field
 * The liquid phi field
 * The solid phi field

The first one contain the velocity of the fluid at every point, the second one defines where the fluid is. This is a signed distance field where a negative value indicates this is a fluid location. Finally the last one contains the location of solid obstacles, again as a signed distance field where the negative values indicate the solid's location. 

Each can be visualised as a texture with the getters:

.. code-block:: cpp

    Renderer::RenderTexture& GetVelocity();
    DistanceField LiquidDistanceField();
    DistanceField SolidDistanceField();

Of course, to get interesting fluid simulations, we need to set values on them. Setting the signed distance fields is straightword (see :ref:`levelsets`):

.. code-block:: cpp

    Renderer::RenderCommand RecordLiquidPhi(Renderer::RenderTarget::DrawableList drawables);
    Renderer::RenderCommand RecordStaticSolidPhi(Renderer::RenderTarget::DrawableList drawables);

Note that this only has to be done once. 

For velocities however, the simulation needs to set the velocities at a specific time during the simulation, so instead of ourselves calling :cpp:func:`Vortex::Renderer::RenderCommand::Submit` we pass the :cpp:func:`Vortex::Renderer::RenderCommand` to the :cpp:func:`World::Fluid::World` class:

.. code-block:: cpp

    Renderer::RenderCommand RecordVelocity(Renderer::RenderTarget::DrawableList drawables);
    void SubmitVelocity(Renderer::RenderCommand& renderCommand);

Stepping through the simulation is done with the :cpp:func:`Vortex::Fluid::World::Step` function, which takes as parameter the number of iterations used in the linear solver.
This can either be a fixed number of steps, or until the error reaches a certain threshhold.

.. code-block:: cpp

   auto iterations = Fluid::FixedParams(12);
   world.Step(iterations);

Smoke World
===========

This is a type of fluid simulation where the fluid area doesn't move. This is used to simulate smoke type effects by having a colored texture be advected by the velocity field.

The class :cpp:class:`Vortex::Fluid::Density` is used for this, it is simply a texture that can be rendered (i.e. a sprite).

The simulation is setup as so:

.. code-block:: cpp

    Fluid::Density density(device, size, vk::Format::eR8G8B8A8);
    Fluid::SmokeWorld world(device, size, 0.033);
    world.FieldBind(density);

Water World
===========

This is a classical water type of fluid simulation. This has a fluid area which evoles over time, i.e. a area of water moving. 
The area of water and non-water can be specified by rendering onto the word, where each pixel indicates the number of particles to add/substract.

.. code-block:: cpp

    Renderer::RenderCommand RecordParticleCount(Renderer::RenderTarget::DrawableList drawables);

The constraint is that the drawable needs to render integer values, which is provided for example by :cpp:class:`Vortec2D::Renderer::IntRectangle` and used:

.. code-block:: cpp

    Renderer::IntRectangle fluid(device, {150.0f, 50.0f});
    fluid.Position = {50.0f, 25.0f};
    fluid.Colour = glm::vec4(4); // can also be -4

    world.RecordParticleCount({fluid}).Submit().Wait();
