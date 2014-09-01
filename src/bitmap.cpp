#include "crystalpicnic.h"
#include "bitmap.h"

int Bitmap::get_width()
{
	return al_get_bitmap_width(bitmap->bitmap);
}

int Bitmap::get_height()
{
	return al_get_bitmap_height(bitmap->bitmap);
}

Wrap::Bitmap *Bitmap::get_bitmap()
{
	return bitmap;
}

void Bitmap::set(Wrap::Bitmap *bmp)
{
	bitmap = bmp;
}

void Bitmap::draw_region(float sx, float sy, int sw, int sh, float dx, float dy, int flags)
{
	draw_region_tinted(al_color_name("white"), sx, sy, sw, sh, dx, dy, flags);
}

void Bitmap::draw_region_tinted_depth(ALLEGRO_COLOR tint, float sx, float sy, int sw, int sh, float dx, float dy, int flags, float depth)
{
	ALLEGRO_TRANSFORM t, backup;
	al_copy_transform(&backup, al_get_current_transform());
	al_copy_transform(&t, al_get_current_transform());
	al_translate_transform_3d(&t, 0, 0, depth);
	al_use_transform(&t);

	al_draw_tinted_bitmap_region(bitmap->bitmap, tint, sx, sy, sw, sh, dx, dy, flags);

	al_use_transform(&backup);
}

void Bitmap::draw_region_tinted(ALLEGRO_COLOR tint, float sx, float sy, int sw, int sh, float dx, float dy, int flags)
{
	al_draw_tinted_bitmap_region(bitmap->bitmap, tint, sx, sy, sw, sh, dx, dy, flags);
}

Bitmap::Bitmap()
{
	bitmap = NULL;
	delete_bitmap = true;
}

Bitmap::Bitmap(bool delete_bitmap)
{
	bitmap = NULL;
	this->delete_bitmap = delete_bitmap;
}

Bitmap::~Bitmap()
{
	if (bitmap && delete_bitmap) {
		Wrap::destroy_bitmap(bitmap);
	}
}

