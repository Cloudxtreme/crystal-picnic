#include "crystalpicnic.h"
#include "graphics.h"
#include "shaders.h"

#ifdef ALLEGRO_WINDOWS
#include <allegro5/allegro_direct3d.h>
#endif

ALLEGRO_DEBUG_CHANNEL("CrystalPicnic")

static Wrap::Shader *shadow_shader = NULL;
static Wrap::Shader *yellow_glow_shader = NULL;
static Wrap::Shader *add_tint_shader = NULL;
static Wrap::Shader *tint_shader = NULL;

static Animation_Set *gauge_anim;
static Animation_Set *thick_gauge_anim;

namespace Graphics {
	
const float SWIPE_SLANT = 100;
const float SWIPE_WIDTH = 500;

void side_swipe_in(ALLEGRO_COLOR color, float percent)
{
	float x = percent * (cfg.screen_w+SWIPE_WIDTH+SWIPE_SLANT) - (SWIPE_WIDTH+SWIPE_SLANT);
	unsigned char r, g, b;
	ALLEGRO_VERTEX verts[6*256];
	int vcount = 0;

	al_unmap_rgb(color, &r, &g, &b);

	for (int i = 0; i < 256; i++) {
		float ratio = (255-i)/255.0;
		float this_w = 1.2345 * sin(ratio*(M_PI/2));

		ALLEGRO_COLOR col = al_map_rgba(r, g, b, i);

		verts[vcount].x = x+SWIPE_SLANT;
		verts[vcount].y = cfg.screen_h;
		verts[vcount].z = 0;
		verts[vcount].color = col;
		vcount++;
		verts[vcount].x = x;
		verts[vcount].y = 0;
		verts[vcount].z = 0;
		verts[vcount].color = col;
		vcount++;
		verts[vcount].x = x+SWIPE_SLANT+this_w;
		verts[vcount].y = cfg.screen_h;
		verts[vcount].z = 0;
		verts[vcount].color = col;
		vcount++;
		verts[vcount].x = x;
		verts[vcount].y = 0;
		verts[vcount].z = 0;
		verts[vcount].color = col;
		vcount++;
		verts[vcount].x = x+SWIPE_SLANT+this_w;
		verts[vcount].y = cfg.screen_h;
		verts[vcount].z = 0;
		verts[vcount].color = col;
		vcount++;
		verts[vcount].x = x+this_w;
		verts[vcount].y = 0;
		verts[vcount].z = 0;
		verts[vcount].color = col;
		vcount++;

		x += this_w;
	}
	
	al_draw_prim(verts, 0, 0, 0, vcount, ALLEGRO_PRIM_TRIANGLE_LIST);

	al_draw_filled_triangle(
		x, 0,
		x+SWIPE_SLANT, 0,
		x+SWIPE_SLANT, cfg.screen_h,
		al_map_rgb(r, g, b)
	);
	al_draw_filled_rectangle(
		x+SWIPE_SLANT, 0,
		cfg.screen_w, cfg.screen_h,
		al_map_rgb(r, g, b)
	);
	
	// Don't know why but my netbook needs this
	if (al_get_display_flags(engine->get_display()) & ALLEGRO_OPENGL) {
		//glFlush();
	}
}

void side_swipe_out(ALLEGRO_COLOR color, float percent)
{
	float x = percent * (cfg.screen_w+SWIPE_WIDTH+SWIPE_SLANT);
	unsigned char r, g, b;
	ALLEGRO_VERTEX verts[6*256];
	int vcount = 0;

	al_unmap_rgb(color, &r, &g, &b);

	for (int i = 0; i < 256; i++) {
		float ratio = (255-i)/255.0;
		float this_w = 1.2345 * sin(ratio*(M_PI/2));

		ALLEGRO_COLOR col = al_map_rgba(r, g, b, i);

		verts[vcount].x = x-SWIPE_SLANT;
		verts[vcount].y = 0;
		verts[vcount].z = 0;
		verts[vcount].color = col;
		vcount++;
		verts[vcount].x = x;
		verts[vcount].y = cfg.screen_h;
		verts[vcount].z = 0;
		verts[vcount].color = col;
		vcount++;
		verts[vcount].x = x-SWIPE_SLANT-this_w;
		verts[vcount].y = 0;
		verts[vcount].z = 0;
		verts[vcount].color = col;
		vcount++;
		verts[vcount].x = x;
		verts[vcount].y = cfg.screen_h;
		verts[vcount].z = 0;
		verts[vcount].color = col;
		vcount++;
		verts[vcount].x = x-SWIPE_SLANT-this_w;
		verts[vcount].y = 0;
		verts[vcount].z = 0;
		verts[vcount].color = col;
		vcount++;
		verts[vcount].x = x-this_w;
		verts[vcount].y = cfg.screen_h;
		verts[vcount].z = 0;
		verts[vcount].color = col;
		vcount++;

		x -= this_w;
	}
		
	al_draw_prim(verts, 0, 0, 0, vcount, ALLEGRO_PRIM_TRIANGLE_LIST);

	al_draw_filled_triangle(
		x-SWIPE_SLANT, 0,
		x-SWIPE_SLANT, cfg.screen_h,
		x, cfg.screen_h,
		al_map_rgb(r, g, b)
	);
	al_draw_filled_rectangle(
		0, 0,
		x-SWIPE_SLANT, cfg.screen_h,
		al_map_rgb(r, g, b)
	);
	
	// Don't know why but my netbook needs this
	if (al_get_display_flags(engine->get_display()) & ALLEGRO_OPENGL) {
		//glFlush();
	}
}

void turn_bitmap(Wrap::Bitmap *bitmap, float angle)
{
	ALLEGRO_TRANSFORM t, backup, backup2;
	ALLEGRO_DISPLAY *display = engine->get_display();
	al_copy_transform(&backup, al_get_projection_transform(display));
	al_copy_transform(&backup2, al_get_current_transform());

	al_identity_transform(&t);
	al_use_transform(&t);

	float x;
	float y;
	if (cfg.screen_w > cfg.screen_h) {
		x = 1.0f / 2.0f;
		y = (float)cfg.screen_h / cfg.screen_w / 2.0f;
	}
	else {
		x = (float)cfg.screen_w / cfg.screen_h / 2.0f;
		y = 1.0f / 2.0f;
	}

	al_scale_transform_3d(&t, 10.0f, 10.0f, 1.0f);	
	al_rotate_transform_3d(&t, 0.0f, 1.0f, 0.0f, angle);
	al_translate_transform_3d(&t, 0, 0, -10.0f);


	al_perspective_transform(
		&t,
		-x,
		-y,
		1,
		x,
		y,
		1000
	);

	al_set_projection_transform(display, &t);
	
	int low, high;
	
	if (angle >= M_PI/2) {
		low = cfg.screen_w;
		high = 0;
	}
	else {
		low = 0;
		high = cfg.screen_w;
	}

	float x1, x2;
	float y1, y2;
#ifdef ALLEGRO_WINDOWS_XXX
	if (al_get_display_flags(engine->get_display()) & ALLEGRO_DIRECT3D) {
		x1 = 0;
		y1 = 0.5f - y;
		x2 = 1;
		y2 = 0.5f + y;
	}
	else
#endif
	{
		x1 = -x;
		y1 = -y;
		x2 = x;
		y2 = y;
	}

	ALLEGRO_VERTEX v[4];
	v[0].x = x1;
	v[0].y = y2;
	v[0].z = 0;
	v[0].u = low;
	v[0].v = cfg.screen_h;
	v[0].color = al_color_name("white");
	v[1].x = x1;
	v[1].y = y1;
	v[1].z = 0;
	v[1].u = low;
	v[1].v = 0;
	v[1].color = al_color_name("white");
	v[2].x = x2;
	v[2].y = y2;
	v[2].z = 0;
	v[2].u = high;
	v[2].v = cfg.screen_h;
	v[2].color = al_color_name("white");
	v[3].x = x2;
	v[3].y = y1;
	v[3].z = 0;
	v[3].u = high;
	v[3].v = 0;
	v[3].color = al_color_name("white");
	
	al_draw_prim(v, 0, bitmap->bitmap, 0, 4, ALLEGRO_PRIM_TRIANGLE_STRIP);
	
	al_set_projection_transform(display, &backup);
	al_use_transform(&backup2);
}

void draw_bitmap_shadow(Wrap::Bitmap *bmp, int x, int y)
{
	if (shadow_shader == NULL) {
		shadow_shader = Shader::get("shadow");
	}
	Wrap::Bitmap *work = engine->get_work_bitmap();
	ALLEGRO_BITMAP *old_target2 = al_get_target_bitmap();
	al_set_target_bitmap(work->bitmap);
	al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));
	al_draw_bitmap(bmp->bitmap, 5, 5, 0);
	al_set_target_bitmap(old_target2);
	int w = al_get_bitmap_width(work->bitmap);
	int h = al_get_bitmap_height(work->bitmap);
	int w2 = al_get_bitmap_width(bmp->bitmap);
	int h2 = al_get_bitmap_height(bmp->bitmap);
	Shader::use(shadow_shader);
	al_set_shader_sampler("bmp", work->bitmap, 0);
	al_set_shader_float("bmp_w", w);
	al_set_shader_float("bmp_h", h);
	al_draw_bitmap_region(work->bitmap, 0, 0, w2+10, h2+10, x-5, y-5, 0);
	Shader::use(NULL);
}

void draw_bitmap_shadow_region_no_intermediate(Wrap::Bitmap *bmp, int sx, int sy, int sw, int sh, int dx, int dy)
{
	if (shadow_shader == NULL) {
		shadow_shader = Shader::get("shadow");
	}
	int w = al_get_bitmap_width(bmp->bitmap);
	int h = al_get_bitmap_height(bmp->bitmap);
	Shader::use(shadow_shader);
	al_set_shader_sampler("bmp", bmp->bitmap, 0);
	al_set_shader_float("bmp_w", w);
	al_set_shader_float("bmp_h", h);
	al_draw_bitmap_region(bmp->bitmap, sx, sy, sw, sh, dx, dy, 0);
	Shader::use(NULL);
}

void draw_focus_circle(float cx, float cy, float radius, ALLEGRO_COLOR color)
{
	ALLEGRO_BITMAP *target = al_get_target_bitmap();
	int target_w = al_get_bitmap_width(target);
	float scale = (float)target_w / cfg.screen_w;
	
	ALLEGRO_TRANSFORM t, backup;
	al_copy_transform(&backup, al_get_current_transform());
	al_identity_transform(&t);
	al_use_transform(&t);

	double time = fmod(al_get_time(), 1.0);
	if (time > 0.5) time = 1.0 - time;

	al_draw_circle(cx*scale, cy*scale, radius*scale+scale*time, color, scale);

	al_use_transform(&backup);
}

// ox, oy are offset of the camera, like area->top which is needed to keep line segments from moving when screen is panned
void draw_stippled_line(float x1, float y1, float x2, float y2, float ox, float oy, ALLEGRO_COLOR color, float thickness, float run1, float run2)
{
	float len_x = x2 - x1;
	float len_y = y2 - y1;
	float len = sqrt(len_x*len_x + len_y*len_y);
	float a = atan2(len_y, len_x);

	for (float done = 0.0f; done < len; done += run1+run2) {
		float run = MIN(done, run1);
		float start_x = x1 + cos(a) * done;
		float start_y = y1 + sin(a) * done;
		float end_x = start_x + cos(a) * run;
		float end_y = start_y + sin(a) * run;
		al_draw_line(start_x-ox, start_y-oy, end_x-ox, end_y-oy, color, thickness);
	}
}

void draw_info_box(float topright_x, float topright_y, int width, int height, std::string text)
{
	Wrap::Bitmap *work = engine->get_work_bitmap();
	ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
	al_set_target_bitmap(work->bitmap);
	al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));
	al_draw_filled_rectangle(
		5, 5,
		5+width, 5+height,
		al_color_name("black")
	);
	al_set_target_bitmap(old_target);
	draw_bitmap_shadow_region_no_intermediate(work, 0, 0, width+10, height+10, topright_x-width-5, topright_y-5);

	al_draw_filled_rectangle(
		topright_x-width,
		topright_y,
		topright_x,
		topright_y+height,
		al_color_name("tan")
	);

	ALLEGRO_COLOR dark = Graphics::change_brightness(al_color_name("tan"), 0.5f);
	al_draw_rectangle(
		topright_x-width+0.5f,
		topright_y+0.5f,
		topright_x-0.5f,
		topright_y+height-0.5f,
		dark,
		1
	);

	General::draw_wrapped_text(text, al_color_name("black"), topright_x-width+2, topright_y+2, width-4);
}

void draw_gauge(int x, int y, int width, bool thick, float percent, ALLEGRO_COLOR gauge_hilight_color, ALLEGRO_COLOR color)
{
	if (gauge_anim == NULL) {
		gauge_anim = new Animation_Set();
		gauge_anim->load("misc_graphics/interface/gauge");
		thick_gauge_anim = new Animation_Set();
		thick_gauge_anim->load("misc_graphics/interface/thick_gauge");
	}

	// draw color
	int startx = x+2;
	int endx = x+width-2;
	int length = (endx-startx) * percent;
	
	al_draw_filled_rectangle(startx, y+1, endx, y+3+(thick ? 1 : 0), al_color_name("black"));

	if (length > 0) {
		if (percent > 0 && length == 0) length = 1;
		ALLEGRO_COLOR darker = color;
		darker.r = MAX(0, darker.r-0.25f);
		darker.g = MAX(0, darker.g-0.25f);
		darker.b = MAX(0, darker.b-0.25f);
		al_draw_filled_rectangle(startx, y+1, startx+length, y+2, darker);
		al_draw_filled_rectangle(startx, y+2, startx+length, y+3+(thick ? 1 : 0), color);
		// Don't know why but my netbook needs this
		//if (al_get_display_flags(engine->get_display()) & ALLEGRO_OPENGL) {
			//glFlush();
		//}
	}

	Animation_Set *anim_set;
	
	if (thick) {
		anim_set = thick_gauge_anim;
	}
	else {
		anim_set = gauge_anim;
	}

	al_hold_bitmap_drawing(true);

	ALLEGRO_BITMAP *left, *right, *middle;

	anim_set->set_sub_animation("left");
	left = anim_set->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap()->bitmap,
	anim_set->set_sub_animation("right");
	right = anim_set->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap()->bitmap,
	anim_set->set_sub_animation("middle");
	middle = anim_set->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap()->bitmap,

	al_draw_tinted_bitmap(left, gauge_hilight_color, x, y, 0);
	al_draw_tinted_scaled_bitmap(
		middle,
		gauge_hilight_color,
		0, 0, al_get_bitmap_width(middle), al_get_bitmap_height(middle),
		x+al_get_bitmap_width(left), y,
		width-al_get_bitmap_width(left)-al_get_bitmap_width(right),
		al_get_bitmap_height(middle),
		0
	);
	al_draw_tinted_bitmap(right, gauge_hilight_color, x+width-al_get_bitmap_width(right), y, 0);

	al_hold_bitmap_drawing(false);
}

ALLEGRO_COLOR change_brightness(ALLEGRO_COLOR in, float amount)
{	
	ALLEGRO_COLOR bright = al_map_rgb_f(
		MAX(0.0f, MIN(1.0f, in.r * amount)),
		MAX(0.0f, MIN(1.0f, in.g * amount)),
		MAX(0.0f, MIN(1.0f, in.b * amount)) 
	);
	
	return bright;
}

void init()
{
	yellow_glow_shader = Shader::get("yellow_glow");
	add_tint_shader = Shader::get("add_tint");
	tint_shader = Shader::get("tint");
}

void shutdown()
{
	if (shadow_shader) {
		Shader::destroy(shadow_shader);
	}
	if (yellow_glow_shader) {
		Shader::destroy(yellow_glow_shader);
	}
	if (add_tint_shader) {
		Shader::destroy(add_tint_shader);
	}
	if (tint_shader) {
		Shader::destroy(tint_shader);
	}

	if (gauge_anim) {
		delete gauge_anim;
		gauge_anim = NULL;
		delete thick_gauge_anim;
		thick_gauge_anim = NULL;
	}
}

void capture_target(ALLEGRO_BITMAP *tmp)
{
	ALLEGRO_BITMAP *target = al_get_target_bitmap();

#ifdef ALLEGRO_ANDROID
	/*
	int flags = al_get_new_bitmap_flags();
	int format = al_get_new_bitmap_format();
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	al_set_new_bitmap_format(al_get_bitmap_format(target));
	ALLEGRO_BITMAP *mem = al_create_bitmap(
		al_get_bitmap_width(target),
		al_get_bitmap_height(target)
	);
	al_set_new_bitmap_flags(flags);
	al_set_new_bitmap_format(format);

	al_set_target_bitmap(mem);
	al_draw_bitmap(target, 0, 0, 0);
	al_set_target_bitmap(tmp);
	al_draw_bitmap(mem, 0, 0, 0);
	al_set_target_bitmap(target);

	al_destroy_bitmap(mem);
	*/
	const int MAX_SIZE = 512;

	int w = al_get_bitmap_width(target);
	int h = al_get_bitmap_height(target);
	
	int sz_w = ceil((float)w / MAX_SIZE);
	int sz_h = ceil((float)h / MAX_SIZE);

	for (int yy = 0; yy < sz_h; yy++) {
		for (int xx = 0; xx < sz_w; xx++) {
			int ww = MIN(MAX_SIZE, w-(xx*MAX_SIZE));
			int hh = MIN(MAX_SIZE, h-(yy*MAX_SIZE));
			ALLEGRO_LOCKED_REGION *lr1 = al_lock_bitmap_region(
				tmp,
				xx*MAX_SIZE, yy*MAX_SIZE, ww, hh,
				al_get_bitmap_format(target), ALLEGRO_LOCK_WRITEONLY
			);
			ALLEGRO_LOCKED_REGION *lr2 = al_lock_bitmap_region(
				target,
				xx*MAX_SIZE, yy*MAX_SIZE, ww, hh,
				al_get_bitmap_format(target), ALLEGRO_LOCK_READONLY
			);
			int pixel_size = al_get_pixel_size(al_get_bitmap_format(target));
			for (int y = 0; y < hh; y++) {
				uint8_t *d1 = (uint8_t *)lr1->data + lr1->pitch * y;
				uint8_t *d2 = (uint8_t *)lr2->data + lr2->pitch * y;
				memcpy(d1, d2, pixel_size*ww);
			}
			al_unlock_bitmap(tmp);
			al_unlock_bitmap(target);
		}
	}
#else
	ALLEGRO_BITMAP *old_target = al_get_target_bitmap();

	al_set_target_bitmap(tmp);

	al_clear_to_color(al_map_rgb_f(0, 0, 0));

	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	
	al_draw_bitmap(target, 0, 0, 0);

	al_set_target_bitmap(old_target);
#endif
}

void draw_tinted_bitmap_region_depth_yellow_glow(
	Wrap::Bitmap *bitmap,
	ALLEGRO_COLOR tint,
	float sx, float sy,
	float sw, float sh,
	float dx, float dy,
	int flags,
	float depth,
	int r1, int g1, int b1,
	int r2, int g2, int b2)
{
	/*
	ALLEGRO_TRANSFORM t, backup;
	al_copy_transform(&backup, al_get_current_transform());
	al_copy_transform(&t, al_get_current_transform());
	al_translate_transform_3d(&t, 0, 0, depth);
	al_use_transform(&t);
	*/

	float p = fmod(al_get_time(), 2.0);
	if (p >= 1.0f) {
		p = 1.0f - (p - 1.0f);
	}
	
	Shader::use(yellow_glow_shader);
	al_set_shader_float("p", p);
	float color1[3] = {
		(float)r1/255.0f,
		(float)g1/255.0f,
		(float)b1/255.0f,
	};
	float color2[3] = {
		(float)r2/255.0f,
		(float)g2/255.0f,
		(float)b2/255.0f,
	};

	al_set_shader_float("color1_r", color1[0]);
	al_set_shader_float("color1_g", color1[1]);
	al_set_shader_float("color1_b", color1[2]);
	al_set_shader_float("color2_r", color2[0]);
	al_set_shader_float("color2_g", color2[1]);
	al_set_shader_float("color2_b", color2[2]);

	al_draw_tinted_bitmap_region(bitmap->bitmap, tint, sx, sy, sw, sh, dx, dy, flags);
	Shader::use(NULL);

	//al_use_transform(&backup);
}

void get_texture_size(ALLEGRO_BITMAP *bmp, int *outw, int *outh)
{
	if (al_get_display_flags(engine->get_display()) & ALLEGRO_OPENGL) {
		al_get_opengl_texture_size(bmp, outw, outh);
	}
#ifdef ALLEGRO_WINDOWS
	else {
		al_get_d3d_texture_size(bmp, outw, outh);
	}
#endif
}

Wrap::Shader *get_add_tint_shader()
{
	return add_tint_shader;
}

Wrap::Shader *get_tint_shader()
{
	return tint_shader;
}

void draw_gradient_rectangle(float x1, float y1, float x2, float y2, ALLEGRO_COLOR x1y1, ALLEGRO_COLOR x2y1, ALLEGRO_COLOR x2y2, ALLEGRO_COLOR x1y2)
{
	ALLEGRO_VERTEX v[4];

	v[0].x = x1;
	v[0].y = y2;
	v[0].z = 0;
	v[0].color = x1y2;
	v[1].x = x1;
	v[1].y = y1;
	v[1].z = 0;
	v[1].color = x1y1;
	v[2].x = x2;
	v[2].y = y1;
	v[2].z = 0;
	v[2].color = x2y1;
	v[3].x = x2;
	v[3].y = y2;
	v[3].z = 0;
	v[3].color = x2y2;

	al_draw_prim(v, NULL, NULL, 0, 4, ALLEGRO_PRIM_TRIANGLE_FAN);
}

ALLEGRO_BITMAP *clone_target()
{
	ALLEGRO_BITMAP *target = al_get_target_bitmap();

	int format = al_get_new_bitmap_format();
	al_set_new_bitmap_format(General::noalpha_bmp_format);
	ALLEGRO_BITMAP *tmp = al_create_bitmap(
		al_get_bitmap_width(target),
		al_get_bitmap_height(target)
	);
	al_set_new_bitmap_format(format);

#ifdef ALLEGRO_ANDROID
	const int MAX_SIZE = 512;

	int w = al_get_bitmap_width(target);
	int h = al_get_bitmap_height(target);
	
	int sz_w = ceil((float)w / MAX_SIZE);
	int sz_h = ceil((float)h / MAX_SIZE);

	for (int yy = 0; yy < sz_h; yy++) {
		for (int xx = 0; xx < sz_w; xx++) {
			int ww = MIN(MAX_SIZE, w-(xx*MAX_SIZE));
			int hh = MIN(MAX_SIZE, h-(yy*MAX_SIZE));
			ALLEGRO_LOCKED_REGION *lr1 = al_lock_bitmap_region(
				tmp,
				xx*MAX_SIZE, yy*MAX_SIZE, ww, hh,
				al_get_bitmap_format(target), ALLEGRO_LOCK_WRITEONLY
			);
			ALLEGRO_LOCKED_REGION *lr2 = al_lock_bitmap_region(
				target,
				xx*MAX_SIZE, yy*MAX_SIZE, ww, hh,
				al_get_bitmap_format(target), ALLEGRO_LOCK_READONLY
			);
			int pixel_size = al_get_pixel_size(al_get_bitmap_format(target));
			for (int y = 0; y < hh; y++) {
				uint8_t *d1 = (uint8_t *)lr1->data + lr1->pitch * y;
				uint8_t *d2 = (uint8_t *)lr2->data + lr2->pitch * y;
				memcpy(d1, d2, pixel_size*ww);
			}
			al_unlock_bitmap(tmp);
			al_unlock_bitmap(target);
		}
	}
#else
	ALLEGRO_BITMAP *old_target = al_get_target_bitmap();

	al_set_target_bitmap(tmp);

	al_clear_to_color(al_map_rgb_f(0, 0, 0));

	ALLEGRO_STATE state;
	al_store_state(&state, ALLEGRO_STATE_BLENDER);

	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	
	al_draw_bitmap(target, 0, 0, 0);

	al_restore_state(&state);

	al_set_target_bitmap(old_target);
#endif

	return tmp;
}

} // end namespace Graphics
