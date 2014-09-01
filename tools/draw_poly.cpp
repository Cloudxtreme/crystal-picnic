#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <vector>
#include <cmath>
#include <cstdio>

extern float g_poly_debug_scale;

int main(int argc, char **argv)
{
	al_init();
	al_init_primitives_addon();
	ALLEGRO_DISPLAY *display = al_create_display(1000, 800);
	ALLEGRO_EVENT_QUEUE *q = al_create_event_queue();
	al_install_keyboard();

	al_register_event_source(q, al_get_keyboard_event_source());

	FILE *f = fopen(argv[1], "r");
	char line[200];
	int count = 0;
	
	fgets(line, 199, f); // skip outline
	fgets(line, 199, f); // skip layer

	float verts[10000];
	int holes[1000];
	int nverts = 0;
	int nholes = 0;

	while (fgets(line, 199, f)) {
		int x, y;
		if (sscanf(line, "\t%d, %d,\n", &x, &y) == 2) {
			if (x == -1 && y == -1) {
				holes[nholes++] = nverts/2;
			}
			else {
				verts[nverts++] = x;
				verts[nverts++] = y;
			}
		}
	}
	fclose(f);

	float scale = 1.0;
	float translate_x = 0;
	float translate_y = 0;

	while (1) {
		al_clear_to_color(al_map_rgb(0, 0, 0));
		//al_draw_polygon_with_holes(verts, nverts/2, holes, nholes, ALLEGRO_LINE_JOIN_MITER, al_map_rgb(200, 200, 200), 5, 25);
		//al_draw_filled_polygon_with_holes(verts, nverts/2, holes, nholes, al_map_rgb(200, 200, 200));
		al_draw_filled_polygon_with_holes(verts, nverts/2, holes, nholes, al_map_rgb(100, 100, 100));
		al_flip_display();

		ALLEGRO_EVENT event;
		al_wait_for_event(q, &event);
		if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
			if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				goto done;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_EQUALS) {
				g_poly_debug_scale += 0.1;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_MINUS) {
				g_poly_debug_scale -= 0.1;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_UP) {
				translate_y += 50;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
				translate_y -= 50;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
				translate_x += 50;
			}
			else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
				translate_x -= 50;
			}
		}

		ALLEGRO_TRANSFORM transform;
		al_identity_transform(&transform);
		al_scale_transform(&transform, g_poly_debug_scale, g_poly_debug_scale);
		al_translate_transform(&transform, translate_x, translate_y);
		al_use_transform(&transform);
	}
done:

	return 0;
}

