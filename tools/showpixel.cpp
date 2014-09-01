#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include <vector>

struct Point {
	int x, y;
};

std::vector<Point> points;

int main(int argc, char **argv)
{
	al_init();
	al_init_image_addon();
	al_install_mouse();
	al_install_keyboard();
	al_init_font_addon();
	al_init_ttf_addon();

	al_create_display(800, 600);

	ALLEGRO_BITMAP *bmp = al_load_bitmap(argv[1]);
	int w = al_get_bitmap_width(bmp);
	int h = al_get_bitmap_height(bmp);
	ALLEGRO_FONT *font = al_load_ttf_font("DejaVuSans.ttf", 16, 0);

	while (1) {
		ALLEGRO_KEYBOARD_STATE st;
		al_get_keyboard_state(&st);
		if (al_key_down(&st, ALLEGRO_KEY_ESCAPE)) {
			break;
		}
		al_clear_to_color(al_map_rgb(255, 0, 255));
		al_draw_bitmap(bmp, 0, 0, 0);
		ALLEGRO_MOUSE_STATE s;
		al_get_mouse_state(&s);
		bool rest = false;
		int x = s.x - w/2;
		int y = s.y - h;
		if (s.buttons) {
			printf("%d, %d,\n", x, y);
			Point p;
			p.x = s.x;
			p.y = s.y;
			points.push_back(p);
			rest = true;
		}
		al_draw_textf(font, al_map_rgb(255, 255, 0), 0, 600-16, 0, "%d, %d", x, y);
		for (size_t i = 0; i < points.size(); i++) {
			for (int j = -1; j <= 1; j++) {
				for (int k = -1; k <= 1; k++) {
					Point p = points[i];
					al_draw_pixel(p.x+k, p.y+j, al_map_rgb(0, 255, 255));
				}
			}
		}
		al_flip_display();
		if (rest) {
			al_rest(0.75);
		}
		else {
			al_rest(0.02);
		}
	}
}

