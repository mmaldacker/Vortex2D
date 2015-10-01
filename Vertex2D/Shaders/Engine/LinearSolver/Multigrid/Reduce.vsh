#version 150

precision highp float;

in vec2 a_Position;
in vec2 a_TexCoords;

uniform mat4 u_Projection;
uniform sampler2D u_texture;

out vec2 v_texCoord;

const vec2 off = vec2(0.5);

void main()
{
    gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);

    vec2 h = textureSize(u_texture, 0);
    vec2 k = ceil(h/vec2(2.0));

    v_texCoord = (vec2(2.0) * (a_TexCoords * k - off) + off) / h;
}
