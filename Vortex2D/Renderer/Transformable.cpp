//
//  Transformable.cpp
//  Vortex2D
//

#include "Transformable.h"

#include <glm/gtx/transform.hpp>

namespace Vortex2D { namespace Renderer {

template<typename T>
property<T>::property(Transformable& t) : t(t)
{

}

template<typename T>
T& property<T>::operator = (const T& i)
{
    value = i;
    t.Update();
    return value;
}

template<typename T>
property<T>::operator T const & () const
{
    return value;
}

template class property<float>;
template class property<glm::vec2>;
template class property<glm::vec4>;

Transformable::Transformable()
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

void Transformable::Update()
{
    mTransform = glm::translate(glm::vec3{(glm::vec2)Position, 0.0f});
    mTransform = glm::scale(mTransform, glm::vec3{(glm::vec2)Scale, 1.0f});
    mTransform = glm::rotate(mTransform, glm::radians((float)Rotation), glm::vec3{0.0f, 0.0f, 1.0f});
    mTransform = glm::translate(mTransform, glm::vec3{-(glm::vec2)Anchor, 0.0f});

    mInverseTransform = glm::inverse(mTransform);
}

const glm::mat4 & Transformable::GetTransform() const
{
    return mTransform;
}

/**
 * @brief Returns the inverse transform matrix
 */
const glm::mat4 & Transformable::GetInverseTransform() const
{
    return mInverseTransform;
}

}}
