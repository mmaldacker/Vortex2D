precision highp float;

varying mediump vec2 v_TexCoords;
uniform sampler2D u_Texture;
uniform mediump vec4 u_Colour;

void main()
{
	gl_FragColor = texture2D(u_Texture, v_TexCoords) * u_Colour;
}
