//
//  Extrapolation.cpp
//  Vortex2D

#include "Extrapolation.h"
#include "Shader.h"
#include "Disable.h"

namespace Vortex2D { namespace Fluid {

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

const char * BoundaryMaskFrag = GLSL(
     in vec2 v_texCoord;

     uniform sampler2D u_dirichlet;
     uniform sampler2D u_neumann;

     void main()
     {
         float x = texture(u_dirichlet, v_texCoord).x;
         float y = texture(u_neumann, v_texCoord).x;
         
         if(x < 1.0 && y < 1.0)
         {
             discard;
         }
     }
);

Extrapolation::Extrapolation(Dimensions dimensions)
    : mExtrapolateValid(dimensions.Size, 1, true, true)
    , mIdentity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
    , mExtrapolate(Renderer::Shader::TexturePositionVert, ExtrapolateFrag)
    , mExtrapolateMask(Renderer::Shader::TexturePositionVert, ExtrapolateMaskFrag)
    , mBoundaryMask(Renderer::Shader::TexturePositionVert, BoundaryMaskFrag)
    , mSurface(dimensions.Size)
{
    mIdentity.Use().Set("u_texture", 0).Unuse();
    mExtrapolate.Use().Set("u_mask", 0).Set("u_texture", 1).Unuse();
    mExtrapolateMask.Use().Set("u_mask", 0).Unuse();
    mBoundaryMask.Use().Set("u_dirichlet", 0).Set("u_neumann", 1).Unuse();
}

void Extrapolation::RenderMask(Buffer & buffer, Buffer & dirichlet, Buffer & neumann)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    Renderer::DisableColorMask c;

    glStencilFunc(GL_ALWAYS, 1, 0xFF); // write 1 in stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // replace value with above
    glStencilMask(0xFF); // enable stencil writing

    buffer = mBoundaryMask(dirichlet, neumann);

    glStencilMask(0x00); // disable stencil writing
}

void Extrapolation::Extrapolate(Buffer & buffer, Buffer & dirichlet, Buffer & neumann)
{
    mExtrapolateValid.Clear(glm::vec4(0.0));
    mExtrapolateValid.ClearStencil();
    RenderMask(mExtrapolateValid, dirichlet, neumann);
    RenderMask(mExtrapolateValid.Swap(), dirichlet, neumann);

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