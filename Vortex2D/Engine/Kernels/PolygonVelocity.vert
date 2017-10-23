#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform Consts
{
    vec2 centre;
}consts;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) in vec2 position;

layout(binding = 0) uniform UBO
{
    mat4 mvp;
};

layout(binding = 1) uniform UBO2
{
    mat4 mv;
};

layout(location = 0) flat out vec2 position_centre;

void main()
{
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    vec4 centre = mv * vec4(consts.centre, 0.0, 1.0);
    position_centre = centre.xy;
}
