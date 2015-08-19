precision mediump float;

uniform mediump vec4 u_Colour;
uniform mediump float u_Smooth;

void main()
{
    vec2 pt = 2.0 * (gl_PointCoord - vec2(0.5));

    float distance = dot(pt,pt);
    float factor = smoothstep(u_Smooth, 1.0, distance);
	gl_FragColor = vec4(u_Colour.xyz, 1.0 - factor);
}
