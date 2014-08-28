#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <zlib.h>

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("Usage: %s <x.png/bmp/tga etc> <size>\n", argv[0]);
		exit(0);
	}

	const char *filename = argv[1];
	int section_size = atoi(argv[2]);
	int bits = 12;

	al_init();
	al_init_image_addon();

	ALLEGRO_DISPLAY *dummy = al_create_display(128, 128);
	
	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA);
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
			ALLEGRO_BITMAP *bmp = al_create_bitmap(this_w, this_h);
			ALLEGRO_STATE state;
			al_store_state(&state, ALLEGRO_STATE_TARGET_BITMAP|ALLEGRO_STATE_BLENDER);
			al_set_target_bitmap(bmp);
			al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO, al_map_rgb(255, 255, 255));
			al_draw_bitmap_region(full_bmp, used_x, used_y,
				this_w, this_h, 0, 0, 0);
			used_x += this_w;
			al_restore_state(&state);
			al_draw_bitmap(bmp, 0, 0, 0);
			al_flip_display();
			char buf[100];
			sprintf(buf, "%04d.test", count++);
			gzFile f = gzopen(buf, "wb");
			al_lock_bitmap(
				bmp,
				ALLEGRO_PIXEL_FORMAT_ANY,
				ALLEGRO_LOCK_READONLY
			);
			for (int y = 0; y < this_h; y++) {
				for (int x = 0; x < this_w; x += 2) {
					int p1 = 0, p2 = 0;
					ALLEGRO_COLOR c = al_get_pixel(bmp, x, y);
					unsigned char r, g, b, a;
					al_unmap_rgba(c, &r, &g, &b, &a);
					if (a != 255) {
						p1 = 0xF0F;
					}
					else {
						p1 = ((r>>4) << 8) |
							((g>>4) << 4) |
							((b>>4));
					}
					c = al_get_pixel(bmp, x+1, y);
					al_unmap_rgba(c, &r, &g, &b, &a);
					if (a != 255) {
						p2 = 0xF0F;
					}
					else {
						p2 = ((r>>4) << 8) |
							((g>>4) << 4) |
							((b>>4));
					}
					gzputc(f, p1 >> 4);
					gzputc(f, ((p1 & 0xF) << 4) | (p2 >> 8));
					gzputc(f, p2 & 0xff);
				}
			}
			gzclose(f);
			al_unlock_bitmap(bmp);
			al_destroy_bitmap(bmp);
		}
		used_y += this_h;
	}
}
