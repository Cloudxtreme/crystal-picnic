#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <cstdio>

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("Usage: %s <input.png> <output>\n", argv[0]);
		return 0;
	}

	const char *in_name = argv[1];
	const char *out_name = argv[2];

	al_init();
	al_init_image_addon();

//	ALLEGRO_DISPLAY *display = al_create_display(128, 128);

	al_set_new_display_flags(ALLEGRO_MEMORY_BITMAP);
	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA);
	ALLEGRO_BITMAP *bmp = al_load_bitmap(in_name);
	al_save_bitmap("test.png", bmp);
	al_lock_bitmap(bmp, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);

	int w = al_get_bitmap_width(bmp);
	int h = al_get_bitmap_height(bmp);

	int bytes_w = w / 8 + (w % 8 == 0 ? 0 : 1);

	unsigned char *bytes = new unsigned char[bytes_w*h];
	memset(bytes, 0, bytes_w*h);

	ALLEGRO_BITMAP *proof = al_create_bitmap(w, h);
	al_set_target_bitmap(proof);
	al_clear_to_color(al_map_rgb(255, 255, 255));
	al_lock_bitmap(proof, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READWRITE);

	int byte;
	int bit;

	for (int y = 0; y < h; y++) {
		byte = y * bytes_w;
		bit = 7;
		for (int x = 0; x < w; x++) {
			ALLEGRO_COLOR pix = al_get_pixel(bmp, x, y);
			unsigned char r, g, b, a;
			al_unmap_rgba(pix, &r, &g, &b, &a);
			bool curr_bit;
			if (r == 255 && g == 255 && b == 255 && a == 255) {
				curr_bit = 0;
			}
			else {
				curr_bit = 1;
			}
			bytes[byte] |= (curr_bit << bit);
			bit--;
			if (bit < 0) {
				for (int i = 0; i < 8; i++) {
					if (bytes[byte] & (1 << (7 - i)))
						al_put_pixel(x-8+i, y, al_map_rgb(0, 0, 0));
				}
				bit = 7;
				byte++;
			}
		}
	}

	al_unlock_bitmap(bmp);
	al_unlock_bitmap(proof);

	al_save_bitmap("proof.png", proof);

	FILE *f = fopen(out_name, "wb");
	fwrite(bytes, bytes_w*h, 1, f);
	fclose(f);

	return 0;
}

