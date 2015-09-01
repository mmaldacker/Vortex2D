precision highp float;

varying mediump vec2 v_texCoord;
varying mediump vec2 v_texCoordxp;
varying mediump vec2 v_texCoordxn;
varying mediump vec2 v_texCoordyp;
varying mediump vec2 v_texCoordyn;

uniform sampler2D u_texture;
uniform sampler2D u_residual;

void main()
{
    float u = texture2D(u_texture, v_texCoord).x;
    float v = texture2D(u_residual, v_texCoord).x;

    gl_FragColor = vec4(u+v, 0.0, 0.0, 0.0);
}