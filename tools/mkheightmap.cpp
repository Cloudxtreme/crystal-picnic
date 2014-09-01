#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("Usage: mkheightmap <input.png> <output>\n");
		return 0;
	}

	al_init();
	al_init_image_addon();

	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);

	ALLEGRO_BITMAP *bmp = al_load_bitmap(argv[1]);
	FILE *out = fopen(argv[2], "wb");

	unsigned char result;

	for (int y = 0; y < al_get_bitmap_height(bmp); y++) {
		for (int x = 0; x < al_get_bitmap_width(bmp); x++) {
			ALLEGRO_COLOR c = al_get_pixel(bmp, x, y);
			unsigned char r, g, b, a;
			al_unmap_rgba(c, &r, &g, &b, &a);
			if (x % 2 == 0) {
				result = a & 0xf0;
				if (x == al_get_bitmap_width(bmp)-1) {
					fputc(result, out);
				}
			}
			else {
				result |= a >> 4;
				if (x != 0) {
					fputc(result, out);
				}
			}
		}
	}

	fclose(out);

	return 0;
}
