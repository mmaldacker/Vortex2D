#version 150

precision highp float;

in vec2 v_texCoord;

out vec4 out_color;

uniform sampler2D u_texture;

void main()
{
    float x = texture(u_texture, v_texCoord).x;

    if(x < 0.0)
    {
        out_color = vec4(1.0, 0.0, 0.0, 0.0);
    }
    else
    {
        out_color = vec4(0.0);
    }
}
