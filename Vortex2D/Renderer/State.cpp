//
//  Sprite.cpp
//  Vortex2D
//

#include "State.h"

namespace Vortex2D { namespace Renderer { namespace State {

namespace
{
    GLuint CurrentProgram = 0;
}

void UseProgram(GLuint program)
{
    if (CurrentProgram != program)
    {
        CurrentProgram = program;
        glUseProgram(program);
    }
}

void ClearProgram(GLuint program)
{
    if (CurrentProgram == program)
    {
        CurrentProgram = 0;
    }
}


}}}
