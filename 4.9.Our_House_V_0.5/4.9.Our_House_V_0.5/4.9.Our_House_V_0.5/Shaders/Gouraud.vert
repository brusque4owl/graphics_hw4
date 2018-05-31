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

uniform vec4 u_global_ambient_color;				// 전역 앰비언트 광원 색깔
#define NUMBER_OF_LIGHTS_SUPPORTED 4				// 광원 4개 사용
uniform LIGHT u_light[NUMBER_OF_LIGHTS_SUPPORTED];	// 광원 변수는 4개
uniform MATERIAL u_material;						// 물질 변수는 1개
uniform mat4 u_ModelViewProjectionMatrix;
uniform mat4 u_ModelViewMatrix;
uniform mat3 u_ModelViewMatrixInvTrans;  

const float zero_f = 0.0f;
const float one_f = 1.0f;

layout (location = 0) in vec3 a_position;	// 위치와
layout (location = 1) in vec3 a_normal;		// 노말 벡터가 들어옴
out vec4 v_shaded_color;					// 쉐이딩된 색깔이 출력됨

// P_EC : 꼭지점, L_EC : 광원
vec4 lighting_equation(in vec3 P_EC, in vec3 N_EC) {
	vec4 color_sum;
	float local_scale_factor, tmp_float; //local_scale_factor는 att와 spot값을 곱함
	vec3 L_EC;	// 아래에서 꼭지점에서 광원을 향하는 벡터가 됨.

	// OpenGL lighting equation의 1항과 2항
	color_sum = u_material.emissive_color + u_global_ambient_color * u_material.ambient_color;
 
	for (int i = 0; i < NUMBER_OF_LIGHTS_SUPPORTED; i++) {
		if (!u_light[i].light_on) continue;

		local_scale_factor = one_f;
		// 1. 점광원 처리
		if (u_light[i].position.w != zero_f) { // point light source w가 0.0이 아니면 점광원
			L_EC = u_light[i].position.xyz - P_EC.xyz;	// 빛을 향하는 방향벡터
			
			// 1-1.빛의 감쇠 처리
			if (u_light[i].light_attenuation_factors.w  != zero_f) {
				vec4 tmp_vec4;

				tmp_vec4.x = one_f;
				tmp_vec4.z = dot(L_EC, L_EC);	// 빛을 향하는 벡터의 크기의 제곱
				tmp_vec4.y = sqrt(tmp_vec4.z);	// z를 sqrt해서 크기를 취함
				tmp_vec4.w = zero_f;
				local_scale_factor = one_f/dot(tmp_vec4, u_light[i].light_attenuation_factors);
			}

			L_EC = normalize(L_EC);	// 빛을 향하는 벡터를 단위벡터로 만듦

			// 1-2.spot 광원 처리
			if (u_light[i].spot_cutoff_angle < 180.0f) { // [0.0f, 90.0f] or 180.0f
				// calmp(x, min, max) : x를 [min, max]범위로 클램프한다.
				// 범위에 있으면 x 그대로 리턴, min보다 작으면 min, max보다 크면 max 리턴
				float spot_cutoff_angle = clamp(u_light[i].spot_cutoff_angle, zero_f, 90.0f);
				vec3 spot_dir = normalize(u_light[i].spot_direction);	//sdli의 단위벡터화

				// 빛에서 꼭지점으로 향하는 벡터와 sdli의 내적-> cos(프사이)
				tmp_float = dot(-L_EC, spot_dir);
				if (tmp_float >= cos(radians(spot_cutoff_angle))) { // spot광원 범위 내부
					tmp_float = pow(tmp_float, u_light[i].spot_exponent); // cos(psi)^srli
				}
				else 
					tmp_float = zero_f;	// spot광원 범위 외부 -> 0.0으로 검은색 처리
				local_scale_factor *= tmp_float;
			}
		}
		//2. 평행 광원 처리
		else {  // directional light source
			L_EC = normalize(u_light[i].position.xyz);
		}	

		// spot 광원에서 0이 되지 않은 경우
		if (local_scale_factor > zero_f) {				
			// 지역 조명 - 앰비언트 반사
			vec4 local_color_sum = u_light[i].ambient_color * u_material.ambient_color;

			tmp_float = dot(N_EC, L_EC);	// n 내적 VPpli벡터
			if (tmp_float > zero_f) {	// 0보다 크면 앞에서 빛이 들어옴
				// 지역 조명 - 난반사
				local_color_sum += u_light[i].diffuse_color*u_material.diffuse_color*tmp_float;
			
				// 하프웨이 벡터 계산
				vec3 H_EC = normalize(L_EC - normalize(P_EC));	// VPpli + VPe(꼭지점에서 원점) // P_EC는 원점에서 꼭지점 방향
				tmp_float = dot(N_EC, H_EC); 
				if (tmp_float > zero_f) {	// n 내적 h(하프웨이 벡터) = cos(알파) // 0.0<알파<90.0
					// 난반사에서의 내적값이 0보다 크기 때문에 정반사에서 f값은 f=1이 되어 여기선 생략해준다.
					local_color_sum += u_light[i].specular_color
										   *u_material.specular_color*pow(tmp_float, u_material.specular_exponent);
				}
			}
			// 만약 뒤에서 빛이 들어오면 앰비언트 반사만 고려해주게 됨.
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