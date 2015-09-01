precision highp float;

varying mediump vec2 v_texCoord;
varying mediump vec2 v_texCoordx;
varying mediump vec2 v_texCoordy;
varying mediump vec2 v_texCoordxy;

uniform sampler2D u_texture;

const vec4 q = vec4(1.0);

void main()
{
    vec4 p;
    p.x = texture2D(u_texture, v_texCoord).y;
    p.y = texture2D(u_texture, v_texCoordx).y;
    p.z = texture2D(u_texture, v_texCoordy).y;
    p.w = texture2D(u_texture, v_texCoordxy).y;

    gl_FragColor = vec4(dot(p,q) * 0.25, 0.0, 0.0, 0.0);
}