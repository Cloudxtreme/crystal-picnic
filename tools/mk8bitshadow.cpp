#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <zlib.h>
#include <cstdio>

void writeInt(gzFile f, int i) {
	gzputc(f, (i >> 0) & 0xff);
	gzputc(f, (i >> 8) & 0xff);
	gzputc(f, (i >> 16) & 0xff);
	gzputc(f, (i >> 24) & 0xff);
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("%s <infile.png> <r> <g> <b> <outfile.alpha>\n", argv[0]);
		return 0;
	}

	al_init();
	al_init_image_addon();

	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA);
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);

	ALLEGRO_BITMAP *b = al_load_bitmap(argv[1]);
	int w = al_get_bitmap_width(b);
	int h = al_get_bitmap_height(b);

	gzFile f = gzopen(argv[5], "wb");
	gzputc(f, (unsigned char)atoi(argv[2]));
	gzputc(f, (unsigned char)atoi(argv[3]));
	gzputc(f, (unsigned char)atoi(argv[4]));
	writeInt(f, w);
	writeInt(f, h);

	al_lock_bitmap(b, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			ALLEGRO_COLOR c = al_get_pixel(b, j, i);
			unsigned char r, g, b, a;
			al_unmap_rgba(c, &r, &g, &b, &a);
			gzputc(f, a);
		}
	}
	
	gzclose(f);

	al_unlock_bitmap(b);

	return 0;
}

