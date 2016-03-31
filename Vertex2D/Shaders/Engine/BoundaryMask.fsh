#version 150

precision highp float;

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
