#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <cfloat>
#include <cmath>
#include <cstdio>

#include "widgets.h"
#include "battled2.h"

void B_TileSelector::set_draw(bool do_draw)
{
	this->do_draw = do_draw;
}

ATLAS *B_TileSelector::getAtlas(void)
{
	return atlas;
}

int B_TileSelector::getSelected(void)
{
	return selected;
}

std::string B_TileSelector::getName(int id)
{
	return tile_names[id];
}

int B_TileSelector::getIndex(std::string name)
{
	// get the id
	int id;
	std::map<int, std::string>::iterator it = tile_names.begin();
	while (it != tile_names.end()) {
		const std::pair<int, std::string> &p = *it;
		if (p.second == name) {
			id = p.first;
			break;
		}
		it++;
	}

	if (it == tile_names.end())
		return 0;
	
	// find index of id
	int num = atlas_get_num_items(atlas);
	for (int i = 0; i < num; i++) {
		ATLAS_ITEM *item = atlas_get_item_by_index(atlas, i);
		int this_id = atlas_get_item_id(item);
		if (this_id == id) {
			return i;
		}
	}
	
	return 0;
}

float B_TileSelector::getScale(void)
{
	float pos = tile_slider->getPosition();
	float scale = 4 * pos + 0.5;
	return scale;
}

void B_TileSelector::draw(int abs_x, int abs_y)
{
	if (!do_draw) {
		return;
	}

	float scale = getScale();
	width = start_w * scale;
	height = start_h * scale;

	for (int i = 0; i < atlas_get_num_sheets(atlas); i++) {
		ALLEGRO_BITMAP *bmp = atlas_get_sheet(atlas, i)->bitmap;
		al_draw_scaled_bitmap(
			bmp, 0, 0, 1024, 1024, abs_x, abs_y+1024*scale*i,
			1024*scale, 1024*scale,
			0
		);
	}
		
	ATLAS_ITEM *item = atlas_get_item_by_index(atlas, selected);
	ALLEGRO_BITMAP *sub = atlas_get_item_sub_bitmap(item)->bitmap;
	int sheet = atlas_get_item_sheet(item);
	int yofs = sheet*1024*scale;
	int tx = atlas_get_item_x(item)*scale;
	int ty = atlas_get_item_y(item)*scale;
	int tw = al_get_bitmap_width(sub)*scale;
	int th = al_get_bitmap_height(sub)*scale;

	al_draw_filled_triangle(abs_x+tx, abs_y+yofs+ty+th,
		abs_x+tx+tw, abs_y+yofs+ty+th,
		abs_x+tx+tw, abs_y+yofs+ty,
		al_map_rgba(128, 128, 0, 128));

	if (scale != last_scale) {
		last_scale = scale;
		layout();
	}
}

void B_TileSelector::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (rel_x < 0 || rel_y < 0)
		return;

	float scale = getScale();	
	rel_x /= scale;
	rel_y /= scale;

	int num = atlas_get_num_items(atlas);
	int sheet = rel_y / 1024;
	int yy = rel_y % 1024;

	for (int i = 0; i < num; i++) {
		ATLAS_ITEM *item = atlas_get_item_by_index(atlas, i);
		int s = atlas_get_item_sheet(item);
		if (sheet != s)
			continue;
		ALLEGRO_BITMAP *sub = atlas_get_item_sub_bitmap(item)->bitmap;
		int tx = atlas_get_item_x(item);
		int ty = atlas_get_item_y(item);
		int tw = al_get_bitmap_width(sub);
		int th = al_get_bitmap_height(sub);
		if (rel_x >= tx && rel_x < tx+tw &&
				yy >= ty && yy < ty+th) {
			selected = i;
			return;
		}
	}
}
	
B_TileSelector::B_TileSelector()
{
	atlas = atlas_create(1024, 1024, 0, 0, true);

	ALLEGRO_FS_ENTRY *dir = al_create_fs_entry("tiles/");
	al_open_directory(dir);
	ALLEGRO_FS_ENTRY *file;
	int id = 0;
	while ((file = al_read_directory(dir))) {
		const char *name = al_get_fs_entry_name(file);
		if (strstr(name, ".png")) {
			ALLEGRO_BITMAP *b = al_load_bitmap(name);
			char *shortname = (char *)name + strlen(name)-1;
			while (shortname != name &&
				*shortname != '/' &&
				*shortname != '\\') {
				shortname--;
			}
			if (*shortname == '/' || *shortname == '\\') {
				shortname++;
			}
			tile_names[id] = std::string(shortname);
			atlas_add(atlas, new Wrap::Bitmap(b, ""), id);
			id++;
		}
		al_destroy_fs_entry(file);
	}
	al_close_directory(dir);
	al_destroy_fs_entry(dir);

	atlas_finish(atlas);

	selected = 0;

	width = 1024;
	height = atlas_get_num_sheets(atlas) * 1024;
	
	start_w = width;
	start_h = height;
	last_scale = 0.125;
}

B_TileSelector::~B_TileSelector()
{
	atlas_destroy(atlas);
}

// --

void B_BattleCanvas::adjustSize(int top, int right, int bottom, int left)
{
	if (width+left+right < 1) return;
	if (height+top+bottom < 1) return;

	width += left + right;
	height += top + bottom;
	start_w = width;
	start_h = height;

	for (unsigned int l = 0; l < tiles.size(); l++) {
		for (unsigned int t = 0; t < tiles[l].size(); t++) {
			tiles[l][t].x += left;
			tiles[l][t].y += top;
		}
	}

	makeOutlines();
}

void B_BattleCanvas::makeOutlines(void)
{
	for (unsigned int i = 0; i < selectedOutlines.size(); i++) {
		free(selectedOutlines[i]);
	}
	selectedOutlines.clear();
	outlineSizes.clear();

	for (unsigned int sel = 0; sel < selected.size(); sel++) {
		int maxverts = 100;
		int nverts = 0;
		ALLEGRO_VERTEX *verts = (ALLEGRO_VERTEX *)malloc(maxverts * sizeof(ALLEGRO_VERTEX));
		Tile *tile = selected[sel];
		ATLAS_ITEM *item = atlas_get_item_by_index(atlas, tile->index);
		ALLEGRO_BITMAP *sub = atlas_get_item_sub_bitmap(item)->bitmap;
		int w = al_get_bitmap_width(sub);
		int h = al_get_bitmap_height(sub);
		al_lock_bitmap(sub, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
		for (int yy = 0; yy < h; yy++) {
			for (int xx = 0; xx < w; xx++) {
				ALLEGRO_COLOR block[9];
				const int offsets[3][6] = {
					{ -1,-1, 0,-1, 1,-1  },
					{  -1,0,  0,0,  1,0  },
					{  -1,1,  0,1,  1,1  }
				};
				int count = 0;
				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 3; j++) {
						int ox = xx+offsets[i][j*2];
						int oy = yy+offsets[i][j*2+1];
						if (ox < 0 || ox >= w || oy < 0 || oy >= h) {
							block[count] = al_map_rgba(0, 0, 0, 0);
						}
						else {
							block[count] = al_get_pixel(sub, ox, oy);
						}
						count++;
					}
				}
				bool add = false;
				if (block[4].a != 0) {
					for (int i = 0; i < 9; i++) {
						if (i == 4) continue;
						if (block[i].a == 0) {
							add = true;
							break;
						}
					}
				}
				if (add) {
					if (nverts >= maxverts) {
						maxverts += 100;
						verts = (ALLEGRO_VERTEX *)realloc(verts, maxverts * sizeof(ALLEGRO_VERTEX));
					}
					verts[nverts].x = xx;
					verts[nverts].y = yy;
					verts[nverts].z = 0;
					nverts++;
				}
			}
		}
		al_unlock_bitmap(sub);
		outlineSizes.push_back(nverts);
		selectedOutlines.push_back(verts);
	}
}

float B_BattleCanvas::getScale(void)
{
	float pos = canvas_slider->getPosition();
	float scale = 4 * pos + 0.5;
	return scale;
}

void B_BattleCanvas::draw(int abs_x, int abs_y)
{
	drawParallax(abs_x, abs_y);

	float scale = getScale();
	width = start_w * scale;
	height = start_h * scale;

	al_hold_bitmap_drawing(true);
	for (unsigned int l = 0; l < tiles.size(); l++) {
		if (!visible[l])
			continue;
		for (unsigned int t = 0; t < tiles[l].size(); t++) {
			Tile &tile = tiles[l][t];
			ATLAS_ITEM *item = atlas_get_item_by_index(atlas, tile.index);
			ALLEGRO_BITMAP *sub = atlas_get_item_sub_bitmap(item)->bitmap;
			int w = al_get_bitmap_width(sub);
			int h = al_get_bitmap_height(sub);
			al_draw_scaled_bitmap(
				sub,
				0, 0, w, h,
				abs_x+tile.x*scale,
				abs_y+tile.y*scale,
				w*scale, h*scale,
				0
			);
		}
	}
	al_hold_bitmap_drawing(false);

	for (unsigned int t = 0; t < selected.size(); t++) {
		Tile *tile = selected[t];
		ATLAS_ITEM *item = atlas_get_item_by_index(atlas, tile->index);
		ALLEGRO_BITMAP *sub = atlas_get_item_sub_bitmap(item)->bitmap;
		int w = al_get_bitmap_width(sub);
		int h = al_get_bitmap_height(sub);
		al_draw_rectangle(
			abs_x+tile->x*scale,
			abs_y+tile->y*scale,
			abs_x+(tile->x+w)*scale,
			abs_y+(tile->y+h)*scale,
			al_map_rgb_f(1, 1, 0),
			1
		);
		/*
		ALLEGRO_VERTEX *v = (ALLEGRO_VERTEX *)malloc(outlineSizes[t]*sizeof(ALLEGRO_VERTEX));
		memcpy(v, selectedOutlines[t], outlineSizes[t]*sizeof(ALLEGRO_VERTEX));
		for (int j = 0; j < outlineSizes[t]; j++) {
			v[j].x *= scale;
			v[j].y *= scale;
			v[j].x += abs_x+tile->x*scale;
			v[j].y += abs_y+tile->y*scale;
			v[j].color = al_map_rgb_f(brightness, brightness, brightness);
		}
		al_draw_prim(v, 0, 0, 0, outlineSizes[t], ALLEGRO_PRIM_POINT_LIST);
		free(v);
		*/
	}

	al_draw_rectangle(abs_x-1, abs_y-1, abs_x+width+1, abs_y+height+1, al_map_rgb(255, 255, 255), 1);

	// draw collision points
	for (unsigned int line = 0; line < lines.size(); line++) {
		for (int pt = 0; pt < (int)lines[line].size(); pt++) {
			Point *p = lines[line][pt];
			al_draw_filled_circle(
				abs_x+p->x*scale,
				abs_y+p->y*scale,
				4,
				al_map_rgb(255, 255, 0)
			);
		}
	}
	for (unsigned int l = 0; l < lines.size(); l++) {
		unsigned int start = 0;
		for (; start < lines[l].size(); start++) {
			Point *p = lines[l][start];
			if ((p->left && !p->right) || (p->right && !p->left))
				break;
		}
		Point *p = lines[l][start];
		Point *p2 = p->left ? p->left : p->right;
		while (p && p2) {
			al_draw_line(
				abs_x+p->x*scale,
				abs_y+p->y*scale,
				abs_x+p2->x*scale,
				abs_y+p2->y*scale,
				al_map_rgb(255, 255, 0),
				1
			);
			Point *tmp = p;
			p = p2;
			p2 = p2->left == tmp ? p2->right : p2->left;
		}
	}
	for (unsigned int i = 0; i < unowned_points.size(); i++) {
		Point *p = unowned_points[i];
		al_draw_filled_circle(
			abs_x+p->x*scale,
			abs_y+p->y*scale,
			4,
			al_map_rgb(255, 255, 0)
		);
	}
	if (isPointSelected) {
		al_draw_filled_circle(
			abs_x+selectedPoint.x*scale,
			abs_y+selectedPoint.y*scale,
			4,
			al_map_rgb(255, 0, 0)
		);
	}

	if (mouseX >= 0) {
		TGUI_ScrollPane *sp = dynamic_cast<TGUI_ScrollPane *>(parent);
		if (sp) {
			float ox, oy;
			sp->get_values(&ox, &oy);
			al_draw_textf(tgui::getFont(),
				al_map_rgb(200, 200, 200),
				abs_x+(width-(sp->getWidth()-TGUI_ScrollPane::SCROLLBAR_THICKNESS))*ox,
				abs_y+(height-(sp->getHeight()-TGUI_ScrollPane::SCROLLBAR_THICKNESS))*oy,
				0, "%d,%d", (int)(mouseX/scale),
				(int)(mouseY/scale));
		}
	}
	
	if (scale != last_scale) {
		last_scale = scale;
		layout();
	}
}

void B_BattleCanvas::keyDown(int keycode)
{
	switch (keycode) {
		case ALLEGRO_KEY_UP:
			push_undo();
			movingY = -1;
			moveSelected();
			break;
		case ALLEGRO_KEY_DOWN:
			push_undo();
			movingY = 1;
			moveSelected();
			break;
		case ALLEGRO_KEY_LEFT:
			push_undo();
			movingX = -1;
			moveSelected();
			break;
		case ALLEGRO_KEY_RIGHT:
			push_undo();
			movingX = 1;
			moveSelected();
			break;
		case ALLEGRO_KEY_LCTRL:
		case ALLEGRO_KEY_RCTRL:
			controlDown = true;
			break;
	}
}

void B_BattleCanvas::keyUp(int keycode)
{
	switch (keycode) {
		case ALLEGRO_KEY_UP:
		case ALLEGRO_KEY_DOWN:
			moveDelay = START_MOVE_DELAY;
			movingY = 0;
			break;
		case ALLEGRO_KEY_LEFT:
		case ALLEGRO_KEY_RIGHT:
			moveDelay = START_MOVE_DELAY;
			movingX = 0;
			break;
		case ALLEGRO_KEY_LCTRL:
		case ALLEGRO_KEY_RCTRL:
			controlDown = false;
			break;
	}
}

void B_BattleCanvas::select(Tile *t)
{
	bool found = false;
	for (unsigned int i = 0; i < selected.size(); i++) {
		if (selected[i] == t) {
			found = true;
			selected.erase(selected.begin()+i);
			break;
		}
	}
	if (!found) {
		selected.push_back(t);
	}
	makeOutlines();
}

void B_BattleCanvas::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (rel_x < 0 || rel_y < 0) {
		return;
	}

	float scale = getScale();

	// Add or remove clicked tiles
	if (tool == TOOL_MOVE) {
		Tile *t = getClickedTile(layer, rel_x, rel_y);
		if (t) {
			if (controlDown) {
				push_undo();
				select(t);
			}
			else {
				bool t_is_selected = false;
				for (unsigned int i = 0; i < selected.size(); i++) {
					if (selected[i] == t) {
						t_is_selected = true;
						break;
					}
				}
				push_undo();
				if (!t_is_selected) {
					clearSelected();
					select(t);
				}
				lastMouseX = abs_x;
				lastMouseY = abs_y;
				dragging = true;
			}
		}
	}
	else if (tool == TOOL_HAND) {
		lastMouseX = abs_x;
		lastMouseY = abs_y;
		dragging = true;
	}
	else if (tool == TOOL_ADD_POINT) {
		addPoint(rel_x/scale, rel_y/scale);
	}
	else if (tool == TOOL_DELETE_POINT) {
		deletePoint(rel_x/scale, rel_y/scale);
	}
	else if (tool == TOOL_CONNECT_POINTS) {
		selectPoint(rel_x/scale, rel_y/scale);
	}
}

void B_BattleCanvas::mouseMove(int rel_x, int rel_y, int abs_x, int abs_y)
{
	if (rel_x >= 0 && rel_y >= 0) {
		if (using_hand_cursor) {
			al_set_mouse_cursor(tgui::getDisplay(), hand_cursor);
		}
		mouseX = rel_x;
		mouseY = rel_y;
	}
	else {
		mouseX = -1;
		mouseY = -1;
	}
}

void B_BattleCanvas::mouseMoveAll(tgui::TGUIWidget *leftOut, int abs_x, int abs_y)
{
	if (dragging) {
		float dx = abs_x - lastMouseX + dragdiffx;
		float dy = abs_y - lastMouseY + dragdiffy;
		float scale = getScale();

		if (tool == TOOL_MOVE) {
			float tmpx = (int)(dx / scale) * scale;
			float tmpy = (int)(dy / scale) * scale;

			dragdiffx = dx - tmpx;
			dragdiffy = dy - tmpy;

			dx = (int)(dx / scale);
			dy = (int)(dy / scale);

			movingX = dx;
			movingY = dy;
			moveSelected();
			movingX = 0;
			movingY = 0;
		}
		else if (tool == TOOL_HAND) {
			dragdiffx = 0;
			dragdiffy = 0;

			TGUI_ScrollPane *sp = dynamic_cast<TGUI_ScrollPane *>(parent);
			if (sp) {
				float curr_ox, curr_oy;
				sp->get_values(&curr_ox, &curr_oy);
				float percent_x = dx / width;
				float percent_y = dy / height;
				curr_ox -= percent_x;
				curr_oy -= percent_y;
				if (percent_x > 0 && curr_ox < 0) curr_ox = 0;
				if (percent_x < 0 && curr_ox > 1) curr_ox = 1;
				if (percent_y > 0 && curr_oy < 0) curr_oy = 0;
				if (percent_y < 0 && curr_oy > 1) curr_oy = 1;
				sp->setValues(curr_ox, curr_oy);
			}
		}
			
		lastMouseX = abs_x;
		lastMouseY = abs_y;
	}
}

void B_BattleCanvas::mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	dragging = false;
	dragdiffx = dragdiffy = 0;
}

void B_BattleCanvas::moveSelected(void)
{
	for (unsigned int i = 0; i < selected.size(); i++) {
		Tile *t = selected[i];
		t->x += movingX;
		t->y += movingY;
	}
}

tgui::TGUIWidget *B_BattleCanvas::update(void)
{
	brightness += binc;
	if (binc > 0 && brightness > 1) { brightness = 1; binc = -binc; }
	else if (binc < 0 && brightness < 0) { brightness = 0; binc = -binc; }

	if (movingX || movingY) {
		moveDelay--;
		if (moveDelay <= 0) {
			moveDelay = 1;
			moveSelected();
		}
	}

	return NULL;
}

void B_BattleCanvas::setTool(Tool t)
{
	tool = t;
}

B_BattleCanvas::Tool B_BattleCanvas::getTool(void)
{
	return tool;
}

void B_BattleCanvas::clearSelected(void)
{
	selected.clear();
	for (unsigned int i = 0; i < selectedOutlines.size(); i++) {
		free(selectedOutlines[i]);
	}
	selectedOutlines.clear();
	outlineSizes.clear();
}

void B_BattleCanvas::setLayer(int l)
{
//	push_undo();

	layer = l;
	while (tiles.size() <= (unsigned)layer) {
		tiles.push_back(std::vector<Tile>());
		visible.push_back(true);
	}
	clearSelected();
}

int B_BattleCanvas::getLayer(void)
{
	return layer;
}

int B_BattleCanvas::getNumLayers(void)
{
	return tiles.size();
}

void B_BattleCanvas::addTile(int index, int x, int y)
{
	push_undo();

	Tile t;
	t.index = index;
	t.x = x;
	t.y = y;
	tiles[layer].push_back(t);
	selected.clear();
	selected.push_back(&tiles[layer][tiles[layer].size()-1]);
	makeOutlines();
}
	
bool B_BattleCanvas::getVisible(int layer)
{
	return visible[layer];
}

void B_BattleCanvas::setVisible(int layer, bool value)
{
	visible[layer] = value;
}

void B_BattleCanvas::addLayer(int before) // -1 = at end
{
	push_undo();

	std::vector<Tile> layer;
	if (before == -1) {
		tiles.push_back(layer);
		visible.push_back(true);
	}
	else {
		tiles.insert(tiles.begin()+before, layer);
		visible.insert(visible.begin()+before, true);
	}
}

void B_BattleCanvas::deleteLayer(int layer)
{
	push_undo();

	tiles.erase(tiles.begin()+layer);
	visible.erase(visible.begin()+layer);
	this->layer = 0;
}

B_BattleCanvas::Tile *B_BattleCanvas::getClickedTile(int layer, int mousex, int mousey)
{
	float scale = getScale();
	mousex /= scale;
	mousey /= scale;

	for (int i = tiles[layer].size()-1; i >= 0; i--) {
		int mx = mousex;
		int my = mousey;
		Tile &t = tiles[layer][i];
		int xx = t.x+(parent ? parent->getX() : 0);
		int yy = t.y+(parent ? parent->getY() : 0);
		ATLAS_ITEM *item = atlas_get_item_by_index(atlas, t.index);
		ALLEGRO_BITMAP *sub = atlas_get_item_sub_bitmap(item)->bitmap;
		int w = al_get_bitmap_width(sub);
		int h = al_get_bitmap_height(sub);
		mx -= xx;
		my -= yy;
		if (mx >= 0 && mx < w && my >= 0 && my < h) {
			ALLEGRO_COLOR c = al_get_pixel(sub, mx, my);
			if (c.a != 0) {
				return &t;
			}
		}
	}

	return NULL;
}

std::vector<B_BattleCanvas::Tile> B_BattleCanvas::copy(void)
{
	std::vector<Tile> v;
	for (unsigned int i = 0; i < selected.size(); i++) {
		v.push_back(*selected[i]);
	}
	return v;
}

void B_BattleCanvas::_delete(void)
{
	push_undo();

	std::vector<int> to_erase;
	for (unsigned int i = 0; i < tiles[layer].size(); i++) {
		unsigned int j;
		for (j = 0; j < selected.size(); j++) {
			if (selected[j] == &tiles[layer][i])
				break;
		}
		if (j < selected.size()) {
			to_erase.push_back(i-to_erase.size());
		}
	}
	for (unsigned int i = 0; i < to_erase.size(); i++) {
		tiles[layer].erase(tiles[layer].begin()+to_erase[i]);
	}
	clearSelected();
}

void B_BattleCanvas::paste(std::vector<Tile> v)
{
	push_undo();

	clearSelected();
	for (unsigned int i = 0; i < v.size(); i++) {
		tiles[layer].push_back(v[i]);
	}
	for (unsigned int i = 0; i < v.size(); i++) {
		selected.push_back(&(tiles[layer][tiles[layer].size()-1-i]));
	}
	makeOutlines();
}

std::vector<B_BattleCanvas::Tile> &B_BattleCanvas::getTiles(int layer)
{
	return tiles[layer];
}
	
void B_BattleCanvas::selectAll(void)
{
	push_undo();

	clearSelected();
	for (unsigned int i = 0; i < tiles[layer].size(); i++) {
		selected.push_back(&tiles[layer][i]);
	}
	makeOutlines();
}

void B_BattleCanvas::invertSelection(void)
{
	push_undo();

	std::vector<Tile *> v;
	for (unsigned int i = 0; i < tiles[layer].size(); i++) {
		bool found = false;
		for (unsigned int j = 0; j < selected.size(); j++) {
			if (&tiles[layer][i] == selected[j]) {
				found = true;
				break;
			}
		}
		if (!found) {
			v.push_back(&tiles[layer][i]);
		}
	}

	selected = v;
}

void B_BattleCanvas::setParallaxDrawer(void (*drawParallax)(int abs_x, int abs_y))
{
	this->drawParallax = drawParallax;
}

void B_BattleCanvas::clearSelectedPoint(void)
{
	isPointSelected = false;
}

void B_BattleCanvas::nearestIsUnowned(
	int x, int y, int l, int p, int nearest, bool *niu, bool *bothNull)
{
	if (l < 0 && nearest < 0) {
		*bothNull = true;
		return;
	}

	*bothNull = false;

	if (l < 0) {
		*niu = true;
		return;
	}
	else if (nearest < 0) {
		*niu = false;
		return;
	}

	float dx1 = x - lines[l][p]->x;
	float dy1 = y - lines[l][p]->y;
	float dist1 = sqrt(dx1*dx1 + dy1*dy1);
	float dx2 = x - unowned_points[nearest]->x;
	float dy2 = y - unowned_points[nearest]->y;
	float dist2 = sqrt(dx2*dx2 + dy2*dy2);

	if (dist1 < dist2)
		*niu = false;
	else
		*niu = true;
}

void B_BattleCanvas::selectPoint(int x, int y)
{
	if (isPointSelected) {
		connectPoints(x, y, selectedPoint.x, selectedPoint.y);
		isPointSelected = false;
		return;
	}

	int l1, p1;

	findNearestPoint(x, y, &l1, &p1);
	int nearest = findNearestUnownedPoint(x, y);
	bool niu, bothNull;
	nearestIsUnowned(x, y, l1, p1, nearest, &niu, &bothNull);

	if (bothNull)
		return;

	if (niu) {
		isPointSelected = true;
		selectedPoint.x = unowned_points[nearest]->x;
		selectedPoint.y = unowned_points[nearest]->y;
	}
	else {
		isPointSelected = true;
		selectedPoint.x = lines[l1][p1]->x;
		selectedPoint.y = lines[l1][p1]->y;
	}
}

void B_BattleCanvas::addPoint(int x, int y)
{
	unowned_points.push_back(new Point(x, y));
}

void B_BattleCanvas::findNearestPoint(int x, int y, int *line_out, int *pt_out)
{
	float nearest_dist = FLT_MAX;
	int line_num = -1;
	int pt_num = -1;

	for (unsigned int line = 0; line < lines.size(); line++) {
		for (unsigned int pt = 0; pt < lines[line].size(); pt++) {
			Point *p = lines[line][pt];
			float dx = p->x - x;
			float dy = p->y - y;
			float dist = sqrt(dx*dx + dy*dy);
			if (dist < nearest_dist) {
				nearest_dist = dist;
				line_num = line;
				pt_num = pt;
			}
		}
	}

	*line_out = line_num;
	*pt_out = pt_num;
}

int B_BattleCanvas::findNearestUnownedPoint(int x, int y)
{
	int nearest = -1;
	float nearest_dist = FLT_MAX;
	for (unsigned int i = 0; i < unowned_points.size(); i++) {
		Point *p = unowned_points[i];
		float dx = p->x - x;
		float dy = p->y - y;
		float dist = sqrt(dx*dx + dy*dy);
		if (dist < nearest_dist) {
			nearest_dist = dist;
			nearest = i;
		}
	}
	return nearest;
}

void B_BattleCanvas::deletePoint(int x, int y)
{
	int line_num, pt_num;

	findNearestPoint(x, y, &line_num, &pt_num);
	int nearest = findNearestUnownedPoint(x, y);
	bool niu, bothNull;
	nearestIsUnowned(x, y, line_num, pt_num, nearest, &niu, &bothNull);

	if (bothNull)
		return;

	if (!niu) {
		Point *p = lines[line_num][pt_num];
		if (p->left) {
			Point *pp = p->left;
			if (pp->left == p)
				pp->left = NULL;
			else if (pp->right == p)
				pp->right = NULL;
		}
		if (p->right) {
			Point *pp = p->right;
			if (pp->left == p)
				pp->left = NULL;
			else if (pp->right == p)
				pp->right = NULL;
		}
		lines.push_back(std::vector<Point *>());
		for (unsigned int i = pt_num+1; i < lines[line_num].size(); i++) {
			lines[lines.size()-1].push_back(lines[line_num][i]);
		}
		delete lines[line_num][pt_num];
		lines[line_num].erase(lines[line_num].begin() + pt_num,
			lines[line_num].end());
		// any 0-1 point line, put it in unowned
		if (lines[lines.size()-1].size() < 2) {
			for (size_t i = 0; i < lines[lines.size()-1].size(); i++) {
				unowned_points.push_back(lines[lines.size()-1][i]);
			}
			lines.erase(lines.begin()+lines.size()-1);
		}
		if (lines[line_num].size() < 2) {
			for (size_t i = 0; i < lines[line_num].size(); i++) {
				unowned_points.push_back(lines[line_num][i]);
			}
			lines.erase(lines.begin()+line_num);
		}
	}
	else {
		delete unowned_points[nearest];
		unowned_points.erase(unowned_points.begin()+nearest);
	}
}

// should have made this an array[2]. oh well.
void B_BattleCanvas::connectPoints(int x1, int y1, int x2, int y2)
{
	int l1, l2;
	int p1, p2;
	Point *pt1 = NULL;
	Point *pt2 = NULL;
	int i1, i2;

	findNearestPoint(x1, y1, &l1, &p1);
	findNearestPoint(x2, y2, &l2, &p2);
	i1 = findNearestUnownedPoint(x1, y1);
	i2 = findNearestUnownedPoint(x2, y2);

	bool niu1, bothNull1;
	bool niu2, bothNull2;

	nearestIsUnowned(x1, y1, l1, p1, i1, &niu1, &bothNull1);
	nearestIsUnowned(x2, y2, l2, p2, i2, &niu2, &bothNull2);

	if (bothNull1 || bothNull2)
		return;
	
	if (niu1)
		pt1 = unowned_points[i1];
	else
		pt1 = lines[l1][p1];
	if (niu2)
		pt2 = unowned_points[i2];
	else
		pt2 = lines[l2][p2];

	if (pt1 == pt2)
		return;

	if ((pt1->left && pt1->right) ||
	    (pt2->left && pt2->right))
		return;

	if (pt1->left == NULL) {
		pt1->left = pt2;
		if (pt2->left == NULL) {
			pt2->left = pt1;
		}
		else {
			pt2->right = pt1;
		}
	}
	else {
		pt1->right = pt2;
		if (pt2->left == NULL) {
			pt2->left = pt1;
		}
		else {
			pt2->right = pt2;
		}
	}

	if (niu1 && niu2) {
		if (i1 < i2) {
			unowned_points.erase(unowned_points.begin()+i1);
			unowned_points.erase(unowned_points.begin()+(i2-1));
		}
		else {
			unowned_points.erase(unowned_points.begin()+i2);
			unowned_points.erase(unowned_points.begin()+(i1-1));
		}
		lines.push_back(std::vector<Point *>());
		lines[lines.size()-1].push_back(pt1);
		lines[lines.size()-1].push_back(pt2);
	}
	else if (niu1) {
		unowned_points.erase(unowned_points.begin()+i1);
		lines[l2].push_back(pt1);
	}
	else if (niu2) {
		unowned_points.erase(unowned_points.begin()+i2);
		lines[l1].push_back(pt2);
	}
	else {
		if (l1 != l2) {
			// combine the lines
			for (unsigned int i = 0; i < lines[l2].size(); i++) {
				lines[l1].push_back(lines[l2][i]);
			}
			lines.erase(lines.begin()+l2);
		}
	}

	// organize points
	for (unsigned int l = 0; l < lines.size(); l++) {
		unsigned int start = 0;
		for (; start < lines[l].size(); start++) {
			Point *p = lines[l][start];
			if ((p->left && !p->right) || (p->right && !p->left))
				break;
		}
		Point *p = lines[l][start];
		Point *p2 = p->left ? p->left : p->right;
		std::vector<Point *> newVec;
		while (p && p2) {
			newVec.push_back(p);
			Point *tmp = p;
			p = p2;
			p2 = p2->left == tmp ? p2->right : p2->left;
		}
		if (p)
			newVec.push_back(p);
		lines[l] = newVec;
	}
}

std::vector< std::vector<B_BattleCanvas::Point *> >
	B_BattleCanvas::getLines(void)
{
	return lines;
}

void B_BattleCanvas::setLines(std::vector< std::vector< B_BattleCanvas::Point *> > l)
{
	lines = l;
}

void B_BattleCanvas::getStartSize(int *w, int *h)
{
	*w = start_w;
	*h = start_h;
}
	
B_BattleCanvas::B_BattleCanvas(void) :
	selectedPoint(0, 0)
{
	x = 0; y = 0;
}

B_BattleCanvas::B_BattleCanvas(ATLAS *atlas, int width, int height) :
	atlas(atlas),
	movingX(0),
	movingY(0),
	layer(0),
	mouseIsDown(false),
	tool(TOOL_HAND),
	moveDelay(START_MOVE_DELAY),
	drawParallax(NULL),
	selectedPoint(0, 0),
	mouseX(-1),
	mouseY(-1)
{
	x = 0;
	y = 0;

	this->width = width;
	this->height = height;

	addLayer(-1);

	start_w = width;
	start_h = height;
	last_scale = 0.125;
	brightness = 0;
	binc = 0.05;
	controlDown = false;
	dragging = false;
	isPointSelected = false;

	dragdiffx = 0;
	dragdiffy = 0;
}

B_BattleCanvas::~B_BattleCanvas(void)
{
}

B_BattleCanvas &B_BattleCanvas::operator=(const B_BattleCanvas& rhs)
{
	atlas = rhs.atlas;
	movingX = rhs.movingX;
	movingY = rhs.movingY;
	layer = rhs.layer;
	mouseIsDown = rhs.mouseIsDown;
	tool = rhs.tool;
	tiles = rhs.tiles;

	for (unsigned int i = 0; i < rhs.selected.size(); i++) {
		bool found = false;
		unsigned int l, t = 0;
		for (l = 0; l < rhs.tiles.size(); l++) {
			for (t = 0; t < rhs.tiles[l].size(); t++) {
				if (&rhs.tiles[l][t] == rhs.selected[i]) {
					found = true;
					break;
				}
			}
			if (found)
				break;
		}
		selected.push_back(&tiles[l][t]);
	}

	outlineSizes = rhs.outlineSizes;

	for (unsigned int i = 0; i < rhs.selectedOutlines.size(); i++) {
		ALLEGRO_VERTEX *v = (ALLEGRO_VERTEX *)malloc(sizeof(ALLEGRO_VERTEX) * rhs.outlineSizes[i]);
		memcpy(v, rhs.selectedOutlines[i], rhs.outlineSizes[i]*sizeof(ALLEGRO_VERTEX));
		selectedOutlines.push_back(v);
	}

	visible = rhs.visible;
	moveDelay = rhs.moveDelay;
	start_w = rhs.start_w;
	start_h = rhs.start_h;
	last_scale = rhs.last_scale;
	brightness = rhs.brightness;
	binc = rhs.binc;
	controlDown = rhs.controlDown;
	dragging = rhs.dragging;
	lastMouseX = rhs.lastMouseX;
	lastMouseY = rhs.lastMouseY;
	drawParallax = rhs.drawParallax;
	isPointSelected = rhs.isPointSelected;
	unowned_points = rhs.unowned_points;
	lines = rhs.lines;
	selectedPoint.x = rhs.selectedPoint.x;
	selectedPoint.y = rhs.selectedPoint.y;
	return *this;
}

const int MAX_UNDO = 50;
static std::vector<B_BattleCanvas *> undo_stack;
static std::vector<B_BattleCanvas *> redo_stack;

void undo(void)
{
	if (undo_stack.size() > 0) {
		redo_stack.push_back(canvas);
		if ((int)redo_stack.size() > MAX_UNDO) {
			delete *(redo_stack.begin());
			redo_stack.erase(redo_stack.begin());
		}
		canvas = undo_stack[undo_stack.size()-1];
		undo_stack.erase(undo_stack.begin()+undo_stack.size()-1);
	}
}

void redo(void)
{
	if (redo_stack.size() > 0) {
		undo_stack.push_back(canvas);
		if ((int)undo_stack.size() > MAX_UNDO) {
			delete *(undo_stack.begin());
			undo_stack.erase(undo_stack.begin());
		}
		canvas = redo_stack[redo_stack.size()-1];
		redo_stack.erase(redo_stack.begin()+redo_stack.size()-1);
	}
}

void push_undo(void)
{
	if (!canvas)
		return;
	
	B_BattleCanvas *new_canvas = new B_BattleCanvas();
	*new_canvas = *canvas;
	undo_stack.push_back(new_canvas);
	if ((int)undo_stack.size() > MAX_UNDO) {
		delete *(undo_stack.begin());
		undo_stack.erase(undo_stack.begin());
	}
}

void clearUndoRedo(void)
{
	undo_stack.clear();
	redo_stack.clear();
}

