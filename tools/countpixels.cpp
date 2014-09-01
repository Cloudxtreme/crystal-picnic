// count non 0 alpha pixels

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <zlib.h>
#include <string>

static long read32(gzFile f)
{
	return gzgetc(f) | (gzgetc(f) << 8) | (gzgetc(f) << 16) | (gzgetc(f) << 24);
}

ALLEGRO_BITMAP *load(const char *filename)
{
	gzFile f = gzopen(filename, "rb");
	if (f == NULL ) {
		return NULL;
	}

	int r = gzgetc(f);
	int g = gzgetc(f);
	int b = gzgetc(f);
	int pixel = r | (g << 8) | (b << 16);
	int w = read32(f);
	int h = read32(f);

	unsigned char *buf = new unsigned char[w*h];

	gzread(f, buf, w*h);

	ALLEGRO_BITMAP *bmp = al_create_bitmap(w, h);

	ALLEGRO_LOCKED_REGION *lr = al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_WRITEONLY);

	int i = 0;

	for (int y = 0; y < h; y++) {
		uint32_t *p = (uint32_t *)((unsigned char *)lr->data + lr->pitch*y);
		for (int x = 0; x < w; x++) {
			int a = buf[i++];
			pixel = (pixel & 0xFFFFFF) | (a << 24);
			*p++ = pixel;
		}
	}

	delete[] buf;

	al_unlock_bitmap(bmp);

	return bmp;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: %s <image.png>\n", argv[0]);
		return 0;
	}

	al_init();
	al_init_image_addon();
	al_register_bitmap_loader(".alpha", load);
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE);

	ALLEGRO_BITMAP *b = al_load_bitmap(argv[1]);
	int w = al_get_bitmap_width(b);
	int h = al_get_bitmap_height(b);

	int count = 0;

	ALLEGRO_LOCKED_REGION *lr = al_lock_bitmap(b, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);

	for (int y = 0; y < h; y++) {
		unsigned char *p = (unsigned char *)lr->data + lr->pitch*y;
		for (int x = 0; x < w; x++) {
			if (p[3] != 0)
				count++;
			p += 4;
		}
	}

	al_unlock_bitmap(b);
	al_destroy_bitmap(b);

	printf("count=%d\n", count);

	return 0;
}

