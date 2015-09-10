#version 150

precision highp float;

in vec2 a_Position;
in vec2 a_TexCoords;

out vec2 v_texCoord;

uniform mat4 u_Projection;

void main()
{
    gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
    v_texCoord = a_TexCoords;
}
