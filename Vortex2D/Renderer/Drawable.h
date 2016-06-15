//
//  Drawable.h
//  Vortex
//
//  Created by Maximilian Maldacker on 29/04/2014.
//
//

#ifndef Vortex_Drawable_h
#define Vortex_Drawable_h

#include "Common.h"

namespace Renderer
{

struct Drawable
{
    virtual ~Drawable(){}
    virtual void Render(const glm::mat4 & ortho) = 0;
};

using DrawablesVector = std::vector<Drawable*>;

}

#endif
