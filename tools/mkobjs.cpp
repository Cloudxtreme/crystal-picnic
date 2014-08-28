#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include <vector>
#include <utility>

struct Crap {
	int x, y;
	bool dirs[9];
};
	
ALLEGRO_BITMAP *mark;

bool extract(ALLEGRO_BITMAP *bmp, int x, int y,
	ALLEGRO_BITMAP **dest, int *dx, int *dy)
{
	int minx = INT_MAX;
	int miny = INT_MAX;
	int maxx = INT_MIN;
	int maxy = INT_MIN;

	int bw = al_get_bitmap_width(bmp);
	int bh = al_get_bitmap_height(bmp);

	std::vector< Crap >  stack;
	
	Crap c;
	c.x = x;
	c.y = y;
	memset(c.dirs, 0, sizeof(c.dirs));
	stack.push_back(c);

	const int offsets[9][2] = {
		{ -1, -1 }, {  0, -1 }, {  1, -1 },
		{ -1,  0 }, {  0,  0 }, {  1,  0 },
		{ -1,  1},  {  0,  1 }, {  1,  1 }
	};

	ALLEGRO_BITMAP *tmp = al_create_bitmap(bw, bh);
	al_set_target_bitmap(tmp);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));

	while (stack.size() > 0) {
		Crap &c = stack[stack.size()-1];
		int d;
		for (d = 0; d < 9; d++) {
			if (c.dirs[d] == false) {
				c.dirs[d] = true;
				break;
			}
		}
		if (d >= 9) {
			stack.pop_back();
			continue;
		}
		int px = c.x + offsets[d][0];
		int py = c.y + offsets[d][1];
		if (px < 0 || py < 0 || px >= bw || py >= bh) {
			continue;
		}
		ALLEGRO_COLOR p = al_get_pixel(bmp, px, py);
		unsigned char r, g, b, a;
		al_unmap_rgba(p, &r, &g, &b, &a);
		ALLEGRO_COLOR p2 = al_get_pixel(mark, px, py);
		unsigned char r2, g2, b2, a2;
		al_unmap_rgba(p2, &r2, &g2, &b2, &a2);
		if (a != 0 && a2 == 0) {
			al_set_target_bitmap(mark);
			al_put_pixel(px, py, p);
			al_set_target_bitmap(tmp);
			al_put_pixel(px, py, p);
			if (px < minx)
				minx = px;
			if (py < miny)
				miny = py;
			if (px > maxx)
				maxx = px;
			if (py > maxy)
				maxy = py;
			Crap c;
			c.x = px;
			c.y = py;
			memset(c.dirs, 0, sizeof(c.dirs));
			c.dirs[8-d] = true;
			stack.insert(stack.begin(), c);
		}
		static int i;
		i++;
	}

	int w = maxx - minx + 1;
	int h = maxy - miny + 1;

	ALLEGRO_BITMAP *result;
	result = al_create_bitmap(w, h);
	al_set_target_bitmap(result);
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
	al_draw_bitmap_region(tmp, minx, miny, w, h, 0, 0, 0);

	*dx = minx;
	*dy = maxy;
	*dest = result;

	al_destroy_bitmap(tmp);
}

bool find(ALLEGRO_BITMAP *bmp, int *x, int *y,
	ALLEGRO_BITMAP **dest, int *dx, int *dy)
{
	int bw = al_get_bitmap_width(bmp);
	int bh = al_get_bitmap_height(bmp);

	for (; *y < bh; (*y)++) {
		for (; *x < bw; (*x)++) {
			ALLEGRO_COLOR pixel = al_get_pixel(bmp, *x, *y);
			unsigned char r, g, b, a;
			al_unmap_rgba(pixel, &r, &g, &b, &a);
			ALLEGRO_COLOR pixel2 = al_get_pixel(mark, *x, *y);
			unsigned char r2, g2, b2, a2;
			al_unmap_rgba(pixel2, &r2, &g2, &b2, &a2);
			if (a != 0 && a2 == 0) {
				extract(bmp, *x, *y, dest, dx, dy);
				return true;
			}
		}
		*x = 0;  
	}

	return false;
}

void mkobjs(const char *prefix, ALLEGRO_BITMAP *bmp)
{
	int x = 0;
	int y = 0;

	ALLEGRO_BITMAP *result;
	int objx, objy;

	while (find(bmp, &x, &y, &result, &objx, &objy)) {
		char name[1000];
		int bw = al_get_bitmap_width(result);
		int bh = al_get_bitmap_height(result);
		sprintf(name, "%s_%d-%d_%dx%d.png", prefix, objx, objy, bw, bh);
		al_save_bitmap(name, result);
		al_destroy_bitmap(result);
	}
}

int main(int argc, char **argv)
{
	al_init();
	al_init_image_addon();

	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ARGB_8888);
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	
	if (argc < 3) {
		printf("Usage: mkobjs <prefix> <bitmap...>\n");
		return 0;
	}
	
	for (int i = 2; i < argc; i++) {
		ALLEGRO_BITMAP *bmp = al_load_bitmap(argv[i]);
		int bw = al_get_bitmap_width(bmp);
		int bh = al_get_bitmap_height(bmp);
		mark = al_create_bitmap(bw, bh);
		al_set_target_bitmap(mark);
		al_clear_to_color(al_map_rgba(0, 0, 0, 0));

		mkobjs(argv[1], bmp);
		al_destroy_bitmap(bmp);
		al_destroy_bitmap(mark);
	}

	return 0;
}

