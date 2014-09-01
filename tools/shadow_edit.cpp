#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc < 5) {
		printf("%s <width> <height> <shadow file> <output.png>\n", argv[0]);
		return 0;
	}

	int w = atoi(argv[1]);
	int h = atoi(argv[2]);
	const char *filename = argv[3];

	if (w % 8 != 0) {
		w += 8 - (w % 8);
	}

	al_init();
	al_install_keyboard();
	al_install_mouse();
	al_init_image_addon();
	al_init_primitives_addon();

	ALLEGRO_DISPLAY *display = al_create_display(800, 600);

	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	ALLEGRO_BITMAP *bmp = al_create_bitmap(w, h);
	al_set_target_bitmap(bmp);

	FILE *f = fopen(filename, "rb");
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x += 8) {
			int c = fgetc(f);
			for (int bit = 0; bit < 8; bit++) {
				int this_bit = (c >> (7-bit)) & 1;
				if (this_bit) {
					al_put_pixel(x+bit, y, al_map_rgb(0, 0, 0));
				}
				else {
					al_put_pixel(x+bit, y, al_map_rgb(255, 255, 255));
				}
			}
		}
	}

	int ox = 0;
	int oy = 0;

	ALLEGRO_COLOR paintColor = al_map_rgb(255, 255, 255);
	bool paintWhite = true;

	while (true) {
		ALLEGRO_KEYBOARD_STATE kstate;
		ALLEGRO_MOUSE_STATE mstate;
		al_get_keyboard_state(&kstate);
		al_get_mouse_state(&mstate);

		al_set_target_bitmap(al_get_backbuffer(display));
		al_clear_to_color(al_map_rgb(255, 0, 255));
		al_draw_bitmap(bmp, ox, oy, 0);
		al_draw_filled_circle(mstate.x, mstate.y, 20, al_map_rgba(0, 0, 255, 100));
		ALLEGRO_COLOR c = paintColor;
		c.a = 0.4;
		al_draw_filled_circle(mstate.x, mstate.y, 10, c);
		al_flip_display();

		if (al_key_down(&kstate, ALLEGRO_KEY_ESCAPE))
			break;
		else if (al_key_down(&kstate, ALLEGRO_KEY_LEFT)) {
			ox += 64;
		}
		else if (al_key_down(&kstate, ALLEGRO_KEY_RIGHT)) {
			ox -= 64;
		}
		else if (al_key_down(&kstate, ALLEGRO_KEY_UP)) {
			oy += 64;
		}
		else if (al_key_down(&kstate, ALLEGRO_KEY_DOWN)) {
			oy -= 64;
		}
		else if (al_key_down(&kstate, ALLEGRO_KEY_SPACE)) {
			paintWhite = !paintWhite;
			if (paintWhite) {
				paintColor = al_map_rgb(255, 255, 255);
			}
			else {
				paintColor = al_map_rgb(0, 0, 0);
			}
			// sad...
			al_rest(0.2);
		}
		if (mstate.buttons) {
			al_set_target_bitmap(bmp);
			al_draw_filled_circle(mstate.x-ox, mstate.y-oy, 20, paintColor);
		}
		al_rest(0.01);
	}

	al_save_bitmap(argv[4], bmp);

	return 0;
}

