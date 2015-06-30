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
	al_install_keyboard();

	ALLEGRO_DISPLAY *display = al_create_display(512, 512);
	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	al_register_event_source(queue, al_get_keyboard_event_source());

	ATLAS *atlas = atlas_create(512, 512, 0, 1, true);

	for (i = 1; i < argc; i++) {
		atlas_add(atlas, Wrap::load_bitmap(argv[i]), i-1);
	}

	atlas_finish(atlas);

	bool done = false;
	int sheet = 0;

	while (!done) {
		al_set_target_backbuffer(display);
		al_clear_to_color(al_map_rgb(0, 0, 0));
		al_draw_bitmap(atlas_get_sheet(atlas, sheet)->bitmap, 0, 0, 0);
		al_flip_display();
		ALLEGRO_EVENT event;
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
			if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
				done = true;
			else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
				if (sheet > 0) sheet--;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
				 if (sheet < atlas_get_num_sheets(atlas)-1) sheet++;
			}
		}
	}

	return 0;
}

