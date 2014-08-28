#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_opengl.h>

#include <cmath>

#include "config.h"

namespace Graphics {

void side_swipe_in(ALLEGRO_COLOR color, float percent);
void side_swipe_out(ALLEGRO_COLOR color, float percent);
void turn_bitmap(Wrap::Bitmap *bitmap, float angle);
void draw_bitmap_shadow(Wrap::Bitmap *bmp, int x, int y);
void draw_bitmap_shadow_region_no_intermediate(Wrap::Bitmap *bmp, int sx, int sy, int sw, int sh, int dx, int dy);
void draw_focus_circle(float cx, float cy, float radius, ALLEGRO_COLOR color);
void draw_stippled_line(float x1, float y1, float x2, float y2, float ox, float oy, ALLEGRO_COLOR color, float thickness, float run1, float run2);
void draw_info_box(float topright_x, float topright_y, int width, int height, std::string text);
void draw_gauge(int x, int y, int width, bool thick, float percent, ALLEGRO_COLOR gauge_hilight_color, ALLEGRO_COLOR color);
ALLEGRO_COLOR change_brightness(ALLEGRO_COLOR in, float amount);
void capture_target(ALLEGRO_BITMAP *tmp);
void draw_tinted_bitmap_region_depth_yellow_glow(
	Wrap::Bitmap *bitmap,
	ALLEGRO_COLOR tint,
	float sx, float sy,
	float sw, float sh,
	float dx, float dy,
	int flags,
	float depth,
	int r1, int g1, int b1,
	int r2, int g2, int b2);

void init();
void shutdown();
void get_texture_size(ALLEGRO_BITMAP *bmp, int *outw, int *outh);
Wrap::Shader *get_add_tint_shader();
Wrap::Shader *get_tint_shader();
void draw_gradient_rectangle(float x1, float y1, float x2, float y2, ALLEGRO_COLOR x1y1, ALLEGRO_COLOR x2y1, ALLEGRO_COLOR x2y2, ALLEGRO_COLOR x1y2);
ALLEGRO_BITMAP *clone_target();

}

#endif // GRAPHICS_H
