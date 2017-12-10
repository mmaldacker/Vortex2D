#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform Consts
{
  int width;
  int height;
  int n;
  int inv;
}consts;

layout(std430, binding = 2) buffer Polygon
{
  vec2 points[];
}polygon;

layout(location = 0) out vec4 out_colour;

// +1 if is left
float orientation(vec2 a, vec2 b, vec2 p)
{
    float v = ((b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x));
    if (v >= 0.0) return 1.0;
    else return -1.0;
}

float dist_to_segment(vec2 a, vec2 b, vec2 p)
{
    vec2 dir = b - a;
    float l = dot(dir, dir);

    float t = clamp(dot(p - a, dir) / l, 0.0, 1.0);
    vec2 proj = a + t * dir;
    return distance(p, proj);
}

void main(void)
{
    vec2 pos = gl_FragCoord.xy;
    float value = (consts.inv == 1 ? 1.0 : -1.0) * max(consts.width, consts.height);
    for (int i = consts.n - 1, j = 0; j < consts.n; i = j++)
    {
        float udist = dist_to_segment(polygon.points[i], polygon.points[j], pos);
        float dist = -orientation(polygon.points[i], polygon.points[j], pos) * udist;

        value = consts.inv == 1 ? min(value, dist) : max(value, dist);
    }

    out_colour = vec4(value);
}
