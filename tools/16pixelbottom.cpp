#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include <cstdio>

int find_last_line_with_pixel(ALLEGRO_BITMAP *b)
{
	al_lock_bitmap(b, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);

	int last = al_get_bitmap_height(b)-1;

	for (int i = 0; i < al_get_bitmap_height(b); i++) {
		int y = last-i;
		bool found = false;

		for (int x = 0; x < al_get_bitmap_width(b); x++) {
			ALLEGRO_COLOR c = al_get_pixel(b, x, y);
			if (c.a != 0.0f) {
				found = true;
				break;
			}
		}

		if (found)
			break;

		last--;
	}

	al_unlock_bitmap(b);

	return last;
}

int main(int argc, char **argv)
{
	if (argc != 3) {
		printf("Usage: 16pixelbottom <input.png> <output.png>\n");
		exit(0);
	}

	al_init();
	al_init_image_addon();

	ALLEGRO_BITMAP *b = al_load_bitmap(argv[1]);
	if (!b) exit(1);

	int last_line_with_pixel = find_last_line_with_pixel(b);
	int move = -(16 - al_get_bitmap_height(b) + last_line_with_pixel);

	ALLEGRO_BITMAP *b2 = al_create_bitmap(al_get_bitmap_width(b), al_get_bitmap_height(b));
	al_set_target_bitmap(b2);
	al_clear_to_color(al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f));

	al_draw_bitmap(b, 0, move, 0);

	al_save_bitmap(argv[2], b2);
}

