#include <allegro5/allegro.h>
#include <allegro5/allegro_memfile.h>

#include <cfloat>
#include <cctype>

#include "crystalpicnic.h"
#include "area_manager.h"
#include "loop.h"
#include "area_manager.h"
#include "collision_detection.h"
#include "player.h"

#define ORIG_SHEET_SIZE 512
#define SHEET_SIZE 1024

ALLEGRO_DEBUG_CHANNEL("CrystalPicnic")

const int MAX_ISO_CORNER_TILES = 15;
// 69 was carefully calculated to be slightly longer than the length
// of resulting edge on the isometric x axis when moving 1 full tile
// into view from the y axis
const float ISO_TILES_INCREASE = 69.0/32.0;

static General::Point<float> iso_offset;

static bool compare_entities(Map_Entity *a, Map_Entity *b)
{
	if (a->get_layer() < b->get_layer())
		return true;
	if (a->get_layer() > b->get_layer())
		return false;
	
	if (a->get_position().y-a->get_sort_offset() < b->get_position().y-b->get_sort_offset())
		return true;
	
	return false;
}

bool Area_Manager::logic()
{
	for (int layer = 0; layer < num_layers; layer++) {
		int sz = tile_groups[layer].size();
		for (int i = 0; i < sz; i++) {
			Tile_Group *tg = tile_groups[layer][i];
			if (tg->duration > 0) {
				tg->duration -= General::LOGIC_MILLIS / 1000.0;
			}
		}
	}

	Lua::call_lua(lua_state, "logic", ">");

	int sz = particles.size();
	for (int i = 0; i < sz; i++) {
		particles[i]->logic();
	}

	for (int i = 0; i < sz; i++) {
		if (particles[i]->get_delete_me()) {
			delete particles[i];
			ERASE_FROM_UNSORTED_VECTOR(particles, i);
			i--;
			sz--;
		}
	}

	sz = entities.size();
	for (int i = 0; i < sz; i++) {
		entities[i]->logic();
		if (early_break) {
			break;
		}
	}

	for (int i = 0; i < sz; i++) {
		Map_Entity *entity = entities[i];
		if (entity && entity->get_delete_me()) {
			delete entity;
			ERASE_FROM_UNSORTED_VECTOR(entities, i);
			i--;
			sz--;
		}
	}
	
	// Updates saveable stuff
	Lua::call_lua(lua_state, "update_level_state", ">");

	if (early_break) {
		early_break = false;
		return false;
	}

	const float HEIGHT_FOLLOW_SPEED = 0.1;
        if (height_offset_dampened < player_height) {
		height_offset_dampened += HEIGHT_FOLLOW_SPEED;
		if (height_offset_dampened > player_height)
			height_offset_dampened = player_height;
        }
        else if (height_offset_dampened > player_height) {
		height_offset_dampened -= HEIGHT_FOLLOW_SPEED;
		if (height_offset_dampened < player_height)
			height_offset_dampened = player_height;
        }

	// Remove dead entities
	for (int i = 0; i < sz; i++) {
		Entity *entity = entities[i];
		if (entity->get_delete_me()) {
			ERASE_FROM_UNSORTED_VECTOR(entities, i);
			delete entity;
			i--;
			sz--;
		}
	}

	return false;
}

void Area_Manager::draw_layer_tiled(int layer)
{
	Tiled_Layer *l = tiled_layers[layer];
	int tx = top.x / General::TILE_SIZE;
	int ty = top.y / General::TILE_SIZE;
	int wt = width_in_tiles + 1;
	int ht = height_in_tiles + 1;

	for (unsigned int sheet = 0; sheet < tile_sheets.size(); sheet++) {
		al_hold_bitmap_drawing(true);
		int sheet_wt = ORIG_SHEET_SIZE / General::TILE_SIZE;
		for (int y = 0; y < ht; y++) {
			if (y+ty < 0 || y+ty >= height) {
				continue;
			}
			for (int x = 0; x < wt; x++) {
				if (x+tx < 0 || x+tx >= width){
					continue;
				}
				Tiled_Layer::Tile &t = l->tiles[y+ty][x+tx];
				if ((int)sheet == t.sheet && t.number >= 0) {
					int tile_sheet_x = t.number % sheet_wt;
					int tile_sheet_y = t.number / sheet_wt;
					int ox = tile_sheet_x * 2 + 1;
					int oy = tile_sheet_y * 2 + 1;
					float tu = tile_sheet_x * General::TILE_SIZE + ox;
					float tv = tile_sheet_y * General::TILE_SIZE + oy;
					al_draw_bitmap_region(
						tile_sheets[sheet]->bitmap,
						tu, tv,
						General::TILE_SIZE,
						General::TILE_SIZE,
						(x+tx)*General::TILE_SIZE-top.x,
						(y+ty)*General::TILE_SIZE-top.y,
						0
					);
				}
			}
		}
		al_hold_bitmap_drawing(false);
	}
}

void Area_Manager::draw_layer_tiled_ungrouped(int layer)
{
	Tiled_Layer *l = tiled_layers[layer];
	int tx = top.x / General::TILE_SIZE;
	int ty = top.y / General::TILE_SIZE;
	int wt = width_in_tiles + 1;
	int ht = height_in_tiles + 1;

	for (unsigned int sheet = 0; sheet < tile_sheets.size(); sheet++) {
		al_hold_bitmap_drawing(true);
		int sheet_wt = ORIG_SHEET_SIZE / General::TILE_SIZE;
		for (int y = 0; y < ht; y++) {
			if (y+ty < 0 || y+ty >= height) {
				continue;
			}
			for (int x = 0; x < wt; x++) {
				if (x+tx < 0 || x+tx >= width){
					continue;
				}
				if (!tile_is_grouped(layer, x+tx, y+ty)) {
					Tiled_Layer::Tile &t = l->tiles[y+ty][x+tx];
					if ((int)sheet == t.sheet && t.number >= 0) {
						int tile_sheet_x = t.number % sheet_wt;
						int tile_sheet_y = t.number / sheet_wt;
						int ox = tile_sheet_x * 2 + 1;
						int oy = tile_sheet_y * 2 + 1;
						float tu = tile_sheet_x * General::TILE_SIZE + ox;
						float tv = tile_sheet_y * General::TILE_SIZE + oy;
						al_draw_bitmap_region(
							tile_sheets[sheet]->bitmap,
							tu, tv,
							General::TILE_SIZE,
							General::TILE_SIZE,
							(x+tx)*General::TILE_SIZE-top.x,
							(y+ty)*General::TILE_SIZE-top.y,
							0
						);
					}
				}
			}
		}
		al_hold_bitmap_drawing(false);
	}
}

static void my_horizontal_shear_transform(ALLEGRO_TRANSFORM* trans, float theta)
{
   float s;
   s = -tanf(theta);

   trans->m[0][0] += trans->m[0][1] * s;
   trans->m[1][0] += trans->m[1][1] * s;
   trans->m[3][0] += trans->m[3][1] * s;
}

void Area_Manager::draw_layer_tiled_groups(int layer)
{
	draw_layer_tiled_ungrouped(layer);

	/* NOTE: group_drawn is only set on visible groups, ie there's a bunch of extra allocated space
	 * that is never used at the end of the array.
	 */

	if (group_drawn[layer] == NULL) {
		group_drawn[layer] = new bool[tile_groups[layer].size()];
	}
	memset(group_drawn[layer], 0, sizeof(bool) * tile_groups[layer].size());

	std::vector<Tile_Group *> visible_groups;

	int sz = tile_groups[layer].size();
	for (int i = 0; i < sz; i++) {
		Tile_Group *tg = tile_groups[layer][i];
		General::Point<float> top_left(tg->top_left_tile_pixel.x, tg->top_left_tile_pixel.y);
		General::Point<float> bottom_right(tg->bottom_right_tile_pixel.x, tg->bottom_right_tile_pixel.y);
		General::Point<float> bottom_right2(top.x+cfg.screen_w, top.y+cfg.screen_h);
		if (checkcoll_box_box(top_left, bottom_right, top, bottom_right2)) {
			visible_groups.push_back(tg);
		}
	}

	sz = visible_groups.size();
	
	al_hold_bitmap_drawing(true);
	
	int sz2 = entities.size();
	for (int i = 0; i < sz2; i++) {
		Map_Entity *e = entities[i];
		if (!e || e->get_layer() != layer) {
			continue;
		}
		General::Point<float> entity_pos = e->get_position();

		for (int j = 0; j < sz; j++) {
			if (group_drawn[layer][j])
				continue;
			Tile_Group *tg = visible_groups[j];
			if (tg->top_left.y+tg->size.h < entity_pos.y) {
				// draw it
				ALLEGRO_TRANSFORM backup;
				if (tg->duration > 0) {
					al_copy_transform(&backup, al_get_current_transform());
					const float max = D2R(1);
					double ti = fmod(al_get_time(), 0.1);
					float theta;
					if (ti > 0.075) {
						theta = -(0.025 - ((ti-0.025)/0.025)) * max;
					}
					else if (ti > 0.05) {
						theta = -(ti/0.025) * max;
					}
					else if (ti > 0.025) {
						theta = (0.025 - ((ti-0.025)/0.025)) * max;
					}
					else {
						theta = (ti/0.025) * max;
					}
					int miny = INT_MAX;
					int maxy = 0;
					int sz = tg->tiles.size();
					for (int k = 0; k < sz; k++) {
						int y = tg->tiles[k].y;
						if (y < miny) {
							miny = y;
						}
						if (y > maxy) {
							maxy = y;
						}
					}
					theta *= 2;
					int h = maxy - miny;
					h *= General::TILE_SIZE;
					float offs = sin(theta) * h;
					ALLEGRO_TRANSFORM t;
					al_identity_transform(&t);
					float y = miny * General::TILE_SIZE - top.y;
					al_translate_transform(&t, offs, -y);
					my_horizontal_shear_transform(&t, theta);
					al_translate_transform(&t, 0, y);
					al_compose_transform(&t, &backup);
					al_use_transform(&t);
				}
				int sz = tg->tiles.size();
				for (int k = 0; k < sz; k++) {
					int x = tg->tiles[k].x;
					int y = tg->tiles[k].y;
					int curr_sheet = tiled_layers[layer]->tiles[y][x].sheet;
					Tiled_Layer::Tile *t = &tiled_layers[layer]->tiles[y][x];
					int tiles_w = ORIG_SHEET_SIZE / General::TILE_SIZE;
					int tile_x = t->number % tiles_w;
					int tile_y = t->number / tiles_w;
					int ox = tile_x * 2 + 1;
					int oy = tile_y * 2 + 1;
					float tu = tile_x * General::TILE_SIZE + ox;
					float tv = tile_y * General::TILE_SIZE + oy;
					al_draw_bitmap_region(
						tile_sheets[curr_sheet]->bitmap,
						tu, tv, General::TILE_SIZE, General::TILE_SIZE,
						x*General::TILE_SIZE-top.x, y*General::TILE_SIZE-top.y,
						0
					);
				}
				group_drawn[layer][j] = true;
				if (tg->duration > 0) {
					al_use_transform(&backup);
				}
			}
		}
		/* draw entity i */
		General::Point<int> start_draw(-1, -1);
		General::Point<int> end_draw(-1, -1);
		draw_entity(e, true, start_draw, end_draw);
	}

	// draw remaining groups
	for (int j = 0; j < sz; j++) {
		if (group_drawn[layer][j])
			continue;

		// draw it

		Tile_Group *tg = visible_groups[j];

		int sz = tg->tiles.size();
		for (int k = 0; k < sz; k++) {
			int x = tg->tiles[k].x;
			int y = tg->tiles[k].y;

			int curr_sheet = tiled_layers[layer]->tiles[y][x].sheet;
			Tiled_Layer::Tile *t = &tiled_layers[layer]->tiles[y][x];
			int tiles_w = ORIG_SHEET_SIZE / General::TILE_SIZE;
			int tile_x = t->number % tiles_w;
			int tile_y = t->number / tiles_w;
			int ox = tile_x * 2 + 1;
			int oy = tile_y * 2 + 1;
			float tu = tile_x * General::TILE_SIZE + ox;
			float tv = tile_y * General::TILE_SIZE + oy;
			al_draw_bitmap_region(
				tile_sheets[curr_sheet]->bitmap,
				tu, tv, General::TILE_SIZE, General::TILE_SIZE,
				x*General::TILE_SIZE-top.x, y*General::TILE_SIZE-top.y,
				0
			);
		}
		group_drawn[layer][j] = true;
	}
	
	al_hold_bitmap_drawing(false);
}

// -1 next_x will flush after layer is drawn here
/// FIXME
void Area_Manager::draw_iso_tile(int layer, int x, int y, int next_x, int next_y)
{
	Tiled_Layer::Tile t = tiled_layers[layer]->tiles[y][x];

	if (t.number >= 0) {
		float dx =
			iso_offset.x +
			x*(General::TILE_SIZE/2) -
			y*(General::TILE_SIZE/2) -
			(General::TILE_SIZE/2) - 
			top.x;
		float dy =
			iso_offset.y +
			x*(General::TILE_SIZE/4) +
			y*(General::TILE_SIZE/4) -
			(General::TILE_SIZE/2) -
			top.y;

		if (!(
			dx <= -General::TILE_SIZE || dx >= cfg.screen_w ||
			dy <= -General::TILE_SIZE || dy >= cfg.screen_h))
		{
			if (curr_sheet < 0) {
				curr_sheet = t.sheet;
			}

			int wid_tiles = ORIG_SHEET_SIZE / General::TILE_SIZE;
			int tiles_w = t.number % wid_tiles;
			int tiles_h = t.number / wid_tiles;
			int u1 = tiles_w * General::TILE_SIZE + tiles_w * 2 + 1;
			int v1 = tiles_h * General::TILE_SIZE + tiles_h * 2 + 1;
			int u2 = u1 + General::TILE_SIZE;
			int v2 = v1 + General::TILE_SIZE;

			ALLEGRO_VERTEX *v = &verts[iso_vert_count];
			ALLEGRO_COLOR white = al_map_rgb_f(1, 1, 1);

			int oy = tile_bounds[curr_sheet][t.number].top_left.y;
			int oh = tile_bounds[curr_sheet][t.number].size.h;
			dy += oy;
			float x2 = dx + General::TILE_SIZE;
			float y2 = dy + oh;

			v[0].x = dx;
			v[0].y = dy;
			v[0].z = 0;
			v[0].color = white;
			v[0].u = u1;
			v[0].v = v1;
			v[1].x = x2;
			v[1].y = dy;
			v[1].z = 0;
			v[1].color = white;
			v[1].u = u2;
			v[1].v = v1;
			v[2].x = dx;
			v[2].y = y2;
			v[2].z = 0;
			v[2].color = white;
			v[2].u = u1;
			v[2].v = v2;
			v[3].x = x2;
			v[3].y = dy;
			v[3].z = 0;
			v[3].color = white;
			v[3].u = u2;
			v[3].v = v1;
			v[4].x = dx;
			v[4].y = y2;
			v[4].z = 0;
			v[4].color = white;
			v[4].u = u1;
			v[4].v = v2;
			v[5].x = x2;
			v[5].y = y2;
			v[5].z = 0;
			v[5].color = white;
			v[5].u = u2;
			v[5].v = v2;

			iso_vert_count += 6;
		}
	}
	
	bool sheet_changes = false;
	int next_sheet;

	if (next_x < 0) {
		next_sheet = -1;
		sheet_changes = true;
	}
	else {
		next_sheet = tiled_layers[layer]->tiles[next_y][next_x].sheet;
		if (next_sheet >= 0 && next_sheet != curr_sheet) {
			sheet_changes = true;
		}
	}

	if (sheet_changes) {
		if (iso_vert_count > 0) {
			al_draw_prim(verts, 0, tile_sheets[curr_sheet]->bitmap, 0, iso_vert_count, ALLEGRO_PRIM_TRIANGLE_LIST);
			iso_vert_count = 0;
		}
		curr_sheet = next_sheet;
	}
}

bool Area_Manager::tile_is_grouped(int layer, int x, int y)
{
	return is_grouped[layer][y][x];
}

void Area_Manager::draw_layer_isometric_ungrouped(int layer)
{
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (!tile_is_grouped(layer, x, y)) {
				draw_iso_tile(layer, x, y, -1, -1);
			}
		}
	}
}

void Area_Manager::draw_layer_isometric_groups(int layer)
{
	draw_layer_isometric_ungrouped(layer);

	if (group_drawn[layer] == NULL) {
		group_drawn[layer] = new bool[tile_groups[layer].size()];
	}
	memset(group_drawn[layer], 0, sizeof(bool) * tile_groups[layer].size());

	General::Point<int> *epos = new General::Point<int>[entities.size()];
	int *sort_offset = new int[entities.size()];
	bool *process_ent = new bool[entities.size()];
	int sz = entities.size();
 	for (int i = 0; i < sz; i++) {
		process_ent[i] = false;
		Map_Entity *e = entities[i];
		if (!e) {
			continue;
		}
		if (e->get_animation_set() == NULL) {
			continue;
		}
		process_ent[i] = true;
		sort_offset[i] = e->get_sort_offset();
		epos[i] = e->get_position();
 	}

	size_t curr_entity = 0;

	while (true) {
		for (; curr_entity < entities.size() && process_ent[curr_entity] == false; curr_entity++)
			;

		int sz = tile_groups[layer].size();
		for (int i = 0; i < sz; i++) {
			if (group_drawn[layer][i] == true) {
				continue;
			}

			Tile_Group *tg = tile_groups[layer][i];
			
			bool before = false;

			if (curr_entity < entities.size()) {
				bool y_overlap = 
				(epos[curr_entity].y-sort_offset[curr_entity] >= tg->top_left.y && epos[curr_entity].y-sort_offset[curr_entity] < tg->bottom_right.y) ||
				(epos[curr_entity].y-sort_offset[curr_entity] >= tg->top_left2.y && epos[curr_entity].y-sort_offset[curr_entity] < tg->bottom_right2.y);


				if (y_overlap) {
					General::Point<float> level_top_left = General::Point<float>(
						tg->top_left.x, tg->top_left.y
					);
					General::Point<float> level_bottom_right = General::Point<float>(
						tg->bottom_right.x, tg->bottom_right.y
					);
					General::Point<float> level_top_left2 = General::Point<float>(
						tg->top_left2.x, tg->top_left2.y
					);
					General::Point<float> level_bottom_right2 = General::Point<float>(
						tg->bottom_right2.x, tg->bottom_right2.y
					);
					General::Point<float> entity_pos = General::Point<float>(
						epos[curr_entity].x, epos[curr_entity].y-sort_offset[curr_entity]
					);
					General::Point<float> far_right = General::Point<float>(
						pixel_size.w, epos[curr_entity].y-sort_offset[curr_entity]
					);
					bool want_miss = tg->size.w < tg->size.h;
					bool check1 = checkcoll_line_line(&level_top_left, &level_bottom_right,
							&entity_pos, &far_right, NULL);
					bool check2 = checkcoll_line_line(&level_top_left2,
							&level_bottom_right2, &entity_pos, &far_right, NULL);
					if (want_miss) {
						if (check1 == false && check2 == false) {
							before = true;
						}
					}
					else {
						//if (check1 == true || check2 == true) {
						if (check1 == true || check2 == true) {
							before = true;
						}
					}
				}
				else {
					before = tg->top_left.y < epos[curr_entity].y-sort_offset[curr_entity];
				}
			}
			else {	
				before = true;
			}

			if (before) {
				// draw
				int sz = tg->tiles.size();
				for (int k = 0; k < sz; k++) {
					General::Point<int> &p = tg->tiles[k];
					int next_x, next_y;
					if (k < (int)tg->tiles.size()-1) {
						next_x = tile_groups[layer][i]->tiles[k+1].x;
						next_y = tile_groups[layer][i]->tiles[k+1].y;
					}
					else {
						next_x = next_y = -1;
					}
					draw_iso_tile(layer, p.x, p.y, next_x, next_y);
				}
				group_drawn[layer][i] = true;
			}
		}

		int count = 0;
		sz = tile_groups[layer].size();
		for (int i = 0; i < sz; i++) {
			if (group_drawn[layer][i]) {
				count++;
			}
		}
		if (count >= (int)tile_groups[layer].size()) {
			break;
		}

		if (curr_entity < entities.size()) {
			Map_Entity *e = entities[curr_entity];
			
			curr_entity++;

			if (e == NULL) {
				e->draw();
				continue;
			}

			General::Point<int> start_draw(-1, -1);
			General::Point<int> end_draw(-1, -1);
			draw_entity(e, true, start_draw, end_draw);
		}
	}
	
	delete[] epos;
	delete[] sort_offset;
	delete[] process_ent;

	// draw whatever is left
	sz = entities.size();
	for (int j = curr_entity; j < sz; j++) {
		Map_Entity *e = entities[j];

		if (e) {
			General::Point<int> start_draw(-1, -1);
			General::Point<int> end_draw(-1, -1);
			draw_entity(e, true, start_draw, end_draw);
		}
		else {
			e->draw();
		}
	}
}

void Area_Manager::next_valid_iso_tile(
	int x, int y, int start_x, int start_y, int width, int height, float curr_width, int end_y, int *outx, int *outy)
{
	while (y < end_y) {
		int nx = start_x - ceil(curr_width)/2 + x;
		int ny = start_y + y;
		bool again = false;
		if (nx >= 0 && nx < width) {
			*outx = nx;
		}
		else {
			again = true;
		}
		if (ny >= 0 && ny < height) {
			*outy = ny;
		}
		else if (ny >= height) {
			*outx = -1;
			*outy = -1;
			return;
		}
		else {
			again = true;
		}
		if (!again) {
			return;
		}
		if (x >= ceil(curr_width)-1) {
			float add = (y < iso_tiles_in_screen.y/2) ? ISO_TILES_INCREASE : -ISO_TILES_INCREASE;
			if (curr_width+add < MAX_ISO_CORNER_TILES) {
				*outx = -1;
				*outy = -1;
				return;
			}
			x = 0;
			y++;
			curr_width += add;
		}
		else {
			x++;
		}
	}

	*outx = -1;
	*outy = -1;
}

void Area_Manager::draw_layer_isometric(int layer)
{
	Map_Entity *player = get_entity(0);
	General::Point<float> player_pos = player->get_position();
	int xoffs = player_pos.x - top.x;
	int yoffs = player_pos.y - top.y;
	int start_x = player_pos.x + (cfg.screen_w-xoffs);
	int start_y = player_pos.y - yoffs;
	General::reverse_iso_project(&start_x, &start_y, iso_offset);
	start_x /= General::TILE_SIZE;
	start_y /= General::TILE_SIZE;
	start_y--;
	float curr_width = MAX_ISO_CORNER_TILES;

	if (start_x < 0) start_x = 0;
	if (start_y < 0) {
		curr_width += (-start_y)*ISO_TILES_INCREASE;
		start_y = 0;
	}
	if (start_x >= width) start_x = width-1;

	int end_y;
	if (start_y + iso_tiles_in_screen.y > height)
		end_y = height - start_y;
	else
		end_y = iso_tiles_in_screen.y;
	
	int tx = 0, ty = 0, next_x = 0 /* silence warning */, next_y;

	for (int y = 0; y < end_y; y++) {
		int end = ceil(curr_width);
		for (int x = 0; x < end; x++) {
			next_valid_iso_tile(x, y, start_x, start_y, width, height, curr_width, end_y, &tx, &ty);
			int nx = x, ny = y;
			bool done = false;
			if (x >= end-1) {
				if (ny >= end_y-1) {
					done = true;
				}
				else {
					float add = (y < iso_tiles_in_screen.y/2) ? ISO_TILES_INCREASE : -ISO_TILES_INCREASE;
					if (curr_width+add < MAX_ISO_CORNER_TILES) {
						done = true;
					}
					else
						nx = start_x - (ceil(curr_width)+add)/2;
					ny++;
				}
			}
			else {
				nx++;
			}
			if (done) {
				next_x = -1;
				next_y = -1;
			}
			else {
				next_valid_iso_tile(nx, ny, start_x, start_y, width, height, curr_width, end_y, &next_x, &next_y);
			}

			draw_iso_tile(layer, tx, ty, next_x, next_y);

			if (next_x < 0)
				goto done;
		}
		if (y < iso_tiles_in_screen.y/2) {
			curr_width += ISO_TILES_INCREASE;
		}
		else {
			curr_width -= ISO_TILES_INCREASE;
		}
	}
done:;
}

Map_Entity *Area_Manager::get_next_entity_in_layer(Map_Entity *start, int layer)
{
	std::vector<Map_Entity *>::iterator it;

	if (start == NULL) {
		it = entities.begin();
	}
	else {
		it = std::find(entities.begin(), entities.end(), start);
		it++;
	}

	while (it != entities.end()) {
		Map_Entity *me = *it;
		if (me->get_layer() == layer)
			break;
		it++;
	}

	return it == entities.end() ? NULL : *it;
}

void Area_Manager::draw_layer(int layer)
{
	if (layer >= 0 && layer <= 9 && layer_toggled_off[layer])
		return;

	if (isometric) {
		curr_sheet = -1;
		iso_vert_count = 0;
		if (tile_groups[layer].size() > 0) {
			draw_layer_isometric_groups(layer);
		}
		else {
			draw_layer_isometric(layer);
		}
	}
	else {
		if (tile_groups[layer].size() > 0) {
			draw_layer_tiled_groups(layer);
		}
		else {
			draw_layer_tiled(layer);
		}
	}

	al_hold_bitmap_drawing(true);
	int sz = particles.size();
	for (int i = 0; i < sz; i++) {
		if (particles[i]->particle_group->layer == layer) {
			particles[i]->draw();
		}
	}
	al_hold_bitmap_drawing(false);

	// KEEPME:
	// FIXME:
	/*
	layer = 4;
	for (int i = 0; i < (int)collision_lines[layer].size(); i++) {
		al_draw_line(
			collision_lines[layer][i].x1-top.x,
			collision_lines[layer][i].y1-top.y,
			collision_lines[layer][i].x2-top.x,
			collision_lines[layer][i].y2-top.y,
			al_map_rgb(0, 255, 0),
			1
		);
	}
	*/

	// KEEPME: draw navigation mesh
	#if 0
	ALLEGRO_VERTEX verts[10000];
	int vcount = 0;
	for (int i = 0; i < (int)nav_meshes[layer].size(); i++) {
		Triangulate::Triangle *t = &nav_meshes[layer][i];
		verts[vcount].x = t->points[0].x-top.x; verts[vcount].y = t->points[0].y-top.y; verts[vcount].z = 0; verts[vcount].color = al_map_rgb(255, 0, 0); vcount++;
		verts[vcount].x = t->points[1].x-top.x; verts[vcount].y = t->points[1].y-top.y; verts[vcount].z = 0; verts[vcount].color = al_map_rgb(255, 0, 0); vcount++;
		verts[vcount].x = t->points[1].x-top.x; verts[vcount].y = t->points[1].y-top.y; verts[vcount].z = 0; verts[vcount].color = al_map_rgb(255, 0, 0); vcount++;
		verts[vcount].x = t->points[2].x-top.x; verts[vcount].y = t->points[2].y-top.y; verts[vcount].z = 0; verts[vcount].color = al_map_rgb(255, 0, 0); vcount++;
		verts[vcount].x = t->points[2].x-top.x; verts[vcount].y = t->points[2].y-top.y; verts[vcount].z = 0; verts[vcount].color = al_map_rgb(255, 0, 0); vcount++;
		verts[vcount].x = t->points[0].x-top.x; verts[vcount].y = t->points[0].y-top.y; verts[vcount].z = 0; verts[vcount].color = al_map_rgb(255, 0, 0); vcount++;
		/*
		for (int j = 0; j < 3; j++) {
			A_Star::Navigation_Link *n = nav_links[layer][i][j];
			Triangulate::Triangle *t2 = n->triangle;
			if (t2) {
				General::Point<float> *t1_p1 = &t->points[j];
				General::Point<float> *t1_p2 = &t->points[(j+1)%3];
				General::Point<float> *t2_p1 = &t2->points[n->edge_start];
				General::Point<float> *t2_p2 = &t2->points[(n->edge_start+1)%3];
				
				int cx = t1_p1->x + (t1_p2->x - t1_p1->x) / 2;
				int cy = t1_p1->y + (t1_p2->y - t1_p1->y) / 2;
				int cx2 = t2_p1->x + (t2_p2->x - t2_p1->x) / 2;
				int cy2 = t2_p1->y + (t2_p2->y - t2_p1->y) / 2;
				al_draw_filled_rectangle(cx-4-top.x, cy-4-top.y, cx2+4-top.x, cy2+4-top.y, al_map_rgba_f(0.5, 0.5, 0, 0.5));
			}
		}
		*/
	}
	al_draw_prim(verts, 0, 0, 0, vcount, ALLEGRO_PRIM_LINE_LIST);
	#endif
}

void Area_Manager::draw_floating_layer(int layer, bool pre)
{
	Floating_Layer_Info &info = find_floating_info(layer);

	for (int i = 0; i < (int)floating.size(); i++) {
		Area_Floating_Block &b = floating[i];

		if (b.layer != layer || b.pre != pre)
			continue;

		int dx, dy;

		if (parallax_x) {
			float p = top.x / (pixel_size.w-cfg.screen_w);
			dx = p * (info.total_width-cfg.screen_w);
			dx = -dx;
			dx += b.offset_x;
		}
		else {
			dx = b.offset_x-(int)top.x;
		}

		dy = b.offset_y-(int)top.y;

		if (b.type == FLOATING_IMAGE) {
			ALLEGRO_STATE state;
			if (b.subtractive) {
				al_store_state(&state, ALLEGRO_STATE_BLENDER);
				al_set_separate_blender(ALLEGRO_DEST_MINUS_SRC, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ADD, ALLEGRO_ZERO, ALLEGRO_ONE);
			}
			al_draw_bitmap(
				b.bitmap->bitmap,
				dx,
				dy,
				0
			);
			if (b.subtractive) {
				al_restore_state(&state);
			}
		}
		else { // FLOATING_RECTANGLE
			ALLEGRO_VERTEX v[6];
			v[0].x = dx;
			v[0].y = dy;
			v[0].z = 0;
			v[0].color = b.tl;
			v[1].x = dx+b.width;
			v[1].y = dy;
			v[1].z = 0;
			v[1].color = b.tr;
			v[2].x = dx;
			v[2].y = dy+b.height;
			v[2].z = 0;
			v[2].color = b.bl;
			v[3].x = dx;
			v[3].y = dy+b.height;
			v[3].z = 0;
			v[3].color = b.bl;
			v[4].x = dx+b.width;
			v[4].y = dy;
			v[4].z = 0;
			v[4].color = b.tr;
			v[5].x = dx+b.width;
			v[5].y = dy+b.height;
			v[5].z = 0;
			v[5].color = b.br;
			al_draw_prim(v, 0, 0, 0, 6, ALLEGRO_PRIM_TRIANGLE_LIST);
		}
	}
}

void Area_Manager::draw_entity(Map_Entity *entity, bool draw_shadow, General::Point<int> start_draw, General::Point<int> end_draw)
{
	General::Point<float> entity_pos = entity->get_position();

	if (!entity->is_visible())
		return;

	// Don't draw if not close to screen
	if (entity_pos.x < top.x-General::TILE_SIZE*5 || entity_pos.y < top.y-General::TILE_SIZE*5 || entity_pos.x > top.x+cfg.screen_w+General::TILE_SIZE*5 || entity_pos.y > top.y+cfg.screen_h+General::TILE_SIZE*5) {
		return;
	}

	Skeleton::Skeleton *skeleton = entity->get_skeleton();
	Animation_Set *anim_set = entity->get_animation_set();

	if (skeleton) {
		if (entity->shadow_is_shown() && draw_shadow) {
			ALLEGRO_BITMAP *shadow;
			if (entity->get_name() == "fieldantboss") {
				shadow = big_character_shadow_bmp->bitmap;
			}
			else {
				shadow = normal_character_shadow_bmp->bitmap;
			}
			al_draw_bitmap(
				shadow,
				entity_pos.x - top.x - al_get_bitmap_width(shadow)/2,
				entity_pos.y - top.y - al_get_bitmap_height(shadow)/2 - 3,
				0
			);
		}
		skeleton->draw(General::Point<float>(entity_pos.x-top.x, entity_pos.y-top.y-entity->get_z()), !entity->is_facing_right(), al_color_name("white"));
	}
	else if (anim_set) {
		Animation *anim = anim_set->get_current_animation();
		Frame *frame = anim->get_current_frame();
		Bitmap *bmp = frame->get_bitmap();

		Bitmap *weapon_bmp = NULL;
		Player *p = dynamic_cast<Player *>(entity);
		if (p) {
			Animation_Set *a = p->get_weapon_animation_set();
			if (a) {
				Animation *an = a->get_current_animation();
				if (strstr(an->get_name().c_str(), "attack")) {
					Frame *f = an->get_current_frame();
					weapon_bmp = f->get_bitmap();
				}
			}
		}

		int bmp_w = bmp->get_width();
		int bmp_h = bmp->get_height();

		if (!(entity_pos.x < top.x-bmp_w/2 || entity_pos.x >= top.x+cfg.screen_w+bmp_w/2
				|| entity_pos.y-entity->get_z()+General::TILE_SIZE < top.y || entity_pos.y-bmp_h-entity->get_z()+General::TILE_SIZE > top.y+cfg.screen_h))
		{
			if (entity->shadow_is_shown() && draw_shadow) {
				ALLEGRO_BITMAP *shadow;
				if (entity->get_name() == "amaysa") {
					shadow = big_character_shadow_bmp->bitmap;
				}
				else {
					shadow = normal_character_shadow_bmp->bitmap;
				}
				al_draw_bitmap(
					shadow,
					entity_pos.x - top.x - al_get_bitmap_width(shadow)/2,
					entity_pos.y - top.y - al_get_bitmap_height(shadow)/2 - 3,
					0
				);
			}

			int flags;

			const char *subname = entity->get_animation_set()->get_sub_animation_name().c_str();
			int len = strlen(subname);
			if ((len >= 3 && !strcmp(subname+strlen(subname)-strlen("-up"), "-up")) || (len >= 5 && !strcmp(subname+strlen(subname)-strlen("-down"), "-down"))) {
				flags = 0;
			}
			else if (entity->is_facing_right()) {
				if (entity->get_animation_set()->get_current_animation()->has_tag("hflip")) {
					flags = ALLEGRO_FLIP_HORIZONTAL;
				}
				else {
					flags = 0;
				}
			}
			else {
				if (entity->get_animation_set()->get_current_animation()->has_tag("hflip")) {
					flags = 0;
				}
				else {
					flags = ALLEGRO_FLIP_HORIZONTAL;
				}
			}

			ALLEGRO_COLOR tint;
			if (shadow_mask && General::check_mask(shadow_mask, shadow_pitch, (int)entity_pos.x, (int)entity_pos.y)) {
				float r, g, b;
				al_unmap_rgb_f(shadow_color, &r, &g, &b);
				tint = al_map_rgba_f(r, g, b, 1);
			}
			else {
				tint = al_map_rgb(255, 255, 255);
			}

			int start_x, start_y, draw_w, draw_h;

			if (start_draw.x < 0) {
				start_x = 0;
				start_y = 0;
				draw_w = bmp_w;
				draw_h = bmp_h;
			}
			else {
				start_x = start_draw.x;
				start_y = start_draw.y;
				draw_w = end_draw.x - start_x;
				draw_h = end_draw.y - start_y;
			}

			// FIXME: only if eyes supposed to be glowing
			if (entity->get_name() == "amaysa" && !engine->milestone_is_complete("beat_game")) {
				Wrap::Bitmap *b = bmp->get_bitmap();
				bool held = al_is_bitmap_drawing_held();
				al_hold_bitmap_drawing(false);
				Graphics::draw_tinted_bitmap_region_depth_yellow_glow(
					b,
					tint,
					start_x, start_y,
					draw_w,
					draw_h,
					start_x + entity_pos.x - top.x - bmp_w/2,
					start_y + entity_pos.y - top.y - bmp_h - entity->get_z() + General::TILE_SIZE,
					flags,
					1.0f,
					255, 85, 211,
					209, 74, 173
				);
				al_hold_bitmap_drawing(held);
			}
			else {
				bmp->draw_region_tinted_depth(
					tint,
					start_x, start_y,
					draw_w,
					draw_h,
					start_x + entity_pos.x - top.x - bmp_w/2,
					start_y + entity_pos.y - top.y - bmp_h - entity->get_z() + General::TILE_SIZE,
					flags,
					1.0f
				);
			}
		
			if (weapon_bmp) {
				weapon_bmp->draw_region_tinted_depth(
					tint,
					start_x, start_y,
					draw_w,
					draw_h,
					start_x + entity_pos.x - top.x - bmp_w/2,
					start_y + entity_pos.y - top.y - bmp_h - entity->get_z() + General::TILE_SIZE,
					flags,
					1.0f
				);
			}

			entity->draw();
		}
	}
	else {
		entity->draw();
	}
}

void Area_Manager::draw()
{
	ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
	for (size_t i = 0; i < 1; i++) {
		al_set_target_bitmap(water_sheets[i]->bitmap);
		ALLEGRO_STATE state;
		al_store_state(&state, ALLEGRO_STATE_BLENDER);
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
		al_draw_bitmap(tile_sheets[i]->bitmap, 0, 0, 0);
		al_restore_state(&state);
		if (cfg.water_shader) {
			float normal_water[3] = {
				68.0f/255.0f,
				114.0f/255.0f,
				183.0f/255.0f,
			};
			float normal_dark_water[3] = {
				37.0f/255.0f,
				79.0f/255.0f,
				139.0f/255.0f,
			};

			float old_forest_water[3] = {
				68.0f/255.0f,
				183.0f/255.0f,
				181.0f/255.0f,
			};
			float old_forest_dark_water[3] = {
				37.0f/255.0f,
				139.0f/255.0f,
				136.0f/255.0f,
			};

			float *water;
			float *dark_water;
			if (name.substr(0, 9) == "oldforest") {
				water = old_forest_water;
				dark_water = old_forest_dark_water;
			}
			else {
				water = normal_water;
				dark_water = normal_dark_water;
			}

			int tex_w, tex_h;
			Graphics::get_texture_size(tile_sheets[i]->bitmap, &tex_w, &tex_h);

			Shader::use(water_shader);
			al_set_shader_float("water_r", water[0]);
			al_set_shader_float("water_g", water[1]);
			al_set_shader_float("water_b", water[2]);
			al_set_shader_float("dark_water_r", dark_water[0]);
			al_set_shader_float("dark_water_g", dark_water[1]);
			al_set_shader_float("dark_water_b", dark_water[2]);
			float t = fmod(al_get_time(), 3.0) / 1.5f;
			if (t > 1.0f) {
				t = 1.0f-(t-1.0f);
			}
			t = t * 2.01f;
			al_set_shader_float("time", t);
			al_set_shader_float("tex_w", tex_w);
			al_set_shader_float("tex_h", tex_h);
			al_hold_bitmap_drawing(true);
			al_draw_bitmap_region(
				tile_sheets[i]->bitmap,
				28*General::TILE_SIZE+28*2,
				10*General::TILE_SIZE+10*2,
				3*General::TILE_SIZE+3*2,
				3*General::TILE_SIZE+3*2,
				28*General::TILE_SIZE+28*2,
				10*General::TILE_SIZE+10*2,
				0
			);
			al_draw_bitmap_region(
				tile_sheets[i]->bitmap,
				28*General::TILE_SIZE+28*2,
				14*General::TILE_SIZE+14*2,
				4*General::TILE_SIZE+4*2,
				4*General::TILE_SIZE+4*2,
				28*General::TILE_SIZE+28*2,
				14*General::TILE_SIZE+14*2,
				0
			);
			al_hold_bitmap_drawing(false);
			Shader::use(NULL);
		}
	}
	al_set_target_bitmap(old_target);

	std::vector<Wrap::Bitmap *> old_sheets = tile_sheets;
	tile_sheets = water_sheets;

	// adjust for scripted camera
	General::Point<float> top_backup = top;
	top.x += _offset.x + rumble_offset.x;
	top.y += _offset.y + rumble_offset.y;

	if (isometric) {
		std::stable_sort(entities.begin(), entities.end(), compare_entities);
	}
	else {
		std::stable_sort(entities.begin(), entities.end(), compare_entities);
	}
	
	// This needs to be recalculated each time as the screen size can change with
	// auto rotation
	if (isometric) {
		int div4 = General::TILE_SIZE/4;
		int div2 = General::TILE_SIZE/2;
		iso_tiles_in_screen.x = cfg.screen_w / div2 + 4;
		iso_tiles_in_screen.y = cfg.screen_h / div4 + 4;
	}
	else {
		width_in_tiles = cfg.screen_w / General::TILE_SIZE;
		if (cfg.screen_w % General::TILE_SIZE)
			width_in_tiles++;
		height_in_tiles = cfg.screen_h / General::TILE_SIZE;
		if (cfg.screen_h % General::TILE_SIZE)
			height_in_tiles++;
	}

	int layer;
	for (layer = 0; layer < num_layers; layer++) {
		top.y += height_offset_dampened;

		if (script_has_pre_draw) {
			Lua::call_lua(lua_state, "pre_draw_layer", "i>", layer);
		}

		draw_floating_layer(layer, true);

		draw_layer(layer);

		draw_floating_layer(layer, false);

		if (script_has_mid_draw) {
			Lua::call_lua(lua_state, "mid_draw_layer", "i>", layer);
		}

		if (player_underlay_bitmap_add && layer == player_underlay_layer) {
			Map_Entity *p = get_entity(0);
			General::Point<float> player_pos = p->get_position();
			Bitmap *bmp =
				p->get_animation_set()->get_current_animation()->get_current_frame()->get_bitmap();
			int player_bmp_h = bmp->get_height();
			int add_bmp_w = al_get_bitmap_width(player_underlay_bitmap_add->bitmap);
			int add_bmp_h = al_get_bitmap_height(player_underlay_bitmap_add->bitmap);
			ALLEGRO_STATE state;
			al_store_state(&state, ALLEGRO_STATE_BLENDER);
			al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
			al_draw_tinted_bitmap(
				player_underlay_bitmap_add->bitmap,
				al_map_rgba(255, 255, 255, 128),
				(int)player_pos.x - (int)top.x - add_bmp_w/2,
				(int)player_pos.y - (int)top.y - player_bmp_h/2 - add_bmp_h/2,
				0
			);
			al_restore_state(&state);
		}

		if (tile_groups[layer].size() <= 0) {
			int sz = entities.size();
			for (int i = 0; i < sz; i++) {
				Map_Entity *entity = entities[i];

				if (entity) {
					if (entity->get_layer() != layer) {
						continue;
					}

					General::Point<int> start_draw(-1, -1);
					General::Point<int> end_draw(-1, -1);

					draw_entity(entity, true, start_draw, end_draw);
				}
			}
		}

		top.y -= height_offset_dampened;
	
		if (script_has_post_draw) {
			Lua::call_lua(lua_state, "post_draw_layer", "i>", layer);
		}
	}
		
	if (player_underlay_bitmap_add && player_underlay_top_also) {
		Map_Entity *entity = get_entity(0);
		General::Point<float> entity_pos = entity->get_position();
		Bitmap *bmp =
			entity->get_animation_set()->get_current_animation()->get_current_frame()->get_bitmap();
		int bmp_h = bmp->get_height();
		int add_bmp_w = al_get_bitmap_width(player_underlay_bitmap_add->bitmap);
		int add_bmp_h = al_get_bitmap_height(player_underlay_bitmap_add->bitmap);
		ALLEGRO_STATE state;
		al_store_state(&state, ALLEGRO_STATE_BLENDER);
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
		al_draw_bitmap(
			player_underlay_bitmap_add->bitmap,
			(int)entity_pos.x - (int)top.x - add_bmp_w/2,
			(int)entity_pos.y - (int)top.y - bmp_h/2 - add_bmp_h/2,
			0
		);
		al_restore_state(&state);
	}

	draw_floating_layer(num_layers, true);
	draw_floating_layer(num_layers, false);

	top = top_backup;

	/*
	// KEEPME: Draw heightmap
	ALLEGRO_LOCKED_REGION *lr = al_lock_bitmap(al_get_target_bitmap(), ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, 0);
	for (int y = 0; y < RENDER_H; y++) {
		unsigned char *p = (unsigned char *)lr->data + lr->pitch * y;
		for (int x = 0; x < RENDER_W; x++) {
			int r, g, b, a;
			r = *(p+0);
			g = *(p+1);
			b = *(p+2);
			a = *(p+3);
			int avg = (r + g + b) / 3;
			int h = height_at(top.x+x, top.y+y);
			h += 28;
			h *= 4;
			avg += h;
			if (avg > 255) avg = 255;
			*(p+0) = avg;
			*(p+1) = 0;
			*(p+2) = 0;
			*(p+3) = a;
			p += 4;
		}
	}
	al_unlock_bitmap(al_get_target_bitmap());
	*/

	tile_sheets = old_sheets;
}

bool Area_Manager::load_tiled_area()
{
	int sz;
	uint8_t *buf = General::slurp(path + "/area", &sz);
	ALLEGRO_FILE *f = al_open_memfile(buf, sz, "rb");

	width = General::read32bits(f);
	height = General::read32bits(f);

	size = General::Size<int>(width, height);

	num_layers = General::read32bits(f);

	group_drawn = new bool*[num_layers];
	for (int i = 0; i < num_layers; i++) {
		group_drawn[i] = NULL;
	}

	std::stringstream ss;
	ss << "Area size: " << width << "x" << height << ", " << num_layers << " layers.";
	General::log_message(ss.str());

	pixel_size.w = width * General::TILE_SIZE;
	pixel_size.h = height * General::TILE_SIZE;

	if (isometric) {
		iso_offset = General::Point<float>((width > height ? width : height) * General::TILE_SIZE / 2, 0);
		int startx = pixel_size.w;
		int starty = 0;
		General::iso_project(&pixel_size.w, &starty, iso_offset);
		General::iso_project(&startx, &pixel_size.h, iso_offset);
	}

	for (int i = 0; i < num_layers; i++) {
		Tiled_Layer *l = new Tiled_Layer();
		for (int y = 0; y < height; y++) {
			std::vector<Tiled_Layer::Tile> row;
			for (int x = 0; x < width; x++) {
				Tiled_Layer::Tile t;
				t.number = General::read32bits(f);
				t.sheet = al_fgetc(f);
				t.solid = al_fgetc(f);

				row.push_back(t);
			}
			l->tiles.push_back(row);
		}
		tiled_layers.push_back(l);
	}
	
	al_fclose(f);

	delete[] buf;
	
	return true;
}

void Area_Manager::create_outline_storage()
{
        collision_lines = new std::vector< General::Line<float> >[num_layers];
	outline_points = new std::vector< General::Point<float> >[num_layers];
	outline_splits = new std::vector<int>[num_layers];
	nav_meshes = new std::vector<Triangulate::Triangle>[num_layers];
}

bool Area_Manager::load(std::string name)
{
	General::log_message("Loading area " + name);

	if (cfg.water_shader) {
		water_shader = Shader::get("water");
		if (!water_shader) {
			cfg.water_shader = false;
		}
	}

	this->name = name;

	normal_character_shadow_bmp = engine->get_hero_shadow();
	big_character_shadow_bmp = engine->get_big_shadow();

	ALLEGRO_DEBUG("Character shadow bitmap loaded");

	path = "areas/" + name + ".area";

	heightmap = General::slurp(path + "/heightmap");

	if (General::exists(path + "/isometric")) {
		isometric = true;
	}

	if (!load_tiled_area())
		return false;
	
	load_sheets();
	
	for (unsigned int i = 0; i < tile_sheets.size(); i++) {
		calc_tile_bounds(i);
	}
	tile_swap_map.clear();
	
	shadow_pitch = (pixel_size.w/8) + (pixel_size.w % 8 == 0 ? 0 : 1);
	General::load_mask(path + "/shadows", shadow_pitch*pixel_size.h, &shadow_mask);
		
	for (int i = 0; i < num_layers; i++) {
		tile_groups.push_back(std::vector<Tile_Group *>());
	}

	create_outline_storage();

	Map_Entity *plyr = get_entity(0);
	General::Point<float> plyr_pos = plyr->get_position();
	player_height = height_at(plyr_pos.x, plyr_pos.y);
        height_offset_dampened = player_height;

	return true;
}

int Area_Manager::add_entity(Map_Entity *entity)
{
	Area_Loop *al = (Area_Loop *)GET_AREA_LOOP;
	entity->set_area_loop(al);

	entities.push_back(entity);
	entity->init();

	return entity->get_id();
}

Map_Entity *Area_Manager::get_entity(int id)
{
	std::vector<Map_Entity *>::iterator it;
	for (it = entities.begin(); it != entities.end(); it++) {
		if ((*it)->get_id() == id) {
			return *it;
		}
	}
	return NULL;
}

General::Point<float> Area_Manager::get_top()
{
	return top;
}

void Area_Manager::set_top(General::Point<float> top)
{
	this->top = top;
}

lua_State *Area_Manager::get_lua_state()
{
	return lua_state;
}

General::Point<float> Area_Manager::get_camera_pos()
{
	return top;
}

bool Area_Manager::move_camera(General::Point<float> delta, General::Point<float> entity_pos, bool tracking_entity)
{
	General::Point<float> backup = top;

	if (
		!tracking_entity ||
		((entity_pos.x >= cfg.screen_w/2) && (entity_pos.x < pixel_size.w - cfg.screen_w/2))
	) {
		top.x += delta.x;
	}
	if (top.x < 0)
		top.x = 0;
	if (top.x > pixel_size.w - cfg.screen_w)
		top.x = pixel_size.w - cfg.screen_w;

	if (
		!tracking_entity ||
		((entity_pos.y >= cfg.screen_h/2) && (entity_pos.y < pixel_size.h - cfg.screen_h/2))
	) {
		top.y += delta.y;
	}
	if (top.y < 0)
		top.y = 0;
	if (top.y > pixel_size.h - cfg.screen_h)
		top.y = pixel_size.h - cfg.screen_h;

	if (pixel_size.w <= cfg.screen_w) {
		delta.x = 0;
		top.x = -(cfg.screen_w-pixel_size.w)/2;
	}
	else {
		delta.x = top.x - backup.x;
	}
	if (pixel_size.h <= cfg.screen_h) {
		delta.y = 0;
		top.y = -(cfg.screen_h-pixel_size.h)/2;
	}
	else {
		delta.y = top.y - backup.y;
	}

	bool moved;

	if (delta.x != 0 || delta.y != 0)
		moved = true;
	else
		moved = false;

	return moved;
}

static int sums(int n)
{
	float sum = 0;

	while (n) {
		sum += n * ISO_TILES_INCREASE;
		n--;
	}

	return sum;
}

void Area_Manager::prepare_for_reshape()
{
	delete[] verts;
}

void Area_Manager::reshape()
{
	if (isometric) {
		int w = cfg.screen_w / General::TILE_SIZE;
		int h = cfg.screen_h / (General::TILE_SIZE/2);
		int n = sums(w+2) + sums(h+2);
		verts = new ALLEGRO_VERTEX[n*6];
	}
	else {
		width_in_tiles = cfg.screen_w / General::TILE_SIZE;
		if (cfg.screen_w % General::TILE_SIZE)
			width_in_tiles++;
		height_in_tiles = cfg.screen_h / General::TILE_SIZE;
		if (cfg.screen_h % General::TILE_SIZE)
			height_in_tiles++;
		verts = new ALLEGRO_VERTEX[6*((width_in_tiles+2)*(height_in_tiles+2))];
	}
	
	reset_top();
}

void Area_Manager::start()
{
	ALLEGRO_DEBUG("initing Lua");

	init_lua();

	ALLEGRO_DEBUG("Lua inited");

	reshape();

	ALLEGRO_DEBUG("Reshaped");

	optimize_tiled_area();
}

bool Area_Manager::area_is_colliding(int layer, General::Point<int> pos, std::vector<Bones::Bone> &bones, bool right, General::Line<float> *ret_line, bool *stop_entity)
{
	if (ret_line) {
		ret_line->x1 =
		ret_line->x2 =
		ret_line->y1 =
		ret_line->y2 = -1;
	}
	if (stop_entity) {
		*stop_entity = true;
	}

	for (size_t i = 0; i < bones.size(); i++) {
		std::vector< General::Point<float> > outline;
		if (right) {
			outline = bones[i].get_outline();
		}
		else {
			outline = bones[i].get_outline_mirrored();
		}

		int sz = outline.size();
		for (int j = 0; j < sz; j++) {
			General::Point<float> &p = outline[j];
			if (p.x+pos.x < 0) {
				if (ret_line) {
					ret_line->x1 = 0;
					ret_line->y1 = 0;
					ret_line->x2 = 0;
					ret_line->y2 = pixel_size.h-1;
				}
				return true;
			}
			if (p.x+pos.x >= pixel_size.w) {
				if (ret_line) {
					ret_line->x1 = pixel_size.w-1;
					ret_line->y1 = 0;
					ret_line->x2 = pixel_size.w-1;
					ret_line->y2 = pixel_size.h-1;
				}
				return true;
			}
			if (p.y+pos.y+General::TILE_SIZE < 0) {
				if (ret_line) {
					ret_line->x1 = 0;
					ret_line->y1 = 0;
					ret_line->x2 = pixel_size.w-1;
					ret_line->y2 = 0;
				}
				return true;
			}
			if (p.y+pos.y+General::TILE_SIZE >= pixel_size.h) {
				if (ret_line) {
					ret_line->x1 = 0;
					ret_line->y1 = pixel_size.h-1;
					ret_line->x2 = pixel_size.w-1;
					ret_line->y2 = pixel_size.h-1;
				}
				return true;
			}
		}
	}

#if 0
	// check map/tile solids
	if (tiled_area && !isometric) {
		for (int i = 0; i < 4; i++) {
			int x = positions[i*2];
			int y = positions[i*2+1];
			x /= General::TILE_SIZE;
			y /= General::TILE_SIZE;
			if (tiled_layers[layer]->tiles[y][x].solid == true) {
				return true;
			}
		}
	}
#endif

	for (size_t m = 0; m < bones.size(); m++) {
		std::vector< General::Point<float> > outline;
		if (right) {
			outline = bones[m].get_outline();
		}
		else {
			outline = bones[m].get_outline_mirrored();
		}

		int sz = outline.size();
		for (int k = 0; k < sz; k++) {
			int l = (k+1) % outline.size();

			General::Point<float> p1(
				outline[k].x + pos.x,
				outline[k].y + pos.y + General::BOTTOM_SPRITE_PADDING
			);

			General::Point<float> p2(
				outline[l].x + pos.x,
				outline[l].y + pos.y + General::BOTTOM_SPRITE_PADDING
			);

			int qx1 = p1.x / QUADRANT_SIZE;
			int qy1 = p1.y / QUADRANT_SIZE;

			int sz = quadrants[layer][qy1][qx1].size();
			for (int i = 0; i < sz; i++) {
				General::Point<float> p3 = quadrants[layer][qy1][qx1][i].first;
				General::Point<float> p4 = quadrants[layer][qy1][qx1][i].second;

				if (checkcoll_line_line(&p1, &p2, &p3, &p4, NULL)) {
					if (ret_line) {
						ret_line->x1 = p3.x;
						ret_line->y1 = p3.y;
						ret_line->x2 = p4.x;
						ret_line->y2 = p4.y;
					}
					return true;
				}
			}

			int qx2 = p2.x / QUADRANT_SIZE;
			int qy2 = p2.y / QUADRANT_SIZE;

			if (qx2 != qx1 || qy2 != qy1) {
				int sz = quadrants[layer][qy2][qx2].size();
				for (int i = 0; i < sz; i++) {
					General::Point<float> p3 = quadrants[layer][qy2][qx2][i].first;
					General::Point<float> p4 = quadrants[layer][qy2][qx2][i].second;

					if (checkcoll_line_line(&p1, &p2, &p3, &p4, NULL)) {
						if (ret_line) {
							ret_line->x1 = p3.x;
							ret_line->y1 = p3.y;
							ret_line->x2 = p4.x;
							ret_line->y2 = p4.y;
						}
						return true;
					}
				}
			}
		}
	}
	return false;
}

void Area_Manager::shake_bushes(Map_Entity *entity, General::Point<float> entity_pos)
{
	if (entity_pos.x < top.x+_offset.x || entity_pos.y < top.y+_offset.y || entity_pos.x > top.x+_offset.x+cfg.screen_w || entity_pos.y > top.y+_offset.y+cfg.screen_h) {
		return;
	}

	std::vector<Bones::Bone> bones;
	entity->collidable_get_bones(bones);

	General::Point<float> pos = General::Point<float>(
		entity_pos.x,
		entity_pos.y
	);

	bool found = false;
	int found_i = -1;
	for (size_t i = 0; i < shaken_entities.size(); i++) {
		if (shaken_entities[i] == entity) {
			found =  true;
			found_i = i;
			break;
		}
	}
	bool go;
	if (found) {
		if (shaken_positions[found_i] == pos) {
			go = false;
		}
		else {
			go = true;
			ERASE_FROM_UNSORTED_VECTOR(shaken_entities, found_i);
			ERASE_FROM_UNSORTED_VECTOR(shaken_positions, found_i);
		}
	}
	else {
		go = true;
	}
	if (go) {
		int layer = entity->get_layer();
		bool done = false;
		int sz = tile_groups[layer].size();
		for (int i = 0; i < sz; i++) {
			Tile_Group *tg = tile_groups[layer][i];
			if (tg->flags & TILE_GROUP_BUSHES) {
				for (size_t i = 0; i < bones.size(); i++) {
					// Do a quick check to try and avoid a long one
					if (General::distance(pos.x, pos.y, tg->top_left.x, tg->top_left.y) < 100) {
						std::vector< General::Point<float> > outline;
						if (entity->is_facing_right()) {
							outline = bones[i].get_outline();
						}
						else {
							outline = bones[i].get_outline_mirrored();
						}
						General::Point<float> top_left(
							tg->top_left.x,
							tg->top_left.y - 8
						);
						General::Point<float> bottom_right(
							tg->top_left.x + tg->size.w,
							tg->top_left.y + tg->size.h - 8
						);
						if (checkcoll_box_polygon(
							top_left,
							bottom_right,
							outline,
							pos,
							NULL))
						{
							shaken_entities.push_back(entity);
							shaken_positions.push_back(pos);
							tg->duration += 0.025;
							if (tg->duration > 0.5) {
								tg->duration = 0.5;
							}
							done = true;
							break;
						}
					}
				}
			}
			if (done) {
				break;
			}
		}
	}
}

std::list<Map_Entity *> Area_Manager::entity_is_colliding(Map_Entity *entity, General::Point<float> entity_pos, bool weapon)
{
	std::list<Map_Entity *> colliding_entities;

	// FIXME: handle no-bones entities if necessary
	if (entity->collidable_get_type() != COLLIDABLE_BONES) {
		return colliding_entities;
	}
	
	std::vector<Bones::Bone> bones;
	bool got_bones = false;

	General::Point<float> pos = General::Point<float>(
		entity_pos.x,
		entity_pos.y
	);
	int entity_layer = entity->get_layer();

	int sz = entities.size();
	for (int i = 0; i < sz; i++) {
		Map_Entity *curr_entity = entities[i];

		if (curr_entity == NULL) {
			continue;
		}

		if (curr_entity->get_skeleton()) {
			continue;
		}

		if (!curr_entity->is_solid_with_entities()) {
			continue;
		}

		if (entity_layer != curr_entity->get_layer()) {
			continue;
		}

		if (entity->get_id() == curr_entity->get_id()) {
			continue;
		}

		General::Point<float> curr_pos = curr_entity->get_position();

		if (curr_entity->collidable_get_type() == COLLIDABLE_BONES) {
			bool collides = false;

			// Do a quick distance check first
			if (General::distance(pos.x, pos.y, curr_pos.x, curr_pos.y) < 100) {
				if (!got_bones) {
					got_bones = true;
					// Don't call this with weapon true on a non-Player!
					if (weapon) {
						bones = ((Player *)entity)->get_current_weapon_bones();
					}
					else {
						entity->collidable_get_bones(bones);
					}
				}

				std::vector<Bones::Bone> bones2;
				curr_entity->collidable_get_bones(bones2);

				for (size_t j = 0; j < bones.size(); j++) {
					std::vector< General::Point<float> > outline;
					if (entity->is_facing_right()) {
						outline = bones[j].get_outline();
					}
					else {
						outline = bones[j].get_outline_mirrored();
					}

					for (size_t k = 0; k < bones2.size(); k++) {
						std::vector< General::Point<float> > outline2;
						if (curr_entity->is_facing_right()) {
							outline2 = bones2[k].get_outline();
						}
						else {
							outline2 = bones2[k].get_outline_mirrored();
						}

						bool b = checkcoll_polygon_polygon(
							outline,
							pos,
							outline2,
							curr_pos
						);

						if (b) {
							collides = true;
							break;
						}
					}

					if (collides) {
						break;
					}
				}
			}

			if (!collides) {
				continue;
			}
		}

		colliding_entities.push_back(curr_entity);
	}

	return colliding_entities;
}

void Area_Manager::shutdown()
{
	prepare_for_reshape();
	
	for (int i = 0; i < (int)floating.size(); i++) {
		if (floating[i].bitmap)
			Wrap::destroy_bitmap(floating[i].bitmap);
	}

	if (player_underlay_bitmap_add)
		Wrap::destroy_bitmap(player_underlay_bitmap_add);
	
	delete[] heightmap;

	for (int l = 0; l < (int)tile_groups.size(); l++) {
		for (int i = 0; i < (int)tile_groups[l].size(); i++) {
			delete tile_groups[l][i];
		}
		tile_groups[l].clear();
	}
	tile_groups.clear();

	for (int i = 0; i < num_layers; i++) {
		delete tiled_layers[i];
	}

	delete[] shadow_mask;

	destroy_sheets();

	if (cfg.water_shader) {
		Shader::destroy(water_shader);
	}
}

Map_Entity *Area_Manager::activate(Map_Entity *activator, General::Point<float> pos, int box_width, int box_length)
{
	int closest = -1;
	float closest_dist = (pixel_size.w > pixel_size.h) ? pixel_size.w : pixel_size.h;

	General::Direction direction = activator->get_direction();

	General::Point<float> topleft;
	General::Point<float> bottomright;

	if (direction == General::DIR_N) {
		topleft = General::Point<float>(pos.x-box_width/2, pos.y-box_length);
		bottomright = General::Point<float>(pos.x+box_width/2, pos.y);
	}
	else if (direction == General::DIR_E) {
		topleft = General::Point<float>(pos.x, pos.y-box_width/2);
		bottomright = General::Point<float>(pos.x+box_length, pos.y+box_width/2);
	}
	else if (direction == General::DIR_S) {
		topleft = General::Point<float>(pos.x-box_width/2, pos.y);
		bottomright = General::Point<float>(pos.x+box_width/2, pos.y+box_length);
	}
	else {
		topleft = General::Point<float>(pos.x-box_length, pos.y-box_width/2);
		bottomright = General::Point<float>(pos.x, pos.y+box_width/2);
	}

	int sz = entities.size();
	for (int i = 0; i < sz; i++) {
		Map_Entity *e = entities[i];
		if (e != activator && !General::is_hero(e->get_name())) {
			General::Point<float> epos = e->get_position();
			General::Size<float> size;
			if (e->collidable_get_type() == COLLIDABLE_BONES) {
				std::vector<Bones::Bone> bones;
				e->collidable_get_bones(bones);
				if (bones.size() > 0) {
					size = bones[0].get_extents();
				}
				else {
					size.w = 8;
					size.h = 8;
				}
			}
			else {
				size.w = 8;
				size.h = 8;
			}
			General::Point<float> topleft2(epos.x-size.w/2, epos.y-size.h);
			General::Point<float> bottomright2(epos.x+size.w/2, epos.y);
			General::log_message(General::itos(size.w) + " x " + General::itos(size.h));
			if (checkcoll_box_box(topleft, bottomright, topleft2, bottomright2)) {
				float dist = General::distance(pos.x, pos.y, epos.x, epos.y);
				if (dist < closest_dist) {
					closest_dist = dist;
					closest = i;
				}
			}
		}
	}

	if (closest >= 0) {
		return entities[closest];
	}

	return NULL;
}

Area_Manager::Area_Manager() :
	top(General::Point<float>(0, 0)),
	verts(NULL),
	lua_state(NULL),
	shadow_color(al_map_rgb_f(0.5, 0.5, 0.5)),
	isometric(false),
	clear_color(al_map_rgb(0, 0, 0)),
	parallax_x(false),
	parallax_y(false),
	early_break(false),
	in_speech_loop(false),
	rumble_offset(0, 0)
{
	player_underlay_bitmap_add = NULL;
	height_offset_dampened = 0.0f;
	
	for (int i = 0; i < 10; i++) {
		layer_toggled_off[i] = false;
	}
}
/* must come after destroying entities because npcs could be
 * A*'ing
 */
void Area_Manager::destroy_outline()
{
	if (collision_lines) {
		A_Star::end_area();
		for (int i = 0; i < num_layers; i++) {
			for (int j = 0; j < (int)nav_meshes[i].size(); j++) {
				for (int k = 0; k < 3; k++) {
					delete nav_links[i][j][k];
				}
				delete[] nav_links[i][j];
			}
			delete[] nav_links[i];
		}
		delete[] nav_links;
		delete[] collision_lines;
		delete[] outline_points;
		delete[] outline_splits;
		delete[] nav_meshes;
	}
}

Area_Manager::~Area_Manager()
{
	// These things can't go in shutdown

	if (lua_state)
		lua_close(lua_state);

	// Delete every entity except the player
	std::vector<Map_Entity *>::iterator it;
	for (it = entities.begin(); it != entities.end(); it++) {
		Map_Entity *entity = *it;

		if (dynamic_cast<Player *>(entity))
			continue;

		NPC *npc;
		if ((npc = dynamic_cast<NPC *>(entity)) != NULL) {
			std::string name = npc->get_name();
			if (General::is_hero(name) && !entity->get_delete_me()) {
				continue;
			}
		}

		delete entity;
	}

	destroy_outline();

	for (int l = 0; l < num_layers; l++) {
		for (int h = 0; h < size.h; h++) {
			delete[] is_grouped[l][h];
		}
		delete[] is_grouped[l];
		delete[] group_drawn[l];
	}
	delete[] is_grouped;
	delete[] group_drawn;
}

int Area_Manager::tile_x(int number)
{
	return number % width;
}

int Area_Manager::tile_y(int number)
{
	return number / width;
}

void Area_Manager::init_lua()
{
	lua_state = luaL_newstate();

	Lua::open_lua_libs(lua_state);

	Lua::register_c_functions(lua_state);

	Lua::load_global_scripts(lua_state);

	unsigned char *bytes;
	bytes = General::slurp("areas/global.lua");
	if (luaL_loadstring(lua_state, (char *)bytes)) {
		Lua::dump_lua_stack(lua_state);
		throw Error("Error loading global area script.\n");
	}
	delete[] bytes;

	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running global area script.");
	}
	
	bytes = General::slurp(path + "/script.inc");
	if (luaL_loadstring(lua_state, (char *)bytes)) {
		Lua::dump_lua_stack(lua_state);
		throw Error("Error loading area script include.\n");
	}
	delete[] bytes;

	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running area script include.");
	}

	bytes = General::slurp(path + "/script.lua");
	if (luaL_loadstring(lua_state, (char *)bytes)) {
		Lua::dump_lua_stack(lua_state);
		throw Error("Error loading area script.\n");
	}
	delete[] bytes;

	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running area script.");
	}

	lua_getglobal(lua_state, "is_dungeon");
	if (!lua_isnil(lua_state, -1)) {
		if (lua_toboolean(lua_state, -1) != 0) {
			_is_dungeon = true;
		}
		else {
			_is_dungeon = false;
		}
	}
	else {
		_is_dungeon = false;
	}
	lua_pop(lua_state, 1);

	// set some globals
	// set all directions to match C++ code ie DIR_S is set in all lua scripts
	for (int i = 0; i < General::NUM_DIRECTIONS; i++) {
		char buf[20];
		sprintf(buf, "dir_%s", General::direction_strings[i]);
		for (int j = 0; buf[j]; j++) {
			buf[j] = toupper(buf[j]);
		}
		lua_pushnumber(lua_state, i);
		lua_setglobal(lua_state, buf);
	}

	engine->delete_tweens();

	bool game_just_loaded = engine->get_game_just_loaded();
	if (game_just_loaded) {
		Lua::call_lua(lua_state, "load_level_state", ">");
	}
	Lua::call_lua(lua_state, "start", "b>", game_just_loaded);
	engine->set_game_just_loaded(false);

	Map_Entity *player = get_entity(0);
	player_start_position = player->get_position();
	player_start_layer = player->get_layer();

	is_grouped = new bool **[num_layers];
	for (int l = 0; l < num_layers; l++) {
		is_grouped[l] = new bool *[size.h];
		for (int y = 0; y < size.h; y++) {
			is_grouped[l][y] = new bool[size.w];
			memset(is_grouped[l][y], 0, sizeof(bool)*size.w);
		}
	}
	for (int l = 0; l < num_layers; l++) {
		for (size_t g = 0; g < tile_groups[l].size(); g++) {
			Tile_Group *tg = tile_groups[l][g];
			for (size_t t = 0; t < tg->tiles.size(); t++) {
				is_grouped[l][tg->tiles[t].y][tg->tiles[t].x] = true;
			}
		}
	}
		
	lua_getglobal(lua_state, "pre_draw_layer");
	script_has_pre_draw = !lua_isnil(lua_state, -1);
	lua_pop(lua_state, 1);
	lua_getglobal(lua_state, "mid_draw_layer");
	script_has_mid_draw = !lua_isnil(lua_state, -1);
	lua_pop(lua_state, 1);
	lua_getglobal(lua_state, "post_draw_layer");
	script_has_post_draw = !lua_isnil(lua_state, -1);
	lua_pop(lua_state, 1);
}

int Area_Manager::get_width()
{
	return width;
}

int Area_Manager::get_height()
{
	return height;
}

General::Size<int> Area_Manager::get_pixel_size()
{
	return pixel_size;
}

int Area_Manager::height_at(int x, int y)
{
	return 0;

	// height values are 4 bit
	int extra;
	if (pixel_size.w % 2 == 1)
		extra = 1;
	else
		extra = 0;

	int number;
	number = (y*(pixel_size.w+extra)+x);

	int byte = heightmap[number / 2];
	bool lo = number % 2 == 0;

	int value;

	if (lo) {
		value = byte & 0xf;
	}
	else {
		value = (byte >> 4) & 0xf;
	}

	return ((15-value)-7)*4;
}

Area_Manager::Area_Floating_Block Area_Manager::create_floating_block(int layer, int offset_x, int offset_y, bool pre)
{
	Area_Floating_Block b;
	
	b.bitmap = NULL;
	b.layer = layer;
	b.offset_x = offset_x;
	b.offset_y = offset_y;
	b.pre = pre;

	return b;
}

void Area_Manager::post_process_floating_block(Area_Floating_Block *b)
{
	Floating_Layer_Info &info = find_floating_info(b->layer);

	int minx = INT_MAX, maxx = INT_MIN;
	int miny = INT_MAX, maxy = INT_MIN;

	for (int i = 0; i < (int)floating.size(); i++) {
		Area_Floating_Block &tmp = floating[i];
		if (tmp.layer != b->layer)
			continue;
		if (tmp.offset_x < minx)
			minx = tmp.offset_x;
		if (tmp.offset_x+tmp.width > maxx)
			maxx = tmp.offset_x+tmp.width;
		if (tmp.offset_y < miny)
			miny = tmp.offset_y;
		if (tmp.offset_y+tmp.height > maxy)
			maxy = tmp.offset_y+tmp.height;
	}

	info.min_x = minx;
	info.min_y = miny;
	info.total_width = maxx - minx;
	info.total_height = maxy - miny;
}

void Area_Manager::add_floating_image(Wrap::Bitmap *bitmap, int layer, int offset_x, int offset_y, bool pre, bool subtractive)
{
	Area_Floating_Block fi = create_floating_block(layer, offset_x, offset_y, pre);

	fi.type = FLOATING_IMAGE;

	fi.bitmap = bitmap;
	fi.width = al_get_bitmap_width(bitmap->bitmap);
	fi.height = al_get_bitmap_height(bitmap->bitmap);
	fi.subtractive = subtractive;
	floating.push_back(fi);

	post_process_floating_block(&fi);
}

void Area_Manager::add_floating_rectangle(ALLEGRO_COLOR tl, ALLEGRO_COLOR tr, ALLEGRO_COLOR br, ALLEGRO_COLOR bl, int width, int height, int layer, int offset_x, int offset_y, bool pre)
{
	Area_Floating_Block r = create_floating_block(layer, offset_x, offset_y, pre);

	r.type = FLOATING_RECTANGLE;

	r.width = width;
	r.height = height;
	r.tl = tl;
	r.tr = tr;
	r.br = br;
	r.bl = bl;
	floating.push_back(r);

	post_process_floating_block(&r);
}

void Area_Manager::add_outline_point(int layer, int x, int y)
{
	outline_points[layer].push_back(General::Point<float>((float)x, (float)y));

	if (outline_splits[layer].size() > 0 && outline_splits[layer][outline_splits[layer].size()-1] == (int)outline_points[layer].size()-1)
		return;
	if (outline_points[layer].size() == 1)
		return;

	General::Point<float> &p1 = outline_points[layer][outline_points[layer].size()-2];
	General::Point<float> &p2 = outline_points[layer][outline_points[layer].size()-1];
	General::Line<float> l;
	l.x1 = p1.x;
	l.y1 = p1.y;
	l.x2 = p2.x;
	l.y2 = p2.y;
	collision_lines[layer].push_back(l);
}

void Area_Manager::add_outline_split(int layer, int index)
{
	General::Point<float> p = outline_points[layer][outline_points[layer].size()-1];

	General::Line<float> l;
	l.x1 = p.x;
	l.y1 = p.y;

	if (outline_splits[layer].size() == 0) {
		p = outline_points[layer][0];
	}
	else {
		p = outline_points[layer][outline_splits[layer][outline_splits[layer].size()-1]];
	}

	l.x2 = p.x;
	l.y2 = p.y;

	collision_lines[layer].push_back(l);
	outline_splits[layer].push_back(index);
}

int Area_Manager::get_num_entities()
{
	return entities.size();
}

int Area_Manager::get_entity_id_by_number(int number)
{
	return entities[number]->get_id();
}
	
Area_Manager::Floating_Layer_Info &Area_Manager::find_floating_info(int layer)
{
	for (int i = 0; i < (int)floating_info.size(); i++) {
		if (floating_info[i].layer == layer)
			return floating_info[i];
	}

	Floating_Layer_Info info;
	info.layer = layer;
	floating_info.push_back(info);
	return floating_info[floating_info.size()-1];
}

void Area_Manager::set_player_underlay_bitmap_add(Wrap::Bitmap *bmp, int layer, bool top_also)
{
	player_underlay_bitmap_add = bmp;
	player_underlay_layer = layer;
	player_underlay_top_also = top_also;
}

void Area_Manager::link_triangles()
{
	for (int i = 0; i < num_layers; i++) {
		int sz = nav_meshes[i].size();
		for (int j = 0; j < sz; j++) {
			Triangulate::Triangle *t1 = &nav_meshes[i][j];
			int sz2 = nav_meshes[i].size();
			for (int k = 0; k < sz2; k++) {
				Triangulate::Triangle *t2 = &nav_meshes[i][k];
				if (t1 == t2)
					continue;
				for (int l = 0; l < 3; l++) {
					General::Point<float> *t1_p1 = &t1->points[l];
					General::Point<float> *t1_p2 = &t1->points[(l+1)%3];
					for (int m = 0; m < 3; m++) {
						General::Point<float> *t2_p1 = &t2->points[m];
						General::Point<float> *t2_p2 = &t2->points[(m+1)%3];
						if ((*t1_p1 == *t2_p1 && *t1_p2 == *t2_p2) || (*t1_p2 == *t2_p1 && *t1_p1 == *t2_p2)) {
							nav_links[i][j][l]->triangle = t2;
							nav_links[i][j][l]->edge_start = m;
							nav_links[i][j][l]->triangle_num = k;
							break;
						}
					}
				}
			}
		}
	}
}

std::vector<Triangulate::Triangle> *Area_Manager::get_nav_meshes()
{
	return nav_meshes;
}

A_Star::Navigation_Link ****Area_Manager::get_nav_links()
{
	return nav_links;
}

std::vector< General::Line<float> > *Area_Manager::get_collision_lines()
{
	return collision_lines;
}

static void subdivide(
	std::vector<Triangulate::Triangle> &in,
	std::vector<Triangulate::Triangle> &out
)
{
	std::vector<Triangulate::Triangle> work = in;
	Triangulate::Triangle out_tri;

	while (work.size() > 0) {
		Triangulate::Triangle t = work[0];
		Triangulate::Triangle t2;
		// find shared edge if there is one
		bool found = false;
		int counted[2][2];
		// start at 1 so never the same two tris tested
		int sz = work.size();
		int i;
		for (i = 1; i < sz; i++) {
			t2 = work[i];
			int count = 0;
			for (int j = 0; j < 3; j++) {
				for (int k = 0; k < 3; k++) {
					if (t.points[j] == t2.points[k]) {
						counted[0][count] = j;
						counted[1][count] = k;
						count++;
						if (count == 2) {
							break;
						}
					}
				}
				if (count == 2) {
					break;
				}
			}
			if (count == 2) {
				found = true;
				break;
			}
		}
		General::Point<float> p;
		if (found) {
			ERASE_FROM_UNSORTED_VECTOR(work, i);
			ERASE_FROM_UNSORTED_VECTOR(work, 0);

			int not_counted = 0;

			int possible1[3] = { 0, 1, 2 };
			for (int i = 0; i < 2; i++) {
				for (int j = 0; j < 3; j++) {
					if (counted[0][i] == possible1[j]) {
						possible1[j] = -1;
						break;
					}
				}
			}
			for (int i = 0; i < 3; i++) {
				if (possible1[i] != -1) {
					not_counted = possible1[i];
					break;
				}
			}

			p.x = (t.points[counted[0][0]].x + t.points[counted[0][1]].x) / 2;
			p.y = (t.points[counted[0][0]].y + t.points[counted[0][1]].y) / 2;
			out_tri.points[0] = t.points[counted[0][0]];
			out_tri.points[1] = t.points[not_counted];
			out_tri.points[2] = p;
			out.push_back(out_tri);
			out_tri.points[0] = p;
			out_tri.points[1] = t.points[not_counted];
			out_tri.points[2] = t.points[counted[0][1]];
			out.push_back(out_tri);

			int possible2[3] = { 0, 1, 2 };
			for (int i = 0; i < 2; i++) {
				for (int j = 0; j < 3; j++) {
					if (counted[1][i] == possible2[j]) {
						possible2[j] = -1;
						break;
					}
				}
			}
			for (int i = 0; i < 3; i++) {
				if (possible2[i] != -1) {
					not_counted = possible2[i];
					break;
				}
			}

			out_tri.points[0] = t2.points[counted[1][0]];
			out_tri.points[1] = t2.points[not_counted];
			out_tri.points[2] = p;
			out.push_back(out_tri);
			out_tri.points[0] = p;
			out_tri.points[1] = t2.points[not_counted];
			out_tri.points[2] = t2.points[counted[1][1]];
			out.push_back(out_tri);
		}
		else {
			ERASE_FROM_UNSORTED_VECTOR(work, 0);

			p.x = (t.points[0].x + t.points[1].x) / 2;
			p.y = (t.points[0].y + t.points[1].y) / 2;
			out_tri.points[0] = t.points[0];
			out_tri.points[1] = t.points[2];
			out_tri.points[2] = p;
			out.push_back(out_tri);
			out_tri.points[0] = p;
			out_tri.points[1] = t.points[2];
			out_tri.points[2] = t.points[1];
			out.push_back(out_tri);
		}
	}
}

void Area_Manager::process_outline()
{
	// First add lines to small quadrants for quick collision detection
	int nquads_w = ceil((float)pixel_size.w / QUADRANT_SIZE);
	int nquads_h = ceil((float)pixel_size.h / QUADRANT_SIZE);
	for (int layer = 0; layer < num_layers; layer++) {
		quadrants.push_back(std::vector< std::vector< std::vector< std::pair< General::Point<float>, General::Point<float> > > > >());
		for (int y = 0; y < nquads_h; y++) {
			quadrants[layer].push_back(std::vector< std::vector< std::pair< General::Point<float>, General::Point<float> > > >());
			for (int x = 0; x < nquads_w; x++) {
				quadrants[layer][y].push_back(std::vector< std::pair< General::Point<float>, General::Point<float> > >());
			}
		}
		int sz = collision_lines[layer].size();
		for (int i = 0; i < sz; i++) {
			float x1 = MAX(MIN(pixel_size.w-1, collision_lines[layer][i].x1), 0);
			float y1 = MAX(MIN(pixel_size.h-1, collision_lines[layer][i].y1), 0);
			float x2 = MAX(MIN(pixel_size.w-1, collision_lines[layer][i].x2), 0);
			float y2 = MAX(MIN(pixel_size.h-1, collision_lines[layer][i].y2), 0);
			General::Point<float> p1(x1, y1);
			General::Point<float> p2(x2, y2);
			for (int y = 0; y < nquads_h; y++) {
				for (int x = 0; x < nquads_w; x++) {
					int x3 = x * QUADRANT_SIZE;
					int y3 = y * QUADRANT_SIZE;
					int x4 = MIN(x3+QUADRANT_SIZE, pixel_size.w);
					int y4 = MIN(y3+QUADRANT_SIZE, pixel_size.h);
					General::Point<float> a(x3, y3);
					General::Point<float> b(x4, y3);
					General::Point<float> c(x4, y4);
					General::Point<float> d(x3, y4);
					if (
						checkcoll_line_line(&p1, &p2, &a, &b, NULL) ||
						checkcoll_line_line(&p1, &p2, &b, &c, NULL) ||
						checkcoll_line_line(&p1, &p2, &c, &d, NULL) ||
						checkcoll_line_line(&p1, &p2, &d, &a, NULL) ||
						checkcoll_point_box(p1, a, c) ||
						checkcoll_point_box(p2, a, c))
					{
						quadrants[layer][y][x].push_back(
							std::pair< General::Point<float>, General::Point<float> >(
								General::Point<float>(x1, y1),
								General::Point<float>(x2, y2)
							)
						);
					}
				}
			}
		}
	}

	int start_compare;
	int end_compare;
	std::vector< General::Point<float> > tmp_pts;
	std::vector<int> tmp_splits;

	for (int i = 0; i < num_layers; i++) {
		if (outline_points[i].size() >= 3) {
			start_compare = 0;
			end_compare = -1;
			tmp_pts.clear();
			tmp_splits.clear();
			int sz = outline_splits[i].size();
			for (int j = 0; j < sz; j++) {
				int k = (j == 0) ? 0 : outline_splits[i][j-1];
				for (; k < outline_splits[i][j]; k++) {
					tmp_pts.push_back(outline_points[i][k]);
				}
				if (end_compare == -1)
					end_compare = outline_splits[i][j];
				tmp_splits.push_back(outline_splits[i][j]-start_compare);
			}
			Triangulate::get_triangles(tmp_pts, tmp_splits, nav_meshes[i]);
		}
	}

	const int subdivisions = 0;
	for (int sub = 0; sub < subdivisions; sub++) {
		for (int n = 0; n < num_layers; n++) {
			// subdivide into `mesh'!
			std::vector<Triangulate::Triangle> mesh;

			subdivide(nav_meshes[n], mesh);

			nav_meshes[n] = mesh;
		}
	}

	nav_links = new A_Star::Navigation_Link ***[num_layers];
	for (int i = 0; i < num_layers; i++) {
		nav_links[i] = new A_Star::Navigation_Link **[nav_meshes[i].size()];
		int sz = nav_meshes[i].size();
		for (int j = 0; j < sz; j++) {
			nav_links[i][j] = new A_Star::Navigation_Link *[3];
			for (int k = 0; k < 3; k++) {
				nav_links[i][j][k] = new A_Star::Navigation_Link;
				nav_links[i][j][k]->triangle = NULL;
				nav_links[i][j][k]->triangle_num = -1;
			}
		}
	}
	
	link_triangles();

	A_Star::start_area(nav_meshes, nav_links, num_layers);

}

void Area_Manager::reset_outline()
{
	destroy_outline();
	create_outline_storage();
}

bool Area_Manager::is_dungeon()
{
	return _is_dungeon;
}

void Area_Manager::set_shadow_color(float r, float g, float b)
{
	shadow_color = al_map_rgb_f(r, g, b);
}

/*
// create a pixel for pixel heightmap from a tile based one
void Area_Manager::load_isometric_heightmap(std::string filename)
{
	int pitch = pixel_size.w / 2 + pixel_size.w % 2;
	heightmap = new unsigned char [pitch*pixel_size.h];
	memset(heightmap, 8, pitch*pixel_size.h);

	int tile_pitch = width / 2 + width % 2;
	unsigned char *tile_heightmap = General::slurp("heightmap");

	for (int y = 0; y < pixel_size.h; y++) {
		for (int x = 0; x < pixel_size.w; x++) {
			int tmp_x = x;
			int tmp_y = y;
			General::reverse_iso_project(&tmp_x, &tmp_y, iso_offset);
			if (tmp_x < 0 || tmp_y < 0 || tmp_x >= width*General::TILE_SIZE || tmp_y >= height*General::TILE_SIZE)
				continue;
			tmp_x /= General::TILE_SIZE;
			bool tile_use_low = tmp_x % 2 == 0;
			tmp_x /= 2;
			tmp_y /= General::TILE_SIZE;
			int tile_byte = tile_heightmap[tmp_y*tile_pitch+tmp_x];
			int hm_offset = y * pitch + x / 2;
			int hm_curr = heightmap[hm_offset];
			bool hm_use_low = x % 2 == 0;
			int height;
			if (tile_use_low) {
				height = tile_byte & 0xf;
			}
			else {
				height = (tile_byte >> 4) & 0xf;
			}
			if (hm_use_low) {
				hm_curr = (hm_curr & 0xf0) | height;
			}
			else {
				hm_curr = (hm_curr & 0xf) | (height << 4);
			}
			heightmap[hm_offset] = hm_curr;
		}
	}

	delete[] tile_heightmap;
}
*/
	
void Area_Manager::set_clear_color(float r, float g, float b)
{
	clear_color = al_map_rgb_f(r, g, b);
}

ALLEGRO_COLOR Area_Manager::get_clear_color()
{
	return clear_color;
}

void Area_Manager::set_parallax_params(bool parallax_x, bool parallax_y)
{
	this->parallax_x = parallax_x;
	this->parallax_y = parallax_y;
}

void Area_Manager::add_tile_group(Tile_Group input)
{
	Tile_Group *group = new Tile_Group;
	group->layer = input.layer;
	group->top_left = input.top_left;
	group->tiles = input.tiles;
	group->size = input.size;
	group->flags = input.flags;
	group->duration = input.duration;

	int top = INT_MAX;
	int left = INT_MAX;
	int bottom = -1;
	int right = -1;
	int sz = input.tiles.size();
	for (int i = 0; i < sz; i++) {
		int x1 = input.tiles[i].x * General::TILE_SIZE;
		int y1 = input.tiles[i].y * General::TILE_SIZE;
		int x2 = input.tiles[i].x * General::TILE_SIZE + General::TILE_SIZE;
		int y2 = input.tiles[i].y * General::TILE_SIZE + General::TILE_SIZE;
		if (x1 < left) {
			left = x1;
		}
		if (y1 < top) {
			top = y1;
		}
		if (x2 > right) {
			right = x2;
		}
		if (y2 > bottom) {
			bottom = y2;
		}
	}

	group->top_left_tile_pixel = General::Point<int>(left, top);
	group->bottom_right_tile_pixel = General::Point<int>(right, bottom);

	if (!isometric) {
		tile_groups[group->layer].push_back(group);
		return;
	}

	int botx = input.top_left.x;
	int boty = input.top_left.y;
	General::reverse_iso_project(&botx, &boty, iso_offset);
	botx += input.size.w;
	boty += input.size.h;
	General::iso_project(&botx, &boty, iso_offset);
	group->bottom_right2.x = botx;
	group->bottom_right2.y = boty;

	if (input.size.w < input.size.h) {
		int botx = input.top_left.x;
		int boty = input.top_left.y;
		General::reverse_iso_project(&botx, &boty, iso_offset);
		boty += input.size.h;
		General::iso_project(&botx, &boty, iso_offset);
		group->bottom_right.x = botx;
		group->bottom_right.y = boty;

		int topx = input.top_left.x;
		int topy = input.top_left.y;
		General::reverse_iso_project(&topx, &topy, iso_offset);
		topx += input.size.w;
		General::iso_project(&topx, &topy, iso_offset);
		group->top_left2.x = topx;
		group->top_left2.y = topy;
	}
	else {
		int botx = input.top_left.x;
		int boty = input.top_left.y;
		General::reverse_iso_project(&botx, &boty, iso_offset);
		botx += input.size.w;
		General::iso_project(&botx, &boty, iso_offset);
		group->bottom_right.x = botx;
		group->bottom_right.y = boty;

		int topx = input.top_left.x;
		int topy = input.top_left.y;
		General::reverse_iso_project(&topx, &topy, iso_offset);
		topy += input.size.h;
		General::iso_project(&topx, &topy, iso_offset);
		group->top_left2.x = topx;
		group->top_left2.y = topy;
	}

	tile_groups[group->layer].push_back(group);
}

General::Point<float> Area_Manager::get_iso_offset()
{
	return iso_offset;
}

General::Point<int> Area_Manager::get_entity_iso_pos(int id)
{
	Map_Entity *e = get_entity(id);
	if (!e) return General::Point<int>(-1, -1);

	General::Point<float> pos = e->get_position();
	int ex = pos.x;
	int ey = pos.y;
	General::reverse_iso_project(&ex, &ey, iso_offset);

	if (ex < 0 || ey < 0 || ex >= width*General::TILE_SIZE || ey >= height*General::TILE_SIZE) {
		return General::Point<int>(-1, -1);
	}

	return General::Point<int>(ex/General::TILE_SIZE, ey/General::TILE_SIZE);
}

bool Area_Manager::is_isometric()
{
	return isometric;
}

void Area_Manager::calc_tile_bounds(int sheet)
{
	Wrap::Bitmap *b = tile_sheets[sheet];
	int wt = al_get_bitmap_width(b->bitmap) / General::TILE_SIZE;
	int ht = al_get_bitmap_height(b->bitmap) / General::TILE_SIZE;
	
#if 0 // OVERKILL
	unsigned char target[2*General::TILE_SIZE] = { 0, };
#endif

	tile_bounds.push_back(std::vector<Tile_Bounds>());
	for (int y = 0; y < ht; y++) {
		for (int x = 0; x < wt; x++) {
			Tile_Bounds bounds;
			/* See: OVERKILL!
			bounds.top_left = General::Point<int>(0, -1);
			bounds.size = General::Size<int>(General::TILE_SIZE, 0);
			*/
			bounds.top_left = General::Point<int>(0, 0);
			bounds.size = General::Size<int>(General::TILE_SIZE, General::TILE_SIZE);
			tile_bounds[sheet].push_back(bounds);
		}
	}

#if 0 // OVERKILL!
	ALLEGRO_LOCKED_REGION *lr = al_lock_bitmap(b, ALLEGRO_PIXEL_FORMAT_RGBA_5551, ALLEGRO_LOCK_READONLY);

	// scan tops
	for (int py = 0; py < General::TILE_SIZE; py++) {
		for (int y = 0; y < ht; y++) {
			unsigned char *p = (unsigned char *)lr->data + lr->pitch*(y*General::TILE_SIZE+py);
			for (int x = 0; x < wt; x++) {
				if (tile_bounds[sheet][y*wt+x].top_left.y == -1) {
					if (memcmp(p, target, 2*General::TILE_SIZE)) {
						tile_bounds[sheet][y*wt+x].top_left.y = py;
					}
					else if (py == General::TILE_SIZE-1) {
						tile_bounds[sheet][y*wt+x].top_left.y = 0;
						tile_bounds[sheet][y*wt+x].size.h = General::TILE_SIZE;
					}
				}
				p += 2*General::TILE_SIZE;
			}
		}
	}

	// scan bottoms
	for (int py = General::TILE_SIZE-1; py >= 0; py--) {
		for (int y = 0; y < ht; y++) {
			unsigned char *p = (unsigned char *)lr->data + lr->pitch*(y*General::TILE_SIZE+py);
			for (int x = 0; x < wt; x++) {
				if (tile_bounds[sheet][y*wt+x].size.h == 0) {
					if (memcmp(p, target, 2*General::TILE_SIZE)) {
						tile_bounds[sheet][y*wt+x].size.h =
							(py - tile_bounds[sheet][y*wt+x].top_left.y) + 1;
					}
				}
				p += 2*General::TILE_SIZE;
			}
		}
	}
	

	/*
	printf("---------------------------------\n");
	for (int y = 0; y < ht; y++) {
		for (int x = 0; x < wt; x++) {
			Tile_Bounds &tb = tile_bounds[sheet][y*wt+x];
			printf("%d,%d = %d,%d %dx%d\n", x, y, tb.top_left.x, tb.top_left.y, tb.size.w, tb.size.h);
		}
	}
	*/

	// scan lefts?
	// scan rights?

	al_unlock_bitmap(b);
#endif
}

void Area_Manager::reset_top()
{
	Map_Entity *player = get_entity(0);
	General::Point<float> player_pos = player->get_position();
	top.x = player_pos.x - cfg.screen_w/2;
	top.y = player_pos.y - cfg.screen_h/2;

	General::Size<int> zero_size(0, 0);
	General::Point<float> zero_move(0, 0);
	move_camera(zero_move, get_entity(0)->get_position(), false);
}

std::vector<Tiled_Layer *>& Area_Manager::get_tiled_layers()
{
	return tiled_layers;
}

std::string Area_Manager::get_path()
{
	return path;
}

void Area_Manager::set_early_break(bool early_break)
{
	this->early_break = early_break;
}

void Area_Manager::toggle_layer(int layer)
{
	if (layer >= 0 && layer <= 9)
		layer_toggled_off[layer] = !layer_toggled_off[layer];
}

// removes invisible tiles
void Area_Manager::optimize_tiled_area()
{
// FIXME
return;
	int count = 0;
	
	for (size_t i = 0; i < tile_sheets.size(); i++) {
		al_lock_bitmap(tile_sheets[i]->bitmap, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
	}

	for (int l = 0; l < num_layers-1; l++) {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				Tiled_Layer::Tile &t = tiled_layers[l]->tiles[y][x];	
				if (t.number < 0 || t.sheet < 0) continue;
				int n = t.number;
				int wt = SHEET_SIZE / General::TILE_SIZE;
				wt -= (wt * 2) / General::TILE_SIZE;
				int sx = n % wt;
				sx = sx * General::TILE_SIZE + (sx * 2) + 1;
				int sy = n / wt;
				sy = sy * General::TILE_SIZE + (sy * 2) + 1;
				bool visible = false;
				for (int yy = 0; yy < General::TILE_SIZE; yy++) {
					for (int xx = 0; xx < General::TILE_SIZE; xx++) {
						// seven tiles surrounding (and on) the current tile could obscure it
						// tile offs x, y, pixel offs x, y
						const int offs[11][4] = {
							{ 0, 0, 0, 0 },
							{ -1, 0, -General::TILE_SIZE/2, -General::TILE_SIZE/4 }, // left
							{ 1, 0, General::TILE_SIZE/2, General::TILE_SIZE/4 }, // right
							{ 0, -1, General::TILE_SIZE/2, -General::TILE_SIZE/4 }, // up
							{ 0, 1, -General::TILE_SIZE/2, General::TILE_SIZE/4, }, // down
							{ -1, -1, 0, -General::TILE_SIZE/2 }, // up left
							{ 1, 1, 0, General::TILE_SIZE/2 }, // down right
							{ 2, 1, General::TILE_SIZE/2, General::TILE_SIZE*3/4 },
							{ 1, 2, -General::TILE_SIZE/2, General::TILE_SIZE*3/4 },
							{ -1, -2, General::TILE_SIZE/2, -General::TILE_SIZE*3/4 },
							{ -2, -1, -General::TILE_SIZE/2, -General::TILE_SIZE*3/4 }
						};
						bool v = true;
						for (int l2 = l+1; l2 < num_layers; l2++) {
							bool broke = false;
							int num_to_process;
							if (isometric) {
								num_to_process = 11;
							}
							else {
								num_to_process = 1;
							}
							for (int o = 0; o < num_to_process; o++) {
								if (y+offs[o][1] < 0 || y+offs[o][1] >= height || x+offs[o][0] < 0 || x+offs[o][0] >= width) {
									continue;
								}
								if (xx-offs[o][2] < 0 || xx-offs[o][2] >= General::TILE_SIZE || yy-offs[o][3] < 0 || yy-offs[o][3] >= General::TILE_SIZE) {
									continue;
								}
								Tiled_Layer::Tile &t2 =
									tiled_layers[l2]->tiles[y+offs[o][1]][x+offs[o][0]];
								if (t2.number < 0 || t2.sheet < 0) continue;
								int n2 = t2.number;
								int sx2 = n2 % wt;
								sx2 = sx2 * General::TILE_SIZE + (sx2 * 2) + 1;
								int sy2 = n2 / wt;
								sy2 = sy2 * General::TILE_SIZE + (sy2 * 2) + 1;
								ALLEGRO_BITMAP *curr2 = tile_sheets[t2.sheet]->bitmap;
								ALLEGRO_COLOR p2 = al_get_pixel(
									curr2,
									sx2+xx-offs[o][2],
									sy2+yy-offs[o][3]
								);
								if (p2.a == 1) {
									v = false;
									broke = true;
									break;
								}
							}
							if (broke) {
								break;
							}
						}
						if (v) {
							visible = true;
							break;
						}
					}
					if (visible) break;
				}
				if (!visible && !tile_is_grouped(l, x, y)) {
					tiled_layers[l]->tiles[y][x].number = -1;
					tiled_layers[l]->tiles[y][x].sheet = -1;
					count++;
				}
			}
		}
	}

	for (size_t i = 0; i < tile_sheets.size(); i++) {
		al_unlock_bitmap(tile_sheets[i]->bitmap);
	}

	General::log_message("Removed " + General::itos(count) + " invisible tiles\n");
}

bool Area_Manager::reposition_entity_in_vector(int id, int before)
{
	Map_Entity *e = get_entity(id);
	if (e) {
		std::vector<Map_Entity *>::iterator it = std::find(entities.begin(), entities.end(), e);
		if (it != entities.end()) {
			entities.erase(it);
			if (before == -1) {
				entities.push_back(e);
			}
			else {
				entities.insert(entities.begin()+before, e);
			}
			return true;
		}
	}

	return false;
}

void Area_Manager::add_ladder(int layer, General::Point<float> topleft, General::Point<float> bottomright)
{
	Ladder l;
	l.layer = layer;
	l.topleft = topleft;
	l.bottomright = bottomright;
	ladders.push_back(l);
}

bool Area_Manager::ladder_is_colliding(int layer, Map_Entity *entity)
{
	for (size_t i = 0; i < ladders.size(); i++) {
		Ladder &l = ladders[i];
		if (l.layer != layer) {
			continue;
		}
		General::Point<float> pos = entity->get_position();
		std::vector<Bones::Bone> bones;
		entity->collidable_get_bones(bones);
		for (size_t j = 0; j < bones.size(); j++) {
			General::Size<float> size = bones[j].get_extents();
			General::Point<float> tl(
				pos.x - size.w / 2,
				pos.y - size.h
			);
			General::Point<float> br(
				pos.x + size.w / 2,
				pos.y
			);
			if (checkcoll_box_box(tl, br, l.topleft, l.bottomright)) {
				return true;
			}
		}
	}

	return false;
}
	
void Area_Manager::add_no_enemy_zone(General::Rectangle<int> zone)
{
	no_enemy_zones.push_back(zone);
}

std::vector< General::Rectangle<int> > &Area_Manager::get_no_enemy_zones()
{
	return no_enemy_zones;
}

bool Area_Manager::point_is_in_no_enemy_zone(int x, int y)
{
	bool found = false;

	for (size_t i = 0; i < no_enemy_zones.size(); i++) {
		General::Rectangle<int> &r = no_enemy_zones[i];
		int x1 = r.x1 * General::TILE_SIZE;
		int y1 = r.y1 * General::TILE_SIZE;
		int x2 = r.x2 * General::TILE_SIZE + General::TILE_SIZE;
		int y2 = r.y2 * General::TILE_SIZE + General::TILE_SIZE;
		if (x >= x1 && x < x2 && y >= y1 && y < y2) {
			found = true;
			break;
		}
	}

	return found;
}

General::Point<float> Area_Manager::get_player_start_position()
{
	return player_start_position;
}

int Area_Manager::get_player_start_layer()
{
	return player_start_layer;
}

void Area_Manager::remove_entity_from_vector(Entity *e)
{
	int sz = entities.size();
	for (int i = 0; i < sz; i++) {
		if (e == entities[i]) {
			entities.erase(entities.begin()+i);
			break;
		}
	}
}

void Area_Manager::remove_entity_from_vector(int id)
{
	remove_entity_from_vector(get_entity(id));
}

std::vector< General::Line<float> > Area_Manager::get_quadrant(int layer, General::Point<float> pos)
{
	std::vector< General::Line<float> > v;

	if (pos.x < 0) pos.x = 0;
	if (pos.y < 0) pos.y = 0;
	if (pos.x >= pixel_size.w) pos.x = pixel_size.w-1;
	if (pos.y >= pixel_size.h) pos.y = pixel_size.h-1;

	int qx = pos.x / QUADRANT_SIZE;
	int qy = pos.y / QUADRANT_SIZE;

	int sz = quadrants[layer][qy][qx].size();
	for (int i = 0; i < sz; i++) {
		General::Point<float> p1 = quadrants[layer][qy][qx][i].first;
		General::Point<float> p2 = quadrants[layer][qy][qx][i].second;
		General::Line<float> l;
		l.x1 = p1.x;
		l.y1 = p1.y;
		l.x2 = p2.x;
		l.y2 = p2.y;
		v.push_back(l);
	}

	return v;
}

void Area_Manager::destroy_sheets()
{
	for (size_t i = 0; i < tile_sheets.size(); i++) {
		Wrap::destroy_bitmap(tile_sheets[i]);
	}
	Wrap::destroy_bitmap(water_sheets[0]);

	tile_sheets.clear();
	water_sheets.clear();
}

void Area_Manager::load_sheets()
{
	ALLEGRO_FILE *f = engine->get_cpa()->load(path + "/info");
	char line[1000];
	al_fgets(f, line, 1000);
	al_fclose(f);
	char tilemapdir[1000];
	sscanf(line, "%s", tilemapdir);

	for (int i = 0; i < 256; i++) {
		char tile_filename[500];
		sprintf(tile_filename, "areas/tiles/%s/tiles%d.cpi", tilemapdir, i);
		Wrap::Bitmap *sheet = Wrap::load_bitmap(tile_filename);
		if (!sheet)
			break;
		tile_sheets.push_back(sheet);
		if (i == 0) {
			sheet = Wrap::create_bitmap_no_preserve(al_get_bitmap_width(sheet->bitmap), al_get_bitmap_height(sheet->bitmap));
		}
		water_sheets.push_back(sheet);
	}
}
	
bool Area_Manager::get_in_speech_loop()
{
	return in_speech_loop;
}

void Area_Manager::set_in_speech_loop(bool in_speech_loop)
{
	this->in_speech_loop = in_speech_loop;
}

void Area_Manager::add_particle(Particle::Particle *p)
{
	particles.push_back(p);
}

void Area_Manager::remove_particle(int id)
{
	int sz = particles.size();
	for (int i = 0; i < sz; i++) {
		if (particles[i]->get_id() == id) {
			particles[i]->set_delete_me(true);
		}
	}
}

Particle::Particle *Area_Manager::get_particle(int id)
{
	int sz = particles.size();
	for (int i = 0; i < sz; i++) {
		if (particles[i]->get_id() == id) {
			return particles[i];
		}
	}

	return NULL;
}

bool Area_Manager::point_collides(int layer, General::Point<float> p)
{
	bool collides_with_outer = checkcoll_point_polygon(p, outline_points[layer], General::Point<float>(0, 0), 0, outline_splits[layer][0]);
	if (collides_with_outer) {
		int sz = outline_splits[layer].size();
		for (int i = 1; i < sz; i++) {
			if (checkcoll_point_polygon(p, outline_points[layer], General::Point<float>(0, 0), outline_splits[layer][i-1], outline_splits[layer][i])) {
				return false;
			}
		}
		return true;
	}
	else {
		return false;
	}
}

void Area_Manager::set_rumble_offset(General::Point<float> offset)
{
	rumble_offset = offset;
}
