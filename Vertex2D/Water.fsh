#version 150

precision highp float;

in vec2 v_texCoord;
uniform sampler2D u_texture;

out vec4 out_color;

void main()
{
    float x = texture(u_texture, v_texCoord).x;
    if(x > 0.0)
    {
        out_color = vec4(0.0, 1.0, 0.0, 1.0);
    }
    else
    {
        out_color = vec4(0.0);
    }
}
