#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <zlib.h>
#include <cstdio>

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("Usage: %s <x.png/bmp/tga etc> <size>\n", argv[0]);
		exit(0);
	}

	const char *filename = argv[1];
	int section_size = atoi(argv[2]);

	al_init();
	al_init_image_addon();

	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE);
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	ALLEGRO_BITMAP *full_bmp = al_load_bitmap(filename);

	int full_w = al_get_bitmap_width(full_bmp);
	int full_h = al_get_bitmap_height(full_bmp);

	int sections_x = full_w / section_size;
	int sections_y = full_h / section_size;

	if (full_w % section_size > 0)
		sections_x++;
	if (full_h % section_size > 0)
		sections_y++;

	printf("image size = %d, %d\n", full_w, full_h);
	printf("sections_x = %d sections_y = %d\n", sections_x, sections_y);

	int used_x = 0;
	int used_y = 0;
	int count = 0;

	for (int y = 0; y < sections_y; y++) {
		used_x = 0;
		int this_h = (full_h-used_y >= section_size) ?
			section_size : (full_h-used_y);
		for (int x = 0; x < sections_x; x++) {
			int this_w = (full_w-used_x >= section_size) ?
				section_size : (full_w-used_x);
			char buf[100];
			sprintf(buf, "%04d", count++);
			FILE *f = fopen(buf, "wb");
			ALLEGRO_LOCKED_REGION *lr = al_lock_bitmap_region(
				full_bmp,
				used_x, used_y,
				this_w, this_h,
				ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE,
				ALLEGRO_LOCK_READONLY
			);
			unsigned char *line_start = (unsigned char *)lr->data;
			unsigned char *p = line_start;
			int count = 0;
			for (int y = 0; y < section_size; y++) {
				for (int x = 0; x < section_size; x ++) {
					if (x+used_x >= full_w ||
						y+used_y >= full_h) {
						fputc(0, f);
						fputc(0, f);
					}
					else {
						unsigned char r, g, b, a;
						int pixel;
						r = *p++;
						g = *p++;
						b = *p++;
						a = *p++;
						// premul yes
						r = r * (a ? 1 : 0);
						g = g * (a ? 1 : 0);
						b = b * (a ? 1 : 0);
						if (x+used_x == 752 && y+used_y == 1104) {
							printf("b=%d\n", b);
						}
						/*
						unsigned char mr, mg, mb;
						mr = r % 8; // 8 steps in 32 bit for each 1 step in 15 bit
						mg = g % 8;
						mb = b % 8;
						*/
						r = (r >> 3);
						g = (g >> 3);
						b = (b >> 3);
						/*
						// favor brightness
						if (mr >= 4 && r < 31)
							r++;
						if (mg >= 4 && g < 31)
							g++;
						if (mb >= 4 && b < 31)
							b++;
						*/
						a = a ? 1 : 0;
						pixel = (r << 11) | (g << 6) |
							(b << 1) | a;
						fputc(pixel & 0xff, f);
						fputc((pixel >> 8) & 0xff, f);
					}
					count++;
				}
				p = line_start + lr->pitch;
				line_start = p;
			}
			printf("count = %d (pixels)\n", count);
			fclose(f);
			al_unlock_bitmap(full_bmp);
			used_x += this_w;
		}
		used_y += this_h;
	}

	return 0;
}
