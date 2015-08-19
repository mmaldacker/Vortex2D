precision highp float;

attribute mediump vec2 a_Position;
attribute mediump vec2 a_TexCoords;

uniform mediump mat4 u_Projection;
uniform vec2 h;

varying mediump vec2 v_texCoord;
varying mediump vec2 v_texCoordxp;
varying mediump vec2 v_texCoordxn;
varying mediump vec2 v_texCoordyp;
varying mediump vec2 v_texCoordyn;

void main()
{
    gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
	v_texCoord = a_TexCoords;
    
    mediump vec2 dx = vec2(1.0/h.x, 0.);
	mediump vec2 dy = vec2(0., 1.0/h.y);
    
	v_texCoordxp = v_texCoord + dx; 
	v_texCoordxn = v_texCoord - dx; 
	v_texCoordyp = v_texCoord + dy;
	v_texCoordyn = v_texCoord - dy;
}