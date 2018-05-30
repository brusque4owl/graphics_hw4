#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Shaders/LoadShaders.h"
#include "My_Shading.h"
GLuint h_ShaderProgram, h_ShaderProgram_PS; // handle to shader program

// for simple shaders
GLint loc_ModelViewProjectionMatrix, loc_primitive_color;

// for Phone Shading shaders
#define NUMBER_OF_LIGHT_SUPPORTED 4 
GLint loc_global_ambient_color;
loc_light_Parameters loc_light[NUMBER_OF_LIGHT_SUPPORTED];
loc_Material_Parameters loc_material;
GLint loc_ModelViewProjectionMatrix_PS, loc_ModelViewMatrix_PS, loc_ModelViewMatrixInvTrans_PS;
// lights in scene
Light_Parameters light[NUMBER_OF_LIGHT_SUPPORTED];

#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.
#include <glm/gtc/matrix_inverse.hpp> // inverseTranspose, etc.

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define PI      3.141592653f 
//////////////////////////////////////////////////////////////////
//////// Setting Cameras /////////////////////////////////////////
//////////////////////////////////////////////////////////////////
bool view_driver=false;			// dirver camera
#define NUMBER_OF_CAMERAS 9		// main, static cctv * 3, dynamic cctv, front view, side view, top view
#define CAM_TRANSLATION_SPEED 0.025f
#define CAM_ROTATION_SPEED 0.1f
glm::mat4 ViewProjectionMatrix[NUMBER_OF_CAMERAS], ViewMatrix[NUMBER_OF_CAMERAS], ProjectionMatrix[NUMBER_OF_CAMERAS];
glm::mat4 ModelViewMatrix[NUMBER_OF_CAMERAS];
glm::mat4 ModelViewProjectionMatrix;
glm::mat3 ModelViewMatrixInvTrans;
typedef struct {
	// eye, viewpoint, up vector.
	glm::vec3 prp, vrp, vup; // in this example code, make vup always equal to the v direction.
	float fov_y, aspect_ratio, near_clip, far_clip, zoom_factor;
	float left, right, bottom, top;
	glm::vec3 uaxis, vaxis, naxis;
} CAMERA;

CAMERA camera[NUMBER_OF_CAMERAS];
glm::mat4 mat_rotate = glm::mat4(1.0f);
glm::vec3 init_camera_vup;
float angle_rotate_u = 0.0f;
float angle_rotate_v = 0.0f;
float angle_rotate_n = 0.0f;
glm::vec3 rotate_axis_u = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 rotate_axis_v = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 rotate_axis_n = glm::vec3(0.0f, 0.0f, 1.0f);
int camera_selected; // 0 for main_camera, 7 for dynamic_cctv 

glm::mat4 ModelMatrix_CAR_BODY, ModelMatrix_CAR_WHEEL, ModelMatrix_CAR_NUT, ModelMatrix_CAR_DRIVER;
glm::mat4 ModelMatrix_CAR_BODY_to_DRIVER; // computed only once in initialize_camera()
#include "Object_Definitions.h"

typedef struct _VIEWPORT {
	int x, y, w, h;
} VIEWPORT;
VIEWPORT viewport[NUMBER_OF_CAMERAS];					// �� ��Ʈ ����

//////////////////////////////////////////////////////////////////
/////// End of Setting Cameras ///////////////////////////////////
//////////////////////////////////////////////////////////////////

typedef struct _CALLBACK_CONTEXT {
	int left_button_status;
	int prevx, prevy;
} CALLBACK_CONTEXT;
CALLBACK_CONTEXT cc;

#define VIEW_CAMERA 0
#define VIEW_CCTV	1
int view_mode;
void display_camera(int cam_index) { // display()�Լ��� ���� ���ʸ��� �ҷ���.

	glViewport(viewport[cam_index].x, viewport[cam_index].y, viewport[cam_index].w, viewport[cam_index].h);

	glUseProgram(h_ShaderProgram);
	glLineWidth(2.0f);
	draw_axes(cam_index);
	if (view_mode == VIEW_CAMERA) {
		draw_main_camera_axes(cam_index);
		draw_frustum_line(cam_index);
	}
	draw_car_dummy(cam_index);
	draw_path(cam_index);
	draw_car_path(cam_index);
	glLineWidth(1.0f);


	glUseProgram(h_ShaderProgram_PS);

	set_material_building();
	draw_static_object(&(static_objects[OBJ_BUILDING]), 0, cam_index);

	set_material_table_0();
	draw_static_object(&(static_objects[OBJ_TABLE]), 0, cam_index);
	set_material_table_1();
	draw_static_object(&(static_objects[OBJ_TABLE]), 1, cam_index);	// takes given teapot

	set_material_light_0();
	draw_static_object(&(static_objects[OBJ_LIGHT]), 0, cam_index);
	set_material_light_1();
	draw_static_object(&(static_objects[OBJ_LIGHT]), 1, cam_index);
	set_material_light_2();
	draw_static_object(&(static_objects[OBJ_LIGHT]), 2, cam_index);
	set_material_light_3();
	draw_static_object(&(static_objects[OBJ_LIGHT]), 3, cam_index);
	set_material_light_4();
	draw_static_object(&(static_objects[OBJ_LIGHT]), 4, cam_index);
	set_material_light_5();
	draw_static_object(&(static_objects[OBJ_LIGHT]), 5, cam_index);			// NEW OBJ_LIGHT

	set_material_teapot_0();
	draw_static_object(&(static_objects[OBJ_TEAPOT]), 0, cam_index);	// on the OBJ_TABLE 1
	set_material_teapot_1();
	draw_static_object(&(static_objects[OBJ_TEAPOT]), 1, cam_index);			// NEW OBJ_TEAPOT
	
	set_material_new_chair_0();
	draw_static_object(&(static_objects[OBJ_NEW_CHAIR]), 0, cam_index);
	set_material_new_chair_1();
	draw_static_object(&(static_objects[OBJ_NEW_CHAIR]), 1, cam_index);		// NEW OBJ_NEW_CHAIR
	
	set_material_frame_0();
	draw_static_object(&(static_objects[OBJ_FRAME]), 0, cam_index);
	set_material_frame_1();
	draw_static_object(&(static_objects[OBJ_FRAME]), 1, cam_index);			// NEW OBJ_FRAME
	
	set_material_new_picture_0();
	draw_static_object(&(static_objects[OBJ_NEW_PICTURE]), 0, cam_index);
	set_material_new_picture_1();
	draw_static_object(&(static_objects[OBJ_NEW_PICTURE]), 1, cam_index);		// NEW OBJ_NEW_PICTURE
	
	set_material_cow();
	draw_static_object(&(static_objects[OBJ_COW]), 0, cam_index);

	set_material_tiger();
	draw_animated_tiger(cam_index);

	glUseProgram(0);
}
void display(void) {   // ���ʸ��� �ҷ���
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (view_mode) {
	case VIEW_CAMERA:
		if(view_driver==true)
			display_camera(8);	// driver_camera
		else
			display_camera(0);	// main_camera
		display_camera(1);	// front_view
		display_camera(2);	// side_view
		display_camera(3);	// top_view
		break;
	case VIEW_CCTV:
		display_camera(4);	// static cctv 1
		display_camera(5);	// static cctv 2
		display_camera(6);	// static cctv 3
		display_camera(7);	// dynamic cctv
		break;
	}
	glutSwapBuffers();
}

void initialize_camera(void);
void motion_translate_uaxis(int x, int y);
void motion_translate_vaxis(int x, int y);
void motion_translate_naxis(int x, int y);
void motion_rotate_uaxis(int x, int y);
void motion_rotate_vaxis(int x, int y);
void motion_rotate_naxis(int x, int y);
void motion_1(int x, int y);
void motion_car(int x, int y);
void motion_car_along_path(int x, int y);

#define CAR_MOV 1.0f

void keyboard(unsigned char key, int x, int y) {
	static int flag_cull_face = 0, polygon_fill_on = 0, depth_test_on = 0;

	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 'x':					// Translation for u axis
		glutMotionFunc(motion_translate_uaxis);
		glutPostRedisplay();
		break;
	case 'y':					// Translation for v axis
		glutMotionFunc(motion_translate_vaxis);
		glutPostRedisplay();
		break;
	case 'z':					// Translation for n axis
		glutMotionFunc(motion_translate_naxis);
		glutPostRedisplay();
		break;
	case 'u':					// Rotation for u axis
		glutMotionFunc(motion_rotate_uaxis);
		glutPostRedisplay();
		break;
	case 'v':					// Rotation for v axis
		glutMotionFunc(motion_rotate_vaxis);
		glutPostRedisplay();
		break;
	case 'n':					// Rotation for n axis
		glutMotionFunc(motion_rotate_naxis);
		glutPostRedisplay();
		break;
	case 'i':					// Zoom in
		camera[camera_selected].fov_y = camera[camera_selected].fov_y - camera[camera_selected].zoom_factor;
		if (camera[camera_selected].fov_y <= 0.0f) // fov_y�� 0�� �Ǹ� ȭ�鿡 �ƹ��͵� �Ⱥ���, ���� ȭ�� ������
			camera[camera_selected].fov_y = 1.0f;

		ProjectionMatrix[camera_selected] = glm::perspective(camera[camera_selected].fov_y*TO_RADIAN, camera[camera_selected].aspect_ratio, camera[camera_selected].near_clip, camera[camera_selected].far_clip);
		ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];
		//printf("camera_selected = %d\n", camera_selected);
		printf("fov_y = %f\n", camera[camera_selected].fov_y);
		//printf("asp_ratio = %f\n", camera[camera_selected].aspect_ratio);

		define_frustum_line(); 	// change viewing volume with fov_y
		glutPostRedisplay();
		break;
	case 'o':					// Zoom out
		camera[camera_selected].fov_y = camera[camera_selected].fov_y + camera[camera_selected].zoom_factor;
		if (camera[camera_selected].fov_y >= 180.0f) // fov_y�� 180�� �Ǹ� ȭ�鿡 �ƹ��͵� �Ⱥ���, ���� ȭ�� ������
			camera[camera_selected].fov_y = 179.0f;

		ProjectionMatrix[camera_selected] = glm::perspective(camera[camera_selected].fov_y*TO_RADIAN, camera[camera_selected].aspect_ratio, camera[camera_selected].near_clip, camera[camera_selected].far_clip);
		ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];
		//printf("camera_selected = %d\n", camera_selected);
		printf("fov_y = %f\n", camera[camera_selected].fov_y);
		//printf("asp_ratio = %f\n", camera[camera_selected].aspect_ratio);

		define_frustum_line();	// change viewing volume with fov_y
		glutPostRedisplay();
		break;
	case ',':		// Increase zoom factor
		camera[camera_selected].zoom_factor += 1.0f;
		glutPostRedisplay();
		break;
	case '.':		// Decrease zoom factor
		camera[camera_selected].zoom_factor -= 1.0f;
		if (camera[camera_selected].zoom_factor <= 0.0f)
			camera[camera_selected].zoom_factor = 1.0f;
		glutPostRedisplay();
		break;

	case '/':		// Initialize main_camera or dynamic_cctv
		switch (view_mode) {
		case VIEW_CAMERA:
			/*
			camera[0].prp = glm::vec3(202.0f, 38.0f, 14.0f);	// ī�޶� ��ġ
			camera[0].vrp = glm::vec3(197.0f, 86.0f, 13.0f);		// �ٶ󺸴� ��
			*/
			camera[0].prp = glm::vec3(212.778366f, 163.684006f, 11.367419f);		// ī�޶� ��ġ
			camera[0].vrp = glm::vec3(212.100739f, 115.430161f, 12.360538f);		// �ٶ󺸴� ��
			camera[0].vup = init_camera_vup;
			camera[0].fov_y = 30.0f;
			angle_rotate_u = 0.0f;
			angle_rotate_v = 0.0f;
			angle_rotate_n = 0.0f;
			rotate_axis_u = glm::vec3(0.0f, 0.0f, 1.0f);
			rotate_axis_v = glm::vec3(0.0f, 0.0f, 1.0f);
			rotate_axis_n = glm::vec3(0.0f, 0.0f, 1.0f);
			break;

		case VIEW_CCTV:
			camera[camera_selected].prp = glm::vec3(50.0f, 50.0f, 50.0f);	// ī�޶� ��ġ
			camera[camera_selected].vrp = glm::vec3(10.0f, 50.0f, 20.0f);		// �ٶ󺸴� ��
			camera[camera_selected].vup = glm::vec3(0.0f, 0.0f, 1.0f);
			camera[camera_selected].fov_y = 100.0f;
			break;
		}
		//u,v,n���͸� lookAt���� ����
		ViewMatrix[camera_selected] = glm::lookAt(camera[camera_selected].prp, camera[camera_selected].vrp, camera[camera_selected].vup);
		camera[camera_selected].zoom_factor = 1.0f;
		ProjectionMatrix[camera_selected] = glm::perspective(camera[camera_selected].fov_y*TO_RADIAN, camera[camera_selected].aspect_ratio, camera[camera_selected].near_clip, camera[camera_selected].far_clip);
		ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];
		define_frustum_line();	// change viewing volume with fov_y
		glutPostRedisplay();
		break;

	case 's':					// change mode between CAMERA MODE and CCTV MODE
		camera_selected = 7 - camera_selected;
		view_mode = 1 - view_mode;
		glutPostRedisplay();
		break;

	case 'm':					// change mode to moving_car
		glutMotionFunc(motion_car_along_path);
		glutPostRedisplay();
		break;

	case 'r':	// view driver camera
		if(view_mode==VIEW_CAMERA)
			view_driver = 1 - view_driver;
		glutPostRedisplay();
		break;

	/*
	case 'c':
		flag_cull_face = (flag_cull_face + 1) % 3;
		switch (flag_cull_face) {
		case 0:
			glDisable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ No faces are culled.\n");
			break;
		case 1: // cull back faces;
			glCullFace(GL_BACK);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ Back faces are culled.\n");
			break;
		case 2: // cull front faces;
			glCullFace(GL_FRONT);
			glEnable(GL_CULL_FACE);
			glutPostRedisplay();
			fprintf(stdout, "^^^ Front faces are culled.\n");
			break;
		}
		break;

	case 'f':
		polygon_fill_on = 1 - polygon_fill_on;
		if (polygon_fill_on) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			fprintf(stdout, "^^^ Polygon filling enabled.\n");
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			fprintf(stdout, "^^^ Line drawing enabled.\n");
		}
		glutPostRedisplay();
		break;

	case 'd':
		depth_test_on = 1 - depth_test_on;
		if (depth_test_on) {
			glEnable(GL_DEPTH_TEST);
			fprintf(stdout, "^^^ Depth test enabled.\n");
		}
		else {
			glDisable(GL_DEPTH_TEST);
			fprintf(stdout, "^^^ Depth test disabled.\n");
		}
		glutPostRedisplay();
		break;

	case 't':	// used for debugging tiger path
		camera[0].prp = glm::vec3(tiger_pos.x, tiger_pos.y, tiger_pos.z+500.0f);		// ī�޶� ��ġ
		camera[0].vrp = glm::vec3(tiger_pos.x, tiger_pos.y, tiger_pos.z);		// �ٶ󺸴� ��
		//u,v,n���͸� lookAt���� ����
		ViewMatrix[camera_selected] = glm::lookAt(camera[camera_selected].prp, camera[camera_selected].vrp, camera[camera_selected].vup);
		camera[camera_selected].zoom_factor = 1.0f;
		ProjectionMatrix[camera_selected] = glm::perspective(camera[camera_selected].fov_y*TO_RADIAN, camera[camera_selected].aspect_ratio, camera[camera_selected].near_clip, camera[camera_selected].far_clip);
		ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];
		glutPostRedisplay();
		break;

	case ']':
		glutMotionFunc(motion_1);
		glutPostRedisplay();
		break;
		*/
	}
}

void reshape(int width, int height) {
	// VIEW_DRIVER mode
	camera[8].aspect_ratio = (float)width / height;	// viewport������ projection������ ����ȭ��Ŵ
													//printf("aspect_ratio = %f\n\n", camera[0].aspect_ratio);
	viewport[8].x = viewport[8].y = 0;
	viewport[8].w = (int)(0.70f*width); viewport[8].h = (int)(0.70f*height);
	ProjectionMatrix[8] = glm::perspective(camera[8].fov_y*TO_RADIAN, camera[8].aspect_ratio, camera[8].near_clip, camera[8].far_clip);
	ViewProjectionMatrix[8] = ProjectionMatrix[8] * ViewMatrix[8];


	// VIEW_CAMERA mode
	camera[0].aspect_ratio = (float)width / height;	// viewport������ projection������ ����ȭ��Ŵ
													//printf("aspect_ratio = %f\n\n", camera[0].aspect_ratio);
	viewport[0].x = viewport[0].y = 0;
	viewport[0].w = (int)(0.70f*width); viewport[0].h = (int)(0.70f*height);
	ProjectionMatrix[0] = glm::perspective(camera[0].fov_y*TO_RADIAN, camera[0].aspect_ratio, camera[0].near_clip, camera[0].far_clip);
	ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];

	// front view
	camera[1].aspect_ratio = camera[0].aspect_ratio;
	viewport[1].x = (int)(0.70f*width); viewport[1].y = (int)(0.70f*height);
	viewport[1].w = (int)(0.30f*width); viewport[1].h = (int)(0.30*height);
	ProjectionMatrix[1] = glm::ortho(camera[1].left, camera[1].right, camera[1].bottom, camera[1].top, camera[1].near_clip, camera[1].far_clip);
	ViewProjectionMatrix[1] = ProjectionMatrix[1] * ViewMatrix[1];

	// side view
	camera[2].aspect_ratio = camera[0].aspect_ratio;
	viewport[2].x = (int)(0.70f*width); viewport[2].y = (int)(0.40f*height);
	viewport[2].w = (int)(0.30f*width); viewport[2].h = (int)(0.30*height);
	ProjectionMatrix[2] = glm::ortho(camera[2].left, camera[2].right, camera[2].bottom, camera[2].top, camera[2].near_clip, camera[2].far_clip);
	ViewProjectionMatrix[2] = ProjectionMatrix[2] * ViewMatrix[2];

	// top view
	camera[3].aspect_ratio = camera[0].aspect_ratio;
	viewport[3].x = (int)(0.70f*width); viewport[3].y = (int)(0.05f*height);
	viewport[3].w = (int)(0.30f*width); viewport[3].h = (int)(0.30*height);
	ProjectionMatrix[3] = glm::ortho(camera[3].left, camera[3].right, camera[3].bottom, camera[3].top, camera[3].near_clip, camera[3].far_clip);
	ViewProjectionMatrix[3] = ProjectionMatrix[3] * ViewMatrix[3];

	// VIEW_CCTV mode
	// static_cctv_1
	camera[4].aspect_ratio = (float)width / height;	// viewport������ projection������ ����ȭ��Ŵ
	viewport[4].x = 0; viewport[4].y = (int)(0.70f*height);
	viewport[4].w = (int)(0.30f*width); viewport[4].h = (int)(0.30f*height);
	ProjectionMatrix[4] = glm::perspective(camera[4].fov_y*TO_RADIAN, camera[4].aspect_ratio, camera[4].near_clip, camera[4].far_clip);
	ViewProjectionMatrix[4] = ProjectionMatrix[4] * ViewMatrix[4];

	// static_cctv_2
	camera[5].aspect_ratio = camera[4].aspect_ratio;
	viewport[5].x = 0; viewport[5].y = (int)(0.35f*height);
	viewport[5].w = (int)(0.30f*width); viewport[5].h = (int)(0.30*height);
	ProjectionMatrix[5] = glm::perspective(camera[5].fov_y*TO_RADIAN, camera[5].aspect_ratio, camera[5].near_clip, camera[5].far_clip);
	ViewProjectionMatrix[5] = ProjectionMatrix[5] * ViewMatrix[5];

	// static_cctv_3
	camera[6].aspect_ratio = camera[4].aspect_ratio;
	viewport[6].x = 0; viewport[6].y = 0;
	viewport[6].w = (int)(0.30f*width); viewport[6].h = (int)(0.30*height);
	ProjectionMatrix[6] = glm::perspective(camera[6].fov_y*TO_RADIAN, camera[6].aspect_ratio, camera[6].near_clip, camera[6].far_clip);
	ViewProjectionMatrix[6] = ProjectionMatrix[6] * ViewMatrix[6];

	// dynamic_cctv
	camera[7].aspect_ratio = camera[4].aspect_ratio;
	viewport[7].x = (int)(0.35f*width); viewport[7].y = 0;
	viewport[7].w = (int)(0.65f*width); viewport[7].h = (int)(0.65*height);
	ProjectionMatrix[7] = glm::perspective(camera[7].fov_y*TO_RADIAN, camera[7].aspect_ratio, camera[7].near_clip, camera[7].far_clip);
	ViewProjectionMatrix[7] = ProjectionMatrix[7] * ViewMatrix[7];

	glutPostRedisplay();
}

unsigned int leftbutton_pressed = 0;
int prevx, prevy;
int path_num = 0;
// ȣ���� �����Ӱ��� �Լ�
void timer_scene(int timestamp_scene) {
	tiger_data.cur_frame = timestamp_scene % N_TIGER_FRAMES;
	//tiger_data.rotation_angle = (timestamp_scene % 360)*TO_RADIAN;

	//// Move tiger along path
	switch(path_num){
	case 0:
		tiger_pos.x = tiger_pos.x + tiger_point[0].mov_x;
		tiger_data.rotation_angle = tiger_point[0].rot_angle*TO_RADIAN;
		if(tiger_pos.x >= tiger_point[0].x) path_num=1;
		break;
	case 1:
		if (tiger_pos.x < tiger_point[1].x) tiger_pos.x = tiger_pos.x + tiger_point[1].mov_x*(tiger_point[1].x - tiger_point[0].x) / (tiger_point[1].y - tiger_point[0].y);
		tiger_pos.y = tiger_pos.y + tiger_point[1].mov_y;
		tiger_data.rotation_angle = (tiger_point[0].rot_angle + tiger_point[1].rot_angle)*TO_RADIAN;

		if (tiger_point[1].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (tiger_point[0].rot_angle + tiger_point[1].rot_angle*tiger_point[1].smooth / SMOOTH_ROT) *TO_RADIAN;
			tiger_point[1].smooth++;
		}

		if(tiger_pos.y >= tiger_point[1].y) path_num=2;
		break;
	case 2:
		if (tiger_pos.y < tiger_point[2].y) tiger_pos.y = tiger_pos.y + tiger_point[2].mov_y*(tiger_point[2].y - tiger_point[1].y) / (tiger_point[2].x - tiger_point[1].x);
		tiger_pos.x = tiger_pos.x + tiger_point[2].mov_x;
		tiger_data.rotation_angle = (tiger_point[0].rot_angle + tiger_point[2].rot_angle)*TO_RADIAN;

		if (tiger_point[2].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (tiger_point[0].rot_angle + tiger_point[1].rot_angle
					- (tiger_point[1].rot_angle - tiger_point[2].rot_angle)*tiger_point[2].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[2].smooth++;
		}

		if(tiger_pos.x >= tiger_point[2].x) path_num=3;
		break;
	case 3:
		tiger_pos.y = tiger_pos.y + tiger_point[3].mov_y;
		tiger_data.rotation_angle = (tiger_point[0].rot_angle + tiger_point[3].rot_angle)*TO_RADIAN;

		if (tiger_point[3].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (tiger_point[0].rot_angle + tiger_point[2].rot_angle
					+ (tiger_point[3].rot_angle - tiger_point[2].rot_angle)*tiger_point[3].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[3].smooth++;
		}

		if(tiger_pos.y >= tiger_point[3].y) path_num=4;
		break;
	case 4:
		tiger_pos.y = tiger_pos.y - tiger_point[4].mov_y;
		tiger_data.rotation_angle = (tiger_point[0].rot_angle + tiger_point[4].rot_angle)*TO_RADIAN;

		if (tiger_point[4].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (tiger_point[0].rot_angle + tiger_point[3].rot_angle
					+ (tiger_point[4].rot_angle - tiger_point[3].rot_angle)*tiger_point[4].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[4].smooth++;
		}

		if(tiger_pos.y <= tiger_point[4].y) path_num=5;
		break;
	case 5:
		if (tiger_pos.y > tiger_point[5].y) tiger_pos.y = tiger_pos.y - tiger_point[5].mov_y*(tiger_point[5].y - tiger_point[4].y) / (tiger_point[5].x - tiger_point[4].x);
		tiger_pos.x = tiger_pos.x - tiger_point[5].mov_x;
		tiger_data.rotation_angle = (-tiger_point[5].rot_angle)*TO_RADIAN;

		if (tiger_point[5].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (-tiger_point[5].rot_angle*tiger_point[5].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[5].smooth++;
		}

		if(tiger_pos.x <= tiger_point[5].x) path_num=6;
		break;
	case 6:
		tiger_pos.y = tiger_pos.y - tiger_point[6].mov_y;
		tiger_data.rotation_angle = (tiger_point[0].rot_angle + tiger_point[6].rot_angle)*TO_RADIAN;

		if (tiger_point[6].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (-tiger_point[5].rot_angle + (tiger_point[5].rot_angle)*tiger_point[6].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[6].smooth++;
		}

		if(tiger_pos.y <= tiger_point[6].y) path_num=7;
		break;
	case 7:
		if (tiger_pos.y > tiger_point[7].y) tiger_pos.y = tiger_pos.y - tiger_point[7].mov_y*((tiger_point[7].x - tiger_point[6].x) / (tiger_point[7].y - tiger_point[6].y));
		tiger_pos.x = tiger_pos.x - tiger_point[7].mov_x;
		tiger_data.rotation_angle = (-tiger_point[7].rot_angle)*TO_RADIAN;

		if (tiger_point[7].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (-tiger_point[7].rot_angle*tiger_point[7].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[7].smooth++;
		}

		if(tiger_pos.x <= tiger_point[7].x) path_num=8;
		break;
	case 8:
		tiger_pos.y = tiger_pos.y - tiger_point[7].mov_y;
		tiger_data.rotation_angle = (tiger_point[0].rot_angle + tiger_point[8].rot_angle)*TO_RADIAN;

		if (tiger_point[8].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (-tiger_point[7].rot_angle + (tiger_point[7].rot_angle)*tiger_point[8].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[8].smooth++;
		}

		if(tiger_pos.y <= tiger_point[8].y) path_num=9;
		break;
	case 9:
		tiger_pos.y = tiger_pos.y + tiger_point[9].mov_y;
		tiger_data.rotation_angle = (tiger_point[9].rot_angle)*TO_RADIAN;

		if (tiger_point[9].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (-(tiger_point[9].rot_angle)*tiger_point[9].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[9].smooth++;
		}

		if(tiger_pos.y >= tiger_point[9].y) path_num=10;
		break;
	case 10:
		if (tiger_pos.x < tiger_point[10].x) tiger_pos.x = tiger_pos.x + tiger_point[10].mov_x*((tiger_point[10].x - tiger_point[9].x) / (tiger_point[10].y - tiger_point[9].y));
		tiger_pos.y = tiger_pos.y + tiger_point[10].mov_y;
		tiger_data.rotation_angle = (180.0f - tiger_point[10].rot_angle)*TO_RADIAN;

		if (tiger_point[10].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (180.0f - tiger_point[10].rot_angle*tiger_point[10].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[10].smooth++;
		}

		if(tiger_pos.y >= tiger_point[10].y) path_num=11;
		break;
	case 11:
		tiger_pos.y = tiger_pos.y + tiger_point[11].mov_y;
		tiger_data.rotation_angle = (180.0f)*TO_RADIAN;

		if (tiger_point[11].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = ((180.0f - tiger_point[10].rot_angle) + tiger_point[10].rot_angle*tiger_point[11].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[11].smooth++;
		}

		if(tiger_pos.y >= tiger_point[11].y) path_num=12;
		break;
	case 12:
		tiger_pos.x = tiger_pos.x - tiger_point[12].mov_x;
		tiger_data.rotation_angle = (180.0f + tiger_point[12].rot_angle)*TO_RADIAN;

		if (tiger_point[12].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (180.0f + tiger_point[12].rot_angle*tiger_point[12].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[12].smooth++;
		}

		if(tiger_pos.x <= tiger_point[12].x) path_num=13;
		break;
	case 13:
		tiger_pos.y = tiger_pos.y - tiger_point[13].mov_y;
		tiger_data.rotation_angle = (0.0f)*TO_RADIAN;

		if (tiger_point[13].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (270.0f + tiger_point[13].rot_angle*tiger_point[13].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[13].smooth++;
		}

		if(tiger_pos.y <= tiger_point[13].y) path_num=14;
		break;
	case 14:
		tiger_pos.x = tiger_pos.x - tiger_point[14].mov_x;
		tiger_data.rotation_angle = tiger_point[14].rot_angle*TO_RADIAN;

		if (tiger_point[14].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (tiger_point[14].rot_angle*tiger_point[14].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[14].smooth++;
		}

		if(tiger_pos.x <= tiger_point[14].x) path_num=15;
		break;
	case 15:
		tiger_pos.x = tiger_pos.x - tiger_point[15].mov_x;
		tiger_data.rotation_angle = (-90.0f + tiger_point[15].rot_angle)*TO_RADIAN;

		if (tiger_point[15].smooth<SMOOTH_ROT) {
			tiger_data.rotation_angle = (-90.0f + tiger_point[15].rot_angle*tiger_point[15].smooth / SMOOTH_ROT)*TO_RADIAN;
			tiger_point[15].smooth++;
		}

		if(tiger_pos.x <= tiger_point[15].x) path_num=16;
		break;
	default:
		tiger_pos.x = 30.5f; tiger_pos.y = 22.2f; tiger_pos.z = 0.0f;
		for (int i = 0; i<NUM_POINTS; i++) {
			tiger_point[i].timer = 0.0f;
			tiger_point[i].smooth = 0;
		}
		path_num=0;
		break;
	}
	////
	glutPostRedisplay();
	glutTimerFunc(100, timer_scene, (timestamp_scene + 1) % INT_MAX);
}

void mousepress(int button, int state, int x, int y) {
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		prevx = x, prevy = y;
		leftbutton_pressed = 1;
		glutPostRedisplay();
	}
	else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
		leftbutton_pressed = 0;
		glutPostRedisplay();
	}
}

#define CAM_ROT_SENSITIVITY 0.15f
#define CAM_TRA_SENSITIVITY 1.0f
void motion_1(int x, int y) {
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	float delx, dely;

	if (leftbutton_pressed && view_mode == VIEW_CAMERA) {
		delx = (float)(x - prevx), dely = -(float)(y - prevy);
		prevx = x, prevy = y;

		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[camera_selected].vrp);
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*delx*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));	//�¿�����̸� v���� �������� 360�� ȸ�� ����
		mat4_tmp = glm::translate(mat4_tmp, -camera[camera_selected].vrp);

		camera[camera_selected].prp = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].prp, 1.0f));	// affine transformation of point (x,y,z,1)
		camera[camera_selected].vup = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vup, 0.0f));	// affine transformation of vector(x,y,z,0)


		vec3_tmp = glm::cross(camera[camera_selected].vup, camera[camera_selected].vrp - camera[camera_selected].prp);		// vrp-prp = -n vector   // result of cross is -u vector
		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[camera_selected].vrp);
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*dely*TO_RADIAN, vec3_tmp);	// rotate for -u vector
		mat4_tmp = glm::translate(mat4_tmp, -camera[camera_selected].vrp);

		camera[camera_selected].prp = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].prp, 1.0f));
		camera[camera_selected].vup = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vup, 0.0f));


		ViewMatrix[camera_selected] = glm::lookAt(camera[camera_selected].prp, camera[camera_selected].vrp, camera[camera_selected].vup);

		ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];
		glutPostRedisplay();
	}
}

void motion_translate_uaxis(int x, int y) {
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	glm::vec3 vec3_unit_uaxis;
	//float sqrt_vector;
	float delx;

	if (leftbutton_pressed && view_mode == VIEW_CAMERA) {
		delx = (float)(x - prevx);
		prevx = x;

		vec3_tmp = glm::cross(camera[camera_selected].vup, camera[camera_selected].prp - camera[camera_selected].vrp);
		//sqrt_vector = sqrt((vec3_tmp.x*vec3_tmp.x) + (vec3_tmp.y*vec3_tmp.y) + (vec3_tmp.z*vec3_tmp.z));
		//vec3_unit_uaxis = glm::vec3( vec3_tmp.x / sqrt_vector, vec3_tmp.y / sqrt_vector, vec3_tmp.z / sqrt_vector);
		vec3_unit_uaxis = glm::normalize(vec3_tmp);
		mat4_tmp = glm::translate(glm::mat4(1.0f), delx*vec3_unit_uaxis);

		camera[camera_selected].prp = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].prp, 1.0f));	// affine transformation of point (x,y,z,1)
		camera[camera_selected].vrp = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vrp, 1.0f));	// affine transformation of point (x,y,z,1)
		camera[camera_selected].vup = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vup, 0.0f));	// affine transformation of vector(x,y,z,0)

		ViewMatrix[camera_selected] = glm::lookAt(camera[camera_selected].prp, camera[camera_selected].vrp, camera[camera_selected].vup);

		ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];

		/*printf("prp.x = %f\t prp.y = %f\t prp.z = %f\nvrp.x = %f\t vrp.y = %f\t vrp.z = %f\nvup.x = %f\t vup.y = %f\t vup.z = %f\n\n",
			camera[camera_selected].prp.x, camera[camera_selected].prp.y, camera[camera_selected].prp.z,
			camera[camera_selected].vrp.x, camera[camera_selected].vrp.y, camera[camera_selected].vrp.z,
			camera[camera_selected].vup.x, camera[camera_selected].vup.y, camera[camera_selected].vup.z);
			*/
		glutPostRedisplay();
	}
}
void motion_translate_vaxis(int x, int y) {
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	float dely;

	if (leftbutton_pressed && view_mode == VIEW_CAMERA) {
		dely = -(float)(y - prevy);
		prevy = y;

		mat4_tmp = glm::translate(glm::mat4(1.0f), dely*camera[camera_selected].vup);

		// prp(ī�޶� ��ġ), vrp(ī�޶� �ٶ󺸴� ��)�� ����  // vup(ī�޶� ��)�� ���� - prp�� �����̴ϱ� vup�� ���󰡾���.
		camera[camera_selected].prp = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].prp, 1.0f));	// affine transformation of point (x,y,z,1)
		camera[camera_selected].vrp = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vrp, 1.0f));	// affine transformation of point (x,y,z,1)
		camera[camera_selected].vup = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vup, 0.0f));	// affine transformation of vector(x,y,z,0)

		ViewMatrix[camera_selected] = glm::lookAt(camera[camera_selected].prp, camera[camera_selected].vrp, camera[camera_selected].vup);

		ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];

		/*
		printf("prp.x = %f\t prp.y = %f\t prp.z = %f\nvrp.x = %f\t vrp.y = %f\t vrp.z = %f\nvup.x = %f\t vup.y = %f\t vup.z = %f\n\n",
			camera[camera_selected].prp.x, camera[camera_selected].prp.y, camera[camera_selected].prp.z,
			camera[camera_selected].vrp.x, camera[camera_selected].vrp.y, camera[camera_selected].vrp.z,
			camera[camera_selected].vup.x, camera[camera_selected].vup.y, camera[camera_selected].vup.z);
			*/
		glutPostRedisplay();
	}
}
void motion_translate_naxis(int x, int y) {
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	glm::vec3 vec3_unit_naxis;
	//float sqrt_vector;
	float dely;

	if (leftbutton_pressed && view_mode == VIEW_CAMERA) {
		dely = -(float)(y - prevy);
		prevy = y;

		vec3_tmp = camera[camera_selected].prp - camera[camera_selected].vrp;			// prp-vrp = n����  <- �� ���� ī�޶� �� ����
																						//sqrt_vector = sqrt((vec3_tmp.x*vec3_tmp.x) + (vec3_tmp.y*vec3_tmp.y) + (vec3_tmp.z*vec3_tmp.z));
																						//vec3_unit_naxis = glm::vec3(vec3_tmp.x / sqrt_vector, vec3_tmp.y / sqrt_vector, vec3_tmp.z / sqrt_vector);
		vec3_unit_naxis = glm::normalize(vec3_tmp);
		mat4_tmp = glm::translate(glm::mat4(1.0f), -dely * vec3_unit_naxis);

		camera[camera_selected].prp = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].prp, 1.0f));	// affine transformation of point (x,y,z,1)
		camera[camera_selected].vrp = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vrp, 1.0f));	// affine transformation of point (x,y,z,1)
		camera[camera_selected].vup = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vup, 0.0f));	// affine transformation of vector(x,y,z,0)

		ViewMatrix[camera_selected] = glm::lookAt(camera[camera_selected].prp, camera[camera_selected].vrp, camera[camera_selected].vup);

		ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];

		/*printf("prp.x = %f\t prp.y = %f\t prp.z = %f\nvrp.x = %f\t vrp.y = %f\t vrp.z = %f\nvup.x = %f\t vup.y = %f\t vup.z = %f\n\n",
			camera[camera_selected].prp.x, camera[camera_selected].prp.y, camera[camera_selected].prp.z,
			camera[camera_selected].vrp.x, camera[camera_selected].vrp.y, camera[camera_selected].vrp.z,
			camera[camera_selected].vup.x, camera[camera_selected].vup.y, camera[camera_selected].vup.z);
			*/
		glutPostRedisplay();
	}
}

void motion_rotate_uaxis(int x, int y) {
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	float dely;

	if (leftbutton_pressed) {
		dely = -(float)(y - prevy);
		prevy = y;

		// prp-vrp = n���� // vup X n = u����   <- �� ���� ī�޶� �� ����
		vec3_tmp = glm::cross(camera[camera_selected].vup, camera[camera_selected].prp - camera[camera_selected].vrp);
		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[camera_selected].prp);
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*dely*TO_RADIAN, vec3_tmp);		// u����(ī�޶��� ������)�� �ѷ��� ȸ�� : ���� ������ ȸ�� ��, ī�޶� ���� �ĵ�ϱ� ȭ���� �Ʒ��� ������
		mat4_tmp = glm::translate(mat4_tmp, -camera[camera_selected].prp);
		if(view_mode != VIEW_CCTV)	angle_rotate_u += CAM_ROT_SENSITIVITY * dely*TO_RADIAN;
		rotate_axis_u = vec3_tmp;

		camera[camera_selected].vrp = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vrp, 1.0f));	// affine transformation of point (x,y,z,1)

		if (view_mode == VIEW_CAMERA) // CCTV�� vup�� (0,0,1)�� ����
			camera[camera_selected].vup = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vup, 0.0f));	// affine transformation of vector(x,y,z,0)

		ViewMatrix[camera_selected] = glm::lookAt(camera[camera_selected].prp, camera[camera_selected].vrp, camera[camera_selected].vup);

		ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];

		/*printf("prp.x = %f\t prp.y = %f\t prp.z = %f\nvrp.x = %f\t vrp.y = %f\t vrp.z = %f\nvup.x = %f\t vup.y = %f\t vup.z = %f\n\n",
			camera[camera_selected].prp.x, camera[camera_selected].prp.y, camera[camera_selected].prp.z,
			camera[camera_selected].vrp.x, camera[camera_selected].vrp.y, camera[camera_selected].vrp.z,
			camera[camera_selected].vup.x, camera[camera_selected].vup.y, camera[camera_selected].vup.z);
			*/
		glutPostRedisplay();
	}
}
void motion_rotate_vaxis(int x, int y) {
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	glm::vec4 uaxis, vaxis, naxis;
	float delx;

	if (leftbutton_pressed) {
		delx = (float)(x - prevx);
		prevx = x;
		// prp�� �������� ���� ��, ���ϴ¸�ŭ ȸ�� ��Ű��, �ٽ� ���ƿ�. 
		// vup����(=v����)�� ����, u, n���Ͱ� ȸ��
		// prp-vrp = n���� // vup X n = u����   <- �� ���� ī�޶� �� ����
		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[camera_selected].prp);
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*-delx * TO_RADIAN, camera[camera_selected].vup);	//�¿�����̸� vup���� �������� 360�� ȸ�� ����
		mat4_tmp = glm::translate(mat4_tmp, -camera[camera_selected].prp);
		if (view_mode != VIEW_CCTV)	angle_rotate_v += CAM_ROT_SENSITIVITY * -delx * TO_RADIAN;
		rotate_axis_v = camera[camera_selected].vup;

		camera[camera_selected].vrp = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vrp, 1.0f));	// affine transformation of point (x,y,z,1)

																										// v�� �ѷ��� ȸ���ϴϱ� vup ����
																										//if (view_mode == VIEW_CAMERA) camera[camera_selected].vup = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vup, 0.0f));	// affine transformation of vector(x,y,z,0)

		ViewMatrix[camera_selected] = glm::lookAt(camera[camera_selected].prp, camera[camera_selected].vrp, camera[camera_selected].vup);

		ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];

		/*printf("prp.x = %f\t prp.y = %f\t prp.z = %f\nvrp.x = %f\t vrp.y = %f\t vrp.z = %f\nvup.x = %f\t vup.y = %f\t vup.z = %f\n\n",
			camera[camera_selected].prp.x, camera[camera_selected].prp.y, camera[camera_selected].prp.z,
			camera[camera_selected].vrp.x, camera[camera_selected].vrp.y, camera[camera_selected].vrp.z,
			camera[camera_selected].vup.x, camera[camera_selected].vup.y, camera[camera_selected].vup.z);
			*/
		glutPostRedisplay();
	}
}
void motion_rotate_naxis(int x, int y) {
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	float delx;

	if (leftbutton_pressed && view_mode == VIEW_CAMERA) {
		delx = (float)(x - prevx);
		prevx = x;

		vec3_tmp = camera[camera_selected].prp - camera[camera_selected].vrp;			// prp-vrp = n����  <- �� ���� ī�޶� �� ����
		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[camera_selected].prp);
		// n����(ī�޶��� ����)�� �ѷ��� ȸ�� : ���� ������ ȸ�� ��, ī�޶� ���� �������� ����ϴϱ� ȭ���� ���������� ���ư�
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*delx*TO_RADIAN, vec3_tmp);
		mat4_tmp = glm::translate(mat4_tmp, -camera[camera_selected].prp);
		angle_rotate_n += CAM_ROT_SENSITIVITY * delx*TO_RADIAN;
		rotate_axis_n = vec3_tmp;

		camera[camera_selected].vrp = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vrp, 1.0f));	// affine transformation of point (x,y,z,1)
		camera[camera_selected].vup = glm::vec3(mat4_tmp*glm::vec4(camera[camera_selected].vup, 0.0f));	// affine transformation of vector(x,y,z,0)

		ViewMatrix[camera_selected] = glm::lookAt(camera[camera_selected].prp, camera[camera_selected].vrp, camera[camera_selected].vup);

		ViewProjectionMatrix[camera_selected] = ProjectionMatrix[camera_selected] * ViewMatrix[camera_selected];

		/*printf("prp.x = %f\t prp.y = %f\t prp.z = %f\nvrp.x = %f\t vrp.y = %f\t vrp.z = %f\nvup.x = %f\t vup.y = %f\t vup.z = %f\n\n",
			camera[camera_selected].prp.x, camera[camera_selected].prp.y, camera[camera_selected].prp.z,
			camera[camera_selected].vrp.x, camera[camera_selected].vrp.y, camera[camera_selected].vrp.z,
			camera[camera_selected].vup.x, camera[camera_selected].vup.y, camera[camera_selected].vup.z);
			*/
		glutPostRedisplay();
	}
}

int direction;
void motion_car_along_path(int x, int y) {
	static float prev_rot = 0.0f;
	static float del_rot = 0.0f;
	static int i=0;
	static bool change_rot = false;
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	float delx, dely;
	float car_delta, car_delta_x, car_delta_y, car_rot_delta;
	if(i!=0){
		car_delta_x = abs(car_path_vertices[i] - car_path_vertices[i-3]);
		car_delta_y = abs(car_path_vertices[i+1] - car_path_vertices[i+1-3]);
	}
	else{
		car_delta_x = car_delta_y = 0.0f;
	}
	if(i!=0)
		car_rot_delta = car_delta = car_delta_y / car_delta_x;
	else
		car_rot_delta = 0.0f;
	if (car_delta_x == 0.0f || car_delta_y == 0.0f) car_rot_delta = car_rot_delta;
	if(car_delta_x==0.0f) car_delta = 1.0f;
	if (leftbutton_pressed) {
		delx = (float)(x - prevx), dely = -(float)(y - prevy);
		prevx = x, prevy = y;

		mat4_tmp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,0.0f,0.0f));

		// 1. Judge direction
		if (car_pos.x >= car_path_vertices[i] && car_pos.y < car_path_vertices[i+1])			direction = 1;	// the 1st quadrant(270~360 degree)
		else if (car_pos.x >= car_path_vertices[i] && car_pos.y >= car_path_vertices[i+1])		direction = 2;	// the 2nd quadrant(0~89)
		else if (car_pos.x < car_path_vertices[i] && car_pos.y >= car_path_vertices[i+1])		direction = 3;	// the 3rd quadrant(90~179)
		else																					direction = 4;	// the 4th quadrant(180~269)
		// 2. rotate
		// 3. translate
		switch(direction){
		case 1:
			if(car_delta_x!=0.0f)
				car_pos.x -= MOV_X;
			car_pos.y += MOV_Y * car_delta;
			mat4_tmp = glm::translate(mat4_tmp, glm::vec3(car_pos.x, car_pos.y, 0.0f));
			if(change_rot==true){
				car_pos.rot = -atan(car_rot_delta)*TO_DEGREE;
				change_rot=false;
			}
			if (car_path_vertices[i] >= car_pos.x && car_path_vertices[i+1] < car_pos.y) {
				car_pos.x = car_path_vertices[i];
				car_pos.y = car_path_vertices[i+1];
				i=i+3; 
				change_rot = true;
				break;
			}
			break;
		case 2:
			if (car_delta_x != 0.0f)
				car_pos.x -= MOV_X;
			car_pos.y -= MOV_Y * car_delta;
			mat4_tmp = glm::translate(mat4_tmp, glm::vec3(car_pos.x, car_pos.y, 0.0f));
			if (change_rot == true) {
				car_pos.rot = atan(car_rot_delta)*TO_DEGREE;
				change_rot = false;
			}
			if (car_path_vertices[i] >= car_pos.x && car_path_vertices[i+1] >= car_pos.y) {
				car_pos.x = car_path_vertices[i];
				car_pos.y = car_path_vertices[i+1];
				i=i+3; 
				change_rot = true;
				break;
			}
			break;
		case 3:
			if (car_delta_x != 0.0f)
				car_pos.x += MOV_X;
			car_pos.y -= MOV_Y * car_delta;
			mat4_tmp = glm::translate(mat4_tmp, glm::vec3(car_pos.x, car_pos.y, 0.0f));
			if (change_rot == true) {
				car_pos.rot = 180.0f - atan(car_rot_delta)*TO_DEGREE;
				change_rot = false;
			}
			if (car_path_vertices[i] < car_pos.x && car_path_vertices[i+1] >= car_pos.y) {
				car_pos.x = car_path_vertices[i];
				car_pos.y = car_path_vertices[i+1];
				i=i+3; 
				change_rot = true;
				break;
			}
			break;
		case 4:
			if (car_delta_x != 0.0f)
				car_pos.x += MOV_X;
			car_pos.y += MOV_Y * car_delta;
			mat4_tmp = glm::translate(mat4_tmp, glm::vec3(car_pos.x, car_pos.y, 0.0f));
			if (change_rot == true) {
				car_pos.rot = - 180.0f + atan(car_rot_delta)*TO_DEGREE;
				change_rot = false;
			}
			if (car_path_vertices[i] < car_pos.x && car_path_vertices[i+1] < car_pos.y) {
				car_pos.x = car_path_vertices[i];
				car_pos.y = car_path_vertices[i+1];
				i=i+3;
				change_rot = true;
				break;
			}
			break;
		}
		
		car_pos.dist += abs(dely) * CAM_ROT_SENSITIVITY;
		if (dely != 0.0f)	car_pos.wheel_rot = car_pos.dist * 180 / pi_rad;	// ���� ȸ���� = �̵��Ÿ�*180/(PI*radius)

	
		//�������� ���� ȸ�� �� �̵� �� �ٽ� ���ƿ´�.
		del_rot = car_pos.rot - prev_rot;
		mat4_tmp = glm::rotate(mat4_tmp, del_rot * TO_RADIAN, camera[8].vup);
		mat4_tmp = glm::translate(mat4_tmp, glm::vec3(-camera[8].prp.x,-camera[8].prp.y, 0.0f));//-camera[8].prp);
		prev_rot = car_pos.rot;

		// camera ������Ʈ
		camera[8].prp = glm::vec3(mat4_tmp*glm::vec4(camera[8].prp, 1.0f));	// affine transformation of point (x,y,z,1)
		camera[8].vrp = glm::vec3(mat4_tmp*glm::vec4(camera[8].vrp, 1.0f));	// affine transformation of point (x,y,z,1)
		camera[8].vup = glm::vec3(mat4_tmp*glm::vec4(camera[8].vup, 0.0f));	// affine transformation of vector(x,y,z,0)

		if (i >= 3 * car_path_n_vertices) {	// back to initial position
			i = 0;
			car_pos.x = car_path_vertices[i];
			car_pos.y = car_path_vertices[i + 1];
			car_pos.rot = 0.0f;
			
			camera[8].prp = glm::vec3(car_pos.x - 3.0f, car_pos.y + 0.5f, car_pos.z + 7.0f);		// ī�޶� ��ġ
			camera[8].vrp = glm::vec3(car_pos.x - 3.0f - 10.0f, car_pos.y + 0.5f, car_pos.z + 7.0f);		// �ٶ󺸴� ��
			camera[8].vup = glm::vec3(0.0f, 0.0f, 1.0f);
			prev_rot = 0.0f;
			del_rot = 0.0;
			
		}

		ViewMatrix[8] = glm::lookAt(camera[8].prp, camera[8].vrp, camera[8].vup);
		ViewProjectionMatrix[8] = ProjectionMatrix[8] * ViewMatrix[8];
		glutPostRedisplay();
	}
}

void motion_car(int x, int y){
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	float delx, dely;
	float car_delta_x, car_delta_y;
	car_delta_x = abs(cos(car_pos.rot*TO_RADIAN));
	car_delta_y = abs(sin(car_pos.rot*TO_RADIAN));
	if (leftbutton_pressed) {
		delx = (float)(x - prevx), dely = -(float)(y - prevy);
		prevx = x, prevy = y;

		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[8].prp);

		// Movement forward, backward
		// the 1st quadrant(270~360 degree)
		if (car_pos.rot >= 270.0f && car_pos.rot<360.0f) {
			car_pos.x -= dely * car_delta_x * CAM_ROT_SENSITIVITY;
			car_pos.y += dely * car_delta_y * CAM_ROT_SENSITIVITY;
			mat4_tmp = glm::translate(mat4_tmp, glm::vec3(-dely * car_delta_x * CAM_ROT_SENSITIVITY, dely * car_delta_y * CAM_ROT_SENSITIVITY, 0.0f));
		}
		// the 2nd quadrant(0~89)
		else if (car_pos.rot >= 0.0f && car_pos.rot<90.0f) {
			car_pos.x -= dely * car_delta_x * CAM_ROT_SENSITIVITY;
			car_pos.y -= dely * car_delta_y * CAM_ROT_SENSITIVITY;
			mat4_tmp = glm::translate(mat4_tmp, glm::vec3(-dely * car_delta_x * CAM_ROT_SENSITIVITY, -dely * car_delta_y * CAM_ROT_SENSITIVITY, 0.0f));
		}
		// the 3rd quadrant(90~179)
		else if (car_pos.rot >= 90.0f && car_pos.rot<180.0f) {
			car_pos.x += dely * car_delta_x * CAM_ROT_SENSITIVITY;
			car_pos.y -= dely * car_delta_y * CAM_ROT_SENSITIVITY;
			mat4_tmp = glm::translate(mat4_tmp, glm::vec3(dely * car_delta_x * CAM_ROT_SENSITIVITY, -dely * car_delta_y * CAM_ROT_SENSITIVITY, 0.0f));
		}
		// the 4th quadrant(180~269)
		else {
			car_pos.x += dely * car_delta_x * CAM_ROT_SENSITIVITY;
			car_pos.y += dely * car_delta_y * CAM_ROT_SENSITIVITY;
			mat4_tmp = glm::translate(mat4_tmp, glm::vec3(dely * car_delta_x * CAM_ROT_SENSITIVITY, dely * car_delta_y * CAM_ROT_SENSITIVITY, 0.0f));
		}
		car_pos.dist += dely * CAM_ROT_SENSITIVITY;
		if(dely!=0.0f)	car_pos.wheel_rot = car_pos.dist * 180 / pi_rad;	// ���� ȸ���� = �̵��Ÿ�*180/(PI*radius)

		// turn left, right
		car_pos.rot -= delx * CAM_ROT_SENSITIVITY;
		if (car_pos.rot >= 360.0f) car_pos.rot = car_pos.rot - 360.0f;
		if (car_pos.rot < 0.0f) car_pos.rot = 360.0f + car_pos.rot;

		//�������� ���� ȸ�� �� �̵� �� �ٽ� ���ƿ´�.
		mat4_tmp = glm::rotate(mat4_tmp, -delx * CAM_ROT_SENSITIVITY *TO_RADIAN, camera[8].vup);
		mat4_tmp = glm::translate(mat4_tmp, -camera[8].prp);

		// camera ������Ʈ
		camera[8].prp = glm::vec3(mat4_tmp*glm::vec4(camera[8].prp, 1.0f));	// affine transformation of point (x,y,z,1)
		camera[8].vrp = glm::vec3(mat4_tmp*glm::vec4(camera[8].vrp, 1.0f));	// affine transformation of point (x,y,z,1)
		camera[8].vup = glm::vec3(mat4_tmp*glm::vec4(camera[8].vup, 0.0f));	// affine transformation of vector(x,y,z,0)

		ViewMatrix[8] = glm::lookAt(camera[8].prp, camera[8].vrp, camera[8].vup);
		ViewProjectionMatrix[8] = ProjectionMatrix[8] * ViewMatrix[8];
		///////////////////////
		glutPostRedisplay();
	}
}

void register_callbacks(void) {
	cc.left_button_status = GLUT_UP;

	glutMouseFunc(mousepress);
	//glutMotionFunc(motion_1);

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	//glutSpecialFunc(arrow_key);
	glutReshapeFunc(reshape);
	glutTimerFunc(100, timer_scene, 0);
	glutCloseFunc(cleanup_OpenGL_stuffs);
}

void prepare_shader_program(void) {
	int i;
	char string[256];

	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
	{ GL_NONE, NULL }
	};

	ShaderInfo shader_info_PS[3] = {
		{ GL_VERTEX_SHADER, "Shaders/Phong.vert" },
	{ GL_FRAGMENT_SHADER, "Shaders/Phong.frag" },
	{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	//glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");

	
	h_ShaderProgram_PS = LoadShaders(shader_info_PS);
	loc_ModelViewProjectionMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewProjectionMatrix");
	loc_ModelViewMatrix_PS = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewMatrix");
	loc_ModelViewMatrixInvTrans_PS = glGetUniformLocation(h_ShaderProgram_PS, "u_ModelViewMatrixInvTrans");

	loc_global_ambient_color = glGetUniformLocation(h_ShaderProgram_PS, "u_global_ambient_color");
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		sprintf(string, "u_light[%d].light_on", i);
		loc_light[i].light_on = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].position", i);
		loc_light[i].position = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].ambient_color", i);
		loc_light[i].ambient_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].diffuse_color", i);
		loc_light[i].diffuse_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].specular_color", i);
		loc_light[i].specular_color = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].spot_direction", i);
		loc_light[i].spot_direction = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].spot_exponent", i);
		loc_light[i].spot_exponent = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].spot_cutoff_angle", i);
		loc_light[i].spot_cutoff_angle = glGetUniformLocation(h_ShaderProgram_PS, string);
		sprintf(string, "u_light[%d].light_attenuation_factors", i);
		loc_light[i].light_attenuation_factors = glGetUniformLocation(h_ShaderProgram_PS, string);
	}

	loc_material.ambient_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.ambient_color");
	loc_material.diffuse_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.diffuse_color");
	loc_material.specular_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.specular_color");
	loc_material.emissive_color = glGetUniformLocation(h_ShaderProgram_PS, "u_material.emissive_color");
	loc_material.specular_exponent = glGetUniformLocation(h_ShaderProgram_PS, "u_material.specular_exponent");
	
}

void initialize_lights_and_material(void) { // follow OpenGL conventions for initialization
	int i;

	glUseProgram(h_ShaderProgram_PS);

	glUniform4f(loc_global_ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	for (i = 0; i < NUMBER_OF_LIGHT_SUPPORTED; i++) {
		glUniform1i(loc_light[i].light_on, 0); // turn off all lights initially
		glUniform4f(loc_light[i].position, 0.0f, 0.0f, 1.0f, 0.0f);
		glUniform4f(loc_light[i].ambient_color, 0.0f, 0.0f, 0.0f, 1.0f);
		if (i == 0) {
			glUniform4f(loc_light[i].diffuse_color, 1.0f, 1.0f, 1.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 1.0f, 1.0f, 1.0f, 1.0f);
		}
		else {
			glUniform4f(loc_light[i].diffuse_color, 0.0f, 0.0f, 0.0f, 1.0f);
			glUniform4f(loc_light[i].specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
		}
		glUniform3f(loc_light[i].spot_direction, 0.0f, 0.0f, -1.0f);
		glUniform1f(loc_light[i].spot_exponent, 0.0f); // [0.0, 128.0]
		glUniform1f(loc_light[i].spot_cutoff_angle, 180.0f); // [0.0, 90.0] or 180.0 (180.0 for no spot light effect)
		glUniform4f(loc_light[i].light_attenuation_factors, 1.0f, 0.0f, 0.0f, 0.0f); // .w != 0.0f for no ligth attenuation
	}

	glUniform4f(loc_material.ambient_color, 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(loc_material.diffuse_color, 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(loc_material.specular_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4f(loc_material.emissive_color, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1f(loc_material.specular_exponent, 0.0f); // [0.0, 128.0]

	glUseProgram(0);
}

void initialize_camera(void) {
	// initialize the 8th camera - driver_cam
	camera[8].prp = glm::vec3(car_pos.x-3.0f, car_pos.y+0.5f, car_pos.z+7.0f);		// ī�޶� ��ġ
	camera[8].vrp = glm::vec3(car_pos.x-3.0f - 50.0f, car_pos.y+0.5f, car_pos.z+7.0f);		// �ٶ󺸴� ��
	camera[8].vup = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::vec3 uaxis_driver, vaxis_driver, naxis_driver;
	naxis_driver = camera[8].prp - camera[8].vrp;
	naxis_driver = glm::normalize(naxis_driver);

	uaxis_driver = glm::cross(camera[8].vup, naxis_driver);
	uaxis_driver = glm::normalize(uaxis_driver);

	vaxis_driver = glm::cross(naxis_driver, uaxis_driver);
	vaxis_driver = glm::normalize(vaxis_driver);


	camera[8].uaxis = uaxis_driver;
	camera[8].vaxis = vaxis_driver;
	camera[8].naxis = naxis_driver;

	ViewMatrix[8] = glm::lookAt(camera[8].prp, camera[8].vrp, camera[8].vup); //u,v,n���͸� lookAt���� ����

	camera[8].fov_y = 30.0f;
	camera[8].aspect_ratio = 1.5f; // will be set when the viewing window popped up.
	camera[8].near_clip = 10.0f;
	camera[8].far_clip = 150.0f; // 1500.0f;  // for debug
	camera[8].zoom_factor = 1.0f; // will be used for zoomming in and out.
	//printf("prp.x = %f\t prp.y = %f\t prp.z =%f\n", camera[8].prp.x, camera[8].prp.y, camera[8].prp.z);
	//printf("vrp.x = %f\t vrp.y = %f\t vrp.z =%f\n", camera[8].vrp.x, camera[8].vrp.y, camera[8].vrp.z);
	//printf("vup.x = %f\t vup.y = %f\t vup.z =%f\n", camera[8].vup.x, camera[8].vup.y, camera[8].vup.z);

	// initialize the 0th camera.
	/*
	camera[0].prp = glm::vec3(202.0f, 38.0f, 14.0f);		// ī�޶� ��ġ
	camera[0].vrp = glm::vec3(197.0f, 86.0f, 13.0f);		// �ٶ󺸴� ��
	camera[0].vup = glm::vec3(0.0f, 0.0f, 1.0f);
	*/

	camera[0].prp = glm::vec3(212.778366f, 163.684006f, 11.367419f);		// ī�޶� ��ġ
	camera[0].vrp = glm::vec3(212.100739f, 115.430161f, 12.360538f);		// �ٶ󺸴� ��
	camera[0].vup = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::vec3 uaxis, vaxis, naxis;
	naxis = camera[0].prp - camera[0].vrp;
	naxis = glm::normalize(naxis);

	uaxis = glm::cross(camera[0].vup, naxis);
	uaxis = glm::normalize(uaxis);

	vaxis = glm::cross(naxis, uaxis);
	vaxis = glm::normalize(vaxis);

	init_camera_vup = camera[0].vup = vaxis;

	//uaxis = glm::cross(camera[camera_selected].vup, camera[camera_selected].prp - camera[camera_selected].vrp);
	//uaxis = glm::normalize(uaxis);
	camera[0].uaxis = uaxis;
	camera[0].vaxis = vaxis;
	camera[0].naxis = naxis;
	//printf("vup.x = %f\tvup.y = %f\tvup.z = %f\n\n", vaxis.x, vaxis.y, vaxis.z);

	ViewMatrix[0] = glm::lookAt(camera[0].prp, camera[0].vrp, camera[0].vup); //u,v,n���͸� lookAt���� ����

																			  //	ProjectionMatrix = glm::perspective(15.0f*TO_RADIAN, aspect_ratio, 1.0f, 10000.0f);
	camera[0].fov_y = 30.0f;
	camera[0].aspect_ratio = 1.5f; // will be set when the viewing window popped up.
	camera[0].near_clip = 10.0f;
	camera[0].far_clip = 150.0f; // 1500.0f;  // for debug
	camera[0].zoom_factor = 1.0f; // will be used for zoomming in and out.

	//initialize the 1st camera. used for front_view
	camera[1].prp = glm::vec3(300.0f, 0.0f, 0.0f);	// ī�޶� ��ġ
	camera[1].vrp = glm::vec3(0.0f, 0.0f, 0.0f);		// �ٶ󺸴� ��
	camera[1].vup = glm::vec3(0.0f, 0.0f, 1.0f);
	ViewMatrix[1] = glm::lookAt(camera[1].prp, camera[1].vrp, camera[1].vup);

	camera[1].left = -40.0f;   // prp�� (0,0,0)���� ���� ����� ��
	camera[1].right = 200.f;   // 160 + 40
	camera[1].bottom = -20.0f; // 0 - 20
	camera[1].top = 70.0f;     // 50 + 20

							   //camera[1].fov_y = 15.0f;
							   //camera[1].aspect_ratio = 1.0f; // will be set when the viewing window popped up.
	camera[1].near_clip = 10.0f;
	camera[1].far_clip = 600.0f;

	//initialize the 2nd camera used for side_view

	camera[2].prp = glm::vec3(230.0f, 230.0f, 0.0f);	// ī�޶� ��ġ
	camera[2].vrp = glm::vec3(230.0f, 0.0f, 0.0f);		// �ٶ󺸴� ��
	camera[2].vup = glm::vec3(0.0f, 0.0f, 1.0f);
	ViewMatrix[2] = glm::lookAt(camera[2].prp, camera[2].vrp, camera[2].vup);

	camera[2].left = -40.0f;  // prp�� (0,0,0)���� ���� ����� ��
	camera[2].right = 270.0f; // 230 + 40
	camera[2].bottom = -20.0f; // 0 - 20
	camera[2].top = 70.0f;	  // 50 + 20

							  //camera[2].fov_y = 15.0f;
							  //camera[2].aspect_ratio = 1.0f; // will be set when the viewing window popped up.
	camera[2].near_clip = 10.0f;
	camera[2].far_clip = 600.0f;
	/*
	camera[2].near_clip = 60.0f;
	camera[2].far_clip = 400.0f;
	*/
	//initialize the 3rd camera used for top_view
	camera[3].prp = glm::vec3(230.0f, 0.0f, 1200.0f);	// ī�޶� ��ġ
	camera[3].vrp = glm::vec3(230.0f, 0.0f, 0.0f);		// �ٶ󺸴� ��
	camera[3].vup = glm::vec3(-10.0f, 0.0f, 0.0f);
	ViewMatrix[3] = glm::lookAt(camera[3].prp, camera[3].vrp, camera[3].vup);

	camera[3].left = -40.0f;  // prp�� (0,0,0)���� ���� ����� ��
	camera[3].right = 200.0f; // 160 + 40
	camera[3].bottom = -20.0f; // 0 - 20
	camera[3].top = 250.0f;	  // 230 + 20

							  //camera[3].fov_y = 15.0f;
							  //camera[3].aspect_ratio = 1.0f; // will be set when the viewing window popped up.
	camera[3].near_clip = 930.0f;    // 1200 - (50 + 220)  - õ������ 220 ��
	camera[3].far_clip = 1220.0f;     // 1200 + (0 + 20)   - �ٴڿ��� 20 ��

	// VIEW_CCTV
	// static cctv 1
	camera[4].prp = glm::vec3(57.0f, 145.0f, 45.0f);	// ī�޶� ��ġ
	camera[4].vrp = glm::vec3(41.0f, 137.0f, 26.0f);		// �ٶ󺸴� ��
	camera[4].vup = glm::vec3(0.0f, 0.0f, 1.0f);
	ViewMatrix[4] = glm::lookAt(camera[4].prp, camera[4].vrp, camera[4].vup);

	camera[4].fov_y = 100.0f;
	camera[4].aspect_ratio = 1.0f; // will be set when the viewing window popped up.
	camera[4].near_clip = 1.0f;
	camera[4].far_clip = 10000.0f;

	// static cctv 2
	camera[5].prp = glm::vec3(162.0f, 62.0f, 45.0f);	// ī�޶� ��ġ
	camera[5].vrp = glm::vec3(127.0f, 82.0f, 21.0f);		// �ٶ󺸴� ��
	camera[5].vup = glm::vec3(0.0f, 0.0f, 1.0f);
	ViewMatrix[5] = glm::lookAt(camera[5].prp, camera[5].vrp, camera[5].vup);

	camera[5].fov_y = 60.0f;
	camera[5].aspect_ratio = 1.0f; // will be set when the viewing window popped up.
	camera[5].near_clip = 1.0f;
	camera[5].far_clip = 10000.0f;

	// static cctv 3
	camera[6].prp = glm::vec3(210.0f, 43.0f, 45.0f);	// ī�޶� ��ġ
	camera[6].vrp = glm::vec3(200.0f, 80.0f, 16.0f);		// �ٶ󺸴� ��
	camera[6].vup = glm::vec3(0.0f, 0.0f, 1.0f);
	ViewMatrix[6] = glm::lookAt(camera[6].prp, camera[6].vrp, camera[6].vup);

	camera[6].fov_y = 80.0f;
	camera[6].aspect_ratio = 1.0f; // will be set when the viewing window popped up.
	camera[6].near_clip = 1.0f;
	camera[6].far_clip = 10000.0f;

	// dynamic cctv
	glm::vec3 cam7_tmp;

	camera[7].prp = glm::vec3(50.0f, 50.0f, 50.0f);	// ī�޶� ��ġ
	camera[7].vrp = glm::vec3(10.0f, 50.0f, 20.0f);		// �ٶ󺸴� ��
	camera[7].vup = glm::vec3(0.0f, 0.0f, 1.0f);
	ViewMatrix[7] = glm::lookAt(camera[7].prp, camera[7].vrp, camera[7].vup);

	camera[7].fov_y = 100.0f;
	camera[7].aspect_ratio = 1.0f; // will be set when the viewing window popped up.
	camera[7].near_clip = 1.0f;
	camera[7].far_clip = 10000.0f;
	camera[7].zoom_factor = 1.0f; // will be used for zoomming in and out.


	camera_selected = 0;
	view_mode = VIEW_CAMERA;
}

void initialize_OpenGL(void) {
	initialize_camera();		// openGL�����ϸ鼭 ī�޶� ����

	glEnable(GL_DEPTH_TEST); // Default state

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClearColor(0.12f, 0.18f, 0.12f, 1.0f);

	initialize_lights_and_material();
}


void set_up_scene_lights(void) {
	int i;
	// point_light_EC: use light 0
	light[0].light_on = 1;
	light[0].position[0] = 0.0f; light[0].position[1] = 10.0f; 	// point light position in EC
	light[0].position[2] = 0.0f; light[0].position[3] = 1.0f;

	light[0].ambient_color[0] = 0.3f; light[0].ambient_color[1] = 0.3f;
	light[0].ambient_color[2] = 0.3f; light[0].ambient_color[3] = 1.0f;

	light[0].diffuse_color[0] = 0.7f; light[0].diffuse_color[1] = 0.7f;
	light[0].diffuse_color[2] = 0.7f; light[0].diffuse_color[3] = 1.0f;

	light[0].specular_color[0] = 0.9f; light[0].specular_color[1] = 0.9f;
	light[0].specular_color[2] = 0.9f; light[0].specular_color[3] = 1.0f;

	// spot_light_WC: use light 1
	light[1].light_on = 1;
	light[1].position[0] = -200.0f; light[1].position[1] = 500.0f; // spot light position in WC
	light[1].position[2] = -200.0f; light[1].position[3] = 1.0f;

	light[1].ambient_color[0] = 0.2f; light[1].ambient_color[1] = 0.2f;
	light[1].ambient_color[2] = 0.2f; light[1].ambient_color[3] = 1.0f;

	light[1].diffuse_color[0] = 0.82f; light[1].diffuse_color[1] = 0.82f;
	light[1].diffuse_color[2] = 0.82f; light[1].diffuse_color[3] = 1.0f;

	light[1].specular_color[0] = 0.82f; light[1].specular_color[1] = 0.82f;
	light[1].specular_color[2] = 0.82f; light[1].specular_color[3] = 1.0f;

	light[1].spot_direction[0] = 0.0f; light[1].spot_direction[1] = -1.0f; // spot light direction in WC
	light[1].spot_direction[2] = 0.0f;
	light[1].spot_cutoff_angle = 20.0f;
	light[1].spot_exponent = 27.0f;

	glUseProgram(h_ShaderProgram_PS);
	glUniform1i(loc_light[0].light_on, light[0].light_on);
	glUniform4fv(loc_light[0].position, 1, light[0].position);
	glUniform4fv(loc_light[0].ambient_color, 1, light[0].ambient_color);
	glUniform4fv(loc_light[0].diffuse_color, 1, light[0].diffuse_color);
	glUniform4fv(loc_light[0].specular_color, 1, light[0].specular_color);

	glUniform1i(loc_light[1].light_on, light[1].light_on);
	// need to supply position in EC for shading
	// �ϴ� 0�� ī�޶� ó��
	glm::vec4 position_EC = ViewMatrix[VIEW_CAMERA] * glm::vec4(light[1].position[0], light[1].position[1],
		light[1].position[2], light[1].position[3]);
	glUniform4fv(loc_light[1].position, 1, &position_EC[0]);
	glUniform4fv(loc_light[1].ambient_color, 1, light[1].ambient_color);
	glUniform4fv(loc_light[1].diffuse_color, 1, light[1].diffuse_color);
	glUniform4fv(loc_light[1].specular_color, 1, light[1].specular_color);
	// need to supply direction in EC for shading in this example shader
	// note that the viewing transform is a rigid body transform
	// thus transpose(inverse(mat3(ViewMatrix)) = mat3(ViewMatrix)
	// �ϴ� 0�� ī�޶� ó��
	glm::vec3 direction_EC = glm::mat3(ViewMatrix[VIEW_CAMERA]) * glm::vec3(light[1].spot_direction[0], light[1].spot_direction[1],
			light[1].spot_direction[2]);
	glUniform3fv(loc_light[1].spot_direction, 1, &direction_EC[0]);
	glUniform1f(loc_light[1].spot_cutoff_angle, light[1].spot_cutoff_angle);
	glUniform1f(loc_light[1].spot_exponent, light[1].spot_exponent);
	glUseProgram(0);
}

char car_body[] = "Data/car_body_triangles_v.txt";
char car_wheel[] = "Data/car_wheel_triangles_v.txt";
char car_nut[] = "Data/car_nut_triangles_v.txt";

void prepare_scene(void) {
	define_axes();
	define_frustum_line();
	define_static_objects();
	define_animated_tiger();
	prepare_geom_obj(GEOM_OBJ_ID_CAR_BODY, car_body, GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_CAR_WHEEL, car_wheel, GEOM_OBJ_TYPE_V);
	prepare_geom_obj(GEOM_OBJ_ID_CAR_NUT, car_nut, GEOM_OBJ_TYPE_V);
	prepare_path();
	prepare_car_path();
	set_up_scene_lights();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void print_message(const char * m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char *argv[]) {
	char program_name[256] = "Sogang CSE4170 Our_House_GLSL_V_0.5";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: 'c', 'f', 'd', 'ESC'" };

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(1200, 800);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}