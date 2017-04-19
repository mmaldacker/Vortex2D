//
//  Transformable.h
//  Vortex2D
//

#ifndef Vortex_Transformable_h
#define Vortex_Transformable_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D { namespace Renderer {

struct Transformable;

/**
 * @brief Simple class to simulate properties like in C#
 */
template<typename T>
class property
{
public:
    property(Transformable& t);

    T& operator = (const T& i);
    operator T const & () const;

private:
    T value;
    Transformable& t;
};

/**
 * @brief Class to represent the transformation of an object: position, scale, rotation and anchor.
 */
struct Transformable
{
    Transformable();

    virtual ~Transformable(){}

    /**
     * @brief Returns the transform matrix
     */
    const glm::mat4& GetTransform() const;

    /**
     * @brief Returns the inverse transform matrix
     */
    const glm::mat4& GetInverseTransform() const;

    /**
     * @brief absolute position
     */
    property<glm::vec2> Position;

    /**
     * @brief scale for the x and y components
     */
    property<glm::vec2> Scale;

    /**
     * @brief Rotation in radians
     */
    property<float> Rotation;

    /**
     * @brief An offset to the position (used for centering a shape)
     */
    property<glm::vec2> Anchor;

    /**
     * @brief Update the transform matrix with the position, scale, rotation and anchor.
     */
    void Update();

private:
    glm::mat4 mTransform;
    glm::mat4 mInverseTransform;
};

}}

#endif
