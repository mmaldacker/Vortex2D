#version 450
#extension GL_ARB_separate_shader_objects : enable

out gl_PerVertex
{
  vec4 gl_Position;
};

layout(location = 0) in vec2 position;

layout(set = 0, binding = 0) uniform UBO
{
  mat4 mvp;
}
u;

void main()
{
  gl_Position = u.mvp * vec4(position, 0.0, 1.0);
}
