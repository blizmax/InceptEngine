#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform MVP
{
	mat4 view;
	mat4 proj;
}mvp;

layout(set = 0, binding = 0) uniform TransformationBuffer
{
	mat4 t[200];
}tBuffer;


layout(location = 0) in vec4 pos;
layout(location = 1) in vec4 color;
layout(location = 2) in vec3 texCoord;
layout(location = 3) in vec4 boneWeights;
layout(location = 4) in uvec4 boneID;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() 
{
    mat4 boneTrans = boneWeights[0] * tBuffer.t[boneID[0]] + boneWeights[1] * tBuffer.t[boneID[1]] + boneWeights[2] * tBuffer.t[boneID[2]] + boneWeights[3] * tBuffer.t[boneID[3]];
    gl_Position = mvp.proj * mvp.view * tBuffer.t[1] * boneTrans * pos;
    fragColor = color;
    fragTexCoord = vec2(texCoord.x, texCoord.y);
}
