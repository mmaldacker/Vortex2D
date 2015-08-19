precision mediump float;

attribute mediump vec2 a_Position;

uniform float u_Radius;
uniform mat4 u_Projection;

void main()
{
    gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
    gl_PointSize = u_Radius;
}
