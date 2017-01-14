//
//  Disable.h
//  Vortex2D
//

#ifndef Vortex2D_Disable_h
#define Vortex2D_Disable_h

#include "Common.h"

namespace Vortex2D { namespace Renderer {

/**
 * @brief a RAII class to disable an OpenGL state (GL_BLEND, GL_STENCIL) and re-enable it
 * if it was enabled before on destruction
 */
struct Disable
{
    /**
     * @brief Disables an OpenGL state
     * @param e an OpenGL enum like GL_BLEND or GL_STENCIL
     */
    Disable(GLenum e): e(e)
    {
        glGetIntegerv(e, &enabled);
        if(enabled) glDisable(e);
    }

    ~Disable()
    {
        if(enabled) glEnable(e);
    }

private:
    GLenum e;
    GLint enabled;
};

/**
 * @brief An RAII class to disable the colour mask and re-enable it on destruction
 */
struct DisableColorMask
{
    DisableColorMask()
    {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }

    ~DisableColorMask()
    {
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
};

/**
 * @brief The opposite of the Disable struct. Will enable an OpenGL state and disable it on destruct
 * if it was already enabled
 */
struct Enable
{
    /**
     * @brief Will enable the state if it wasn't already
     * @param e An OpenGL state like GL_BLEND
     */
    Enable(GLenum e): e(e)
    {
        glGetIntegerv(e, &enabled);
        if(!enabled) glEnable(e);
    }

    ~Enable()
    {
        if(!enabled) glDisable(e);
    }

private:
    GLenum e;
    GLint enabled;
};

/**
 * @brief An RAII class to set a new point parameter (and restore on desctruction)
 */
struct EnablePointParameter
{
    EnablePointParameter(GLenum e, GLint value)
        : pointParamName(e)
    {
        glGetIntegerv(pointParamName, &pointValue);
        glPointParameteri(pointParamName, value);
    }

    ~EnablePointParameter()
    {
        glPointParameteri(pointParamName, pointValue);
    }


private:
    GLenum pointParamName;
    GLint pointValue;
};

/**
 * @brief RAII class to set the blend equations and restore the previous one on destruction
 */
struct BlendState
{
    /**
     * @brief Saves the current blend equations and set new ones
     * @param eq the blend equation
     * @param src the blend source
     * @param dst the blend destination
     */
    BlendState(GLint eq, GLint src, GLint dst)
    {
        // save blend state
        glGetIntegerv(GL_BLEND_EQUATION, &blendEquation);
        glGetIntegerv(GL_BLEND_SRC_RGB, &blendSrc);
        glGetIntegerv(GL_BLEND_DST_RGB, &blendDst);

        glBlendFunc(src, dst);
        glBlendEquation(eq);
    }

    ~BlendState()
    {
        // restore blend state
        glBlendFunc(blendSrc, blendDst);
        glBlendEquation(blendEquation);
    }

private:
    GLint blendEquation, blendSrc, blendDst;
};

}}

#endif
