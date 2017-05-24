#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 a_Position;

layout(binding = 0) uniform UBO
{
    mat4 mvp;
} u;

layout(std140, binding = 1) uniform Size
{
    float size;
    vec2 radius;
    mat2 rotation;
} s;

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_PointSize;
};

void main()
{
    gl_PointSize = 2 * s.size + 1;
    gl_Position = u.mvp * vec4(a_Position, 0.0, 1.0);
}
