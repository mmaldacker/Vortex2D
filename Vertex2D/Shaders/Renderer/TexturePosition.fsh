#version 150

precision highp float;

in vec2 v_TexCoords;
uniform sampler2D u_Texture;

out vec4 out_color;

void main()
{
	out_color = texture2D(u_Texture, v_TexCoords);
}
