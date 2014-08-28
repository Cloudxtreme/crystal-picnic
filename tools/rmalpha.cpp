#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("Usage: rmalpha <infile> <outfile>\n");
		return 0;
	}

	al_init();
	al_init_image_addon();

	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA);
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);

	ALLEGRO_BITMAP *in = al_load_bitmap(argv[1]);
	ALLEGRO_BITMAP *out = al_create_bitmap(
		al_get_bitmap_width(in),
		al_get_bitmap_height(in)
	);
	al_set_target_bitmap(out);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
	al_lock_bitmap(in, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
	al_lock_bitmap(out, ALLEGRO_PIXEL_FORMAT_ANY, 0);

	for (int y = 0; y < al_get_bitmap_height(in); y++) {
		for (int x = 0; x < al_get_bitmap_width(in); x++) {
			ALLEGRO_COLOR c = al_get_pixel(in, x, y);
			if (c.a == 1.0) {
				al_put_pixel(x, y, c);
			}
		}
	}

	al_unlock_bitmap(in);
	al_unlock_bitmap(out);

	al_save_bitmap(argv[2], out);

	return 0;
}

