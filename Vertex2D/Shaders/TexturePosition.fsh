precision highp float;

varying mediump vec2 v_TexCoords;
uniform sampler2D u_Texture;

void main()
{
	gl_FragColor = texture2D(u_Texture, v_TexCoords);
}
