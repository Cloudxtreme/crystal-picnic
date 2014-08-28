#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

int main(int argc, char **argv)
{
	al_init();
	al_init_image_addon();

	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);

	ALLEGRO_BITMAP *b = al_load_bitmap(argv[1]);
	ALLEGRO_BITMAP *b2 = al_create_bitmap(
		al_get_bitmap_width(b),
		al_get_bitmap_height(b)
	);

	al_set_target_bitmap(b2);
	al_clear_to_color(al_map_rgba(255, 0, 0, 128));

	al_save_bitmap(argv[2], b2);
}

