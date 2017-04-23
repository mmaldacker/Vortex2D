out vec4 out_color;

uniform vec4 u_Colour;
uniform float u_size;
uniform vec2 u_radius;
uniform mat2 u_rotation;

void main()
{
    float size = 2 * u_size + 1;
    vec2 pos = (gl_PointCoord * size) - u_size;
    pos = u_rotation * pos;
    float distance = dot(pos / u_radius, pos / u_radius);
    if (distance - 1.0 <= 0.0)
    {
        out_color = u_Colour;
    }
    else
    {
        discard;
    }
}
