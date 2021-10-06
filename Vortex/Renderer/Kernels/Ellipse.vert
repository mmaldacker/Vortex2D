#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 a_Position;
layout(location = 1) out vec4 a_Centre;

layout(binding = 0) uniform UBO
{
  mat4 mvp;
}
u;

out gl_PerVertex
{
  vec4 gl_Position;
};

void main()
{
  gl_Position = u.mvp * vec4(a_Position, 0.0, 1.0);
  a_Centre = u.mvp * vec4(0.0, 0.0, 0.0, 1.0);
}
