#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

bool display(int n, int x, int y)
{
	char buf[10];
	sprintf(buf, "%04d", n);
	FILE *f = fopen(buf, "rb");
	if (!f)
		return false;
	if (!(x*128+128 >= 800 || y*128+128 >= 600)) {
	al_lock_bitmap_region(al_get_target_bitmap(), x*128, y*128, 128, 128, 	ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
	for (int j = 0; j < 128; j++)
		for (int i = 0 ; i < 128; i++) {
			int pix = fgetc(f) | (fgetc(f) << 8);
			int r = (pix & 0xF800) >> 8;
			int g = (pix & 0x07C0) >> 3;
			int b = (pix & 0x003E) << 2;
			al_put_pixel(x*128+i, y*128+j, al_map_rgb(r, g, b));
		}
	al_unlock_bitmap(al_get_target_bitmap());
	}
	fclose(f);
	return true;
}

int main(int argc, char **argv)
{
	al_init();
	al_init_image_addon();

	int width = atoi(argv[1]);
	int sx = atoi(argv[2]);
	int sy = atoi(argv[3]);

	ALLEGRO_DISPLAY *d = al_create_display(800, 600);

	int y = sy;

	while (1) {
		for (int i = 0; i < width; i++) {
			if (!display(y*width+sx+i, i, y-sy))
				goto done;
		}
		y++;
	}
done:
	al_flip_display();
	al_rest(5);
}

