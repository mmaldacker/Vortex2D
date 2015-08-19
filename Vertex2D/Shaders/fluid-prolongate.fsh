precision highp float;

varying mediump vec2 v_texCoord;
varying mediump vec2 v_texCoordxp;
varying mediump vec2 v_texCoordxn;
varying mediump vec2 v_texCoordyp;
varying mediump vec2 v_texCoordyn;

uniform sampler2D u_texture;

void main()
{
    float x = texture2D(u_texture, v_texCoord).x;

    gl_FragColor = vec4(x, 0.0, 0.0, 0.0);
}