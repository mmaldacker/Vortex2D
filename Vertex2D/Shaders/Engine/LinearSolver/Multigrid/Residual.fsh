precision highp float;

varying mediump vec2 v_texCoord;
varying mediump vec2 v_texCoordxp;
varying mediump vec2 v_texCoordxn;
varying mediump vec2 v_texCoordyp;
varying mediump vec2 v_texCoordyn;

uniform sampler2D u_texture;
uniform sampler2D u_weights;

const vec4 q = vec4(1.0);

void main()
{
    // cell.x is div and cell.y is pressure
    vec2 cell = texture2D(u_texture, v_texCoord).xy;

    vec4 p;
    p.x = texture2D(u_texture, v_texCoordxp).y;
    p.y = texture2D(u_texture, v_texCoordxn).y;
    p.z = texture2D(u_texture, v_texCoordyp).y;
    p.w = texture2D(u_texture, v_texCoordyn).y;

    vec4 c = texture2D(u_weights, v_texCoord);

    float residual = dot(p,c) - dot(q,c) * cell.x + cell.y;

    gl_FragColor = vec4(residual, 0.0, 0.0, 0.0);
}