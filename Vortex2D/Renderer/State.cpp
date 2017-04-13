//
//  Sprite.cpp
//  Vortex2D
//

#include "State.h"

#include <array>

namespace Vortex2D { namespace Renderer { namespace State {

using ViewPort = std::array<GLint, 4>;

namespace
{
    GLuint CurrentProgram = 0;

    GLuint BoundId[4] = {0};
    int ActiveUnit = -1;

    ViewPort CurrentViewPort = {{-1}};
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

void BindTexture(GLuint texture, int textureUnit)
{
    if (ActiveUnit != textureUnit)
    {
        ActiveUnit = textureUnit;
        glActiveTexture(GL_TEXTURE0+textureUnit);
    }

    if (BoundId[textureUnit] != texture)
    {
        BoundId[textureUnit] = texture;
        glBindTexture(GL_TEXTURE_2D, texture);
    }
}

void ClearTexture(GLuint texture)
{
    for (auto& id : BoundId)
    {
        if (id == texture)
        {
            id = 0;
        }
    }
}

void SetViewPort(GLint x, GLint y, GLsizei width, GLsizei height)
{
    ViewPort viewPort = {{x, y, width, height}};
    if (CurrentViewPort != viewPort)
    {
        CurrentViewPort = viewPort;
        glViewport(viewPort[0], viewPort[1], viewPort[2], viewPort[3]);
    }
}

}}}
