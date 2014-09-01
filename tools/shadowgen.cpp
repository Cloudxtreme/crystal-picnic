#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <cstdlib>

int main(int argc, char **argv)
{
	al_init();
	al_init_image_addon();

	ALLEGRO_DISPLAY *display = al_create_display(100, 100);

	int w = atoi(argv[1]);
	int h = atoi(argv[2]);

	ALLEGRO_BITMAP *shadow = al_create_bitmap(w, h);

	al_set_target_bitmap(shadow);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));

	const float count = 5.0;

	for (int i = 0; i < count; i++) {
		al_draw_filled_ellipse(w/2, h/2, (float)(1.0-(i/count))*w/2.0,
			(float)(1.0-(i/count))*h/2.0, al_map_rgba(0, 0, 0, 120));
	}

	al_save_bitmap("out.png", shadow);
}

