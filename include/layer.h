#ifndef LAYER_H
#define LAYER_H

#include <allegro5/allegro.h>

#include "general.h"

struct Painted_Layer {
	struct Tile {
		int number;
		int layer;
		General::Point<float> area_pos;
		General::Point<float> tex_pos;
		bool used;
	};
	Tile **tiles;
	Wrap::Bitmap *texture;
};

struct Tiled_Layer {
	struct Tile {
		int number;
		char sheet;
		bool solid;
	};
	// Using a vector might open the possibility for some neat
	// area resizing effects
	std::vector< std::vector< Tile > > tiles;
};

#endif // LAYER_H
