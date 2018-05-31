#version 400

// textbook pg. 313
struct LIGHT {
	vec4 position; // assume point or direction in EC in this example shader
	vec4 ambient_color, diffuse_color, specular_color;	// acli, dcli, scli
	vec4 light_attenuation_factors; // att // compute this effect only if .w != 0.0f
	vec3 spot_direction;			// sdli
	float spot_exponent;			// srli  more larger more dark faster
	float spot_cutoff_angle;		// crli  0.0~90.0 or 180.0(no meaning)
	bool light_on;
};

struct MATERIAL {
	vec4 ambient_color;		// acm
	vec4 diffuse_color;		// dcm
	vec4 specular_color;	// scm
	vec4 emissive_color;	// ecm
	float specular_exponent;
};

uniform vec4 u_global_ambient_color;				// ���� �ں��Ʈ ���� ����
#define NUMBER_OF_LIGHTS_SUPPORTED 4				// ���� 4�� ���
uniform LIGHT u_light[NUMBER_OF_LIGHTS_SUPPORTED];	// ���� ������ 4��
uniform MATERIAL u_material;						// ���� ������ 1��
uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_ModelViewMatrix;
uniform mat3 u_ModelViewMatrixInvTrans;  

const float zero_f = 0.0f;
const float one_f = 1.0f;

layout (location = 0) in vec3 a_position;	// ��ġ��
layout (location = 1) in vec3 a_normal;		// �븻 ���Ͱ� ����
out vec4 v_shaded_color;					// ���̵��� ������ ��µ�

// P_EC : ������, L_EC : ����
vec4 lighting_equation(in vec3 P_EC, in vec3 N_EC) {
	vec4 color_sum;
	float local_scale_factor, tmp_float; //local_scale_factor�� att�� spot���� ����
	vec3 L_EC;	// �Ʒ����� ���������� ������ ���ϴ� ���Ͱ� ��.

	// OpenGL lighting equation�� 1�װ� 2��
	color_sum = u_material.emissive_color + u_global_ambient_color * u_material.ambient_color;
 
	for (int i = 0; i < NUMBER_OF_LIGHTS_SUPPORTED; i++) {
		if (!u_light[i].light_on) continue;

		local_scale_factor = one_f;
		// 1. ������ ó��
		if (u_light[i].position.w != zero_f) { // point light source w�� 0.0�� �ƴϸ� ������
			L_EC = u_light[i].position.xyz - P_EC.xyz;	// ���� ���ϴ� ���⺤��
			
			// 1-1.���� ���� ó��
			if (u_light[i].light_attenuation_factors.w  != zero_f) {
				vec4 tmp_vec4;

				tmp_vec4.x = one_f;
				tmp_vec4.z = dot(L_EC, L_EC);	// ���� ���ϴ� ������ ũ���� ����
				tmp_vec4.y = sqrt(tmp_vec4.z);	// z�� sqrt�ؼ� ũ�⸦ ����
				tmp_vec4.w = zero_f;
				local_scale_factor = one_f/dot(tmp_vec4, u_light[i].light_attenuation_factors);
			}

			L_EC = normalize(L_EC);	// ���� ���ϴ� ���͸� �������ͷ� ����

			// 1-2.spot ���� ó��
			if (u_light[i].spot_cutoff_angle < 180.0f) { // [0.0f, 90.0f] or 180.0f
				// calmp(x, min, max) : x�� [min, max]������ Ŭ�����Ѵ�.
				// ������ ������ x �״�� ����, min���� ������ min, max���� ũ�� max ����
				float spot_cutoff_angle = clamp(u_light[i].spot_cutoff_angle, zero_f, 90.0f);
				vec3 spot_dir = normalize(u_light[i].spot_direction);	//sdli�� ��������ȭ

				// ������ ���������� ���ϴ� ���Ϳ� sdli�� ����-> cos(������)
				tmp_float = dot(-L_EC, spot_dir);
				if (tmp_float >= cos(radians(spot_cutoff_angle))) { // spot���� ���� ����
					tmp_float = pow(tmp_float, u_light[i].spot_exponent); // cos(psi)^srli
				}
				else 
					tmp_float = zero_f;	// spot���� ���� �ܺ� -> 0.0���� ������ ó��
				local_scale_factor *= tmp_float;
			}
		}
		//2. ���� ���� ó��
		else {  // directional light source
			L_EC = normalize(u_light[i].position.xyz);
		}	

		// spot �������� 0�� ���� ���� ���
		if (local_scale_factor > zero_f) {				
			// ���� ���� - �ں��Ʈ �ݻ�
			vec4 local_color_sum = u_light[i].ambient_color * u_material.ambient_color;

			tmp_float = dot(N_EC, L_EC);	// n ���� VPpli����
			if (tmp_float > zero_f) {	// 0���� ũ�� �տ��� ���� ����
				// ���� ���� - ���ݻ�
				local_color_sum += u_light[i].diffuse_color*u_material.diffuse_color*tmp_float;
			
				// �������� ���� ���
				vec3 H_EC = normalize(L_EC - normalize(P_EC));	// VPpli + VPe(���������� ����) // P_EC�� �������� ������ ����
				tmp_float = dot(N_EC, H_EC); 
				if (tmp_float > zero_f) {	// n ���� h(�������� ����) = cos(����) // 0.0<����<90.0
					// ���ݻ翡���� �������� 0���� ũ�� ������ ���ݻ翡�� f���� f=1�� �Ǿ� ���⼱ �������ش�.
					local_color_sum += u_light[i].specular_color
										   *u_material.specular_color*pow(tmp_float, u_material.specular_exponent);
				}
			}
			// ���� �ڿ��� ���� ������ �ں��Ʈ �ݻ縸 ������ְ� ��.
			color_sum += local_scale_factor*local_color_sum;
		}
	}
 	return color_sum;
}

void main(void) {	
	vec3 position_EC = vec3(u_ModelViewMatrix*vec4(a_position, one_f));
	vec3 normal_EC = normalize(u_ModelViewMatrixInvTrans*a_normal);  

	v_shaded_color = lighting_equation(position_EC, normal_EC);
	gl_Position = u_ModelViewProjectionMatrix*vec4(a_position, one_f);
}