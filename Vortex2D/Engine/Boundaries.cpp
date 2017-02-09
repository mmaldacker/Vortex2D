//
//  Boundaries.cpp
//  Vortex
//

#include "Boundaries.h"

#include <Vortex2D/Renderer/Disable.h>

namespace Vortex2D { namespace Fluid {

namespace
{

const char* SolidBoundariesFrag = GLSL(
    uniform sampler2D u_obstacles;

    in vec2 v_texCoord;
    out vec4 out_color;

    void main(void)
    {
        float f = texture(u_obstacles, v_texCoord).w;
        if (f > 0.0)
        {
            out_color = vec4(1.0);
        }
        else
        {
            out_color = vec4(-1.0);
        }
    }
);

const char* LiquidBoundariesFrag = GLSL(
    uniform sampler2D u_fluid;
    uniform sampler2D u_obstacles;

    in vec2 v_texCoord;
    out vec4 out_color;

    void main(void)
    {
        float f = texture(u_fluid, v_texCoord).w;
        if (f > 0.0)
        {
            float s = texture(u_obstacles, v_texCoord).w;
            if (s > 0.0)
            {
                out_color = vec4(1.0);
            }
            else
            {
                out_color = vec4(-1.0);
            }
        }
        else
        {
            out_color = vec4(1.0);
        }
    }
);

}

Boundaries::Boundaries(Dimensions dimensions, LevelSet& liquidPhi, LevelSet& solidPhi)
    : mDimensions(dimensions)
    , mLiquidPhi(liquidPhi)
    , mSolidPhi(solidPhi)
    , mLiquidDraw(dimensions.Size.x, dimensions.Size.y, Renderer::Texture::PixelFormat::RGBA8888)
    , mSolidDraw(dimensions.Size.x, dimensions.Size.y, Renderer::Texture::PixelFormat::RGBA8888)
    , mSolidBoundaries(Renderer::Shader::TexturePositionVert, SolidBoundariesFrag)
    , mLiquidBoundaries(Renderer::Shader::TexturePositionVert, LiquidBoundariesFrag)
{
    mSolidBoundaries.Use().Set("u_obstacles", 1).Unuse();
    mLiquidBoundaries.Use().Set("u_fluid", 0).Set("u_obstacles", 1).Unuse();
}

Boundaries::~Boundaries()
{
    mLiquidPhi = mLiquidBoundaries(mLiquidDraw, mSolidDraw);
    mSolidPhi = mSolidBoundaries(mSolidDraw);

    mLiquidPhi.Redistance(100);
    mSolidPhi.Redistance(100);
}

void Boundaries::DrawLiquid(Renderer::Drawable& drawable, bool invert)
{

    if (invert)
    {

    }
    else
    {
        Renderer::Disable d(GL_BLEND);
        mLiquidDraw.Render(drawable, mDimensions.InvScale);
    }
}

void Boundaries::DrawSolid(Renderer::Drawable& drawable, bool invert)
{
    if (invert)
    {

    }
    else
    {
        Renderer::Disable d(GL_BLEND);
        mSolidDraw.Render(drawable, glm::scale(glm::vec3(2.0f, 2.0f, 1.0f))*mDimensions.InvScale);
    }
}

void Boundaries::ClearSolid()
{
    mSolidDraw.Clear(glm::vec4(0.0f));
}

void Boundaries::ClearLiquid()
{
    mLiquidDraw.Clear(glm::vec4(0.0f));
}

}}
