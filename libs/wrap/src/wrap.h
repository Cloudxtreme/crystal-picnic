#ifndef WRAP_H
#define WRAP_H

#include <allegro5/allegro.h>

#include <string>

// Define these
ALLEGRO_BITMAP *my_load_bitmap(std::string filename);
ALLEGRO_SHADER *my_create_shader(std::string vertex_source, std::string pixel_source);

namespace Wrap
{

class Bitmap {
public:
	Bitmap() {}
	Bitmap(ALLEGRO_BITMAP *b, std::string fn) {
		bitmap = b;
		filename = fn;
		flags = al_get_new_bitmap_flags();
		format = al_get_new_bitmap_format();
	}

	ALLEGRO_BITMAP *bitmap;
	std::string filename;
	int flags;
	int format;
};

class Shader {
public:
	Shader(std::string vertex_source, std::string pixel_source)
	{
		shader = my_create_shader(vertex_source, pixel_source);
		this->vertex_source = vertex_source;
		this->pixel_source = pixel_source;
	}

	ALLEGRO_SHADER *shader;
	std::string vertex_source, pixel_source;
};

Bitmap *load_bitmap(std::string filename);
Bitmap *create_bitmap(int width, int height);
Bitmap *create_bitmap_no_preserve(int width, int height);
Bitmap *create_sub_bitmap(Bitmap *parent, int x, int y, int w, int h);
void destroy_bitmap(Bitmap *b);
void destroy_loaded_bitmaps();
void reload_loaded_bitmaps();

Shader *create_shader(std::string vertex_source, std::string pixel_source);
void destroy_shader(Shader *s);
void destroy_loaded_shaders();
void reload_loaded_shaders();

} // end namepsace Wrap

#endif // BITMAP_WRAPPER_H
