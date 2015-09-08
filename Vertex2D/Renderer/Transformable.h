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

namespace Renderer
{

struct Transformable;

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

    glm::mat4 GetTransform(const glm::mat4 & m = glm::mat4())
    {
        return m*mTransform;
    }

    const glm::mat4 & GetInverseTransform()
    {
        return mInverseTransform;
    }

    property<glm::vec2> Position;
    property<glm::vec2> Scale;
    property<float> Rotation;
    property<glm::vec2> Anchor;

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

}

#endif
