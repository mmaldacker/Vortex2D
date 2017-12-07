#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (local_size_x_id = 1, local_size_y_id = 2) in;

layout(push_constant) uniform Consts
{
  int width;
  int height;
  float radius;
}consts;

layout(binding = 0) uniform MVP
{
  mat4 value;
}mvp;

layout(binding = 1, r32f) uniform image2D LevelSet;

layout(std430, binding = 2) buffer Circle
{
  vec2 point;
}circle;

const float udist_bias = 0.001;

void main(void)
{
    uvec2 localSize = gl_WorkGroupSize.xy; // Hack for Mali-GPU

    ivec2 pos = ivec2(gl_GlobalInvocationID);
    if (pos.x < consts.width && pos.y < consts.height)
    {
        float value = imageLoad(LevelSet, pos).x;
        vec2 dir = circle.point - pos;
        float scale = sqrt(mvp.value[0][0] * mvp.value[1][1]);
        float dist = length(dir) - consts.radius * scale;

        imageStore(LevelSet, pos, vec4(min(value, dist), 0.0, 0.0, 0.0));
    }
}
