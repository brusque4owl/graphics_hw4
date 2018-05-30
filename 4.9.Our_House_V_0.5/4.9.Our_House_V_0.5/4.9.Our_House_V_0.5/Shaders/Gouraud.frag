#version 400

in vec4 v_shaded_color;
layout (location = 0) out vec4 final_color;
// gouraud shading은 per-vertex shading이기에 vertex shader에서 넘어온 값을 받아들이기만 함
void main(void) {
   final_color = v_shaded_color;	
}
