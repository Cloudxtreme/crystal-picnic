#ifndef BITMAP_H
#define BITMAP_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>

#include <string>

#include <wrap.h>

class Bitmap
{
public:
	int get_width();
	int get_height();

	Wrap::Bitmap *get_bitmap();

	void set(Wrap::Bitmap *bmp);
	bool load(std::string filename);
	void draw_region(float sx, float sy, int sw, int sh, float dx, float dy, int flags);
	void draw_region_tinted(ALLEGRO_COLOR tint, float sx, float sy, int sw, int sh, float dx, float dy, int flags);
	void draw_region_tinted_depth(ALLEGRO_COLOR tint, float sx, float sy, int sw, int sh, float dx, float dy, int flags, float depth);

	Bitmap();
	Bitmap(bool delete_bitmap);
	~Bitmap();

private:
	Wrap::Bitmap *bitmap;
	bool delete_bitmap;
};

#endif // BITMAP_H
