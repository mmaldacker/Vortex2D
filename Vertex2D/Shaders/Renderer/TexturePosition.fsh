#version 150

precision highp float;

in vec2 v_texCoord;
uniform sampler2D u_texture;

out vec4 out_color;

void main()
{
	out_color = texture(u_texture, v_texCoord);
}
