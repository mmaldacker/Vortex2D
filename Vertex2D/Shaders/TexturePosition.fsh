#version 150

precision highp float;

in mediump vec2 v_TexCoords;
uniform sampler2D u_Texture;

void main()
{
	gl_FragColor = texture2D(u_Texture, v_TexCoords);
}
