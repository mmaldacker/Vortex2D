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
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

namespace Renderer
{

struct Transformable
{
    Transformable()
        : mPosition({0.0f, 0.0f})
        , mScale({1.0f, 1.0f})
        , mRotation(0.0f)
        , mAnchor({0.0f, 0.0f})
    {
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

    glm::vec2 & Position() { return mPosition; }
    void SetPosition(glm::vec2 position) { mPosition = position; Update(); }

    glm::vec2 & Scale() { return mScale; }
    void SetScale(glm::vec2 scale) { mScale = scale; Update(); }

    float & Rotation() { return mRotation; }
    void SetRotation(float rotation) { mRotation = rotation; Update(); }

    glm::vec2 & Anchor() { return mAnchor; }
    void SetAnchor(glm::vec2 anchor) { mAnchor = anchor; Update(); }

private:

    void Update()
    {
        mTransform = glm::translate(glm::vec3{mPosition, 0.0f});
        mTransform = glm::scale(mTransform, glm::vec3{mScale, 1.0f});
        mTransform = glm::rotate(mTransform, mRotation, glm::vec3{0.0f, 0.0f, 1.0f});
        mTransform = glm::translate(mTransform, glm::vec3{-mAnchor, 0.0f});

        mInverseTransform = glm::inverse(mTransform);
    }

    glm::mat4 mTransform;
    glm::mat4 mInverseTransform;

    glm::vec2 mPosition;
    glm::vec2 mScale;
    float mRotation;
    glm::vec2 mAnchor;
};

}

#endif
