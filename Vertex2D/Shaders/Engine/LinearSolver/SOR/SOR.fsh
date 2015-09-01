precision highp float;

varying mediump vec2 v_texCoord;
varying mediump vec2 v_texCoordxp;
varying mediump vec2 v_texCoordxn;
varying mediump vec2 v_texCoordyp;
varying mediump vec2 v_texCoordyn;

uniform sampler2D u_texture; // this is the pressure 
uniform sampler2D u_weights;
uniform float w;

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

    float factor = dot(q,c);
    float pressure = 0.0;
    if(factor > 0.0)
    {
        pressure = mix(cell.y, (dot(p,c) + cell.x) / factor, w);
    }

    gl_FragColor = vec4(cell.x, pressure, 0.0, 0.0);
}