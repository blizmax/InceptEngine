#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform MVP
{
	mat4 model;
	mat4 view;
	mat4 proj;
}mvp;
layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 color;
layout(location = 0) out vec4 fragColor;


void main() {
    gl_Position = mvp.proj * mvp.view * mvp.model * pos;
    fragColor = color;
}
