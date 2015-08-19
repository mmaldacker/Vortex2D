precision highp float;

attribute mediump vec2 a_Position;
attribute mediump vec4 a_Colour;

uniform mat4 u_Projection;

varying mediump vec4 v_Colour;

void main()
{
    gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
    v_Colour = a_Colour;
}
