#include <allegro5/allegro5.h>
#include "allegro5/allegro_shader.h"
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_opengl.h>
#include <cstdio>

static const char *glsl_vertex_source =
   "attribute vec4 pos;\n"
   "attribute vec4 color;\n"
   "attribute vec2 texcoord;\n"
   "uniform mat4 proj_matrix;\n"
   "uniform mat4 view_matrix;\n"
   "uniform mat4 user_matrix;\n"
   "varying vec4 varying_color;\n"
   "varying vec2 varying_texcoord;\n"
   "void main()\n"
   "{\n"
   "  varying_color = color;\n"
   "  varying_texcoord = texcoord;\n"
   "  gl_Position = pos * user_matrix * view_matrix * proj_matrix;\n"
   "}\n";

static const char *glsl_pixel_source_vert =
   "uniform sampler2D t;\n"
   "uniform float radius;\n"
   "uniform float img_height;\n"
   "varying vec4 varying_color;\n"
   "varying vec2 varying_texcoord;\n"
   "void main()\n"
   "{\n"
   "  vec4 result = vec4(0.0, 0.0, 0.0, 0.0);\n"
   "  float count = 0.0;\n"
   "  float y;\n"
   "  for (y = -radius; y < radius; y+=1.0) {\n"
   "    float yy = varying_texcoord.t + y / img_height;\n"
   "    vec4 color = texture2D(t, vec2(varying_texcoord.s, yy));\n"
   "    result.r = ((result.r * count) + color.r) / (count+1.0);\n"
   "    result.g = ((result.g * count) + color.g) / (count+1.0);\n"
   "    result.b = ((result.b * count) + color.b) / (count+1.0);\n"
   "    result.a = ((result.a * count) + color.a) / (count+1.0);\n"
   "    count = count + 1.0;\n"
   "  }\n"
   "  gl_FragColor = result;\n"
   "}\n";

static const char *glsl_pixel_source_horz =
   "uniform sampler2D t;\n"
   "uniform float radius;\n"
   "uniform float img_width;\n"
   "varying vec4 varying_color;\n"
   "varying vec2 varying_texcoord;\n"
   "void main()\n"
   "{\n"
   "  vec4 result = vec4(0.0, 0.0, 0.0, 0.0);\n"
   "  float count = 0.0;\n"
   "  float x;\n"
   "  for (x = -radius; x < radius; x+=1.0) {\n"
   "    float xx = varying_texcoord.s + x / img_width;\n"
   "    vec4 color = texture2D(t, vec2(xx, varying_texcoord.t));\n"
   "    result.r = ((result.r * count) + color.r) / (count+1.0);\n"
   "    result.g = ((result.g * count) + color.g) / (count+1.0);\n"
   "    result.b = ((result.b * count) + color.b) / (count+1.0);\n"
   "    result.a = ((result.a * count) + color.a) / (count+1.0);\n"
   "    count = count + 1.0;\n"
   "  }\n"
   "  result.r *= 1.9;\n"
   "  result.g *= 1.9;\n"
   "  result.b *= 1.9;\n"
   "  result.a *= 1.9;\n"
   "  gl_FragColor = result;\n"
   "}\n";

int main(int argc, char **argv)
{
	ALLEGRO_DISPLAY *display;
	ALLEGRO_BITMAP *bmp, *tmp;
	ALLEGRO_SHADER *horz_shader;
	ALLEGRO_SHADER *vert_shader;

	al_init();
	al_install_keyboard();
	al_init_image_addon();

	display = al_create_display(480, 320);
	bmp = al_load_bitmap("glow.png");
	ALLEGRO_BITMAP *bg = al_load_bitmap("bg.png");
	tmp = al_create_bitmap(
		al_get_bitmap_width(bmp),
		al_get_bitmap_height(bmp)
	);

	horz_shader = al_create_shader(ALLEGRO_SHADER_GLSL);
	vert_shader = al_create_shader(ALLEGRO_SHADER_GLSL);

	al_attach_shader_source(
		horz_shader,
		ALLEGRO_VERTEX_SHADER,
		glsl_vertex_source
	);
	al_attach_shader_source(
		horz_shader,
		ALLEGRO_PIXEL_SHADER,
		glsl_pixel_source_horz
	);
	al_link_shader(horz_shader);

	al_attach_shader_source(
		vert_shader,
		ALLEGRO_VERTEX_SHADER,
		glsl_vertex_source
	);
	al_attach_shader_source(
		vert_shader,
		ALLEGRO_PIXEL_SHADER,
		glsl_pixel_source_vert
	);
	al_link_shader(vert_shader);

	ALLEGRO_COLOR white = al_map_rgb(255, 255, 255);
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, white);

	ALLEGRO_VERTEX *v;
	v = new ALLEGRO_VERTEX[6*4];

	v[0].x = 0;
	v[0].y = 320;
	v[0].z = 0;
	v[0].u = 0;
	v[0].v = 0;
	v[0].color = white;
	v[1].x = 0;
	v[1].y = 0;
	v[1].z = 0;
	v[1].u = 0;
	v[1].v = 1;
	v[1].color = white;
	v[2].x = 480;
	v[2].y = 0;
	v[2].z = 0;
	v[2].u = 1;
	v[2].v = 1;
	v[2].color = white;

	v[3].x = 0;
	v[3].y = 320;
	v[3].z = 0;
	v[3].u = 0;
	v[3].v = 0;
	v[3].color = white;
	v[4].x = 480;
	v[4].y = 0;
	v[4].z = 0;
	v[4].u = 1;
	v[4].v = 1;
	v[4].color = white;
	v[5].x = 480;
	v[5].y = 320;
	v[5].z = 0;
	v[5].u = 1;
	v[5].v = 0;
	v[5].color = white;

	al_set_shader_vertex_array(horz_shader, &v[0].x, sizeof(ALLEGRO_VERTEX));
	al_set_shader_color_array(horz_shader, (unsigned char *)&v[0].color, sizeof(ALLEGRO_VERTEX));
	al_set_shader_texcoord_array(horz_shader, &v[0].u, sizeof(ALLEGRO_VERTEX));

	al_set_shader_vertex_array(vert_shader, &v[0].x, sizeof(ALLEGRO_VERTEX));
	al_set_shader_color_array(vert_shader, (unsigned char *)&v[0].color, sizeof(ALLEGRO_VERTEX));
	al_set_shader_texcoord_array(vert_shader, &v[0].u, sizeof(ALLEGRO_VERTEX));

	float radius = 1;
	float rinc = 4;

	while (1) {
		al_set_target_bitmap(tmp);
		al_clear_to_color(al_map_rgba(0, 0, 0,0));
		ALLEGRO_KEYBOARD_STATE s;
		al_get_keyboard_state(&s);
		if (al_key_down(&s, ALLEGRO_KEY_ESCAPE))
			break;

		radius += rinc;
		if (rinc > 0 && radius >= 25) {
			rinc = -rinc;
		}
		else if (rinc < 0 && radius < 1) {
			rinc = -rinc;
		}

		al_set_shader_sampler(horz_shader, "t", bmp, 0);
		al_set_shader_float(horz_shader, "img_width", al_get_bitmap_width(bmp));
		al_set_shader_float(horz_shader, "radius", radius);
		al_use_shader(horz_shader, true);
		al_draw_bitmap(bmp, 0, 0, 0);
		al_use_shader(horz_shader, false);

		al_set_target_bitmap(al_get_backbuffer());
		al_draw_bitmap(bg, 0, 0, 0);
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE, white);
		al_set_shader_sampler(vert_shader, "t", tmp, 0);
		al_set_shader_float(vert_shader, "img_height", al_get_bitmap_height(bmp));
		al_set_shader_float(vert_shader, "radius", radius);
		al_use_shader(vert_shader, true);
		al_draw_bitmap(tmp, 0, 0, 0);
		al_use_shader(vert_shader, false);
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, white);

		al_draw_bitmap(bmp, 0, 0, 0);

		al_flip_display();
		al_rest(0.016);
	}

	al_save_bitmap("bgxx.png", bg);
	al_save_bitmap("tmpxx.png", tmp);

	return 0;
}
