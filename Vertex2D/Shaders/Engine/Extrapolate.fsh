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

    out_color = vec4((wxp + wxn + wyp + wyn)/vec2(4.0), 0.0, 0.0);
}
