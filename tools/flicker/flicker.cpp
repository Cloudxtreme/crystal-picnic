#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>

int main()
{
	al_init();
	al_init_image_addon();

	ALLEGRO_BITMAP *logo = al_load_bitmap("logo_no_alpha.png");
	ALLEGRO_BITMAP *shine = al_load_bitmap("shine.png");
	int w = al_get_bitmap_width(logo);
	int h = al_get_bitmap_height(logo);
	ALLEGRO_BITMAP *buf = al_create_bitmap(w, h);


	for (int i = 0; i < 20; i++) {
		char filename[100];
		sprintf(filename, "frame%02d.png", i);
		al_set_target_bitmap(buf);
		al_clear_to_color(al_map_rgb(0x22, 0x22, 0x22));
		float f = (rand() % 1000) / 1000.0 / 2.0;
		if (rand() % 2) {
			f = f + 0.5;
			al_draw_tinted_bitmap(shine, al_map_rgba_f(f, f, f, f), 0, 0, 0);
		}
		else {
			al_draw_bitmap(shine, 0, 0, 0);
			al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
			al_draw_tinted_bitmap(shine, al_map_rgba_f(f, f, f, f), 0, 0, 0);
			al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
		}
		al_draw_bitmap(logo, 0, 0, 0);
		al_save_bitmap(filename, buf);
	}
}

