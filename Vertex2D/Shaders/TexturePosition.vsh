#version 150

precision highp float;

in mediump vec2 a_Position;
in mediump vec2 a_TexCoords;

out mediump vec2 v_TexCoords;

uniform mat4 u_Projection;

void main()
{
    gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
    v_TexCoords = a_TexCoords;
}
