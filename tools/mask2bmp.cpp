#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <cstdio>

int main(int argc, char **argv)
{
	if (argc < 4) {
		printf("Usage: mask2bmp <mask> <w> <h>\n");
		return 0;
	}

	al_init();
	al_init_image_addon();
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	ALLEGRO_BITMAP *bmp = al_create_bitmap(atoi(argv[2]), atoi(argv[3]));
	al_set_target_bitmap(bmp);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
	al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);

	FILE *f = fopen(argv[1], "rb");
	int bytes = (atoi(argv[2])+7)/8;

	for (int y = 0; y < atoi(argv[3]); y++) {
		for (int b = 0; b < bytes; b++) {
			int byte = fgetc(f);
			for (int i = 0; i < 8; i++) {
				int bit = (byte >> (7-i)) & 1;
				if (bit) {
					al_put_pixel(b*8+i, y, al_map_rgb(0, 0, 0));
				}
			}
		}
	}

	al_unlock_bitmap(bmp);

	al_save_bitmap("out.png", bmp);
}

