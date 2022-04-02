#version 460 core

uniform usampler2D win_tex;
uniform usampler2D tex;

in vec2 vert_uv;
out uvec4 out_frag;

void main()
{
	out_frag = uvec4(texture(tex, vert_uv)/2);
}