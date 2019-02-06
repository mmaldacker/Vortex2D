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
=============

Static bodies act on the fluid, but the fluid doesn't act on the fluid. 
They have a velocity that is imparted on the fluid. Think of motorized objects pushing through the fluid.

Weak/Strong bodies
==============

Weak rigidbodies are affected by the fluid. They can also in turn, affect the fluid, which is called a strong coupling with the fluid.