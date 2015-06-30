#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_native_dialog.h>

#include <cstdio>
#include <cctype>

#include <tgui2.hpp>
#include <tgui2_widgets.hpp>

#include "widgets.h"

const int SCR_W = 1200;
const int SCR_H = 680;

ALLEGRO_DISPLAY *display;
ALLEGRO_FONT *font;
TGUI_Slider *canvas_slider;
TGUI_Slider *tile_slider;
B_BattleCanvas *canvas;
bool using_hand_cursor = false;
ALLEGRO_MOUSE_CURSOR *hand_cursor;

static bool in_size_menu = false;
static TGUI_Button *size_ok_button;
static TGUI_Frame *size_frame;
// clockwise
static TGUI_Label *size_labels[4];
static TGUI_TextField *size_textfields[4];
static TGUI_Splitter *main_size_splitter, *size_splitter1, *size_splitter2, *size_splitter3;
static TGUI_Label *current_size_label;

static TGUI_Splitter *menu_splitter;
static TGUI_Splitter *layer_select_menu;
static TGUI_Splitter *layer_toggle_menu;
static TGUI_SubMenuItem *layer_select_item;
static TGUI_SubMenuItem *layer_toggle_item;
static B_TileSelector *tilesel;
static TGUI_RadioGroup radioGroup;
static TGUI_MenuBar *menu_bar;
static TGUI_ScrollPane *canvas_scrollpane;
static std::vector<B_BattleCanvas::Tile> clipboard;
static ALLEGRO_TIMER *timer;
static TGUI_Splitter *canvas_button_splitter;

static std::string current_filename = "";

static bool showTools = true;

static void showToolWidgets(void)
{
	if (showTools) {
		tgui::addWidget(canvas_slider);
		tgui::addWidget(canvas_button_splitter);
	}
}

static void hideToolWidgets(void)
{
	canvas_slider->remove();
	canvas_button_splitter->remove();
}

static std::string itos(int i)
{
	char s[100];
	sprintf(s, "%d", i);
	return std::string(s);
}

static bool int_validate(const std::string str)
{
	const char *p = str.c_str();
	
	if (*p == 0) return true;

	if (!isdigit(*p) && (*p != '-'))
		return false;
	
	if (*p == '-' && *(p+1) == 0)
		return false;

	p++;

	while (*p != 0) {
		if (!isdigit(*p))
			return false;
		p++;
	}

	return true;
}

static void write32(FILE *f, int i) {
	fputc((i >> 0) & 0xff, f);
	fputc((i >> 8) & 0xff, f);
	fputc((i >> 16) & 0xff, f);
	fputc((i >> 24) & 0xff, f);
}

static int read32(FILE *f) {
	unsigned char b1 = fgetc(f);
	unsigned char b2 = fgetc(f);
	unsigned char b3 = fgetc(f);
	unsigned char b4 = fgetc(f);
	return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}


static void writeNString(std::string s, FILE *f)
{
	write32(f, s.length());
	for (unsigned int i = 0; i < s.length(); i++) {
		fputc(s.c_str()[i], f);
	}
}

static std::string readNString(FILE *f)
{
	int len = read32(f);
	char data[len+1];
	data[len] = 0;
	for (int i = 0; i < len; i++) {
		data[i] = fgetc(f);
	}
	return std::string(data);
}

static void save(const char *filename)
{
	current_filename = filename;

	int nlayers = canvas->getNumLayers();
	ATLAS *atlas = tilesel->getAtlas();

	FILE *f = fopen(filename, "wb");

	int w, h;
	canvas->getStartSize(&w, &h);
	write32(f, w);
	write32(f, h);

	// save tile filenames first
	std::vector<std::string> names;
	std::vector<std::string> names2;
	for (int l = 0; l < nlayers; l++) {
		std::vector<B_BattleCanvas::Tile> &layer = canvas->getTiles(l);
		for (unsigned int t = 0; t < layer.size(); t++) {
			B_BattleCanvas::Tile &tile = layer[t];
			ATLAS_ITEM *item = atlas_get_item_by_index(atlas, tile.index);
			int id = atlas_get_item_id(item);
			std::string name = tilesel->getName(id);
			names.push_back(name);
			char buf[1000];
			strncpy(buf, name.c_str(), 1000);
			*strrchr(buf, '.') = 0;
			names2.push_back(buf);
		}
	}
	std::sort(names.begin(), names.end());
	std::sort(names2.begin(), names2.end());
	std::vector<std::string>::iterator end = std::unique(names.begin(), names.end());
	std::vector<std::string>::iterator end2 = std::unique(names2.begin(), names2.end());
	names.erase(end, names.end());
	names2.erase(end2, names2.end());
	write32(f, names.size());
	for (unsigned int i = 0; i < names.size(); i++) {
		writeNString(names2[i], f);
	}

	// save tile info
	write32(f, nlayers);
	for (int l = 0; l < nlayers; l++) {
		std::vector<B_BattleCanvas::Tile> &layer = canvas->getTiles(l);
		write32(f, layer.size());
		for (unsigned int t = 0; t < layer.size(); t++) {
			B_BattleCanvas::Tile &tile = layer[t];
			ATLAS_ITEM *item = atlas_get_item_by_index(atlas, tile.index);
			int id = atlas_get_item_id(item);
			std::string name = tilesel->getName(id);
			// FIXME:
			//if (name == "abw17.png") name = "abw12.png";
			int file_index = 0;
			for (;; file_index++) {
				if (name == names[file_index])
					break;
			}
			write32(f, file_index);
			write32(f, tile.x);
			write32(f, tile.y);
		}
	}

	// write collision lines
	std::vector< std::vector<B_BattleCanvas::Point *> > lines =
		canvas->getLines();
	write32(f, lines.size());
	for (unsigned int l = 0; l < lines.size(); l++) {
		write32(f, lines[l].size());
		for (unsigned int p = 0; p < lines[l].size(); p++) {
			write32(f, lines[l][p]->x);
			write32(f, lines[l][p]->y);
		}
	}

	fclose(f);
}

static std::vector<ALLEGRO_BITMAP *> parallax_bmps;

static bool parallax_sort(ALLEGRO_BITMAP *a, ALLEGRO_BITMAP *b)
{
	if (al_get_bitmap_width(a) < al_get_bitmap_width(b))
		return true;
	return false;
}

static void load_parallax(void)
{
	ALLEGRO_FILECHOOSER *fc = al_create_native_file_dialog(
		"./parallax/",
		"Open parallax image...",
		"*.*",
		ALLEGRO_FILECHOOSER_FILE_MUST_EXIST
	);

	al_stop_timer(timer);
	al_show_native_file_dialog(display, fc);
	al_start_timer(timer);

	if (al_get_native_file_dialog_count(fc) <= 0) {
		al_destroy_native_file_dialog(fc);
		return;
	}
	
	char path[1000];
	strcpy(path, al_get_native_file_dialog_path(fc, 0));
	
	al_destroy_native_file_dialog(fc);

	parallax_bmps.push_back(al_load_bitmap(path));
	std::sort(parallax_bmps.begin(), parallax_bmps.end(), parallax_sort);
}

static void draw_parallax(int abs_x, int abs_y)
{
	float scale = canvas->getScale();

	for (unsigned int i = 0; i < parallax_bmps.size(); i++) {
		ALLEGRO_BITMAP *b = parallax_bmps[i];
		int w = al_get_bitmap_width(b) * scale;
		int cw = canvas->getWidth();
		int spw = canvas_scrollpane->getWidth();
		int diff1 = w - spw;
		int diff2 = cw - spw;
		int dx = (float)abs_x/diff2 * diff1;
		al_draw_scaled_bitmap(
			b,
			0, 0, al_get_bitmap_width(b), al_get_bitmap_height(b),
			dx, abs_y, w, al_get_bitmap_height(b)*scale,
			0
		);
	}
}

static void load(const char *filename)
{
	current_filename = "";

	delete canvas;
	canvas = NULL;
	clearUndoRedo();

	FILE *f = fopen(filename, "rb");
	
	int w = read32(f);
	int h = read32(f);
	
	canvas = new B_BattleCanvas(tilesel->getAtlas(), w, h);
	canvas->setParallaxDrawer(draw_parallax);

	int ntiles = read32(f);
	std::vector<std::string> names;
	for (int i = 0; i < ntiles; i++) {
		names.push_back(readNString(f) + ".png");
	}

	int nlayers = read32(f);

	for (int l = 0; l < nlayers; l++) {
		canvas->setLayer(l);
		int num = read32(f);
		for (int i = 0; i < num; i++) {
			int file_index = read32(f);
			int x = read32(f);
			int y = read32(f);
			int index = tilesel->getIndex(names[file_index]);
			canvas->addTile(index, x, y);
		}
	}

	int nlines = read32(f);
	std::vector< std::vector<B_BattleCanvas::Point *> > lines;
	for (int i = 0; i < nlines; i++) {
		lines.push_back(std::vector<B_BattleCanvas::Point *>());
		int pts = read32(f);
		B_BattleCanvas::Point *prev = NULL;
		for (int p = 0; p < pts; p++) {
			int x = read32(f);
			int y = read32(f);
			B_BattleCanvas::Point *point =
				new B_BattleCanvas::Point(x, y);
			lines[i].push_back(point);
			if (p > 0) {
				point->left = prev;
				prev->right = point;
			}
			prev = point;
		}
	}
	canvas->setLines(lines);

	fclose(f);
}

static void create_layer_menus(void)
{
	// destroy the old ones if there are any
	if (layer_select_menu) {
		std::vector<tgui::TGUIWidget *> &w1 = layer_select_menu->getWidgets();
		for (unsigned int i = 0; i < w1.size(); i++) {
			delete w1[i];
		}
		delete layer_select_menu;
		std::vector<tgui::TGUIWidget *> &w2 = layer_toggle_menu->getWidgets();
		for (unsigned int i = 0; i < w2.size(); i++) {
			delete w2[i];
		}
		delete layer_toggle_menu;
	}

	std::vector<tgui::TGUIWidget *> select_widgets;
	std::vector<tgui::TGUIWidget *> toggle_widgets;
	int layers = canvas->getNumLayers();
	radioGroup.selected = 0;
	for (int i = 0; i < layers; i++) {
		char buf[100];
		sprintf(buf, "Layer %d", i+1);
		TGUI_RadioMenuItem *i1 = new TGUI_RadioMenuItem(
			std::string(buf), 0, &radioGroup, i);
		select_widgets.push_back(i1);
		TGUI_CheckMenuItem *i2 = new TGUI_CheckMenuItem(
			std::string(buf), 0, canvas->getVisible(i));
		toggle_widgets.push_back(i2);
	}
	
	layer_select_menu = new TGUI_Splitter(
		0, 0,
		250, TGUI_TextMenuItem::HEIGHT*select_widgets.size(),
		TGUI_VERTICAL,
		false,
		select_widgets
	);
	layer_select_item->setSubMenu(layer_select_menu);

	layer_toggle_menu = new TGUI_Splitter(
		0, 0,
		250, TGUI_TextMenuItem::HEIGHT*toggle_widgets.size(),
		TGUI_VERTICAL,
		false,
		toggle_widgets
	);
	layer_toggle_item->setSubMenu(layer_toggle_menu);
}

static std::string getcwd_str(void)
{
	char path[1000];
	getcwd(path, 999);
	path[strlen(path)+1] = 0;
	path[strlen(path)] = '/';
	return std::string(path);
}

static void save_as(void)
{
	if (current_filename == "")
		current_filename = getcwd_str();
	
	char currf[1000];
	strcpy(currf, current_filename.c_str());

	ALLEGRO_FILECHOOSER *fc = al_create_native_file_dialog(
		currf,
		"Save As...",
		"*.*",
		ALLEGRO_FILECHOOSER_SAVE
	);

	al_stop_timer(timer);
	al_show_native_file_dialog(display, fc);
	al_start_timer(timer);

	if (al_get_native_file_dialog_count(fc) <= 0) {
		al_destroy_native_file_dialog(fc);
		return;
	}
	
	save(al_get_native_file_dialog_path(fc, 0));
	al_destroy_native_file_dialog(fc);
}

static void save(void)
{
	if (current_filename == "") {
		save_as();
		return;
	}
	char currf[1000];
	strcpy(currf, current_filename.c_str());
	save(currf);
}

static void setLayerMenusMenuBarsWorker(TGUI_Splitter *splitter)
{
	std::vector<tgui::TGUIWidget *> &w = splitter->getWidgets();
	for (unsigned int i = 0; i < w.size(); i++) {
		TGUI_TextMenuItem *item = dynamic_cast<TGUI_TextMenuItem *>(w[i]);
		if (item) {
			item->setMenuBar(menu_bar);
		}
	}
}

static void setLayerMenusMenuBars(void)
{
	setLayerMenusMenuBarsWorker(layer_select_menu);
	setLayerMenusMenuBarsWorker(layer_toggle_menu);
}

static void load(void)
{
	if (current_filename == "")
		current_filename = getcwd_str();

	char currf[1000];
	strcpy(currf, current_filename.c_str());

	ALLEGRO_FILECHOOSER *fc = al_create_native_file_dialog(
		currf,
		"Open...",
		"*.*",
		ALLEGRO_FILECHOOSER_FILE_MUST_EXIST
	);

	al_stop_timer(timer);
	al_show_native_file_dialog(display, fc);
	al_start_timer(timer);

	if (al_get_native_file_dialog_count(fc) <= 0) {
		al_destroy_native_file_dialog(fc);
		return;
	}
	
	char path[1000];
	strcpy(path, al_get_native_file_dialog_path(fc, 0));
	load(path);

	current_filename = al_get_native_file_dialog_path(fc, 0);
	al_destroy_native_file_dialog(fc);

	canvas_scrollpane->setChild(canvas);
	canvas->setParent(canvas_scrollpane);
	create_layer_menus();
	setLayerMenusMenuBars();
}

void layout(void)
{
	menu_splitter->layout();
}

int main(int argc, char **argv)
{
	al_init();
	al_install_keyboard();
	al_install_mouse();

	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();

	display = al_create_display(SCR_W, SCR_H);

	font = al_load_ttf_font("DejaVuSans.ttf", 14, 0);

	timer = al_create_timer(1.0f/10.0f);
	al_start_timer(timer);

	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(timer));

	hand_cursor = al_create_mouse_cursor(
		al_load_bitmap("hand_cursor.png"), 8, 8);

	tgui::init(display);
	tgui::setFont(font);

	TGUI_TextMenuItem *open_menu_item = new TGUI_TextMenuItem(
		"Open", ALLEGRO_KEY_O
	);
	TGUI_TextMenuItem *save_menu_item = new TGUI_TextMenuItem(
		"Save", ALLEGRO_KEY_S
	);
	TGUI_TextMenuItem *save_as_menu_item = new TGUI_TextMenuItem(
		"Save As...", 0
	);
	TGUI_TextMenuItem *sep3_menu_item = new TGUI_TextMenuItem(
		"", 0
	);
	TGUI_TextMenuItem *parallax_menu_item = new TGUI_TextMenuItem(
		"Load parallax...", ALLEGRO_KEY_P
	);
	TGUI_TextMenuItem *sep4_menu_item = new TGUI_TextMenuItem(
		"", 0
	);
	TGUI_TextMenuItem *quit_menu_item = new TGUI_TextMenuItem(
		"Quit", ALLEGRO_KEY_Q
	);
	std::vector<tgui::TGUIWidget *> file_menu_items;
	file_menu_items.push_back(open_menu_item);
	file_menu_items.push_back(save_menu_item);
	file_menu_items.push_back(save_as_menu_item);
	file_menu_items.push_back(sep3_menu_item);
	file_menu_items.push_back(parallax_menu_item);
	file_menu_items.push_back(sep4_menu_item);
	file_menu_items.push_back(quit_menu_item);
	TGUI_Splitter *file_menu_splitter = new TGUI_Splitter(
		0, 0,
		250, TGUI_TextMenuItem::HEIGHT*file_menu_items.size(),
		TGUI_VERTICAL,
		false,
		file_menu_items
	);

	TGUI_TextMenuItem *undo_menu_item = new TGUI_TextMenuItem(
		"Undo", ALLEGRO_KEY_Z
	);
	TGUI_TextMenuItem *redo_menu_item = new TGUI_TextMenuItem(
		"Redo", ALLEGRO_KEY_Y
	);
	TGUI_TextMenuItem *sep_menu_item = new TGUI_TextMenuItem(
		"", 0
	);
	TGUI_TextMenuItem *unselect_point_menu_item = new TGUI_TextMenuItem(
		"Unselect point", ALLEGRO_KEY_U
	);
	TGUI_TextMenuItem *toggle_tools_menu_item = new TGUI_TextMenuItem(
		"Toggle tools on/off", ALLEGRO_KEY_T
	);
	TGUI_TextMenuItem *size_menu_item = new TGUI_TextMenuItem(
		"Size...", ALLEGRO_KEY_E
	);
	TGUI_TextMenuItem *sep2_menu_item = new TGUI_TextMenuItem(
		"", 0
	);
	TGUI_TextMenuItem *cut_menu_item = new TGUI_TextMenuItem(
		"Cut", ALLEGRO_KEY_X
	);
	TGUI_TextMenuItem *copy_menu_item = new TGUI_TextMenuItem(
		"Copy", ALLEGRO_KEY_C
	);
	TGUI_TextMenuItem *paste_menu_item = new TGUI_TextMenuItem(
		"Paste", ALLEGRO_KEY_V
	);
	TGUI_TextMenuItem *delete_menu_item = new TGUI_TextMenuItem(
		"Delete", ALLEGRO_KEY_DELETE
	);
	TGUI_TextMenuItem *select_all_menu_item = new TGUI_TextMenuItem(
		"Select All", ALLEGRO_KEY_A
	);
	TGUI_TextMenuItem *invert_selection_menu_item = new TGUI_TextMenuItem(
		"Invert Selection", ALLEGRO_KEY_I
	);
	std::vector<tgui::TGUIWidget *> edit_menu_items;
	edit_menu_items.push_back(undo_menu_item);
	edit_menu_items.push_back(redo_menu_item);
	edit_menu_items.push_back(sep_menu_item);
	edit_menu_items.push_back(unselect_point_menu_item);
	edit_menu_items.push_back(toggle_tools_menu_item);
	edit_menu_items.push_back(size_menu_item);
	edit_menu_items.push_back(sep2_menu_item);
	edit_menu_items.push_back(cut_menu_item);
	edit_menu_items.push_back(copy_menu_item);
	edit_menu_items.push_back(paste_menu_item);
	edit_menu_items.push_back(delete_menu_item);
	edit_menu_items.push_back(select_all_menu_item);
	edit_menu_items.push_back(invert_selection_menu_item);
	TGUI_Splitter *edit_menu_splitter = new TGUI_Splitter(
		0, 0,
		250, TGUI_TextMenuItem::HEIGHT*edit_menu_items.size(),
		TGUI_VERTICAL,
		false,
		edit_menu_items
	);

	TGUI_TextMenuItem *insert_layer_before_item = new TGUI_TextMenuItem(
		"Insert Layer Before", ALLEGRO_KEY_B
	);
	TGUI_TextMenuItem *insert_layer_after_item = new TGUI_TextMenuItem(
		"Insert Layer After", ALLEGRO_KEY_A
	);
	TGUI_TextMenuItem *delete_layer_item = new TGUI_TextMenuItem(
		"Delete Layer", ALLEGRO_KEY_L
	);
	layer_select_item = new TGUI_SubMenuItem(
		"Active Layer", NULL
	);
	layer_toggle_item = new TGUI_SubMenuItem(
		"Layer Visibility", NULL
	);
	std::vector<tgui::TGUIWidget *> layer_menu_items;
	layer_menu_items.push_back(insert_layer_before_item);
	layer_menu_items.push_back(insert_layer_after_item);
	layer_menu_items.push_back(delete_layer_item);
	layer_menu_items.push_back(layer_select_item);
	layer_menu_items.push_back(layer_toggle_item);
	TGUI_Splitter *layer_menu_splitter = new TGUI_Splitter(
		0, 0,
		250, TGUI_TextMenuItem::HEIGHT*layer_menu_items.size(),
		TGUI_VERTICAL,
		false,
		layer_menu_items
	);
	
	tilesel = new B_TileSelector();
	tilesel->set_draw(false);
	TGUI_ScrollPane *tile_scrollpane = new TGUI_ScrollPane(tilesel);

	canvas = new B_BattleCanvas(tilesel->getAtlas(), 480, 240);
	canvas->setParallaxDrawer(draw_parallax);
	// hack
	if (argc > 1) {
		load(argv[1]);
	}
	canvas_scrollpane = new TGUI_ScrollPane(canvas);

	create_layer_menus();

	std::vector<std::string> main_menu_names;
	main_menu_names.push_back("File");
	main_menu_names.push_back("Edit");
	main_menu_names.push_back("Layer");
	std::vector<TGUI_Splitter *> main_menu_sub_menus;
	main_menu_sub_menus.push_back(file_menu_splitter);
	main_menu_sub_menus.push_back(edit_menu_splitter);
	main_menu_sub_menus.push_back(layer_menu_splitter);
	menu_bar = new TGUI_MenuBar(
		0, 0, SCR_W, TGUI_MenuBar::HEIGHT,
		main_menu_names,
		main_menu_sub_menus
	);

	setLayerMenusMenuBars();

	TGUI_Button *show_tiles_button = new TGUI_Button(
		"<",
		0, 0, 1, 1
	);

	TGUI_Button *hide_tiles_button = new TGUI_Button(
		">",
		0, 0, 1, 1
	);

	std::vector<tgui::TGUIWidget *> canvas_splitter_items;
	canvas_splitter_items.push_back(canvas_scrollpane);
	canvas_splitter_items.push_back(show_tiles_button);
	TGUI_Splitter *canvas_splitter = new TGUI_Splitter(
		0, 0,
		SCR_W, SCR_H,
		TGUI_HORIZONTAL,
		false,
		canvas_splitter_items
	);
	canvas_splitter->set_size(0, SCR_W-32);
	canvas_splitter->set_size(1, 32);

	ALLEGRO_BITMAP *tool_plus = al_load_bitmap("plus.png");
	ALLEGRO_BITMAP *tool_move = al_load_bitmap("move.png");
	ALLEGRO_BITMAP *tool_hand = al_load_bitmap("hand.png");
	ALLEGRO_BITMAP *tool_add_point = al_load_bitmap("add_point.png");
	ALLEGRO_BITMAP *tool_delete_point = al_load_bitmap("delete_point.png");
	ALLEGRO_BITMAP *tool_connect_points = al_load_bitmap("connect_points.png");
	TGUI_IconButton *plus_button = new TGUI_IconButton(
		tool_plus, 0, 0, 64, 64, 0, 0, 0
	);
	TGUI_IconButton *move_button = new TGUI_IconButton(
		tool_move, 0, 0, 64, 64, 0, 0, 0
	);
	TGUI_IconButton *hand_button = new TGUI_IconButton(
		tool_hand, 0, 0, 64, 64, 0, 0, 0
	);
	TGUI_IconButton *add_point_button = new TGUI_IconButton(
		tool_add_point, 0, 0, 64, 64, 0, 0, 0
	);
	TGUI_IconButton *delete_point_button = new TGUI_IconButton(
		tool_delete_point, 0, 0, 64, 64, 0, 0, 0
	);
	TGUI_IconButton *connect_points_button = new TGUI_IconButton(
		tool_connect_points, 0, 0, 64, 64, 0, 0, 0
	);
	std::vector<tgui::TGUIWidget *> canvas_button_splitter_items;
	canvas_button_splitter_items.push_back(plus_button);
	canvas_button_splitter_items.push_back(move_button);
	canvas_button_splitter_items.push_back(hand_button);
	canvas_button_splitter_items.push_back(add_point_button);
	canvas_button_splitter_items.push_back(delete_point_button);
	canvas_button_splitter_items.push_back(connect_points_button);
	canvas_button_splitter = new TGUI_Splitter(
		SCR_W-32-64-24, 16+45,
		64, 64*6,
		TGUI_VERTICAL,
		false,
		canvas_button_splitter_items
	);
	for (int i = 0; i < 3; i++) {
		canvas_button_splitter->set_size(i, 64);
	}

	std::vector<tgui::TGUIWidget *> tile_splitter_items;
	tile_splitter_items.push_back(hide_tiles_button);
	tile_splitter_items.push_back(tile_scrollpane);
	TGUI_Splitter *tile_splitter = new TGUI_Splitter(
		0, 0,
		SCR_W, SCR_H,
		TGUI_HORIZONTAL,
		false,
		tile_splitter_items
	);
	tile_splitter->set_size(0, 32);
	tile_splitter->set_size(1, SCR_W-32);

	std::vector<tgui::TGUIWidget *> menu_splitter_items;
	menu_splitter_items.push_back(menu_bar);
	menu_splitter_items.push_back(canvas_splitter);
	menu_splitter = new TGUI_Splitter(
		0, 0,
		SCR_W, SCR_H,
		TGUI_VERTICAL,
		false,
		menu_splitter_items
	);
	menu_splitter->set_size(0, 16);
	menu_splitter->set_size(1, SCR_H-16);

	const int SCROLLER_SIZE = 128;
	canvas_slider = new TGUI_Slider(
		SCR_W-32-SCROLLER_SIZE-40, 16+10,
		SCROLLER_SIZE, TGUI_HORIZONTAL
	);
	tile_slider = new TGUI_Slider(
		SCR_W-32-SCROLLER_SIZE-40, 16+10,
		SCROLLER_SIZE, TGUI_HORIZONTAL
	);
				
	const int SIZE_FRAME_W = 600;
	const int SIZE_FRAME_H = 400;
	size_frame = new TGUI_Frame("Size", SCR_W/2-SIZE_FRAME_W/2, SCR_H/2-SIZE_FRAME_H/2, SIZE_FRAME_W, SIZE_FRAME_H);
	size_ok_button = new TGUI_Button("OK", 0, 0, 100, 30);
	size_labels[0] = new TGUI_Label("Top", al_color_name("white"), SIZE_FRAME_W/6, 0, ALLEGRO_ALIGN_CENTRE);
	size_textfields[0] = new TGUI_TextField("0", 0, 0, 40);
	size_labels[1] = new TGUI_Label("Right", al_color_name("white"), SIZE_FRAME_W/6, 0, ALLEGRO_ALIGN_CENTRE);
	size_textfields[1] = new TGUI_TextField("0", 0, 0, 40);
	size_labels[2] = new TGUI_Label("Bottom", al_color_name("white"), SIZE_FRAME_W/6, 0, ALLEGRO_ALIGN_CENTRE);
	size_textfields[2] = new TGUI_TextField("0", 0, 0, 40);
	size_labels[3] = new TGUI_Label("Left", al_color_name("white"), SIZE_FRAME_W/6, 0, ALLEGRO_ALIGN_CENTRE);
	size_textfields[3] = new TGUI_TextField("0", 0, 0, 40);

	for (int i = 0; i < 4; i++) {
		size_textfields[i]->setValidator(int_validate);
	}

	std::vector<tgui::TGUIWidget *> splitter1_widgets;
	splitter1_widgets.push_back(NULL);
	splitter1_widgets.push_back(NULL);
	splitter1_widgets.push_back(size_labels[3]);
	splitter1_widgets.push_back(size_textfields[3]);
	splitter1_widgets.push_back(NULL);
	splitter1_widgets.push_back(NULL);
	splitter1_widgets.push_back(NULL);
	splitter1_widgets.push_back(NULL);
	splitter1_widgets.push_back(NULL);
	std::vector<tgui::TGUIWidget *> splitter2_widgets;
	splitter2_widgets.push_back(size_labels[0]);
	splitter2_widgets.push_back(size_textfields[0]);
	splitter2_widgets.push_back(NULL);
	splitter2_widgets.push_back(NULL);
	splitter2_widgets.push_back(size_labels[2]);
	splitter2_widgets.push_back(size_textfields[2]);
	splitter2_widgets.push_back(NULL);
	splitter2_widgets.push_back(size_ok_button);
	splitter2_widgets.push_back(NULL);
	std::vector<tgui::TGUIWidget *> splitter3_widgets;
	splitter3_widgets.push_back(NULL);
	splitter3_widgets.push_back(NULL);
	splitter3_widgets.push_back(size_labels[1]);
	splitter3_widgets.push_back(size_textfields[1]);
	splitter3_widgets.push_back(NULL);
	splitter3_widgets.push_back(NULL);
	splitter3_widgets.push_back(NULL);
	splitter3_widgets.push_back(NULL);
	splitter3_widgets.push_back(NULL);
	size_splitter1 = new TGUI_Splitter(
		0, 0, SIZE_FRAME_W/3, SIZE_FRAME_H-6-al_get_font_line_height(tgui::getFont()),
		TGUI_VERTICAL,
		false,
		splitter1_widgets
	);
	size_splitter2 = new TGUI_Splitter(
		0, 0, SIZE_FRAME_W/3, SIZE_FRAME_H-6-al_get_font_line_height(tgui::getFont()),
		TGUI_VERTICAL,
		false,
		splitter2_widgets
	);
	size_splitter3 = new TGUI_Splitter(
		0, 0, SIZE_FRAME_W/3, SIZE_FRAME_H-6-al_get_font_line_height(tgui::getFont()),
		TGUI_VERTICAL,
		false,
		splitter3_widgets
	);
	std::vector<tgui::TGUIWidget *> main_splitter_widgets;
	main_splitter_widgets.push_back(size_splitter1);
	main_splitter_widgets.push_back(size_splitter2);
	main_splitter_widgets.push_back(size_splitter3);
	main_size_splitter = new TGUI_Splitter(
		0,
		6+al_get_font_line_height(tgui::getFont()),
		SIZE_FRAME_W,
		SIZE_FRAME_H-6-al_get_font_line_height(tgui::getFont()),
		TGUI_HORIZONTAL,
		false,
		main_splitter_widgets	
	);
	size_splitter1->setDrawLines(false);
	size_splitter2->setDrawLines(false);
	size_splitter3->setDrawLines(false);
	main_size_splitter->setDrawLines(false);

	canvas_slider->setPosition(0.5);
	tile_slider->setPosition(0.5);

	tgui::setNewWidgetParent(0);
	tgui::addWidget(menu_splitter);
	showToolWidgets();

	bool quit = false;

	while (!quit) {
		bool redraw = false;
		while (!al_event_queue_is_empty(queue)) {
			ALLEGRO_EVENT event;
			al_get_next_event(queue, &event);
			tgui::handleEvent(&event);
		
			if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				quit = true;
				break;
			}
			else if (event.type == ALLEGRO_EVENT_TIMER) {
				redraw = true;
				tgui::TGUIWidget *widget = tgui::update();
				if (widget == quit_menu_item) {
					quit = true;
					break;
				}
				else if (widget == show_tiles_button) {
					tilesel->set_draw(true);
					menu_splitter->set_widget(1, tile_splitter);
					canvas_slider->remove();
					canvas_button_splitter->remove();
					tgui::addWidget(tile_slider);
				}
				else if (widget == hide_tiles_button) {
					tilesel->set_draw(false);
					menu_splitter->set_widget(1, canvas_splitter);
					tile_slider->remove();
					showToolWidgets();
				}
				else if (widget == plus_button ||
					 widget == move_button ||
					 widget == hand_button ||
					 widget == add_point_button ||
					 widget == delete_point_button ||
					 widget == connect_points_button) {
					for (int i = 0; i < 6; i++) {
						TGUI_IconButton *b = (TGUI_IconButton *)
							canvas_button_splitter_items[i];
						b->setClearColor(al_color_name("purple"));
					}
					TGUI_IconButton *b = (TGUI_IconButton *)widget;
					b->setClearColor(al_color_name("pink"));
					if (widget == plus_button) {
						using_hand_cursor = false;
						al_set_system_mouse_cursor(display, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
						int cw = canvas->getWidth() / canvas->getScale();
						int ch = canvas->getHeight() / canvas->getScale();
						ATLAS *a = tilesel->getAtlas();
						int sel = tilesel->getSelected();
						ATLAS_ITEM *i = atlas_get_item_by_index(
							a, sel
						);
						ALLEGRO_BITMAP *sub =
							atlas_get_item_sub_bitmap(
								i
							)->bitmap;
						int iw = al_get_bitmap_width(sub);
						int ih = al_get_bitmap_height(sub);
						int x = (cw-iw)/2;
						int y = (ch-ih)/2;
						// this select the tile
						canvas->addTile(sel, x, y);
					}
					else if (widget == move_button) {
						using_hand_cursor = false;
						al_set_system_mouse_cursor(display, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
						canvas->setTool(B_BattleCanvas::TOOL_MOVE);
					}
					else if (widget == hand_button) {
						using_hand_cursor = true;
						al_set_mouse_cursor(display, hand_cursor);
						canvas->setTool(B_BattleCanvas::TOOL_HAND);
					}
					else if (widget == add_point_button) {
						using_hand_cursor = false;
						al_set_system_mouse_cursor(display, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
						canvas->setTool(B_BattleCanvas::TOOL_ADD_POINT);
					}
					else if (widget == delete_point_button) {
						using_hand_cursor = false;
						al_set_system_mouse_cursor(display, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
						canvas->setTool(B_BattleCanvas::TOOL_DELETE_POINT);
					}
					else if (widget == connect_points_button) {
						using_hand_cursor = false;
						al_set_system_mouse_cursor(display, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
						canvas->setTool(B_BattleCanvas::TOOL_CONNECT_POINTS);
					}
				}
				else if (widget == insert_layer_before_item) {
					int layer = canvas->getLayer();
					canvas->addLayer(layer);
					create_layer_menus();
					setLayerMenusMenuBars();
				}
				else if (widget == insert_layer_after_item) {
					int layer = canvas->getLayer() + 1;
					if (layer >= canvas->getNumLayers())
						layer = -1;
					canvas->addLayer(layer);
					create_layer_menus();
					setLayerMenusMenuBars();
				}
				else if (widget == delete_layer_item) {
					if (canvas->getNumLayers() > 1) {
						int layer = canvas->getLayer();
						canvas->deleteLayer(layer);
						create_layer_menus();
						setLayerMenusMenuBars();
					}
				}
				else if (widget == open_menu_item) {
					load();
				}
				else if (widget == save_menu_item) {
					save();
				}
				else if (widget == save_as_menu_item) {
					save_as();
				}
				else if (widget == cut_menu_item) {
					clipboard = canvas->copy();
					canvas->_delete();
				}
				else if (widget == copy_menu_item) {
					clipboard = canvas->copy();
				}
				else if (widget == paste_menu_item) {
					canvas->paste(clipboard);
				}
				else if (widget == delete_menu_item) {
					canvas->_delete();
				}
				else if (widget == select_all_menu_item) {
					canvas->selectAll();
				}
				else if (widget == invert_selection_menu_item) {
					canvas->invertSelection();
				}
				else if (widget == undo_menu_item) {
					undo();
					canvas_scrollpane->setChild(canvas);
					canvas->setParent(canvas_scrollpane);
				}
				else if (widget == redo_menu_item) {
					redo();
					canvas_scrollpane->setChild(canvas);
					canvas->setParent(canvas_scrollpane);
				}
				else if (widget == size_menu_item && !in_size_menu) {
					in_size_menu = true;
					tgui::push();
					tgui::setNewWidgetParent(0);
					tgui::addWidget(size_frame);
					tgui::setNewWidgetParent(size_frame);
					tgui::addWidget(main_size_splitter);
					tgui::setNewWidgetParent(0);
					current_size_label = new TGUI_Label("" + itos(canvas->getWidth()) + "x" + itos(canvas->getHeight()), al_map_rgb(255, 255, 255), 0, 0, 0);
					size_splitter1->set_widget(0, current_size_label);
					for (int i = 0; i < 4; i++) {
						size_textfields[i]->setText("0");
					}
				}
				else if (widget == size_ok_button) {
					size_frame->remove();
					tgui::pop();
					in_size_menu = false;
					delete current_size_label;
					size_splitter1->set_widget(0, NULL);
					bool valid = true;
					for (int i = 0; i < 4; i++) {
						if (!size_textfields[i]->isValid()) {
							valid = false;
							break;
						}
					}
					if (valid) {
						canvas->adjustSize(
							atoi(size_textfields[0]->getText().c_str()),
							atoi(size_textfields[1]->getText().c_str()),
							atoi(size_textfields[2]->getText().c_str()),
							atoi(size_textfields[3]->getText().c_str())
						);
					}
				}
				else if (widget == parallax_menu_item) {
					load_parallax();
				}
				else if (widget == toggle_tools_menu_item) {
					showTools = !showTools;
					if (showTools)
						showToolWidgets();
					else
						hideToolWidgets();
				}
				else if (widget == unselect_point_menu_item) {
					canvas->clearSelectedPoint();
				}
				else {
					std::vector<tgui::TGUIWidget *> layer_sel_widgets = layer_select_menu->getWidgets();
					for (unsigned int i = 0; i < layer_sel_widgets.size(); i++) {
						if (widget == layer_sel_widgets[i]) {
							canvas->setLayer(radioGroup.selected);
							break;
						}
					}
					std::vector<tgui::TGUIWidget *> layer_toggle_widgets = layer_toggle_menu->getWidgets();
					for (unsigned int i = 0; i < layer_toggle_widgets.size(); i++) {
						if (widget == layer_toggle_widgets[i]) {
							TGUI_CheckMenuItem *item = dynamic_cast<TGUI_CheckMenuItem *>(widget);
							if (item) {
								canvas->setVisible(i, item->isChecked());
								break;
							}
						}
					}
				}
			}

			if (redraw) {
				al_clear_to_color(al_color_name("black"));
				tgui::draw();
				al_flip_display();
			}
			else {
				al_rest(0.001);
			}
		}
	}

	return 0;
}

ALLEGRO_BITMAP *my_load_bitmap(std::string filename)
{
	ALLEGRO_FILE *f;

	f = al_fopen(filename.c_str(), "rb");

	if (!f) {
		return NULL;
	}

	ALLEGRO_BITMAP *bmp = al_load_bitmap_f(f, ".png");

	al_fclose(f);

	return bmp;
}

ALLEGRO_SHADER *my_create_shader(std::string vertex_source, std::string pixel_source)
{
	// Not used
	return NULL;
}

