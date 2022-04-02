#version 460 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;

//uniform mat4 in_view;
//uniform mat4 in_proj;

out vec2 vert_uv;

void main()
{
	vert_uv = uv;
	gl_Position = vec4(pos, 1.);
	//gl_Position = in_proj * in_view * vec4(pos, 1.);
}