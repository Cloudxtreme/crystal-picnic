#include <allegro5/allegro.h>

#include "engine.h"

/* This has to load bitmaps from CPA and disk (for skeled) */
ALLEGRO_BITMAP *my_load_bitmap(std::string filename)
{
	CPA *cpa;
	ALLEGRO_FILE *f;
	if (engine) {
		cpa = engine->get_cpa();
	}
	else {
		cpa = NULL;
	}
	if (cpa) {
		f = cpa->load(filename);
	}
	else {
		f = al_fopen(filename.c_str(), "rb");
	}
	if (!f) {
		return NULL;
	}
	ALLEGRO_BITMAP *bmp;
//#if defined ANDROID || defined ALLEGRO_IPHONE || defined ALLEGRO_RASPBERRYPI
	bmp = al_load_bitmap_f(f, ".png");
/*#else
	if (!engine || !cpa) {
		bmp = al_load_bitmap_f(f, ".png");
	}
	else {
		bmp = al_load_bitmap_f(f, ".cpi");
	}
#endif*/
	al_fclose(f);
	return bmp;
}

