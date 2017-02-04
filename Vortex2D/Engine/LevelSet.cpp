//
//  LevelSet.cpp
//  Vortex2D
//

#include "LevelSet.h"
#include "Disable.h"

namespace Vortex2D { namespace  Fluid {

namespace
{

const char* RedistanceFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_levelSet;
    uniform sampler2D u_levelSet0;
    uniform float delta;

    const float dx = 1.0;

    float g(float s, float w, float wxp, float wxn, float wyp, float wyn)
    {
       float a = (w - wxn) / dx;
       float b = (wxp - w) / dx;
       float c = (w - wyn) / dx;
       float d = (wyp - w) / dx;

       if (s > 0)
       {
           float ap = max(a,0);
           float bn = min(b,0);
           float cp = max(c,0);
           float dn = min(d,0);

           return sqrt(max(ap * ap, bn * bn) + max(cp * cp, dn * dn)) - 1.0;
       }
       else
       {
           float an = min(a,0);
           float bp = max(b,0);
           float cn = min(c,0);
           float dp = max(d,0);

           return sqrt(max(an * an, bp * bp) + max(cn * cn, dp * dp)) - 1.0;
       }

    }

    void main()
    {
       float w0 = texture(u_levelSet0, v_texCoord).x;
       float wxp0 = textureOffset(u_levelSet0, v_texCoord, ivec2(1,0)).x;
       float wxn0 = textureOffset(u_levelSet0, v_texCoord, ivec2(-1,0)).x;
       float wyp0 = textureOffset(u_levelSet0, v_texCoord, ivec2(0,1)).x;
       float wyn0 = textureOffset(u_levelSet0, v_texCoord, ivec2(0,-1)).x;

       float w = texture(u_levelSet, v_texCoord).x;
       float wxp = textureOffset(u_levelSet, v_texCoord, ivec2(1,0)).x;
       float wxn = textureOffset(u_levelSet, v_texCoord, ivec2(-1,0)).x;
       float wyp = textureOffset(u_levelSet, v_texCoord, ivec2(0,1)).x;
       float wyn = textureOffset(u_levelSet, v_texCoord, ivec2(0,-1)).x;

       float s = sign(w0);

       if (w0 * wxp0 < 0.0 || w0 * wxn0 < 0.0 || w0 * wyp0 < 0.0 || w0 * wyn0 < 0.0)
       {
           /*
           float wx0 = wxp0 - wxn0;
           float wy0 = wyp0 - wyn0;
           float d = 2 * dx * w0 / sqrt(wx0 * wx0 + wy0 * wy0);
           */

           float wx0 = max(max(abs(0.5 * (wxp0 - wxn0)),
                               abs(wxp0 - w0)),
                           max(abs(w0 - wxn0),
                               0.001));
           float wy0 = max(max(abs(0.5 * (wyp0 - wyn0)),
                               abs(wyp0 - w0)),
                           max(abs(w0 - wyn0),
                               0.001));
           float d = dx * w0 / sqrt(wx0 * wx0 + wy0 * wy0);

           out_color = vec4(w - delta * (s * abs(w) - d) / dx, 0.0, 0.0, 0.0);
       }
       else
       {
           out_color = vec4(w - delta * s * g(s, w, wxp, wxn, wyp, wyn), 0.0, 0.0, 0.0);
       }

    }
);



}

using Renderer::Back;

LevelSet::LevelSet(const glm::vec2& size)
    : Renderer::Buffer(size, 1, true, true)
    , mLevelSet0(size, 1)
    , mRedistance(Renderer::Shader::TexturePositionVert, RedistanceFrag)
    , mIdentity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
{
    ClampToEdge();
    Linear();

    mLevelSet0.ClampToEdge();
    mRedistance.Use().Set("delta", 0.1f).Set("u_levelSet", 0).Set("u_levelSet0", 1).Unuse();
    mIdentity.Use().Set("u_texture", 0).Unuse();
}

void LevelSet::Redistance(int iterations)
{
    Renderer::Disable d(GL_BLEND);

    mLevelSet0 = mIdentity(*this);

    for (int i = 0; i < iterations; i++)
    {
        Swap();
        *this = mRedistance(Back(*this), mLevelSet0);
    }
}

}}
