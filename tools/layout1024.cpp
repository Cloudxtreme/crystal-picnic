#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <cstdio>

int main(int argc, char **argv)
{
	if (argc < 5) {
		printf("Usage: layout1024 width height in.png out.png\n");
		return 0;
	}

	al_init();
	al_init_image_addon();

	int w = atoi(argv[1]);
	int h = atoi(argv[2]);
	const char *fn1 = argv[3];
	const char *fn2 = argv[4];

	ALLEGRO_BITMAP *inbmp = al_load_bitmap(fn1);
	ALLEGRO_BITMAP *outbmp = al_create_bitmap(1024, 1024);
	
	al_set_target_bitmap(outbmp);
	al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));

	int tx = 0;
	int ty = 0;

	for (int y = 0; y < al_get_bitmap_height(inbmp); y += h) {
		for (int x = 0; x < al_get_bitmap_width(inbmp); x += w) {
			al_draw_bitmap_region(
				inbmp,
				x, y,
				w, h,
				tx, ty,
				0
			);
			tx += w;
			if (tx+w >= 1024) {
				tx = 0;
				ty += h;
			}
		}
	}

	al_save_bitmap(fn2, outbmp);

	return 0;
}

