#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <cstdio>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

int main(int argc, char **argv)
{
	if (argc < 5) {
		printf("splitbitmap <input.png> <prefix> <width> <height>\n");
		return 0;
	}

	al_init();
	al_init_image_addon();

	const char *input_filename = argv[1];
	const char *prefix = argv[2];
	int split_w = atoi(argv[3]);
	int split_h = atoi(argv[4]);

	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP|ALLEGRO_NO_PREMULTIPLIED_ALPHA);

	ALLEGRO_BITMAP *input = al_load_bitmap(input_filename);
	int input_w = al_get_bitmap_width(input);
	int input_h = al_get_bitmap_height(input);

	int x_chunks = (input_w / split_w) + ((input_w % split_w) ? 1 : 0);
	int y_chunks = (input_h / split_h) + ((input_h % split_h) ? 1 : 0);
	int y_used = 0;

	for (int y = 0; y < y_chunks; y++) {
		int x_used = 0;
		int this_h = MIN(input_h-y_used, split_h);
		for (int x = 0; x < x_chunks; x++) {
			int this_w = MIN(input_w-x_used, split_w);
			ALLEGRO_BITMAP *b = al_create_bitmap(this_w, this_h);
			al_set_target_bitmap(b);
			al_clear_to_color(al_map_rgba(0, 0, 0, 0));
			al_draw_bitmap_region(
				input,
				x*split_w, y*split_h,
				this_w, this_h,
				0, 0, 0
			);
			char filename[1000];
			sprintf(filename, "%s_%d-%d.png", prefix, x, y);
			al_save_bitmap(filename, b);
			al_destroy_bitmap(b);
			x_used += this_w;
		}
		y_used += this_h;
	}

	return 0;
}

