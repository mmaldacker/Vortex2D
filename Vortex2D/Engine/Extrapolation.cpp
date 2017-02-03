//
//  Extrapolation.cpp
//  Vortex2D

#include "Extrapolation.h"
#include "Shader.h"
#include "Disable.h"

namespace Vortex2D { namespace Fluid {

namespace
{

const char * ExtrapolateFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_mask;
    uniform sampler2D u_texture;

    void main()
    {
        vec2 sum = vec2(0.0);
        float count = 0.0;

        if(texture(u_mask, v_texCoord).x == 0.0)
        {
            if(textureOffset(u_mask, v_texCoord, ivec2(1,0)).x == 1.0)
            {
                sum += textureOffset(u_texture, v_texCoord, ivec2(1,0)).xy;
                count += 1.0;
            }
            if(textureOffset(u_mask, v_texCoord, ivec2(0,1)).x == 1.0)
            {
                sum += textureOffset(u_texture, v_texCoord, ivec2(0,1)).xy;
                count += 1.0;
            }
            if(textureOffset(u_mask, v_texCoord, ivec2(-1,0)).x == 1.0)
            {
                sum += textureOffset(u_texture, v_texCoord, ivec2(-1,0)).xy;
                count += 1.0;
            }
            if(textureOffset(u_mask, v_texCoord, ivec2(0,-1)).x == 1.0)
            {
                sum += textureOffset(u_texture, v_texCoord, ivec2(0,-1)).xy;
                count += 1.0;
            }

            if(count > 0)
            {
                out_color = vec4(sum/count, 0.0, 0.0);
            }
            else
            {
                out_color = vec4(texture(u_texture, v_texCoord).xy, 0.0, 0.0);
            }
        }
        else
        {
            out_color = vec4(texture(u_texture, v_texCoord).xy, 0.0, 0.0);
        }
    }
);

const char * ExtrapolateMaskFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_mask;

    void main()
    {
        if(texture(u_mask, v_texCoord).x == 1.0 ||
           textureOffset(u_mask, v_texCoord, ivec2(1,0)).x == 1.0 ||
           textureOffset(u_mask, v_texCoord, ivec2(0,1)).x == 1.0 ||
           textureOffset(u_mask, v_texCoord, ivec2(-1,0)).x == 1.0 ||
           textureOffset(u_mask, v_texCoord, ivec2(0,-1)).x == 1.0)
        {
            out_color = vec4(1.0, 0.0, 0.0, 0.0);
        }
        else
        {
            out_color = vec4(0.0);
        }
    }
);

}

using Renderer::Back;

Extrapolation::Extrapolation(Dimensions dimensions)
    : mExtrapolateValid(dimensions.Size, 1, true, true)
    , mIdentity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
    , mExtrapolate(Renderer::Shader::TexturePositionVert, ExtrapolateFrag)
    , mExtrapolateMask(Renderer::Shader::TexturePositionVert, ExtrapolateMaskFrag)
    , mSurface(dimensions.Size)
{
    mIdentity.Use().Set("u_texture", 0).Unuse();
    mExtrapolate.Use().Set("u_mask", 0).Set("u_texture", 1).Unuse();
    mExtrapolateMask.Use().Set("u_mask", 0).Unuse();
}

void Extrapolation::Extrapolate(Renderer::Buffer& buffer, LevelSet& neumann, LevelSet& dirichlet)
{
    mExtrapolateValid.Clear(glm::vec4(0.0));
    mExtrapolateValid.ClearStencil();

    //neumann.RenderMask(mExtrapolateValid);
    //dirichlet.RenderMask(mExtrapolateValid);

    buffer.Swap() = mIdentity(Back(buffer));

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0, 0xFF);

    mSurface.Colour = glm::vec4(1.0f);
    mExtrapolateValid.Render(mSurface);
    mExtrapolateValid.Swap().Render(mSurface);

    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

    for(int i = 0 ; i < 20 ; i++)
    {
        buffer.Swap() = mExtrapolate(mExtrapolateValid, Back(buffer));
        mExtrapolateValid.Swap() = mExtrapolateMask(Back(mExtrapolateValid));
    }
}

}}
