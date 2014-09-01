#include "crystalpicnic.h"
#include "animation.h"
#include "graphics.h"
#include "shaders.h"

void Animation::draw_scaled(int sx, int sy, int sw, int sh, int dx, int dy,
	int dw, int dh, int flags)
{
	Wrap::Bitmap *bmp = frames[current_frame]->get_bitmap()->get_bitmap();
	
	al_draw_scaled_bitmap(bmp->bitmap, sx, sy, sw, sh, dx, dy, dw, dh, flags);
}

void Animation::draw(int x, int y, int flags)
{
	draw_tinted(al_color_name("white"), x, y, flags);
}

void Animation::draw_tinted_rotated(ALLEGRO_COLOR tint, int x, int y, int flags)
{
	if (!rotating) {
		draw_tinted(tint, x, y, flags);
		return;
	}

	Bitmap *b = frames[current_frame]->get_bitmap();
	Wrap::Bitmap *bmp = b->get_bitmap();
	al_draw_tinted_rotated_bitmap(
		bmp->bitmap,
		tint,
		rotation_center.x, rotation_center.y,
		x+rotation_center.x, y+rotation_center.y,
		(flags & ALLEGRO_FLIP_HORIZONTAL) ? -angle : angle,
		flags
	);
}

void Animation::draw_add_tinted_rotated(ALLEGRO_COLOR tint, float p, int x, int y, int flags)
{
	if (!rotating) {
		draw_add_tinted(tint, p, x, y, flags);
		return;
	}

	Bitmap *b = frames[current_frame]->get_bitmap();
	Wrap::Bitmap *bmp = b->get_bitmap();
	Wrap::Shader *tinter = Graphics::get_add_tint_shader();
	Shader::use(tinter);
	al_set_shader_float("p", p);
	al_set_shader_float("color_r", tint.r);
	al_set_shader_float("color_g", tint.g);
	al_set_shader_float("color_b", tint.b);
	al_draw_tinted_rotated_bitmap(
		bmp->bitmap,
		al_color_name("white"),
		rotation_center.x, rotation_center.y,
		x+rotation_center.x, y+rotation_center.y,
		(flags & ALLEGRO_FLIP_HORIZONTAL) ? -angle : angle,
		flags
	);
	Shader::use(NULL);
}

void Animation::draw_rotated(int x, int y, int flags)
{
	draw_tinted_rotated(al_map_rgb_f(1, 1, 1), x, y, flags);
}

void Animation::draw_tinted(ALLEGRO_COLOR tint, int x, int y, int flags)
{
	Bitmap *b = frames[current_frame]->get_bitmap();
	b->draw_region_tinted(tint, 0, 0, b->get_width(), b->get_height(),
		x, y, flags);
}

void Animation::draw_add_tinted(ALLEGRO_COLOR tint, float p, int x, int y, int flags)
{
	Bitmap *b = frames[current_frame]->get_bitmap();
	ALLEGRO_BITMAP *bmp = b->get_bitmap()->bitmap;
	Wrap::Shader *tinter = Graphics::get_add_tint_shader();
	Shader::use(tinter);
	al_set_shader_float("p", p);
	al_set_shader_float("color_r", tint.r);
	al_set_shader_float("color_g", tint.g);
	al_set_shader_float("color_b", tint.b);
	al_draw_bitmap_region(bmp, 0, 0, b->get_width(), b->get_height(), x, y, flags);
	Shader::use(NULL);
}

void Animation::set_looping(bool l)
{
	looping = l;
}

void Animation::set_loop_mode(Loop_Mode m)
{
	loop_mode = m;
}

int Animation::get_length()
{
	int l = 0;

	for (int i = 0; i < num_frames; i++) {
		Frame *f = frames[i];
		l += f->get_delay();
	}
	
	if (loop_mode == LOOP_PINGPONG) {
		l = l + (l - frames[num_frames-1]->get_delay());
	}

	return l;
}

/* Returns false on fail.
 * frame is not copied so should not be destroyed.
 */
bool Animation::add_frame(Frame *frame)
{
	try {
		frames.push_back(frame);
		num_frames++;
	}
	catch (std::bad_alloc a) {
		return false;
	}
	return true;
}

/* Returns how many frames passed
 * Can go backwards.
 * Step is in milliseconds.
 */
int Animation::update(int step)
{
	angle += angle_inc;

	int passed = 0;

	if (step < 0) {
		count += step;
		while (count < 0) {
			passed++;
			current_frame -= increment;
			wrap();
			int thisDelay = frames[current_frame]->get_delay();
			count += thisDelay;
			if (thisDelay <= 0)
				break;
		}
	}
	else {
		count += step;
		int thisDelay = frames[current_frame]->get_delay();
		while (count >= thisDelay) {
			if ((!looping && ((loop_mode == LOOP_NORMAL && current_frame == num_frames-1) || (loop_mode == LOOP_PINGPONG && increment == -1 && current_frame == 0))) ||
					(num_frames == 1))
				break;
			count -= thisDelay;
			if (thisDelay <= 0)
				break;
			passed++;
			if (!(looping == false && loop_mode == LOOP_PINGPONG && increment == -1 && current_frame <= 0))
				current_frame+=increment;
			wrap();
			thisDelay = frames[current_frame]->get_delay();
		}
	}

	return passed;
}

int Animation::update(void)
{
	return update(General::LOGIC_MILLIS);
}

void Animation::set_frame(int frame)
{
	current_frame = frame;
}

void Animation::reset(void)
{
	current_frame = 0;
	count = 0;
	increment = 1;
}

void Animation::set_tags(std::vector<std::string> tags)
{
	this->tags = tags;

	// set up things for rotating animations
	for (unsigned int i = 0; i < tags.size(); i++) {
		std::string s = tags[i];
		if (s.substr(0, 12) == "rotate_speed") {
			angle_inc = atof(s.substr(13).c_str());
			rotating = true;
		}
		else if (s.substr(0, 13) == "rotate_center") {
			int xofs = s.find(' ') + 1;
			int xend = s.find(' ', xofs);
			int yofs = xend+1;
			int yend = s.find(' ', yofs);
			std::string xS = s.substr(xofs, xend-xofs);
			std::string yS = s.substr(yofs, yend-yofs);
			float xx = atof(xS.c_str());
			float yy = atof(yS.c_str());
			rotation_center = General::Point<float>(xx, yy);
			rotating = true;
		}
	}
}

Animation::Animation(std::string name) :
	num_frames(0),
	rotating(false),
	angle(0),
	loop_start(-1)
{
	this->name = name;
	current_frame = 0;
	count = 0;
	looping = true;
	loop_mode = LOOP_NORMAL;
	increment = 1;
}

/* Frames are destroyed
 */
Animation::~Animation(void)
{
	for (int i = 0; i < num_frames; i++) {
		delete frames[i];
	}

	frames.clear();
}

void Animation::wrap(void)
{
	if (loop_mode == LOOP_NORMAL) {
		if (looping) {
			if (current_frame < 0) {
				current_frame = num_frames-1;
			}
			else if (current_frame >= num_frames) {
				current_frame = (loop_start > 0) ? loop_start : 0;
			}
		}
		else {
			if (current_frame >= num_frames) {
				current_frame = num_frames-1;
			}
		}
	}
	else if (loop_mode == LOOP_PINGPONG) {
		if (looping) {
			if (current_frame < 0) {
				current_frame = num_frames == 1 ? 0 : 1;
				increment = 1;
			}
			else if (current_frame >= num_frames) {
				current_frame = num_frames == 1 ? num_frames-1 : num_frames-2;
				increment = -1;
			}
		}
		else {
			if (current_frame >= num_frames) {
				current_frame = num_frames-1;
				increment = -1;
			}
			else if (current_frame < 0 && increment < 0) {
				current_frame = 0;
			}
		}
		
	}
}

void Animation::set_loop_start(int loop_start)
{
	this->loop_start = loop_start;
}

