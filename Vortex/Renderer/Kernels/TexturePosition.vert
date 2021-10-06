#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoords;

layout(location = 0) out vec2 v_texCoord;

layout(binding = 0) uniform UniformBufferObject
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
  v_texCoord = a_TexCoords;
}
