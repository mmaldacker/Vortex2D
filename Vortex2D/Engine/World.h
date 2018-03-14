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
     * @brief Construct an Engine with a size, linear solver and time step.
     */
    World(const Renderer::Device& device, Dimensions dimensions, float dt);
    virtual ~World() {}

    virtual void Solve() = 0;

    Renderer::RenderCommand RecordVelocity(Renderer::RenderTarget::DrawableList drawables);
    void SubmitVelocity(Renderer::RenderCommand& renderCommand);
    Renderer::RenderCommand RecordLiquidPhi(Renderer::RenderTarget::DrawableList drawables);
    Renderer::RenderCommand RecordStaticSolidPhi(Renderer::RenderTarget::DrawableList drawables);

    DistanceField LiquidDistanceField();
    DistanceField SolidDistanceField();

    RigidBody* CreateRigidbody(vk::Flags<RigidBody::Type> type, ObjectDrawable& drawable, const glm::vec2& centre);

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

class SmokeWorld : public World
{
public:
    SmokeWorld(const Renderer::Device& device, Dimensions dimensions, float dt);

    void Solve() override;

    void FieldBind(Density& density);
};

class WaterWorld : public World
{
public:
    WaterWorld(const Renderer::Device& device, Dimensions dimensions, float dt);

    void Solve() override;

    Renderer::RenderCommand RecordParticleCount(Renderer::RenderTarget::DrawableList drawables);

private:
    Renderer::GenericBuffer mParticles;
    ParticleCount mParticleCount;
    Renderer::CommandBuffer mClearVelocity;
};

}}

#endif
