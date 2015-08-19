precision highp float;

attribute mediump vec2 a_Position;

uniform mat4 u_Projection;

void main()
{
    gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
}
