#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

int main(int argc, char **argv)
{
	al_init();
	al_init_image_addon();

	if (argc < 6) {
		printf("Usage: divide_bmp <image.png> <nx> <ny> <new_name_prefix> <filetype>\n");
		return 0;
	}

	const char *filename = argv[1];
	int nx = atoi(argv[2]);
	int ny = atoi(argv[3]);
	const char *prefix = argv[4];
	const char *filetype = argv[5];

	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	ALLEGRO_BITMAP *bmp = al_load_bitmap(filename);

	int w = al_get_bitmap_width(bmp);
	int h = al_get_bitmap_height(bmp);

	for (int y = 0; y < ny; y++) {
		for (int x = 0; x < nx; x++) {
			int ww = w / nx;
			int hh = h / ny;
			ALLEGRO_BITMAP *tmp = al_create_bitmap(ww, hh);
			al_set_target_bitmap(tmp);
			al_draw_bitmap_region(
				bmp,
				x*ww, y*hh,
				ww, hh,
				0, 0,
				0
			);
			char buf[500];
			sprintf(buf, "%s_%d-%d.%s", prefix, x, y, filetype);
			al_save_bitmap(buf, tmp);
			al_destroy_bitmap(tmp);
		}
	}
}

