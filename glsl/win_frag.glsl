#version 460 core

uniform usampler2D in_tex;

in vec2 vert_uv;
out uvec4 out_frag;

void main()
{
    out_frag = uvec4(texture(in_tex, vert_uv));
}