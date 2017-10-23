#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform Consts
{
    layout(offset = 8) int width;
    layout(offset = 12) int height;
}consts;

layout(binding = 2) uniform PolygonProperties
{
    vec2 velocity;
    float angular_velocity;
}properties;

layout(location = 0) in vec2 position_centre;

layout(location = 0) out vec4 out_colour;

vec2 get_velocity(vec2 pos)
{
    vec2 dir = vec2(-pos.y, pos.x);
    return properties.velocity + dir * properties.angular_velocity;
}

void main(void)
{
    vec2 velocity;
    velocity.x = get_velocity(gl_FragCoord.xy - vec2(0.0, 0.5) - position_centre).x / float(consts.width);
    velocity.y = get_velocity(gl_FragCoord.xy - vec2(0.5, 0.0) - position_centre).y / float(consts.width);

    out_colour = vec4(velocity, 0.0, 0.0);
}
