#define ALLEGRO_STATICLINK
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <wrap.h>

#include "atlas.h"
#include "atlas_internal.h"

ALLEGRO_BITMAP *my_load_bitmap(std::string filename)
{
	ALLEGRO_FILE *f;

	f = al_fopen(filename.c_str(), "rb");

	if (!f) {
		return NULL;
	}

	ALLEGRO_BITMAP *bmp = al_load_bitmap_f(f, ".png");

	al_fclose(f);

	return bmp;
}

ALLEGRO_SHADER *my_create_shader(std::string vertex_source, std::string pixel_source)
{
	// not used
	return NULL;
}

int main(int argc, char **argv)
{
	int i;

	al_init();
	al_init_image_addon();

	ATLAS *atlas = atlas_create(1024, 1024, 0, 0, true);

	for (i = 1; i < argc; i++) {
		atlas_add(atlas, Wrap::load_bitmap(argv[i]), i-1);
	}

	atlas_finish(atlas);

	al_save_bitmap("test.png", atlas_get_sheet(atlas, 0)->bitmap);

	return 0;
}

