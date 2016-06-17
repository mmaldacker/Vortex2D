//
//  Transformable.h
//  Vortex
//
//  Created by Maximilian Maldacker on 01/05/2014.
//
//

#ifndef Vortex_Transformable_h
#define Vortex_Transformable_h

#include "Common.h"

namespace Vortex2D { namespace Renderer {

struct Transformable;

/**
 * @brief Simple class to simulate properties like in C#
 */
template<typename T, typename Transformable = Transformable>
class property
{
    T value;
    Transformable & t;
public:
    property(Transformable & t) : t(t) {}

    T & operator = (const T &i)
    {
        value = i;
        t.Update();
        return value;
    }

    operator T const & () const
    {
        return value;
    }
};

/**
 * @brief Class to represent the transformation of an object: position, scale, rotation and anchor.
 */
struct Transformable
{
    Transformable()
        : Position(*this)
        , Scale(*this)
        , Rotation(*this)
        , Anchor(*this)
    {
        Position = {0.0f, 0.0f};
        Scale = {1.0f, 1.0f};
        Rotation = 0.0f;
        Anchor = {0.0f, 0.0f};
    }

    virtual ~Transformable(){}

    /**
     * @brief Returns the transform matrix
     */
    const glm::mat4 & GetTransform() const
    {
        return mTransform;
    }

    /**
     * @brief Returns the inverse transform matrix
     */
    const glm::mat4 & GetInverseTransform() const
    {
        return mInverseTransform;
    }

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
    void Update()
    {
        mTransform = glm::translate(glm::vec3{(glm::vec2)Position, 0.0f});
        mTransform = glm::scale(mTransform, glm::vec3{(glm::vec2)Scale, 1.0f});
        mTransform = glm::rotate(mTransform, (float)Rotation, glm::vec3{0.0f, 0.0f, 1.0f});
        mTransform = glm::translate(mTransform, glm::vec3{-(glm::vec2)Anchor, 0.0f});

        mInverseTransform = glm::inverse(mTransform);
    }

private:
    glm::mat4 mTransform;
    glm::mat4 mInverseTransform;
};

}}

#endif
