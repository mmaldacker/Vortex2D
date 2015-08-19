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

struct Enable
{
    Enable(GLenum e): e(e)
    {
        glEnable(e);
    }

    ~Enable()
    {
        glDisable(e);
    }
    
    GLenum e;
};

}

#endif
