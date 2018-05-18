#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform Consts
{
  int n;
  int inv;
}consts;

layout(set = 0, binding = 1) uniform UBO
{
    mat4 mv;
}u;

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
    vec2 proj = (1.0 - t) * a + t * b;
    return distance(p, proj);
}

const float max_dist = 100000.0;

void main(void)
{
    vec2 pos = gl_FragCoord.xy - vec2(0.5);
    float value = (consts.inv == 1 ? 1.0 : -1.0) * max_dist;
    for (int i = consts.n - 1, j = 0; j < consts.n; i = j++)
    {
        vec2 a = (u.mv * vec4(polygon.points[i], 0.0, 1.0)).xy;
        vec2 b = (u.mv * vec4(polygon.points[j], 0.0, 1.0)).xy;

        float udist = dist_to_segment(a, b, pos);
        float dist = -orientation(a, b, pos) * udist;

        value = consts.inv == 1 ? min(value, dist) : max(value, dist);
    }

    out_colour = vec4(value);
}
