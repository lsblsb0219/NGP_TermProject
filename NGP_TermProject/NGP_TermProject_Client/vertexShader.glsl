#version 330

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 in_TexCoord;

out vec3 FragPos; //--- 객체의 위치값을 프래그먼트 세이더로 보낸다.
out vec3 Normal; //--- 노멀값을 프래그먼트 세이더로 보낸다.
out vec2 out_TexCoord;

uniform mat4 modelTransform;
uniform mat4 projectionTransform;
uniform mat4 viewTransform;

void main(void) 
{
	gl_Position =  projectionTransform * viewTransform * modelTransform * vec4 (in_Position, 1.0);

	FragPos = vec3(modelTransform * vec4(in_Position, 1.0));	//--- 객체에 대한 조명 계산을 프래그먼트 셰이더에서 한다.
	//--- 따라서 월드공간에 있는 버텍스 값을 프래그먼트 셰이더로 보낸다.
	Normal = vec3(modelTransform * vec4(vNormal, 1.0f));		//--- 노멀값을 프래그먼트 세이더로 보낸다.
	out_TexCoord = in_TexCoord;
}