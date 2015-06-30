#include <allegro5/allegro.h>
#include <allegro5/allegro_memfile.h>

#include <vector>
#include <algorithm>

#include "wrap.h"

#define ERASE_FROM_UNSORTED_VECTOR(v, i) \
	v[i] = v[v.size()-1]; \
	v.pop_back();

static std::vector<Wrap::Bitmap *> bitmaps;
static std::vector<Wrap::Shader *> shaders;

namespace Wrap
{

Bitmap *load_bitmap(std::string filename)
{
	Bitmap *b = new Bitmap;
	if (!b) {
		return NULL;
	}
	b->bitmap = my_load_bitmap(filename);
	if (!b->bitmap) {
		delete b;
		return NULL;
	}
	b->filename = filename;
	b->flags = al_get_new_bitmap_flags();
	b->format = al_get_new_bitmap_format();
	bitmaps.push_back(b);
	return b;
}

Bitmap *create_bitmap(int width, int height)
{
	Bitmap *b = new Bitmap;
	int flags = al_get_new_bitmap_flags();
	al_set_new_bitmap_flags(flags & ~ALLEGRO_NO_PRESERVE_TEXTURE);
	b->bitmap = al_create_bitmap(width, height);
	al_set_new_bitmap_flags(flags);
	b->filename = "";
	return b;
}

Bitmap *create_bitmap_no_preserve(int width, int height)
{
	Bitmap *b = new Bitmap;
	b->bitmap = al_create_bitmap(width, height);
	b->filename = "";
	return b;
}

Bitmap *create_sub_bitmap(Bitmap *b, int x, int y, int w, int h)
{
	Bitmap *sub = new Bitmap;
	sub->bitmap = al_create_sub_bitmap(b->bitmap, x, y, w, h);
	return sub;
}

void destroy_bitmap(Bitmap *b)
{
	al_destroy_bitmap(b->bitmap);

	for (size_t i = 0; i < bitmaps.size(); i++) {
		if (bitmaps[i] == b) {
			ERASE_FROM_UNSORTED_VECTOR(bitmaps, i);
			break;
		}
	}

	delete b;
}

void destroy_loaded_bitmaps()
{
	for (size_t i = 0; i < bitmaps.size(); i++) {
		if (bitmaps[i]->filename != "") {
			al_destroy_bitmap(bitmaps[i]->bitmap);
		}
	}
}

void reload_loaded_bitmaps()
{
	for (size_t i = 0; i < bitmaps.size(); i++) {
		if (bitmaps[i]->filename != "") {
			int flags = al_get_new_bitmap_flags();
			int format = al_get_new_bitmap_format();
			al_set_new_bitmap_flags(bitmaps[i]->flags);
			al_set_new_bitmap_format(bitmaps[i]->format);
			bitmaps[i]->bitmap = my_load_bitmap(bitmaps[i]->filename);
			al_set_new_bitmap_flags(flags);
			al_set_new_bitmap_format(format);
		}
	}
}

void destroy_shader(Wrap::Shader *shader)
{
	std::vector<Wrap::Shader *>::iterator it = std::find(shaders.begin(), shaders.end(), shader);
	if (it != shaders.end()) {
		shaders.erase(it);
	}
	if (shader->shader) {
		al_destroy_shader(shader->shader);
	}
	delete shader;
}

void destroy_loaded_shaders()
{
	for (size_t i = 0; i < shaders.size(); i++) {
		if (!shaders[i]->shader) {	
			continue;
		}
		al_destroy_shader(shaders[i]->shader);
		shaders[i]->shader = NULL;
	}
}

void reload_loaded_shaders()
{
	for (size_t i = 0; i < shaders.size(); i++) {
		shaders[i]->shader = my_create_shader(shaders[i]->vertex_source, shaders[i]->pixel_source);
	}
}

Wrap::Shader *create_shader(std::string vertex_source, std::string pixel_source)
{
	Wrap::Shader *s = new Wrap::Shader(vertex_source, pixel_source);
	shaders.push_back(s);
	return s;
}

} // end namespace Wrap
