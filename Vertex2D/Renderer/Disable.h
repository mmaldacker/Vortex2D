//
//  Disable.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 19/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_Disable_h
#define Vertex2D_Disable_h

#include "Common.h"

namespace Renderer
{

struct Disable
{
    Disable(GLenum e): e(e)
    {
        glGetIntegerv(e, &enabled);
        if(enabled) glDisable(e);
    }

    ~Disable()
    {
        if(enabled) glEnable(e);
    }

    GLenum e;
    GLint enabled;
};

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

struct Enable
{
    Enable(GLenum e): e(e)
    {
        glGetIntegerv(e, &enabled);
        if(!enabled) glEnable(e);
    }

    ~Enable()
    {
        if(!enabled) glDisable(e);
    }
    
    GLenum e;
    GLint enabled;
};

struct BlendState
{
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

    GLint blendEquation, blendSrc, blendDst;
};

}

#endif
