precision highp float;

varying mediump vec2 v_texCoord;
varying mediump vec2 v_texCoordxp;
varying mediump vec2 v_texCoordxn;
varying mediump vec2 v_texCoordyp;
varying mediump vec2 v_texCoordyn;

uniform sampler2D u_texture; // this is the obstacles

void main()
{
    float o = texture2D(u_texture, v_texCoord).x;

    vec4 p;
    p.x = texture2D(u_texture, v_texCoordxp).x;
    p.y = texture2D(u_texture, v_texCoordxn).x;
    p.z = texture2D(u_texture, v_texCoordyp).x;
    p.w = texture2D(u_texture, v_texCoordyn).x;

    gl_FragColor = vec4(1.0) - p;
}
