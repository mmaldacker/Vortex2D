//
//  Extrapolation.cpp
//  Vortex2D

#include "Extrapolation.h"

#include <Vortex2D/Renderer/Shader.h>
#include <Vortex2D/Renderer/Disable.h>

#include <Vortex2D/Engine/HelperFunctions.h>

namespace Vortex2D { namespace Fluid {

namespace
{

const char* ExtrapolateFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_valid;
    uniform sampler2D u_texture;

    float Extrapolate(int index)
    {
        float sum = 0.0;
        float count = 0.0;

        if (textureOffset(u_valid, v_texCoord, ivec2(1,0))[index] == 1.0)
        {
            sum += textureOffset(u_texture, v_texCoord, ivec2(1,0))[index];
            count += 1.0;
        }
        if (textureOffset(u_valid, v_texCoord, ivec2(0,1))[index] == 1.0)
        {
            sum += textureOffset(u_texture, v_texCoord, ivec2(0,1))[index];
            count += 1.0;
        }
        if (textureOffset(u_valid, v_texCoord, ivec2(-1,0))[index] == 1.0)
        {
            sum += textureOffset(u_texture, v_texCoord, ivec2(-1,0))[index];
            count += 1.0;
        }
        if (textureOffset(u_valid, v_texCoord, ivec2(0,-1))[index] == 1.0)
        {
            sum += textureOffset(u_texture, v_texCoord, ivec2(0,-1))[index];
            count += 1.0;
        }

        if (count > 0.0)
        {
            return sum / count;
        }
        else
        {
            return texture(u_texture, v_texCoord)[index];
        }
    }

    void main()
    {
        vec2 extrapolated_velocity = texture(u_texture, v_texCoord).xy;

        if (texture(u_valid, v_texCoord).x == 0.0)
        {
            extrapolated_velocity.x = Extrapolate(0);
        }

        if (texture(u_valid, v_texCoord).y == 0.0)
        {
            extrapolated_velocity.y = Extrapolate(1);
        }

        out_color = vec4(extrapolated_velocity, 0.0, 0.0);
    }
);

const char* ValidExtrapolateFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_valid;

    void main()
    {
        vec2 valid = vec2(0.0);

        if (texture(u_valid, v_texCoord).x == 1.0 ||
                textureOffset(u_valid, v_texCoord, ivec2(1,0)).x == 1.0 ||
                textureOffset(u_valid, v_texCoord, ivec2(0,1)).x == 1.0 ||
                textureOffset(u_valid, v_texCoord, ivec2(-1,0)).x == 1.0 ||
                textureOffset(u_valid, v_texCoord, ivec2(0,-1)).x == 1.0)
        {
            valid.x = 1.0;
        }

        if (texture(u_valid, v_texCoord).y == 1.0 ||
                textureOffset(u_valid, v_texCoord, ivec2(1,0)).y == 1.0 ||
                textureOffset(u_valid, v_texCoord, ivec2(0,1)).y == 1.0 ||
                textureOffset(u_valid, v_texCoord, ivec2(-1,0)).y == 1.0 ||
                textureOffset(u_valid, v_texCoord, ivec2(0,-1)).y == 1.0)
        {
            valid.y = 1.0;
        }

        out_color = vec4(valid, 0.0, 0.0);
    }
);

const char* ValidVelocitiesFrag = GLSL(
    uniform sampler2D u_velocity;

    in vec2 v_texCoord;
    out vec4 out_color;

    void main()
    {
        vec2 valid = vec2(0.0);

        if (texture(u_velocity, v_texCoord).x != 0.0)
        {
            valid.x = 1.0;
        }

        if (texture(u_velocity, v_texCoord).y != 0.0)
        {
            valid.y = 1.0;
        }

        out_color = vec4(valid, 0.0, 0.0);
    }

);

const char* ConstrainVelocityFrag = GLSL(
    uniform sampler2D u_velocity;
    uniform sampler2D u_obstacles;

    in vec2 v_texCoord;
    out vec4 out_color;

    vec2 get_weight(vec2 texCoord, sampler2D solid_phi);

    void main()
    {
        vec2 uv = texture(u_velocity, v_texCoord).xy;

        float v00 = texture(u_obstacles, v_texCoord).x;
        float v10 = textureOffset(u_obstacles, v_texCoord, ivec2(2,0)).x;
        float v01 = textureOffset(u_obstacles, v_texCoord, ivec2(0,2)).x;
        float v11 = textureOffset(u_obstacles, v_texCoord, ivec2(2,2)).x;

        vec2 constrained = vec2(0.0);
        vec2 wuv = get_weight(v_texCoord, u_obstacles);

        if (wuv.x == 0.0)
        {
            vec2 grad = vec2(mix(v10 - v00, v11 - v01, 0.5), v01 - v00);
            vec2 normal = normalize(grad);

            float vn = textureOffset(u_velocity, v_texCoord, ivec2(-1, 0)).y;
            float vp = textureOffset(u_velocity, v_texCoord, ivec2(-1, 1)).y;
            float up = textureOffset(u_velocity, v_texCoord, ivec2(0, 1)).y;

            float v = (vn + uv.y + vp + up) * 0.25;

            float perp_component = dot(normal, vec2(uv.x, v));

            constrained.x = normal.x * perp_component;
        }

        if (wuv.y == 0.0)
        {
            vec2 grad = vec2(v10 - v00, mix(v01 - v00, v11 - v10, 0.5));
            vec2 normal = normalize(grad);

            float vn = textureOffset(u_velocity, v_texCoord, ivec2(0, -1)).x;
            float vp = textureOffset(u_velocity, v_texCoord, ivec2(1, -1)).x;
            float up = textureOffset(u_velocity, v_texCoord, ivec2(1, 0)).x;

            float u = (vn + uv.x + vp + up) * 0.25;

            float perp_component = dot(normal, vec2(u, uv.y));

            constrained.y = normal.y * perp_component;
        }

        // FIXME need to set the obstacle velocity (means we don't have to set it in project?)
        out_color = vec4(uv - constrained, 0.0, 0.0);
    }
);

}

using Renderer::Back;

Extrapolation::Extrapolation(const glm::vec2& size,
                             Renderer::Buffer& velocity,
                             Renderer::Buffer& solidPhi)
    : mVelocity(velocity)
    , mSolidPhi(solidPhi)
    , mExtrapolateValid(size, 2, true)
    , mIdentity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
    , mExtrapolate(Renderer::Shader::TexturePositionVert, ExtrapolateFrag)
    , mValidExtrapolate(Renderer::Shader::TexturePositionVert, ValidExtrapolateFrag)
    , mValidVelocities(Renderer::Shader::TexturePositionVert, ValidVelocitiesFrag)
    , mConstrainVelocity(Renderer::Shader::TexturePositionVert, ConstrainVelocityFrag, WeightHelperFrag)
    , mSurface(size)
{
    mIdentity.Use().Set("u_texture", 0);
    mExtrapolate.Use().Set("u_valid", 0).Set("u_texture", 1);
    mValidExtrapolate.Use().Set("u_valid", 0);
    mValidVelocities.Use().Set("u_velocity", 0);
    mConstrainVelocity.Use().Set("u_velocity", 0).Set("u_obstacles", 1);
}

void Extrapolation::Extrapolate()
{
    mExtrapolateValid = mValidVelocities(mVelocity);

    const int layers = 10;
    for (int i = 0; i < layers; i++)
    {
        mVelocity.Swap();
        mVelocity = mExtrapolate(mExtrapolateValid, Back(mVelocity));
        mExtrapolateValid.Swap();
        mExtrapolateValid = mValidExtrapolate(Back(mExtrapolateValid));
    }
}

void Extrapolation::ConstrainVelocity()
{
    mVelocity.Swap();
    mVelocity = mConstrainVelocity(Back(mVelocity), mSolidPhi);
}


}}
