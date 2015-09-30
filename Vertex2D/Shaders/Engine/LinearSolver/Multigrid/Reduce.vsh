#version 150

precision highp float;

in vec2 a_Position;
in vec2 a_TexCoords;

uniform mat4 u_Projection;
uniform vec2 h;

out vec2 v_texCoord;
out vec2 v_texCoordx;
out vec2 v_texCoordy;
out vec2 v_texCoordxy;

const vec2 off = vec2(0.5);

void main()
{
    gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);

    vec2 dx = vec2(1.0/h.x, 0.);
    vec2 dy = vec2(0., 1.0/h.y);
    vec2 k = ceil(h/vec2(2.0));

    v_texCoord = (vec2(2.0) * (a_TexCoords * k - off) + off) / h;
    v_texCoordx = v_texCoord + dx;
    v_texCoordy = v_texCoord + dy;
    v_texCoordxy = v_texCoord + dx + dy;
}
