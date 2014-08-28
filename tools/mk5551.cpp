// mk5551.cpp

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <string>
#include <zlib.h>
#include <cstdio>
	
void iputl(gzFile f, int i) {
	gzputc(f, (i >> 0) & 0xff);
	gzputc(f, (i >> 8) & 0xff);
	gzputc(f, (i >> 16) & 0xff);
	gzputc(f, (i >> 24) & 0xff);
}

void iputl_alleg(ALLEGRO_FILE *f, int i) {
	al_fputc(f, (i >> 0) & 0xff);
	al_fputc(f, (i >> 8) & 0xff);
	al_fputc(f, (i >> 16) & 0xff);
	al_fputc(f, (i >> 24) & 0xff);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: mk5551 [-u] <input> [<output>]\n");
		return 0;
	}

	bool uncompressed = !strcmp(argv[1], "-u");

    std::string out;
    if ((uncompressed && argc < 4) || (!uncompressed && argc < 3))
    {
        out = argv[uncompressed ? 2 : 1];
        size_t dot = out.find_first_of('.');
        if (dot != std::string::npos)
            out.replace(dot, std::string::npos, uncompressed ? ".5551u" : ".5551");
        else
            out += uncompressed ? ".5551u" : ".5551";
    }
    else if (uncompressed)
        out = argv[3];
    else
        out = argv[2];

	al_init();
	al_init_image_addon();

	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_RGBA_5551);
	
	// FIXME: toggle on and off as needed
	al_set_new_bitmap_flags(ALLEGRO_NO_PREMULTIPLIED_ALPHA);

	ALLEGRO_BITMAP *input = al_load_bitmap(argv[uncompressed ? 2 : 1]);

	if (uncompressed) {
		ALLEGRO_FILE *f = al_fopen(out.c_str(), "wb");

		ALLEGRO_LOCKED_REGION *lr =
			al_lock_bitmap(
				input,
				ALLEGRO_PIXEL_FORMAT_RGBA_5551,
				ALLEGRO_LOCK_READONLY
			);

		int width = al_get_bitmap_width(input);
		int height = al_get_bitmap_height(input);

		iputl_alleg(f, width);
		iputl_alleg(f, height);
		
		for (int y = 0; y < height; y++) {
			uint16_t *pixptr = (uint16_t *)((unsigned char *)lr->data + (y * lr->pitch));
			for (int x = 0; x < width; x++) {
				uint16_t pixel = *pixptr++;
				if (pixel & 1) {
					al_fputc(f, pixel & 0xff);
					al_fputc(f, pixel >> 8);
				}
				else {
					al_fputc(f, 0);
					al_fputc(f, 0);
				}
			}
		}

		al_fclose(f);
	}
	else {
		gzFile f = gzopen(out.c_str(), "wb");

		ALLEGRO_LOCKED_REGION *lr =
			al_lock_bitmap(
				input,
				ALLEGRO_PIXEL_FORMAT_RGBA_5551,
				ALLEGRO_LOCK_READONLY
			);

		int width = al_get_bitmap_width(input);
		int height = al_get_bitmap_height(input);

		iputl(f, width);
		iputl(f, height);
		
		for (int y = 0; y < height; y++) {
			uint16_t *pixptr = (uint16_t *)((unsigned char *)lr->data + (y * lr->pitch));
			for (int x = 0; x < width; x++) {
				uint16_t pixel = *pixptr++;
				if (pixel & 1) {
					gzputc(f, pixel & 0xff);
					gzputc(f, pixel >> 8);
				}
				else {
					gzputc(f, 0);
					gzputc(f, 0);
				}
			}
		}

		gzclose(f);
	}

	return 0;
}

