//
//  RenderTarget.h
//  Vortex2D
//
//  Created by Maximilian Maldacker on 15/06/2016.
//
//

#ifndef RenderTarget_h
#define RenderTarget_h

#include "Common.h"
#include "Drawable.h"

namespace Renderer
{

/**
 * @brief And interface to represent a target that can be rendered to.
 * This is implemented by the RenderWindow (in the examples) and the RenderTexture
 */
struct RenderTarget
{
    RenderTarget(float width, float height)
        : Orth(glm::ortho(0.0f, width, 0.0f, height))
    {}

    /**
     * @brief Clear clears the whole target with the colour
     */
    virtual void Clear(const glm::vec4 & colour) = 0;

    /**
     * @brief Render the object to the target
     * @param object An object whose class implements Drawable
     * @param transform An optional aditional transformation matrix to be applied before rendering
     */
    virtual void Render(Drawable & object, const glm::mat4 & transform = glm::mat4()) = 0;

    glm::mat4 Orth;
};

}

#endif /* RenderTarget_h */
