#version 400

struct LIGHT {
	vec4 position; // assume point or direction in EC in this example shader
	vec4 ambient_color, diffuse_color, specular_color;
	vec4 light_attenuation_factors; // compute this effect only if .w != 0.0f
	vec3 spot_direction;
	float spot_exponent;
	float spot_cutoff_angle;
	bool light_on;
};

struct MATERIAL {
	vec4 ambient_color;
	vec4 diffuse_color;
	vec4 specular_color;
	vec4 emissive_color;
	float specular_exponent;
};

uniform vec4 u_global_ambient_color;
#define NUMBER_OF_LIGHTS_SUPPORTED 4
uniform LIGHT u_light[NUMBER_OF_LIGHTS_SUPPORTED];
uniform MATERIAL u_material;

uniform bool screen_effect = false;
uniform float screen_width = 0.1f;
uniform float screen_density = 1.0f;

uniform bool u_blind_effect = false;
uniform float u_blind_density = 90.0f;

const float zero_f = 0.0f;
const float one_f = 1.0f;

in vec3 v_position_EC;
in vec3 v_normal_EC;
in vec2 v_position_sc;

layout (location = 0) out vec4 final_color;

vec4 lighting_equation(in vec3 P_EC, in vec3 N_EC) {
	vec4 color_sum;
	float local_scale_factor, tmp_float; 
	vec3 L_EC;

	// Lighting Equation���� 1��, 2��
	color_sum = u_material.emissive_color + u_global_ambient_color * u_material.ambient_color;
 
	for (int i = 0; i < NUMBER_OF_LIGHTS_SUPPORTED; i++) {
		if (!u_light[i].light_on) continue;

		local_scale_factor = one_f;
		if (u_light[i].position.w != zero_f) { // point light source
			L_EC = u_light[i].position.xyz - P_EC.xyz;	// VPpli vector

			if (u_light[i].light_attenuation_factors.w  != zero_f) {
				vec4 tmp_vec4;

				tmp_vec4.x = one_f;
				tmp_vec4.z = dot(L_EC, L_EC);
				tmp_vec4.y = sqrt(tmp_vec4.z);
				tmp_vec4.w = zero_f;
				local_scale_factor = one_f/dot(tmp_vec4, u_light[i].light_attenuation_factors);
			}

			L_EC = normalize(L_EC);

			if (u_light[i].spot_cutoff_angle < 180.0f) { // [0.0f, 90.0f] or 180.0f
				float spot_cutoff_angle = clamp(u_light[i].spot_cutoff_angle, zero_f, 90.0f);
				vec3 spot_dir = normalize(u_light[i].spot_direction);

				tmp_float = dot(-L_EC, spot_dir);	// cos(psi)

				if (tmp_float >= cos(radians(spot_cutoff_angle))) {		// cos(psi) >= cos(c_rli)  <==>   psi <= c_rli
					if(u_blind_effect){
						tmp_float = pow(tmp_float, u_light[i].spot_exponent) * cos(u_blind_density*acos(tmp_float));
					}
					else tmp_float = pow(tmp_float, u_light[i].spot_exponent);
				}


				else 
					tmp_float = zero_f;
				local_scale_factor *= tmp_float;
			}
		}
		else {  // directional light source
			L_EC = normalize(u_light[i].position.xyz);
		}	

		if (local_scale_factor > zero_f) {				
			vec4 local_color_sum = u_light[i].ambient_color * u_material.ambient_color;

			tmp_float = dot(N_EC, L_EC);
			if (tmp_float > zero_f) {
				local_color_sum += u_light[i].diffuse_color*u_material.diffuse_color*tmp_float;
			
				vec3 H_EC = normalize(L_EC - normalize(P_EC));
				tmp_float = dot(N_EC, H_EC); 
				if (tmp_float > zero_f) {
					local_color_sum += u_light[i].specular_color
										   *u_material.specular_color*pow(tmp_float, u_material.specular_exponent);
				}
			}
			color_sum += local_scale_factor*local_color_sum;
		}
	}
 	return color_sum;
}

void main(void) {   
	// final_color = vec4(gl_FragCoord.x/800.0f, gl_FragCoord.y/800.0f, 0.0f, 1.0f); // what is this?
    // final_color = vec4(0.0f,  0.0f, 1.0 - gl_FragCoord.z/1.0f, 1.0f); // what is this?
	
	if(screen_effect){
		
		float x_mod, y_mod;
		
		x_mod = mod(v_position_sc.x*3.0f, screen_density);
		y_mod = mod(v_position_sc.y*2.0f, screen_density);
		
		if( (x_mod > screen_width) && (x_mod < 1.0f-screen_width) && (y_mod > screen_width) && (y_mod < 1.0f-screen_width) )
			discard;
		
	}
	final_color = lighting_equation(v_position_EC, normalize(v_normal_EC)); // for normal rendering
}
