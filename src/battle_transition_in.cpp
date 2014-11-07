#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <map>
#include <algorithm>
#include <vector>

#include "general.h"
#include "engine.h"
#include "config.h"
#include "battle_transition_in.h"

struct BMP {
	float scale;
	int x, y;
};
	
static int BLOCK_SIZE;

static int w, h;
bool **used;
static ALLEGRO_BITMAP *bmp, *bmp2;
static float min_scale, total;

static void find(int *outx, int *outy)
{
	int x = General::rand() % (w / BLOCK_SIZE);
	int y = General::rand() % (h / BLOCK_SIZE);
	
	while (used[y][x]) {
		x++;
		if (x == (w / BLOCK_SIZE)) {
			x = 0;
			y++;
		}
		if (y == (h / BLOCK_SIZE)) {
			y = 0;
		}
	}
	
	used[y][x] = true;
	
	*outx = x;
	*outy = y;
}

static void draw(std::vector<BMP> &v)
{
	for (size_t i = 0; i < v.size(); i++) {
		BMP &b = v[i];
		float scale = b.scale - total;
		if (scale <= min_scale) {
			int sz = BLOCK_SIZE;
			al_draw_scaled_bitmap(
				bmp2,
				b.x*BLOCK_SIZE, b.y*BLOCK_SIZE,
				BLOCK_SIZE, BLOCK_SIZE,
				b.x*BLOCK_SIZE-(sz-BLOCK_SIZE)/2,
				b.y*BLOCK_SIZE-(sz-BLOCK_SIZE)/2,
				sz, sz,
				0
			);
		}
	}
}

void battle_transition_in(Wrap::Bitmap *start_bmp, Wrap::Bitmap *end_bmp)
{
	BLOCK_SIZE = cfg.screen_w / 100;
	
	w = cfg.screen_w;
	h = cfg.screen_h;

	Wrap::Bitmap *buf = Wrap::create_bitmap_no_preserve(cfg.screen_w, cfg.screen_h);

	bmp = start_bmp->bitmap;
	bmp2 = end_bmp->bitmap;

	used = new bool *[h / BLOCK_SIZE];
	for (int i = 0; i < h / BLOCK_SIZE; i++) {
		used[i] = new bool[w / BLOCK_SIZE];
		memset(used[i], 0, sizeof(bool) * (w / BLOCK_SIZE));
	}
	
	std::vector<BMP> bmps;
	
	int num = (w / BLOCK_SIZE) * (h / BLOCK_SIZE);

	min_scale = 1000;
	float max_scale = 0;
	
	for (int i = 0; i < num; i++) {
		int x, y;
		find(&x, &y);
		BMP b;
		b.x = x;
		b.y = y;
		b.scale = 4 + (General::rand() % 1000) / 250.0f;
		if (b.scale < min_scale) {
			min_scale = b.scale;
		}
		if (b.scale > max_scale) {
			max_scale = b.scale;
		}
		bmps.push_back(b);
	}
	
	float diff = max_scale - min_scale;
	double length = 0.5f;
	total = 0.0f;
	
	double start = al_get_time();

	while (total < diff) {
		al_set_target_bitmap(buf->bitmap);

		al_clear_to_color(al_color_name("black"));

		al_draw_bitmap(bmp, 0, 0, 0);
		al_hold_bitmap_drawing(true);
		draw(bmps);
		al_hold_bitmap_drawing(false);

		engine->draw_to_display(buf->bitmap);
		al_flip_display();

		double now = al_get_time();
		double elapsed = now - start;
		double percent = elapsed / length;
		total = diff * percent;
		al_rest(1.0 / 60.0);
	}

	Wrap::destroy_bitmap(buf);

	for (int i = 0; i < h / BLOCK_SIZE; i++) {
		delete[] used[i];
	}
	delete[] used;
}
