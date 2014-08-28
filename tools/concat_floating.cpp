#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <vector>

int main(int argc, char **argv)
{
	const char *prefix = argv[1];

	al_init();
	al_init_image_addon();

	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA);
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);

	std::vector< std::vector<ALLEGRO_BITMAP *> > bmps;
	int w = 0, h = 0, count_h = 0, count_v = 0;

	for (int y = 0; y < 100; y++) {
		bmps.push_back(std::vector<ALLEGRO_BITMAP *>());
		for (int x = 0; x < 100; x++) {
			char path[1000];
			sprintf(path, "%s%d-%d.png", prefix, x, y);
			ALLEGRO_BITMAP *b = al_load_bitmap(path);
			if (!b)
				break;
			if (y == 0) {
				count_h++;
				w += al_get_bitmap_width(b);
			}
			if (x == 0) {
				h += al_get_bitmap_height(b);
			}
			bmps[y].push_back(b);
		}
		if (bmps[y].size() <= 0)
			break;
		count_v++;
	}

	printf("count_h=%d count_v=%d\n", count_h, count_v);

	ALLEGRO_BITMAP *result = al_create_bitmap(w, h);
	al_set_target_bitmap(result);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);

	int dy = 0;

	for (int y = 0; y < count_v; y++) {
		int dx = 0;
		for (int x = 0; x < count_h; x++) {
			ALLEGRO_BITMAP *b = bmps[y][x];
			al_draw_bitmap(b, dx, dy, 0);
			dx += al_get_bitmap_width(b);
		}
		dy += al_get_bitmap_height(bmps[y][0]);
	}
	al_save_bitmap("out.png", result);
}
