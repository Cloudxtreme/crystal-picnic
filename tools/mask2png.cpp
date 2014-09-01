#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc < 5) {
		printf("%s <input mask> <mask width> <mask height> <output.png>\n", argv[0]);
		return 0;
	}

	const char *filename = argv[1];
	int w = atoi(argv[2]);
	int h = atoi(argv[3]);
	int orig_w = w;

	if (w % 8 != 0) {
		w += 8 - (w % 8);
	}

	al_init();
	al_install_keyboard();
	al_install_mouse();
	al_init_image_addon();
	al_init_primitives_addon();

	ALLEGRO_DISPLAY *display = al_create_display(800, 600);

	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	ALLEGRO_BITMAP *bmp = al_create_bitmap(orig_w, h);
	al_set_target_bitmap(bmp);

	FILE *f = fopen(filename, "rb");
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x += 8) {
			int c = fgetc(f);
			for (int bit = 0; bit < 8; bit++) {
				int this_bit = (c >> (7-bit)) & 1;
				if (this_bit) {
					al_put_pixel(x+bit, y, al_map_rgb(0, 0, 0));
				}
				else {
					al_put_pixel(x+bit, y, al_map_rgb(255, 255, 255));
				}
			}
		}
	}


	al_save_bitmap(argv[4], bmp);

	return 0;
}

