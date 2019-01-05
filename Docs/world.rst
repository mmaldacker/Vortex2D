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

For velocities however, the simulation needs to set the velocities at a specific time during the simulation, so instead of ourselves calling :cpp:func:`Vortex2D::Renderer::RenderCommand::Submit` we pass the :cpp:func:`Vortex2D::Renderer::RenderCommand` to the :cpp:func:`World::Fluid::World` class:

 .. code-block:: cpp

	Renderer::RenderCommand RecordVelocity(Renderer::RenderTarget::DrawableList drawables);
    void SubmitVelocity(Renderer::RenderCommand& renderCommand);

Smoke World
===========

This is a type of fluid simulation where the fluid area doesn't move.

Water World
===========

This is a classical water type of fluid simulation.