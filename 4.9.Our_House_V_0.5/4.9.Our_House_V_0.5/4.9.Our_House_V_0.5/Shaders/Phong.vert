#version 400

uniform mat4 u_ModelMatrix;
uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_ModelViewMatrix;
uniform mat3 u_ModelViewMatrixInvTrans;  
uniform mat3 u_ModelMatrixInvTrans;  

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
out vec3 v_position_EC;
out vec3 v_normal_EC;
out vec2 v_position_sc;

void main(void) {	
	//v_position_EC = vec3(u_ModelViewMatrix*vec4(a_position, 1.0f));
	//v_normal_EC = normalize(u_ModelViewMatrixInvTrans*a_normal);  
	// Do not follow camera!
	v_position_EC = vec3(u_ModelMatrix*vec4(a_position, 1.0f));
	v_normal_EC = normalize(u_ModelMatrixInvTrans*a_normal);  
	v_position_sc = vec2(a_position);

	gl_Position = u_ModelViewProjectionMatrix*vec4(a_position, 1.0f);
}