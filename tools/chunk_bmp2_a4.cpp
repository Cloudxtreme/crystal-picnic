#include <allegro.h>
#include <loadpng.h>
#include <cstring>
#include <zlib.h>

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("Usage: %s <x.png/bmp/tga etc> <size>\n", argv[0]);
		exit(0);
	}

	const char *filename = argv[1];
	int section_size = atoi(argv[2]);

	allegro_init();
	register_png_file_type();

	set_color_depth(8);
	PALETTE pal;

	BITMAP *full_bmp = load_png(filename, pal);

	/*
	gzFile pf = gzopen("palette.gz", "wb");
	for (int i = 0; i < 256; i++) {
		gzPutc
	}

	// FIXMD!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	return 0;
	*/

	int full_w = full_bmp->w;
	int full_h = full_bmp->h;

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
			BITMAP *bmp = create_bitmap(this_w, this_h);
			blit(full_bmp, bmp, used_x, used_y, 0, 0, this_w, this_h);
			used_x += this_w;
			char buf[100];
			sprintf(buf, "%04d.gz", count++);
			gzFile f = gzopen(buf, "wb");
			for (int y = 0; y < bmp->h; y++) {
				for (int x = 0; x < bmp->w; x++) {
					unsigned char c = getpixel(bmp, x, y);
					gzputc(f, c);
				}
			}
			gzclose(f);
			destroy_bitmap(bmp);
		}
		used_y += this_h;
	}
}
END_OF_MAIN()

