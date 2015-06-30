#ifndef ATLAS_INTERNAL_H
#define ATLAS_INTERNAL_H

#include <vector>
#include <map>

#include <wrap.h>

#include "atlas.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ATLAS_ITEM {
	Wrap::Bitmap *sub;
	int sheet;
	int x;
	int y;
	int id;
};

typedef struct {
	int x, y, w, h;
	int sheet;
} ARECT;

typedef struct {
	Wrap::Bitmap *bitmap;
	int id;
} ABMP;

struct ATLAS {
	bool finished;
	int width;
	int height;
	int flags;
	int border;
	bool destroy_bmps;
	bool preserve_bitmap;
	int num_sheets;
	Wrap::Bitmap **sheets;
	int num_items;
	ATLAS_ITEM **items;
	std::map<int, ATLAS_ITEM *> item_map;
	std::vector <ABMP *> bmp_list;
};

#ifdef __cplusplus
}
#endif

#endif
