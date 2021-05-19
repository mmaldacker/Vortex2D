==========
Rigid body
==========

Rigid bodies are the way to have dynamic interations with the fluid (other then changing the velocity field directly).
Vortex2D only provides a way to get current forces applied to the rigidbodies, and applying velocities from the rigidbody to the fluid.
Updating the rigidbody's forces, velocities and position needs to be done by a seperate engine, such as Box2D.

Rigidbodies have three types:

 * Static
 * Weak
 * Strong

Static bodies
-------------

Static bodies act on the fluid, but the fluid doesn't act on the fluid. 
They have a velocity that is imparted on the fluid. Think of motorized objects pushing through the fluid.

Weak/Strong bodies
------------------

Weak rigidbodies are affected by the fluid. They can also in turn, affect the fluid, which is called a strong coupling with the fluid.

Rigid body updates
==================

Mass and velocity is set using simple setter functions:

.. code-block:: cpp

    Rigidbody rigidbody(device, size, drawable, type);
    rigidbody.SetMassData(mass, inertia);
    rigidbody.SetVelocities(velocity, angle);

Position and orientation is updated the same as with shapes:

.. code-block:: cpp

    rigidbody.Position = {100.0f, 100.0f}
    rigidbody.Rotation = 43.0f;

Rigid body coupling
===================

To have the fluid influence the rigid bodies and vice versa, 
two functions need to be implemented by deriving:

 * :cpp:func:`Vortex::Fluid::RigidBody::ApplyForces`
 * :cpp:func:`Vortex::Fluid::RigidBody::ApplyVelocities`

The first one has forces from the fluid applied to the rigidbody.
The second has velocities from the rigidbody applied to the fluid.

An example implementation with Box2D is as follow:

.. code-block:: cpp

    void Box2DRigidbody::ApplyForces()
    {
      if (GetType() & Vortex::Fluid::RigidBody::Type::eWeak)
      {
        auto force = GetForces();
        b2Vec2 b2Force = {force.velocity.x, force.velocity.y};

        mBody->ApplyForceToCenter(b2Force, true);
        mBody->ApplyTorque(force.angular_velocity, true);
      }
    }

    void Box2DRigidbody::ApplyVelocities()
    {
      auto pos = mBody->GetPosition();
      Position = {pos.x, pos.y};
      Rotation = glm::degrees(mBody->GetAngle());

      if (GetType() & Vortex::Fluid::RigidBody::Type::eStatic)
      {
        glm::vec2 vel = {mBody->GetLinearVelocity().x, mBody->GetLinearVelocity().y};
        float angularVelocity = mBody->GetAngularVelocity();
        SetVelocities(vel, angularVelocity);
      }
    }

Note that any rigidbody physics can be used: chipmonk, bullet, etc.

Engine updates
==============

Finally the rigidbody also needs to be updates, in lock-step with the fluid simulation.

Again, this is done by implementing :cpp:func:`Vortex::Fluid::RigidBody::Step`.

An example implementation with Box2D:

.. code-block:: cpp

    void Box2DSolver::Step(float delta)
    {
      const int velocityStep = 8;
      const int positionStep = 3;
      mWorld.Step(delta, velocityStep, positionStep);
    }

 The delta is the same used to create the world object.
