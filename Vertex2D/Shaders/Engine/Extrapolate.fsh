#version 150

precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture;

out vec4 out_color;

void main()
{
    vec2 wxp = textureOffset(u_texture, v_texCoord, ivec2(1,0)).xy;
    vec2 wxn = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).xy;
    vec2 wyp = textureOffset(u_texture, v_texCoord, ivec2(0,1)).xy;
    vec2 wyn = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).xy;

    vec2 total = vec2(0.0);
    float count = 0.0;

    if(wxp.x*wxp.x + wxp.y*wxp.y > 0.0)
    {
        total += wxp;
        count += 1.0;
    }

    if(wxn.x*wxn.x + wxn.y*wxn.y > 0.0)
    {
        total += wxn;
        count += 1.0;
    }

    if(wyp.x*wyp.x + wyp.y*wyp.y > 0.0)
    {
        total += wyp;
        count += 1.0;
    }

    if(wyn.x*wyn.x + wyn.y*wyn.y > 0.0)
    {
        total += wyn;
        count += 1.0;
    }

    out_color = vec4(total/count, 0.0, 0.0);
}
