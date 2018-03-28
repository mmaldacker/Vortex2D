//
//  World.h
//  Vortex2D
//

#ifndef Vortex2D_Engine_h
#define Vortex2D_Engine_h

#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Shapes.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/LinearSolver/ConjugateGradient.h>
#include <Vortex2D/Engine/LinearSolver/Multigrid.h>
#include <Vortex2D/Engine/Size.h>
#include <Vortex2D/Engine/Extrapolation.h>
#include <Vortex2D/Engine/LevelSet.h>
#include <Vortex2D/Engine/Pressure.h>
#include <Vortex2D/Engine/Advection.h>
#include <Vortex2D/Engine/Particles.h>
#include <Vortex2D/Engine/Velocity.h>
#include <Vortex2D/Engine/Rigidbody.h>
#include <Vortex2D/Engine/Boundaries.h>
#include <Vortex2D/Engine/Density.h>

#include <vector>
#include <memory>
#include <functional>

namespace Vortex2D { namespace Fluid {

/**
 * @brief The main class of the framework. Each instance manages a grid and this class
 * is used to set forces, define boundaries, solve the incompressbility equations and do the
 * advection.
 */
class World
{
public:
    /**
     * @brief Construct an Engine with a size and time step.
     * @param device vulkan device
     * @param dimensions dimensions of the simulation
     * @param dt timestamp of the simulation, e.g. 0.016 for 60FPS simulations.
     */
    World(const Renderer::Device& device, Dimensions dimensions, float dt);
    virtual ~World() {}

    /**
     * @brief Perform one step of the simulation.
     */
    virtual void Solve() = 0;

    /**
     * @brief Record drawables to the velocity field. The colour (r,g) will be used as the velocity (x, y)
     * @param drawables a list of drawable field
     * @return render command
     */
    Renderer::RenderCommand RecordVelocity(Renderer::RenderTarget::DrawableList drawables);

    /**
     * @brief submit the render command created with @ref RecordVelocity
     * @param renderCommand the render command
     */
    void SubmitVelocity(Renderer::RenderCommand& renderCommand);

    /**
     * @brief Record drawables to the liquid level set, i.e. to define the fluid area.
     * The drawables need to make a signed distance field, if not the result is undefined.
     * @param drawables a list of signed distance field drawables
     * @return render command
     */
    Renderer::RenderCommand RecordLiquidPhi(Renderer::RenderTarget::DrawableList drawables);

    /**
     * @brief Record drawables to the solid level set, i.e. to define the boundary area.
     * The drawables need to make a signed distance field, if not the result is undefined.
     * @param drawables a list of signed distance field drawables
     * @return render command
     */
    Renderer::RenderCommand RecordStaticSolidPhi(Renderer::RenderTarget::DrawableList drawables);

    /**
     * @brief Create sprite that can be rendered to visualize the liquid level set.
     * @return a sprite
     */
    DistanceField LiquidDistanceField();

    /**
     * @brief Create sprite that can be rendered to visualize the solid level set.
     * @return a sprite
     */
    DistanceField SolidDistanceField();

    /**
     * @brief Create a rigid body and add it to the World. The lifetime of the rigidbody is tied to the lifetime of the World.
     * @param type type of the rigidbody: static, weak or strong
     * @param drawable a drawable that renders a signed distance field. This can be the sprite of a level set.
     * @param centre the centre of the drawable. Use for rotations.
     * @return a pointer to the rigid body.
     */
    RigidBody* CreateRigidbody(vk::Flags<RigidBody::Type> type, Renderer::Drawable& drawable, const glm::vec2& centre);

protected:
    const Renderer::Device& mDevice;
    Dimensions mDimensions;

    Multigrid mPreconditioner;
    ConjugateGradient mLinearSolver;

    LinearSolver::Data mData;
    Fluid::Velocity mVelocity;
    LevelSet mLiquidPhi;
    LevelSet mStaticSolidPhi;
    LevelSet mDynamicSolidPhi;

    Renderer::Buffer<glm::ivec2> mValid;

    Advection mAdvection;
    Pressure mProjection;
    Extrapolation mExtrapolation;

    Renderer::CommandBuffer mClearValid, mCopySolidPhi;
    Renderer::Buffer<RigidBody::Velocity> mForce;

    std::vector<std::unique_ptr<RigidBody>> mRigidbodies;

    std::vector<std::reference_wrapper<Renderer::RenderCommand>> mVelocities;
};

/**
 * @brief A concrete implementation of @ref World to simulate 'smoke', or more accurately dye in a liquid.
 * The liquid cannot change location or size.
 */
class SmokeWorld : public World
{
public:
    SmokeWorld(const Renderer::Device& device, Dimensions dimensions, float dt);

    void Solve() override;

    /**
     * @brief Bind a density field to be moved around with the fluid
     * @param density the density field
     */
    void FieldBind(Density& density);
};

/**
 * @brief A concrete implementation of @ref World to simulate water.
 */
class WaterWorld : public World
{
public:
    WaterWorld(const Renderer::Device& device, Dimensions dimensions, float dt);

    void Solve() override;

    /**
     * @brief The water simulation uses particles to define the water area.
     * In fact, the level set is built from the particles. This means to be able to set an area,
     * we can't use @ref RecordLiquidPhi. To define the particle area, simply draw a regular shape.
     * The colour r is used to determine if we add or remove particles, use r = 4 to add and r = -4 to remove.
     * @param drawables list of drawables object with colour 4 or -4
     * @return render command
     */
    Renderer::RenderCommand RecordParticleCount(Renderer::RenderTarget::DrawableList drawables);

private:
    Renderer::GenericBuffer mParticles;
    ParticleCount mParticleCount;
    Renderer::CommandBuffer mClearVelocity;
};

}}

#endif
