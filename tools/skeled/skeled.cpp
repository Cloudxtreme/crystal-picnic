#define ALLEGRO_STATICLINK
#include <cstdio>
#include <map>

#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>
#include <tgui2.hpp>
#include <tgui2_widgets.hpp>

#include "skeleton.h"
#include "config.h"
#include "collision_detection.h"
#include "crystalpicnic.h"
#include "resource_manager.h"

#include "skeled.hpp"
#include "widgets.hpp"
#include "widgets.cpp"

/* Main resources */
ALLEGRO_DISPLAY *display;
ALLEGRO_EVENT_QUEUE *queue;
ALLEGRO_FONT *font;
ALLEGRO_MENU *menu;
ALLEGRO_MENU *popup_menu;
Skeleton::Skeleton *skeleton;
General::Point<float> right_click_point;
std::string selected_part = "";
bool mouse_down = false;
General::Point<float> mouse_down_point;
bool playing = false;
Timeline *timeline;
std::vector<Skeleton::Skeleton *> undos;
std::vector<Skeleton::Skeleton *> redos;

/* Info about skeleton */
int curr_id = 1000;
int curr_part_id = 1000;
std::map<Skeleton::Animation *, int> anim_menu_id_map;
General::Point<float> center;
 
/* Main interface widgets */
TGUI_Frame *transform_frame;
TGUI_List *transform_list;
TGUI_Button *cancel_button;
tgui::TGUIWidget *modal_result;

/* Add transform modal dialog */
TGUI_Button *add_translate_transform_button;
TGUI_Button *add_rotate_transform_button;
TGUI_Button *add_scale_transform_button;
TGUI_Button *add_bitmap_transform_button;

/* Yes/No prompt modal dialog */
TGUI_Button *yes_button;
TGUI_Button *no_button;

/* Ok prompt modal dialog */
TGUI_Button *ok_button;

enum {
	FILE_ID = 1,
	FILE_OPEN_ID,
	FILE_SAVE_ID,
	FILE_EXPORT_PNGS_ID,
	FILE_QUIT_ID,
	EDIT_ID,
	EDIT_UNDO_ID,
	EDIT_REDO_ID,
	PART_ID,
	PART_DELETE_ID,
	PART_SELECT_NONE_ID,
	PART_EDIT_OUTLINE_ID,
	PART_PROPERTIES_ID,
	FRAME_ID,
	FRAME_NEW_ID,
	FRAME_CLONE_ID,
	FRAME_DELETE_ID,
	FRAME_PROPERTIES_ID,
	FRAME_VIEW_HIERARCHY_ID,
	ANIMATION_ID,
	ANIMATION_NEW_ID,
	ANIMATION_DELETE_ID,
	ANIMATION_PLAY_ID,
	ANIMATION_PROPERTIES_ID,
	NEW_PART_ID
};

ALLEGRO_MENU_INFO main_menu_info[] = {
	ALLEGRO_START_OF_MENU("File", FILE_ID),
		{ "Open", FILE_OPEN_ID, 0, NULL },
		{ "Save", FILE_SAVE_ID, 0, NULL },
		{ "Export PNGs", FILE_EXPORT_PNGS_ID, 0, NULL },
#ifndef ALLEGRO_MACOSX
		{ "Quit", FILE_QUIT_ID, 0, NULL },
#endif
		ALLEGRO_END_OF_MENU,
	
	ALLEGRO_START_OF_MENU("Edit", EDIT_ID),
		{ "Undo", EDIT_UNDO_ID, 0, NULL },
		{ "Redo", EDIT_REDO_ID, 0, NULL },
		ALLEGRO_END_OF_MENU,

	ALLEGRO_START_OF_MENU("Part", PART_ID),
		{ "Delete", PART_DELETE_ID, 0, NULL },
		{ "Select None", PART_SELECT_NONE_ID, 0, NULL },
		{ "Edit Bitmap Outline", PART_EDIT_OUTLINE_ID, 0, NULL },
		{ "Properties", PART_PROPERTIES_ID, 0, NULL },
		ALLEGRO_END_OF_MENU,
	
	ALLEGRO_START_OF_MENU("Frame", FRAME_ID),
		{ "New", FRAME_NEW_ID, 0, NULL },
		{ "Delete", FRAME_DELETE_ID, 0, NULL },
		{ "Properties", FRAME_PROPERTIES_ID, 0, NULL },
		ALLEGRO_MENU_SEPARATOR,
		{ "View Hierarchy", FRAME_VIEW_HIERARCHY_ID, 0, NULL },
		ALLEGRO_END_OF_MENU,
	
	ALLEGRO_START_OF_MENU("Animation", ANIMATION_ID),
		{ "New", ANIMATION_NEW_ID, 0, NULL },
		{ "Delete", ANIMATION_DELETE_ID, 0, NULL },
		{ "Play", ANIMATION_PLAY_ID, ALLEGRO_MENU_ITEM_CHECKBOX, NULL },
		{ "Properties", ANIMATION_PROPERTIES_ID, 0, NULL },
		ALLEGRO_MENU_SEPARATOR,
		ALLEGRO_END_OF_MENU,
	
	ALLEGRO_END_OF_MENU
};

static ALLEGRO_BITMAP *clone_target()
{
	ALLEGRO_BITMAP *target = al_get_target_bitmap();
	ALLEGRO_BITMAP *tmp = al_create_bitmap(
		al_get_bitmap_width(target),
		al_get_bitmap_height(target)
	);

#ifdef ALLEGRO_ANDROID
	const int MAX_SIZE = 512;

	int w = al_get_bitmap_width(target);
	int h = al_get_bitmap_height(target);
	
	int sz_w = ceil((float)w / MAX_SIZE);
	int sz_h = ceil((float)h / MAX_SIZE);

	for (int yy = 0; yy < sz_h; yy++) {
		for (int xx = 0; xx < sz_w; xx++) {
			int ww = MIN(MAX_SIZE, w-(xx*MAX_SIZE));
			int hh = MIN(MAX_SIZE, h-(yy*MAX_SIZE));
			ALLEGRO_LOCKED_REGION *lr1 = al_lock_bitmap_region(
				tmp,
				xx*MAX_SIZE, yy*MAX_SIZE, ww, hh,
				al_get_bitmap_format(target), ALLEGRO_LOCK_WRITEONLY
			);
			ALLEGRO_LOCKED_REGION *lr2 = al_lock_bitmap_region(
				target,
				xx*MAX_SIZE, yy*MAX_SIZE, ww, hh,
				al_get_bitmap_format(target), ALLEGRO_LOCK_READONLY
			);
			int pixel_size = al_get_pixel_size(al_get_bitmap_format(target));
			for (int y = 0; y < hh; y++) {
				uint8_t *d1 = (uint8_t *)lr1->data + lr1->pitch * y;
				uint8_t *d2 = (uint8_t *)lr2->data + lr2->pitch * y;
				memcpy(d1, d2, pixel_size*ww);
			}
			al_unlock_bitmap(tmp);
			al_unlock_bitmap(target);
		}
	}
#else
	ALLEGRO_BITMAP *old_target = al_get_target_bitmap();

	al_set_target_bitmap(tmp);

	al_clear_to_color(al_map_rgb_f(0, 0, 0));

	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	
	al_draw_bitmap(target, 0, 0, 0);

	al_set_target_bitmap(old_target);
#endif

	return tmp;
}

static void doModal(
	ALLEGRO_EVENT_QUEUE *queue,
	ALLEGRO_COLOR clear_color, // if alpha == 1 then don't use background image
	ALLEGRO_BITMAP *background,
	bool (*callback)(tgui::TGUIWidget *widget),
	bool (*check_draw_callback)(),
	void (*before_flip_callback)(),
	void (*resize_callback)()
	)
{
	ALLEGRO_BITMAP *back;
	if (clear_color.a != 0) {
		back = NULL;
	}
	else if (background) {
		back = background;
	}
	else {
		back = clone_target();
	}

	bool lost = false;

	int redraw = 0;
	ALLEGRO_TIMER *logic_timer = al_create_timer(1.0/60.0);
	al_register_event_source(queue, al_get_timer_event_source(logic_timer));
	al_start_timer(logic_timer);

	while (1) {
		ALLEGRO_EVENT event;

		while (!al_event_queue_is_empty(queue)) {
			al_wait_for_event(queue, &event);

			if (event.type == ALLEGRO_EVENT_TIMER && event.timer.source != logic_timer) {
				continue;
			}
			else if (event.type == ALLEGRO_EVENT_TIMER) {
				redraw++;
			}
			else if (event.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION) {
				al_reconfigure_joysticks();
			}
			else if (resize_callback && event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
				resize_callback();
			}
			else if (event.type == ALLEGRO_EVENT_DISPLAY_LOST) {
				lost = true;
			}
			else if (event.type == ALLEGRO_EVENT_DISPLAY_FOUND) {
				lost = false;
			}

			tgui::handleEvent(&event);

			tgui::TGUIWidget *w = tgui::update();

			if (callback(w)) {
				goto done;
			}
		}

		if (!lost && redraw && (!check_draw_callback || check_draw_callback())) {
			redraw = 0;

			al_clear_to_color(al_map_rgb_f(0.0f, 0.0f, 0.0f));
			ALLEGRO_TRANSFORM t, backup;
			al_copy_transform(&backup, al_get_current_transform());
			al_identity_transform(&t);
			al_use_transform(&t);
			if (back) {
				al_draw_tinted_bitmap(back, al_map_rgb_f(0.5f, 0.5f, 0.5f), 0, 0, 0);
			}
			else {
				al_clear_to_color(clear_color);
			}
			al_use_transform(&backup);

			tgui::draw();

			if (before_flip_callback) {
				before_flip_callback();
			}

			al_flip_display();
		}
	}

done:
	if (clear_color.a == 0 && !background) {
		al_destroy_bitmap(back);
	}

	al_destroy_timer(logic_timer);
}

void draw_blinking_rect(float x1, float y1, float x2, float y2)
{
	float f = fmod(al_get_time(), 0.5);
	ALLEGRO_COLOR c;
	if (f < 0.25) {
		c = al_color_name("white");
	}
	else {
		c = al_color_name("black");
	}

	al_draw_rectangle(x1, y1, x2, y2, c, 1);
}

void get_bitmap_icon_sizes(ALLEGRO_BITMAP *bmp, int *w, int *h, int *new_w, int *new_h)
{
	*w = al_get_bitmap_width(bmp);
	*h = al_get_bitmap_height(bmp);

	float r;

	if (*w > *h) {
		r = 100.0f / *w;
	}
	else {
		r = 100.0f / *h;
	}

	*new_w = *w * r;
	*new_h = *h * r;
}

static ALLEGRO_BITMAP *screencap()
{
	ALLEGRO_BITMAP *bmp = al_create_bitmap(al_get_display_width(display), al_get_display_height(display));
	al_set_target_bitmap(bmp);
	al_draw_bitmap(al_get_backbuffer(display), 0, 0, 0);
	al_set_target_backbuffer(display);
	return bmp;
}

static Skeleton::Animation *get_curr_anim()
{
	return skeleton->get_animations()[skeleton->get_curr_anim()];
}

static void add_bitmap_to_part_worker(Skeleton::Link *l, ALLEGRO_BITMAP *bitmap, std::string filename, std::vector<Bones::Bone> &bone, std::string part_name)
{
	if (l->part->get_name() == part_name) {
		l->part->get_bitmaps().push_back(new Wrap::Bitmap(al_clone_bitmap(bitmap), part_name));
		l->part->get_bitmap_names().push_back(filename);
		l->part->add_bone(bone);
	}

	for (int i = 0; i < l->num_children; i++) {
		add_bitmap_to_part_worker(l->children[i], bitmap, filename, bone, part_name);
	}
}

static void add_bitmap_to_part(ALLEGRO_BITMAP *bitmap, std::string filename, std::vector<Bones::Bone> &bone, std::string part_name)
{
	Skeleton::Animation *anim = get_curr_anim();

	for (size_t i = 0; i < anim->frames.size(); i++) {
		add_bitmap_to_part_worker(anim->frames[i], bitmap, filename, bone, part_name);
	}

	add_bitmap_to_part_worker(anim->work, bitmap, filename, bone, part_name);
}

void interpolate()
{
	Skeleton::Animation *anim = get_curr_anim();
	Skeleton::interpolate(
		0.0f,
		anim->frames[anim->curr_frame],
		anim->frames[anim->curr_frame],
		anim->work
	);
}

static Skeleton::Link *find_link_named(Skeleton::Link *node, const std::string &name)
{
	if (node->part && node->part->get_name() == name) {
		return node;
	}

	for (int i = 0; i < node->num_children; i++) {
		Skeleton::Link *l = find_link_named(node->children[i], name);
		if (l) return l;
	}

	return NULL;
}

static void update_transform_list()
{
	std::vector<std::string> transform_names;

	if (selected_part != "") {
		Skeleton::Animation *anim = get_curr_anim();
		std::vector<Skeleton::Transform *> &transforms = find_link_named(anim->frames[anim->curr_frame], selected_part)->part->get_transforms();

		for (size_t i = 0; i < transforms.size(); i++) {
			Skeleton::Transform *t = transforms[i];
			switch (t->type) {
				case Skeleton::TRANSLATION:
					transform_names.push_back("Translation");
					break;
				case Skeleton::ROTATION:
					transform_names.push_back("Rotation");
					break;
				case Skeleton::SCALE:
					transform_names.push_back("Scale");
					break;
				case Skeleton::BITMAP:
					transform_names.push_back("Bitmap");
					break;
			}
		}
	}

	transform_list->setLabels(transform_names);
}

static void remove_transform()
{
	int sel = transform_list->getSelected();

	Skeleton::Animation *anim = get_curr_anim();

	std::vector<Skeleton::Transform *> &transforms =
		find_link_named(anim->work, selected_part)->part->get_transforms();
	if (transforms.size() == 0) {
		return;
	}
	transforms.erase(transforms.begin()+sel);

	for (size_t i = 0; i < anim->frames.size(); i++) {
		std::vector<Skeleton::Transform *> &transforms =
			find_link_named(anim->frames[i], selected_part)->part->get_transforms();
		transforms.erase(transforms.begin()+sel);
	}

	if (transforms.size() == 0) {
		transform_list->setSelected(-1);
	}
	else if (sel >= (int)transforms.size()) {
		sel--;
		transform_list->setSelected(sel);
	}

	update_transform_list();
}

static void move_transform_up()
{
	int sel = transform_list->getSelected();

	if (sel == 0) {
		return;
	}

	Skeleton::Animation *anim = get_curr_anim();

	std::vector<Skeleton::Transform *> &transforms =
		find_link_named(anim->work, selected_part)->part->get_transforms();
	if (transforms.size() == 0) {
		return;
	}
	Skeleton::Transform *t = transforms[sel];
	transforms.erase(transforms.begin()+sel);
	transforms.insert(transforms.begin()+(sel-1), t);

	for (size_t i = 0; i < anim->frames.size(); i++) {
		std::vector<Skeleton::Transform *> &transforms =
			find_link_named(anim->frames[i], selected_part)->part->get_transforms();
		Skeleton::Transform *t = transforms[sel];
		transforms.erase(transforms.begin()+sel);
		transforms.insert(transforms.begin()+(sel-1), t);
	}

	sel--;
	transform_list->setSelected(sel);

	update_transform_list();
}

static void move_transform_down()
{
	int sel = transform_list->getSelected();

	if (sel >= (int)transform_list->getLabels().size()-1) {
		return;
	}

	Skeleton::Animation *anim = get_curr_anim();

	std::vector<Skeleton::Transform *> &transforms =
		find_link_named(anim->work, selected_part)->part->get_transforms();
	if (transforms.size() == 0) {
		return;
	}
	Skeleton::Transform *t = transforms[sel];
	transforms.erase(transforms.begin()+sel);
	transforms.insert(transforms.begin()+(sel+1), t);

	for (size_t i = 0; i < anim->frames.size(); i++) {
		std::vector<Skeleton::Transform *> &transforms =
			find_link_named(anim->frames[i], selected_part)->part->get_transforms();
		Skeleton::Transform *t = transforms[sel];
		transforms.erase(transforms.begin()+sel);
		transforms.insert(transforms.begin()+(sel+1), t);
	}

	sel++;
	transform_list->setSelected(sel);

	update_transform_list();
}

void draw_frame_number()
{
	if (skeleton->get_animations().size() == 0 || skeleton->get_animations()[skeleton->get_curr_anim()]->frames.size() == 0) {
		return;
	}

	int x = 10;
	int y = al_get_display_height(display) - 130;
	ALLEGRO_COLOR c = al_map_rgba_f(0.8f, 0.8f, 0.8f, 0.8f);
	Skeleton::Animation *anim = get_curr_anim();
	al_draw_textf(tgui::getFont(), c, x, y, 0, "frame:%d", anim->curr_frame);
}

void draw_transform_info()
{
	if (selected_part == "") {
		return;
	}

	Skeleton::Animation *anim = get_curr_anim();
	std::vector<Skeleton::Transform *> &transforms = find_link_named(anim->frames[anim->curr_frame], selected_part)->part->get_transforms();

	if (transforms.size() == 0 || transform_list->getSelected() < 0) {
		return;
	}

	Skeleton::Transform *t = transforms[transform_list->getSelected()];

	int x = al_get_display_width(display) - 10;
	int y = al_get_display_height(display) - 130;
	ALLEGRO_COLOR c = al_map_rgba_f(0.8f, 0.8f, 0.8f, 0.8f);

	switch (t->type) {
		case Skeleton::TRANSLATION:
			al_draw_textf(tgui::getFont(), c, x, y, ALLEGRO_ALIGN_RIGHT, "x:%f y:%f", t->x, t->y);
			break;
		case Skeleton::ROTATION:
			al_draw_textf(tgui::getFont(), c, x, y, ALLEGRO_ALIGN_RIGHT, "angle:%f", t->angle);
			break;
		case Skeleton::SCALE:
			al_draw_textf(tgui::getFont(), c, x, y, ALLEGRO_ALIGN_RIGHT, "sx:%f sy:%f", t->scale_x, t->scale_y);
			break;
		case Skeleton::BITMAP:
			al_draw_textf(tgui::getFont(), c, x, y, ALLEGRO_ALIGN_RIGHT, "bitmap:%d", t->bitmap_index);
			break;
	}
}

static void create_cancel_button()
{
	if (cancel_button == NULL) {
		cancel_button = new TGUI_Button("Cancel", 0, 0, 90, 25);
	}
}

static bool callback(tgui::TGUIWidget *w)
{
	if (w == NULL) return false;
	modal_result = w;
	return true;
}

static void create_transform_type_dialog()
{
	add_translate_transform_button = new TGUI_Button("Translate", 0, 0, 1, 1);
	add_rotate_transform_button = new TGUI_Button("Rotate", 0, 0, 1, 1);
	add_scale_transform_button = new TGUI_Button("Scale", 0, 0, 1, 1);
	add_bitmap_transform_button = new TGUI_Button("Bitmap", 0, 0, 1, 1);
}

static void show_transform_type_dialog()
{
	tgui::push();

	add_translate_transform_button->setX(0);
	add_rotate_transform_button->setX(0);
	add_scale_transform_button->setX(0);
	add_bitmap_transform_button->setX(0);
	cancel_button->setX(0);

	int lh = al_get_font_line_height(tgui::getFont());
	TGUI_Frame *transform_type_frame = new TGUI_Frame("Transform Type", 0, 0, 510, lh+TGUI_Frame::TITLE_PADDING*2 + 45);
	std::vector<tgui::TGUIWidget *> widgets;
	widgets.push_back(add_translate_transform_button);
	widgets.push_back(add_rotate_transform_button);
	widgets.push_back(add_scale_transform_button);
	widgets.push_back(add_bitmap_transform_button);
	widgets.push_back(cancel_button);
	TGUI_Splitter *transform_type_splitter = new TGUI_Splitter(
		5, transform_type_frame->barHeight()+10,
		500, 25,
		TGUI_HORIZONTAL,
		false,
		widgets
	);
	transform_type_splitter->setDrawLines(false);
	transform_type_splitter->setPadding(5, 0);
	transform_type_splitter->setClearColor(al_color_name("slategray"));

	tgui::setNewWidgetParent(NULL);
	tgui::addWidget(transform_type_frame);
	tgui::setNewWidgetParent(transform_type_frame);
	tgui::addWidget(transform_type_splitter);

	tgui::centerWidget(transform_type_frame, al_get_display_width(display)/2, al_get_display_height(display)/2);

	ALLEGRO_BITMAP *bg = screencap();
	doModal(queue, al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f), bg, callback, NULL, NULL, NULL);
	al_destroy_bitmap(bg);

	tgui::pop();
}

static void create_yes_no_dialog()
{
	yes_button = new TGUI_Button("Yes", 0, 0, 1, 1);
	no_button = new TGUI_Button("No", 0, 0, 1, 1);
}

static void show_yes_no_dialog(std::string title)
{
	tgui::push();

	yes_button->setX(0);
	no_button->setX(0);

	int lh = al_get_font_line_height(tgui::getFont());
	TGUI_Frame *frame = new TGUI_Frame(title.c_str(), 0, 0, 210, lh+TGUI_Frame::TITLE_PADDING*2 + 45);
	std::vector<tgui::TGUIWidget *> widgets;
	widgets.push_back(yes_button);
	widgets.push_back(no_button);
	TGUI_Splitter *splitter = new TGUI_Splitter(
		5, frame->barHeight()+10,
		200, 25,
		TGUI_HORIZONTAL,
		false,
		widgets
	);
	splitter->setDrawLines(false);
	splitter->setPadding(5, 0);
	splitter->setClearColor(al_color_name("slategray"));

	tgui::setNewWidgetParent(NULL);
	tgui::addWidget(frame);
	tgui::setNewWidgetParent(frame);
	tgui::addWidget(splitter);

	tgui::centerWidget(frame, al_get_display_width(display)/2, al_get_display_height(display)/2);

	ALLEGRO_BITMAP *bg = screencap();
	doModal(queue, al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f), bg, callback, NULL, NULL, NULL);
	al_destroy_bitmap(bg);

	tgui::pop();
}

static void create_ok_dialog()
{
	ok_button = new TGUI_Button("OK", 0, 0, 50, 25);
	tgui::centerWidget(ok_button, 205, 35/2);
	ok_button->setTamperingEnabled(false);
}

static void show_ok_dialog(std::string caption)
{
	tgui::push();

	int lh = al_get_font_line_height(tgui::getFont());

	TGUI_Label *label = new TGUI_Label(caption, al_color_name("black"), 0, 0, 0);
	tgui::centerWidget(label, 205, (lh+10)/2);
	label->setTamperingEnabled(false);

	TGUI_Frame *frame = new TGUI_Frame("Message", 0, 0, 420, lh+TGUI_Frame::TITLE_PADDING*2 + lh + 55);
	std::vector<tgui::TGUIWidget *> widgets;
	widgets.push_back(label);
	widgets.push_back(ok_button);
	TGUI_Splitter *splitter = new TGUI_Splitter(
		10, frame->barHeight()+5,
		410, 45+lh,
		TGUI_VERTICAL,
		false,
		widgets
	);
	splitter->set_size(0, lh+10);
	splitter->setDrawLines(false);
	splitter->setPadding(0, 5);
	splitter->setClearColor(al_color_name("slategray"));

	tgui::setNewWidgetParent(NULL);
	tgui::addWidget(frame);
	tgui::setNewWidgetParent(frame);
	tgui::addWidget(splitter);

	tgui::centerWidget(frame, al_get_display_width(display)/2, al_get_display_height(display)/2);

	ALLEGRO_BITMAP *bg = screencap();
	doModal(queue, al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f), bg, callback, NULL, NULL, NULL);
	al_destroy_bitmap(bg);

	tgui::pop();
}

/* returns -1 for cancel, -2 for add else 0-8 */
int show_bitmap_select_dialog(Skeleton::Part *p)
{
	tgui::push();

	int lh = al_get_font_line_height(tgui::getFont());
	
	TGUI_Button *cancel_button = new TGUI_Button("Cancel", 0, 0, 90, 25);
	TGUI_Button *add_button = new TGUI_Button("Add", 0, 0, 90, 25);

	TGUI_Frame *frame = new TGUI_Frame("Choose Image", 0, 0, 340, lh+TGUI_Frame::TITLE_PADDING*2 + 450);

	TGUI_Icon *icons[9] = { NULL, };

	std::vector<Wrap::Bitmap *> bitmaps = p->get_bitmaps();

	for (int i = 0; i < 9; i++) {
		if (i < (int)bitmaps.size()) {
			ALLEGRO_COLOR border_color;

			int sel = 0;
			Skeleton::Animation *anim = get_curr_anim();
			Skeleton::Part *p = find_link_named(anim->frames[anim->curr_frame], selected_part)->part;
			std::vector<Skeleton::Transform *> &transforms = p->get_transforms();
			for (size_t j = 0; j < transforms.size(); j++) {
				if (transforms[j]->type == Skeleton::BITMAP) {
					sel = transforms[j]->bitmap_index;
					break;
				}
			}

			if (i == sel) {
				border_color = al_color_name("white");
			}
			else {
				border_color = al_map_rgba_f(0.1f, 0.1f, 0.1f, 0.1f);
			}

			ALLEGRO_BITMAP *bmp = al_create_bitmap(100, 100);
			ALLEGRO_BITMAP *target = al_get_target_bitmap();
			al_set_target_bitmap(bmp);
			al_clear_to_color(al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f));
			al_draw_rectangle(0.5, 0.5, 98.5, 98.5, border_color, 1);

			int w, h, new_w, new_h;
			get_bitmap_icon_sizes(bitmaps[i]->bitmap, &w, &h, &new_w, &new_h);

			al_draw_scaled_bitmap(
				bitmaps[i]->bitmap,
				0, 0, w, h,
				50-new_w/2, 50-new_h/2,
				new_w, new_h,
				0
			);
			al_set_target_bitmap(target);
			icons[i] = new TGUI_Icon(bmp, 0, 0, 0);
		}
		else {
			icons[i] = NULL;
		}
	}

	std::vector<tgui::TGUIWidget *> widgets[5];
	TGUI_Splitter *hsplitters[4];

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			widgets[i].push_back(icons[(i*3)+j]);
		}
		hsplitters[i] = new TGUI_Splitter(
			0, 0,
			330, 100,
			TGUI_HORIZONTAL,
			false,
			widgets[i]
		);
		hsplitters[i]->setDrawLines(false);
		hsplitters[i]->setPadding(5, 5);
		hsplitters[i]->setClearColor(al_color_name("slategray"));
	}
	widgets[3].push_back(NULL);
	widgets[3].push_back(cancel_button);
	widgets[3].push_back(add_button);
	hsplitters[3] = new TGUI_Splitter(
		0, 0,
		330, 100,
		TGUI_HORIZONTAL,
		false,
		widgets[3]
	);
	hsplitters[3]->setDrawLines(false);
	hsplitters[3]->setPadding(5, 5);
	hsplitters[3]->setClearColor(al_color_name("slategray"));

	for (int i = 0; i < 4; i++) {
		widgets[4].push_back(hsplitters[i]);
	}

	TGUI_Splitter *vsplitter = new TGUI_Splitter(
		5, frame->barHeight()+5,
		330, 440,
		TGUI_VERTICAL,
		false,
		widgets[4]
	);
	vsplitter->setDrawLines(false);
	vsplitter->setPadding(0, 0);
	vsplitter->setClearColor(al_color_name("slategray"));

	tgui::setNewWidgetParent(NULL);
	tgui::addWidget(frame);
	tgui::setNewWidgetParent(frame);
	tgui::addWidget(vsplitter);

	tgui::centerWidget(frame, al_get_display_width(display)/2, al_get_display_height(display)/2);

	ALLEGRO_BITMAP *bg = screencap();
	doModal(queue, al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f), bg, callback, NULL, NULL, NULL);
	al_destroy_bitmap(bg);

	int ret = -1;

	for (int i = 0; i < 3; i++) {
		for (size_t j = 0; j < widgets[i].size(); j++) {
			if (modal_result && modal_result == widgets[i][j]) {
				ret = (i*3)+j;
				break;
			}
		}
		if (ret != -1) break;
	}

	tgui::pop();
	
	if (modal_result == add_button) {
		ret = -2;
	}

	for (int i = 0; i < 4; i++) {
		for (size_t j = 0; j < widgets[i].size(); j++) {
			delete widgets[i][j];
		}
		delete hsplitters[i];
	}

	return ret;
}

static bool int_validator(std::string s)
{
	const char *p = s.c_str();
	while (*p) {
		if (!isdigit(*p)) {
			return false;
		}
		p++;
	}
	return true;
}

static bool signed_int_validator(std::string s)
{
	const char *p = s.c_str();
	while (*p) {
		if (!isdigit(*p) && (*p != '-')) {
			return false;
		}
		p++;
	}
	return true;
}

static bool framenum_validator(std::string s)
{
	if (!int_validator(s))
		return false;
	Skeleton::Animation *anim = get_curr_anim();
	return (atoi(s.c_str()) >= 0 && atoi(s.c_str()) < (int)anim->frames.size());
}

static void show_frame_properties_dialog()
{
	tgui::push();

	int lh = al_get_font_line_height(tgui::getFont());
	TGUI_Frame *frame = new TGUI_Frame("Frame Properties", 0, 0, 430, lh+TGUI_Frame::TITLE_PADDING*2 + 115);

	Skeleton::Animation *anim = get_curr_anim();
	
	TGUI_Label *label1 = new TGUI_Label("Frame number (0-" + General::itos(anim->frames.size()-1) + "):", al_color_name("black"), 0, 0, 0);
	TGUI_Label *label2 = new TGUI_Label("Duration (millis):", al_color_name("black"), 0, 0, 0);

	TGUI_TextField *tf1 = new TGUI_TextField(General::itos(anim->curr_frame), 0, 0, 200);
	TGUI_TextField *tf2 = new TGUI_TextField(General::itos(anim->delays[anim->curr_frame]), 0, 0, 200);

	tf1->setValidator(framenum_validator);
	tf2->setValidator(int_validator);

	TGUI_Button *ok_button = new TGUI_Button("OK", 0, 0, 1, 1);

	std::vector<tgui::TGUIWidget *> widgets;
	
	widgets.push_back(label1);
	widgets.push_back(tf1);

	TGUI_Splitter *hsplitters[2];
	
	hsplitters[0] = new TGUI_Splitter(
		0, 0,
		420, 35,
		TGUI_HORIZONTAL,
		false,
		widgets
	);
	hsplitters[0]->setDrawLines(false);
	hsplitters[0]->setPadding(5, 5);
	hsplitters[0]->setClearColor(al_color_name("slategray"));

	widgets.clear();
	widgets.push_back(label2);
	widgets.push_back(tf2);

	hsplitters[1] = new TGUI_Splitter(
		0, 0,
		420, 35,
		TGUI_HORIZONTAL,
		false,
		widgets
	);
	hsplitters[1]->setDrawLines(false);
	hsplitters[1]->setPadding(5, 5);
	hsplitters[1]->setClearColor(al_color_name("slategray"));

	widgets.clear();
	widgets.push_back(hsplitters[0]);
	widgets.push_back(hsplitters[1]);
	widgets.push_back(ok_button);

	TGUI_Splitter *vsplitter = new TGUI_Splitter(
		5, frame->barHeight()+5,
		420, 105,
		TGUI_VERTICAL,
		false,
		widgets
	);
	vsplitter->setDrawLines(false);
	vsplitter->setPadding(0, 0);
	vsplitter->setClearColor(al_color_name("slategray"));

	tgui::setNewWidgetParent(NULL);
	tgui::addWidget(frame);
	tgui::setNewWidgetParent(frame);
	tgui::addWidget(vsplitter);

	tgui::centerWidget(frame, al_get_display_width(display)/2, al_get_display_height(display)/2);

	ALLEGRO_BITMAP *bg = screencap();
	doModal(queue, al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f), bg, callback, NULL, NULL, NULL);
	al_destroy_bitmap(bg);

	if (tf1->isValid()) {
		std::string fnS = tf1->getText();
		int fn = atoi(fnS.c_str());
		Skeleton::Link *l = anim->frames[anim->curr_frame];
		int delay = anim->delays[anim->curr_frame];
		anim->frames.erase(anim->frames.begin() + anim->curr_frame);
		anim->delays.erase(anim->delays.begin() + anim->curr_frame);
		anim->curr_frame = fn;
		anim->frames.insert(anim->frames.begin()+fn, l);
		anim->delays.insert(anim->delays.begin()+fn, delay);
		interpolate();
	}

	if (tf2->isValid()) {
		anim->delays[anim->curr_frame] = atoi(tf2->getText().c_str());
	}

	tgui::pop();

	delete hsplitters[0];
	delete hsplitters[1];
	delete label1;
	delete label2;
	delete tf1;
	delete tf2;
	delete ok_button;
}

/*
static bool float_validator(std::string s)
{
	const char *p = s.c_str();
	int count = 0;
	int ndecimals = 0;
	while (*p) {
		if (!((count == 0 && *p == '-') || (ndecimals == 0 && *p == '.') || isdigit(*p))) {
			return false;
		}
		p++;
	}
	return true;
}
*/

static bool anim_name_validator(std::string s)
{
	const char *p = s.c_str();

	while (*p) {
		if (isspace(*p) || isdigit(*p) || (*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) {
			p++;
			continue;
		}
		return false;
	}

	return true;
}

static void show_png_export_dialog()
{
	tgui::push();

	int lh = al_get_font_line_height(tgui::getFont());
	TGUI_Frame *frame = new TGUI_Frame("Export PNG", 0, 0, 430, lh+TGUI_Frame::TITLE_PADDING*2 + 115);

	TGUI_Label *label1 = new TGUI_Label("Prefix:", al_color_name("black"), 0, 0, 0);
	TGUI_Label *label2 = new TGUI_Label("Step (millis):", al_color_name("black"), 0, 0, 0);

	TGUI_TextField *tf1 = new TGUI_TextField("", 0, 0, 200);
	TGUI_TextField *tf2 = new TGUI_TextField("100", 0, 0, 200);

	tf1->setValidator(anim_name_validator);
	tf2->setValidator(int_validator);

	TGUI_Button *ok_button = new TGUI_Button("OK", 0, 0, 1, 1);

	std::vector<tgui::TGUIWidget *> widgets;
	
	widgets.push_back(label1);
	widgets.push_back(tf1);

	TGUI_Splitter *hsplitters[2];
	
	hsplitters[0] = new TGUI_Splitter(
		0, 0,
		420, 35,
		TGUI_HORIZONTAL,
		false,
		widgets
	);
	hsplitters[0]->setDrawLines(false);
	hsplitters[0]->setPadding(5, 5);
	hsplitters[0]->setClearColor(al_color_name("slategray"));

	widgets.clear();
	widgets.push_back(label2);
	widgets.push_back(tf2);

	hsplitters[1] = new TGUI_Splitter(
		0, 0,
		420, 35,
		TGUI_HORIZONTAL,
		false,
		widgets
	);
	hsplitters[1]->setDrawLines(false);
	hsplitters[1]->setPadding(5, 5);
	hsplitters[1]->setClearColor(al_color_name("slategray"));

	widgets.clear();
	widgets.push_back(hsplitters[0]);
	widgets.push_back(hsplitters[1]);
	widgets.push_back(ok_button);

	TGUI_Splitter *vsplitter = new TGUI_Splitter(
		5, frame->barHeight()+5,
		420, 105,
		TGUI_VERTICAL,
		false,
		widgets
	);
	vsplitter->setDrawLines(false);
	vsplitter->setPadding(0, 0);
	vsplitter->setClearColor(al_color_name("slategray"));

	tgui::setNewWidgetParent(NULL);
	tgui::addWidget(frame);
	tgui::setNewWidgetParent(frame);
	tgui::addWidget(vsplitter);

	tgui::centerWidget(frame, al_get_display_width(display)/2, al_get_display_height(display)/2);

	ALLEGRO_BITMAP *bg = screencap();
	doModal(queue, al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f), bg, callback, NULL, NULL, NULL);
	al_destroy_bitmap(bg);

	if (tf1->isValid() && tf2->isValid()) {
		std::string prefix = tf1->getText();
		std::string millisS = tf2->getText();
		int millis = atoi(millisS.c_str());
		Skeleton::Animation *anim = get_curr_anim();
		anim->curr_frame = 0;
		anim->curr_time = 0;
		int total_time = 0;
		for (size_t i = 0; i < anim->delays.size(); i++) {
			total_time += anim->delays[i];
		}
		int nframes = total_time / millis;
		int time_per_frame = total_time / nframes;
		int elapsed = 0;
		int frames_written = 0;
		for (;;) {
			elapsed += General::LOGIC_MILLIS;
			skeleton->update(General::LOGIC_MILLIS);
			if (elapsed >= time_per_frame) {
				elapsed -= time_per_frame;
				ALLEGRO_BITMAP *bmp = al_create_bitmap(800, 600);
				ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
				al_set_target_bitmap(bmp);
				draw(false, false);
				al_set_target_bitmap(old_target);
				char fn[1000];
				sprintf(fn, "%s%04d.png", prefix.c_str(), frames_written);
				al_save_bitmap(fn, bmp);
				frames_written++;
			}
			if (frames_written >= nframes) {
				break;
			}
		}
		anim->curr_frame = 0;
		anim->curr_time = 0;
		interpolate();
	}

	tgui::pop();

	delete hsplitters[0];
	delete hsplitters[1];
	delete label1;
	delete label2;
	delete tf1;
	delete tf2;
	delete ok_button;
}

static void show_animation_properties_dialog()
{
	tgui::push();

	int lh = al_get_font_line_height(tgui::getFont());
	TGUI_Frame *frame = new TGUI_Frame("Frame Properties", 0, 0, 430, lh+TGUI_Frame::TITLE_PADDING*2 + 90);

	Skeleton::Animation *anim = get_curr_anim();
	
	TGUI_Label *label = new TGUI_Label("Animation name:", al_color_name("black"), 0, 0, 0);

	TGUI_TextField *tf = new TGUI_TextField(anim->name, 0, 0, 200);

	tf->setValidator(anim_name_validator);

	TGUI_Button *ok_button = new TGUI_Button("OK", 0, 0, 1, 1);

	std::vector<tgui::TGUIWidget *> widgets;
	
	widgets.push_back(label);
	widgets.push_back(tf);

	TGUI_Splitter *hsplitter;
	
	hsplitter = new TGUI_Splitter(
		0, 0,
		420, 35,
		TGUI_HORIZONTAL,
		false,
		widgets
	);
	hsplitter->setDrawLines(false);
	hsplitter->setPadding(5, 5);
	hsplitter->setClearColor(al_color_name("slategray"));

	widgets.clear();
	widgets.push_back(hsplitter);
	widgets.push_back(ok_button);

	TGUI_Splitter *vsplitter = new TGUI_Splitter(
		5, frame->barHeight()+5,
		420, 80,
		TGUI_VERTICAL,
		false,
		widgets
	);
	vsplitter->setDrawLines(false);
	vsplitter->setPadding(0, 0);
	vsplitter->setClearColor(al_color_name("slategray"));

	tgui::setNewWidgetParent(NULL);
	tgui::addWidget(frame);
	tgui::setNewWidgetParent(frame);
	tgui::addWidget(vsplitter);

	tgui::centerWidget(frame, al_get_display_width(display)/2, al_get_display_height(display)/2);

	ALLEGRO_BITMAP *bg = screencap();
	doModal(queue, al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f), bg, callback, NULL, NULL, NULL);
	al_destroy_bitmap(bg);

	if (tf->isValid()) {
		std::string name = tf->getText();
		anim->name = name;
		int index = -(skeleton->get_curr_anim() + 5);
		al_set_menu_item_caption(al_find_menu(menu, ANIMATION_ID), index, name.c_str());
	}

	tgui::pop();

	delete hsplitter;
	delete label;
	delete tf;
	delete ok_button;
}

static void show_part_properties_dialog()
{
	tgui::push();

	int lh = al_get_font_line_height(tgui::getFont());
	TGUI_Frame *frame = new TGUI_Frame("Part Properties", 0, 0, 430, lh+TGUI_Frame::TITLE_PADDING*2 + 90);

	Skeleton::Animation *anim = get_curr_anim();
	
	TGUI_Label *label = new TGUI_Label("Layer:", al_color_name("black"), 0, 0, 0);

	Skeleton::Link *l = find_link_named(anim->work, selected_part);
	TGUI_TextField *tf = new TGUI_TextField(General::itos(l->part->get_layer()), 0, 0, 200);

	tf->setValidator(signed_int_validator);

	TGUI_Button *ok_button = new TGUI_Button("OK", 0, 0, 1, 1);

	std::vector<tgui::TGUIWidget *> widgets;
	
	widgets.push_back(label);
	widgets.push_back(tf);

	TGUI_Splitter *hsplitter;
	
	hsplitter = new TGUI_Splitter(
		0, 0,
		420, 35,
		TGUI_HORIZONTAL,
		false,
		widgets
	);
	hsplitter->setDrawLines(false);
	hsplitter->setPadding(5, 5);
	hsplitter->setClearColor(al_color_name("slategray"));

	widgets.clear();
	widgets.push_back(hsplitter);
	widgets.push_back(ok_button);

	TGUI_Splitter *vsplitter = new TGUI_Splitter(
		5, frame->barHeight()+5,
		420, 80,
		TGUI_VERTICAL,
		false,
		widgets
	);
	vsplitter->setDrawLines(false);
	vsplitter->setPadding(0, 0);
	vsplitter->setClearColor(al_color_name("slategray"));

	tgui::setNewWidgetParent(NULL);
	tgui::addWidget(frame);
	tgui::setNewWidgetParent(frame);
	tgui::addWidget(vsplitter);

	tgui::centerWidget(frame, al_get_display_width(display)/2, al_get_display_height(display)/2);

	ALLEGRO_BITMAP *bg = screencap();
	doModal(queue, al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f), bg, callback, NULL, NULL, NULL);
	al_destroy_bitmap(bg);

	if (tf->isValid()) {
		int layer = atoi(tf->getText().c_str());
		for (size_t i = 0; i < anim->frames.size(); i++) {
			Skeleton::Link *l = find_link_named(anim->frames[i], selected_part);
			if (l) {
				l->part->set_layer(layer);
			}
		}
		Skeleton::Link *l = find_link_named(anim->work, selected_part);
		if (l) {
			l->part->set_layer(layer);
		}
	}

	tgui::pop();

	delete hsplitter;
	delete label;
	delete tf;
	delete ok_button;
}

static void show_frame_hierarchy()
{
	tgui::push();

	TGUI_Frame *frame = new TGUI_Frame("Hierarchy", 0, 0, 800, 600);

	Hierarchy *hierarchy = new Hierarchy(0, 0, 800, 600-25-frame->barHeight(), skeleton);
	
	TGUI_Button *ok_button = new TGUI_Button("OK", 0, 0, 1, 1);

	std::vector<tgui::TGUIWidget *> widgets;
	
	widgets.push_back(hierarchy);
	widgets.push_back(ok_button);

	TGUI_Splitter *vsplitter = new TGUI_Splitter(
		0, frame->barHeight(),
		800, 600-frame->barHeight(),
		TGUI_VERTICAL,
		false,
		widgets
	);
	vsplitter->setDrawLines(false);
	vsplitter->setPadding(0, 0);
	vsplitter->setClearColor(al_color_name("slategray"));

	vsplitter->set_size(0, 600-25-frame->barHeight());
	vsplitter->set_size(1, 25);

	tgui::setNewWidgetParent(NULL);
	tgui::addWidget(frame);
	tgui::setNewWidgetParent(frame);
	tgui::addWidget(vsplitter);

	tgui::centerWidget(frame, al_get_display_width(display)/2, al_get_display_height(display)/2);

	ALLEGRO_BITMAP *bg = screencap();
	doModal(queue, al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f), bg, callback, NULL, NULL, NULL);
	al_destroy_bitmap(bg);

	tgui::pop();

	delete hierarchy;
	delete ok_button;
}

static void create_modal_dialog_widgets()
{
	create_cancel_button();
	create_transform_type_dialog();
	create_yes_no_dialog();
	create_ok_dialog();
}

static void real_find_link(Skeleton::Link *link, General::Point<float> p, Skeleton::Link **result)
{
	if (link->part != NULL) {
		std::vector< std::vector<Bones::Bone> > &transformed_bones = link->part->get_transformed_bones();

		int bitmap = link->part->get_curr_bitmap();

		std::vector<Bones::Bone> &v = transformed_bones[bitmap];

		for (size_t i = 0; i < v.size(); i++) {
			Bones::Bone &b = v[i];
			std::vector<Triangulate::Triangle> &tris = b.get();
			for (size_t j = 0; j < tris.size(); j++) {
				Triangulate::Triangle &t = tris[j];
				if (checkcoll_point_triangle(&p, &t.points[0], &t.points[1], &t.points[2])) {
					*result = link;
				}
			}
		}
	}

	for (int i = 0; i < link->num_children; i++) {
		real_find_link(link->children[i], p, result);
	}
}

static Skeleton::Link *find_link_with_point(General::Point<float> p)
{
	p.x -= center.x;
	p.y -= center.y;

	skeleton->transform(center, false);

	if (skeleton->get_animations().size() == 0) {
		return NULL;
	}

	Skeleton::Animation *anim = get_curr_anim();
	Skeleton::Link *root = anim->work;

	if (!root) {
		return NULL;
	}

	Skeleton::Link *l = NULL;
	real_find_link(root, p, &l);

	if (l == NULL) {
		return NULL;
	}

	return l;
}

static void insert_part_worker(Skeleton::Part *p, Skeleton::Link *parent)
{
	Skeleton::Part *cloned = p->clone();
	if (parent->num_children > 0) {
		Skeleton::Link **children = new Skeleton::Link *[parent->num_children];
		memcpy(children, parent->children, parent->num_children*sizeof(Skeleton::Link *));
		delete[] parent->children;
		parent->num_children++;
		parent->children = new Skeleton::Link *[parent->num_children];
		memcpy(parent->children, children, (parent->num_children-1)*sizeof(Skeleton::Link *));
		delete[] children;
		Skeleton::Link *l = Skeleton::new_link();
		l->part = cloned;
		parent->children[parent->num_children-1] = l;
	}
	else {
		parent->num_children = 1;
		parent->children = new Skeleton::Link *[1];
		Skeleton::Link *l = Skeleton::new_link();
		l->part = cloned;
		parent->children[0] = l;
	}
}

static void insert_part(Skeleton::Part *p, Skeleton::Link *parent)
{
	Skeleton::Animation *anim = get_curr_anim();

	for (size_t i = 0; i < anim->frames.size(); i++) {
		if (parent == NULL) {
			anim->frames[i]->part = p->clone();
		}
		else {
			insert_part_worker(p, find_link_named(anim->frames[i], parent->part->get_name()));
		}
	}

	if (parent == NULL) {
		anim->work->part = p->clone();
	}
	else {
		insert_part_worker(p, find_link_named(anim->work, parent->part->get_name()));
	}

	delete p;
}

static void check_only_current_anim(int forced = -1)
{
	Skeleton::Animation *curr_anim;
	
	if (forced >= 0) {
		curr_anim = skeleton->get_animations()[forced];
	}
	else {
		curr_anim = get_curr_anim();
	}

	std::map<Skeleton::Animation *, int>::iterator it;

	int i = 0;
	for (it = anim_menu_id_map.begin(); it != anim_menu_id_map.end(); it++, i++) {
		std::pair<Skeleton::Animation *, int> p = *it;
		int index = -(i + 5);
		int flags = al_get_menu_item_flags(al_find_menu(menu, ANIMATION_ID), index);
		if (p.first != curr_anim && (flags & ALLEGRO_MENU_ITEM_CHECKED)) {
			al_toggle_menu_item_flags(al_find_menu(menu, ANIMATION_ID), index, ALLEGRO_MENU_ITEM_CHECKED);
		}
		else if (p.first == curr_anim && !(flags & ALLEGRO_MENU_ITEM_CHECKED)) {
			al_toggle_menu_item_flags(al_find_menu(menu, ANIMATION_ID), index, ALLEGRO_MENU_ITEM_CHECKED);
		}
	}
}

static bool new_animation(const std::string &name)
{
	std::vector<Skeleton::Animation *> &anims = skeleton->get_animations();

	for (size_t i = 0; i < anims.size(); i++) {
		Skeleton::Animation *a = anims[i];
		if (a->name == name) {
			return false;
		}
	}

	Skeleton::Link *l = NULL;
	Skeleton::Animation *anim;
	if (skeleton->get_animations().size() > 0) {
		anim = get_curr_anim();
		if (anim && anim->frames.size() > 0) {
			l = Skeleton::new_link();
			Skeleton::clone_link(l, anim->frames[0]);
		}
	}

	Skeleton::Animation *a = new Skeleton::Animation;
	a->curr_time = 0;
	a->name = name;
	a->curr_frame= 0;
	a->work = NULL;
	anims.push_back(a);
	skeleton->set_curr_anim(anims.size()-1);
	anim = get_curr_anim();
	if (l) {
		anim->frames.push_back(l);
		anim->delays.push_back(1000);
		Skeleton::destroy_links(anim->work);
		anim->work = Skeleton::new_link();
		Skeleton::clone_link(anim->work, anim->frames[anim->curr_frame]);
	}
	
	al_append_menu_item(al_find_menu(menu, ANIMATION_ID), name.c_str(), curr_id, ALLEGRO_MENU_ITEM_CHECKBOX, NULL, NULL);
	anim_menu_id_map[a] = curr_id;
	curr_id++;
	check_only_current_anim();

	return true;
}

void get_selected_part_bounds(int *minx, int *miny, int *maxx, int *maxy)
{
	int min_x = INT_MAX;
	int min_y = INT_MAX;
	int max_x = INT_MIN;
	int max_y = INT_MIN;

	skeleton->transform(center, false);

	Skeleton::Animation *anim = get_curr_anim();

	Skeleton::Link *l = find_link_named(anim->work, selected_part);
	std::vector< std::vector<Bones::Bone> > &transformed_bones = l->part->get_transformed_bones();

	std::vector<Bones::Bone> &v = transformed_bones[l->part->get_curr_bitmap()];
	
	for (size_t i = 0; i < v.size(); i++) {
		Bones::Bone &b = v[i];
		std::vector<Triangulate::Triangle> &tris = b.get();
		for (size_t j = 0; j < tris.size(); j++) {
			Triangulate::Triangle &t = tris[j];
			for (int k = 0; k < 3; k++) {
				General::Point<float> &p = t.points[k];
				if (p.x < min_x) min_x = p.x;
				if (p.y < min_y) min_y = p.y;
				if (p.x > max_x) max_x = p.x;
				if (p.y > max_y) max_y = p.y;
			}
		}
	}

	*minx = min_x;
	*miny = min_y;
	*maxx = max_x;
	*maxy = max_y;
}

void draw(bool main_drawing_call, bool flip)
{
	al_clear_to_color(al_color_name("darkslateblue"));

	skeleton->draw(center, false, al_map_rgba_f(1, 1, 1, 1));

	if (main_drawing_call) {
		if (selected_part != "") {
			int min_x, min_y, max_x, max_y;

			get_selected_part_bounds(&min_x, &min_y, &max_x, &max_y);

			draw_blinking_rect(min_x+0.5+center.x, min_y+0.5+center.y, max_x+0.5+center.x, max_y+0.5+center.y);
		}

		tgui::draw();

		al_draw_line(center.x-15, center.y, center.x+15, center.y, al_color_name("white"), 1);
		al_draw_line(center.x, center.y-5, center.x, center.y+5, al_color_name("white"), 1);

		draw_frame_number();
		draw_transform_info();
	}

	if (flip) {
		al_flip_display();
	}
}

static int saved_part_name;

void print_tabs(int num, FILE *out)
{
	for (int i = 0; i < num; i++) {
		fprintf(out, "\t");
	}
}

void save_link(Skeleton::Link *l, int tabs, FILE *out)
{
	if (l->part == NULL) {
		return;
	}

	int this_name = saved_part_name;
	saved_part_name++;
	print_tabs(tabs, out);
	fprintf(out, "<part>\n");
	print_tabs(tabs+1, out);
	fprintf(out, "<name>%d</name>\n", this_name);
	print_tabs(tabs+1, out);
	fprintf(out, "<layer>%d</layer>\n", l->part->get_layer());
	print_tabs(tabs+1, out);
	fprintf(out, "<bitmaps>");
	std::vector<std::string> &bitmap_names = l->part->get_bitmap_names();
	for (size_t i = 0; i < bitmap_names.size(); i++) {
		if (i > 0) {
			fprintf(out, ",");
		}
		fprintf(out, "%s", bitmap_names[i].c_str());
	}
	fprintf(out, "</bitmaps>\n");
	print_tabs(tabs+1, out);
	fprintf(out, "<transforms>\n");
	std::vector<Skeleton::Transform *> &transforms = l->part->get_transforms();
	for (size_t i = 0; i < transforms.size(); i++) {
		Skeleton::Transform *t = transforms[i];
		print_tabs(tabs+2, out);
		fprintf(out, "<unnamed>\n");
		print_tabs(tabs+3, out);
		fprintf(out, "<type>%d</type>\n", t->type);
		switch (t->type) {
			case Skeleton::TRANSLATION:
				print_tabs(tabs+3, out);
				fprintf(out, "<x>%f</x>\n", t->x);
				print_tabs(tabs+3, out);
				fprintf(out, "<y>%f</y>\n", t->y);
				break;
			case Skeleton::ROTATION:
				print_tabs(tabs+3, out);
				fprintf(out, "<angle>%f</angle>\n", t->angle);
				break;
			case Skeleton::SCALE:
				print_tabs(tabs+3, out);
				fprintf(out, "<scale_x>%f</scale_x>\n", t->scale_x);
				print_tabs(tabs+3, out);
				fprintf(out, "<scale_y>%f</scale_y>\n", t->scale_y);
				break;
			case Skeleton::BITMAP:
				print_tabs(tabs+3, out);
				fprintf(out, "<bitmap_index>%d</bitmap_index>\n", t->bitmap_index);
				break;
		}
		print_tabs(tabs+2, out);
		fprintf(out, "</unnamed>\n");
	}
	print_tabs(tabs+1, out);
	fprintf(out, "</transforms>\n");
	print_tabs(tabs+1, out);
	fprintf(out, "<children>\n");
	for (int i = 0; i < l->num_children; i++) {
		save_link(l->children[i], tabs+2, out);
	}
	print_tabs(tabs+1, out);
	fprintf(out, "</children>\n");
	print_tabs(tabs, out);
	fprintf(out, "</part>\n");
}

void save_animation(Skeleton::Animation *anim, FILE *out)
{
	fprintf(out, "<%s>\n", anim->name.c_str());
	fprintf(out, "\t<delays>\n");
	for (size_t i = 0; i < anim->delays.size(); i++) {
		fprintf(out, "\t\t<%d>%d</%d>\n", (int)i, anim->delays[i], (int)i);
	}
	fprintf(out, "\t</delays>\n");
	for (size_t i = 0; i < anim->frames.size(); i++) {
		saved_part_name = 0;
		save_link(anim->frames[i], 1, out);
	}
	fprintf(out, "</%s>\n", anim->name.c_str());
}

void save(std::string savename = "")
{
	bool go = true;
	FILE *out;

	if (savename == "") {
		ALLEGRO_FILECHOOSER *c = al_create_native_file_dialog(
		   "", "Open Part Image", "*.png", ALLEGRO_FILECHOOSER_SAVE
		);
		al_show_native_file_dialog(display, c);
		if (al_get_native_file_dialog_count(c) == 1) {
			char fn[1000];
			strncpy(fn, al_get_native_file_dialog_path(c, 0), 1000);

			/*
			if (al_filename_exists(fn)) {
				show_yes_no_dialog("File exists! Overwrite?");
				if (modal_result == no_button) {
					go = false;
				}
			}
			*/

			if (go) {
				//out = fopen(fn, "w");
				savename = fn;
			}
		}
		else {
			go = false;
		}
		
		al_destroy_native_file_dialog(c);
	}

	if (!go) {
		return;
	}

	out = fopen(savename.c_str(), "w");
	
	std::vector<Skeleton::Animation *> &anims = skeleton->get_animations();

	for (size_t i = 0; i < anims.size(); i++) {
		save_animation(anims[i], out);
	}

	fclose(out);
}

// Sort of hackish way to do this but it works!
Skeleton::Skeleton *clone_skeleton()
{
	std::string file = "skeletons/xml/__tmp__.xml";
	save(file);
	Skeleton::Skeleton *s = new Skeleton::Skeleton("__tmp__.xml");
	s->load();
	remove(file.c_str());
	return s;
}

void push_undo(Skeleton::Skeleton *s)
{
	undos.push_back(s);
	if (undos.size() > 25) {
		delete undos[0];
		undos.erase(undos.begin());
	}
}

void push_redo(Skeleton::Skeleton *s)
{
	redos.push_back(s);
	if (redos.size() > 25) {
		delete redos[0];
		redos.erase(redos.begin());
	}
}

Skeleton::Skeleton *pop_undo()
{
	Skeleton::Skeleton *s = undos[undos.size()-1];
	undos.erase(undos.begin()+(undos.size()-1));
	return s;
}

Skeleton::Skeleton *pop_redo()
{
	Skeleton::Skeleton *s = redos[redos.size()-1];
	redos.erase(redos.begin()+(redos.size()-1));
	return s;
}

void clear_redos()
{
	for (size_t i = 0; i < redos.size(); i++) {
		delete redos[i];
	}
	redos.clear();
}

void push_undo()
{
	clear_redos();
	push_undo(clone_skeleton());
}

void undo_redo_reset()
{
	selected_part = "";

	std::map<Skeleton::Animation *, int>::iterator it;
	for (it = anim_menu_id_map.begin();
			it != anim_menu_id_map.end(); it++) {
		al_remove_menu_item(
			al_find_menu(menu, ANIMATION_ID),
			-5
		);
	}
	anim_menu_id_map.clear();
	std::vector<Skeleton::Animation *> &v =
		skeleton->get_animations();
	for (size_t i = 0; i < v.size(); i++) {
		Skeleton::Animation *a = v[i];
		al_append_menu_item(
			al_find_menu(menu, ANIMATION_ID),
			a->name.c_str(),
			curr_id,
			ALLEGRO_MENU_ITEM_CHECKBOX,
			NULL, NULL
		);
		al_rest(0.5);
		anim_menu_id_map[a] = curr_id;
		curr_id++;
	}

	if (skeleton->get_animations().size() != 0) {
		check_only_current_anim();
	}

	timeline->setSkeleton(skeleton);
						
	if (transform_list->getLabels().size() > 0) {
		if (transform_list->getSelected() >= (int)transform_list->getLabels().size()) {
			transform_list->setSelected(0);
		}
	}
	
	update_transform_list();
}

void doUndo()
{
	if (undos.size() == 0) {
		return;
	}

	push_redo(clone_skeleton());

	Skeleton::Skeleton *s = pop_undo();

	delete skeleton;

	skeleton = s;

	undo_redo_reset();
}

void doRedo()
{
	if (redos.size() == 0) {
		return;
	}

	push_undo(clone_skeleton());

	Skeleton::Skeleton *s = pop_redo();

	delete skeleton;

	skeleton = s;

	undo_redo_reset();
}

static void ensure_out_txt(std::string imgname)
{
	remove("out.txt");

	show_ok_dialog("Running boneed, press OK.");
	draw();
	draw();

	while (true) {
#ifdef ALLEGRO_WINDOWS
		std::string cmd = "boneed.exe";
#else
		std::string cmd = "./boneed";
#endif
		cmd += " " + imgname;

		system(cmd.c_str());
		
		if (!al_filename_exists("out.txt")) {
			show_ok_dialog("out.txt not written. Try again.");
			draw();
			draw();
		}
		else {
			break;
		}
	}
}

static bool check_for_bone(std::string imgname)
{
	std::string name = "skeletons/parts/bones/" + imgname.replace(imgname.length()-3, 3, "xml");
	return al_filename_exists(name.c_str());
}

static void new_frame()
{
	Skeleton::Animation *anim = get_curr_anim();
	if (anim->frames.size() == 0) {
		Skeleton::Link *l = Skeleton::new_link();
		anim->frames.push_back(l);
		anim->delays.push_back(1000);
		anim->curr_frame = 0;
	}
	else {
		Skeleton::Link *l = Skeleton::new_link();
		Skeleton::clone_link(l, anim->frames[anim->curr_frame]);
		anim->frames.push_back(l);
		anim->delays.push_back(1000);
		anim->curr_frame = anim->frames.size()-1;
	}
	Skeleton::destroy_links(anim->work);
	anim->work = Skeleton::new_link();
	Skeleton::clone_link(anim->work, anim->frames[anim->curr_frame]);
}

static void destroy_part_worker(Skeleton::Link *l)
{
	for (int i = 0; i < l->num_children; i++) {
		if (l->children[i]->part->get_name() == selected_part) {
			Skeleton::destroy_links(l->children[i]);
			std::vector<Skeleton::Link *> children_to_keep;
			for (int j = 0; j < l->num_children; j++) {
				if (j == i) continue;
				children_to_keep.push_back(l->children[j]);
			}
			delete[] l->children;
			l->num_children--;
			if (l->num_children > 0) {
				l->children = new Skeleton::Link *[l->num_children];
			}
			for (size_t j = 0; j < children_to_keep.size(); j++) {
				l->children[j] = children_to_keep[j];
			}
			return;
		}
	}
	
	for (int i = 0; i < l->num_children; i++) {
		destroy_part_worker(l->children[i]);
	}
}

static void destroy_part()
{
	Skeleton::Animation *anim = get_curr_anim();

	if (anim->work->part->get_name() == selected_part) {
		for (size_t i = 0; i < anim->frames.size(); i++) {
			Skeleton::destroy_links(anim->frames[i]);
		}
		anim->frames.clear();
		anim->delays.clear();
		anim->curr_frame = 0;
		anim->curr_time = 0;
		Skeleton::destroy_links(anim->work);
		anim->work = NULL;
	}
	else {
		for (size_t i = 0; i < anim->frames.size(); i++) {
			destroy_part_worker(anim->frames[i]);
		}
		destroy_part_worker(anim->work);
	}

	selected_part = "";

	if (anim->work) {
		interpolate();
	}
}

int main(int argc, char **argv)
{
	int menu_height;
	int width = 800;
	int height = 600;

	center = General::Point<float>(width/2, height-200);

	/* Baryon engine initialization doesn't run so set have to set these manually */
	cfg.screens_w = 1;
	cfg.screens_h = 1;
	
	al_init();
	al_init_native_dialog_addon();
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();
	al_install_keyboard();
	al_install_mouse();

	//al_set_new_display_flags(ALLEGRO_OPENGL);

	queue = al_create_event_queue();

	ALLEGRO_BITMAP *icon = al_load_bitmap("skeled.png");

	display = al_create_display(width, height);

	al_set_display_icon(display, icon);

	menu = al_build_menu(main_menu_info);
	
	popup_menu = al_create_popup_menu();
	if (popup_menu) {
		al_append_menu_item(popup_menu, "New Part", NEW_PART_ID, 0, NULL, NULL);
	}

	al_rest(0.25);

	if (!al_set_display_menu(display, menu)) {
		return 1;
	}

	resource_manager = new Resource_Manager();

	skeleton = new Skeleton::Skeleton();

	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_default_menu_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_mouse_event_source());

	font = al_load_ttf_font("DejaVuSans.ttf", 14, 0);

	tgui::init(display);

	tguiWidgetsSetColors(
		al_color_name("white"),
		al_color_name("slategray")
	);

	tgui::setFont(font);

	transform_frame = new TGUI_Frame("Transforms", 20, 20, 150, 250);
	tgui::addWidget(transform_frame);

	transform_list = new TGUI_List(0, 0, 140-TGUI_ScrollPane::SCROLLBAR_THICKNESS);
	TGUI_ScrollPane *scrollPane = new TGUI_ScrollPane(transform_list);
	scrollPane->setX(0);
	scrollPane->setY(0);
	scrollPane->setWidth(140);
	scrollPane->setHeight(250-41-transform_frame->barHeight());
	ALLEGRO_BITMAP *plus = al_load_bitmap("plus.png");
	ALLEGRO_BITMAP *minus = al_load_bitmap("minus.png");
	ALLEGRO_BITMAP *up = al_load_bitmap("up.png");
	ALLEGRO_BITMAP *down = al_load_bitmap("down.png");
	TGUI_IconButton *plus_button = new TGUI_IconButton(plus, 0, 5,
		16, 16, 0, 0, 0);
	TGUI_IconButton *minus_button = new TGUI_IconButton(minus, 0, 5, 
		16, 16, 0, 0, 0);
	TGUI_IconButton *up_button = new TGUI_IconButton(up, 0, 5, 
		16, 16, 0, 0, 0);
	TGUI_IconButton *down_button = new TGUI_IconButton(down, 0, 5, 
		16, 16, 0, 0, 0);
	std::vector<tgui::TGUIWidget *> button_widgets;
	button_widgets.push_back(plus_button);
	button_widgets.push_back(minus_button);
	button_widgets.push_back(up_button);
	button_widgets.push_back(down_button);
	TGUI_Splitter *button_splitter = new TGUI_Splitter(
		0, 0, 16*4, 26,
		TGUI_HORIZONTAL,
		false,
		button_widgets
	);
	button_splitter->setDrawLines(false);
	std::vector<tgui::TGUIWidget *> main_widgets;
	main_widgets.push_back(scrollPane);
	main_widgets.push_back(button_splitter);
	TGUI_Splitter *main_splitter = new TGUI_Splitter(
		5, 5+transform_frame->barHeight(),
		140, 240-transform_frame->barHeight(),
		TGUI_VERTICAL,
		false,
		main_widgets
	);
	main_splitter->setDrawLines(false);
	main_splitter->set_size(0, main_splitter->getHeight()-26);
	main_splitter->set_size(1, 26);
	button_splitter->setClearColor(al_color_name("slategray"));
	main_splitter->setClearColor(al_color_name("slategray"));
	tgui::setNewWidgetParent(transform_frame);
	tgui::addWidget(main_splitter);

	timeline = new Timeline(0, al_get_display_height(display)-100, 800, 100, skeleton);
	tgui::setNewWidgetParent(NULL);
	tgui::addWidget(timeline);

	create_modal_dialog_widgets();

	ALLEGRO_TIMER *logic_timer = al_create_timer(1.0/60.0);
	ALLEGRO_TIMER *draw_timer = al_create_timer(1.0/15.0);
	al_register_event_source(queue, al_get_timer_event_source(logic_timer));
	al_register_event_source(queue, al_get_timer_event_source(draw_timer));
	al_start_timer(logic_timer);
	al_start_timer(draw_timer);
	int draw_count = 0;
	
	while (true) {
		ALLEGRO_EVENT event;
		
		while (!al_event_queue_is_empty(queue)) {

			al_get_next_event(queue, &event);

			tgui::handleEvent(&event);

			if (event.type == ALLEGRO_EVENT_TIMER) {
				if (event.timer.source == logic_timer) {
					if (playing) {
						skeleton->update(General::LOGIC_MILLIS);
					}
				}
				else if (event.timer.source == draw_timer) {
					draw_count++;
				}
			}
			else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				if (event.display.source == display) {
					goto done;
				}
			}
			else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
				if (skeleton->get_animations().size() > 0 && get_curr_anim()->frames.size() > 0) {
					Skeleton::Animation *anim = get_curr_anim();
					if (event.keyboard.keycode == ALLEGRO_KEY_OPENBRACE) {
						int frame = anim->curr_frame;
						frame--;
						if (frame < 0) frame = anim->frames.size()-1;

						if (frame != anim->curr_frame) {
							anim->curr_frame = frame;

							Skeleton::destroy_links(anim->work);
							anim->work = Skeleton::new_link();
							Skeleton::clone_link(anim->work, anim->frames[anim->curr_frame]);
							interpolate();
						}
					}
					else if (event.keyboard.keycode == ALLEGRO_KEY_CLOSEBRACE) {
						int frame = anim->curr_frame;
						frame++;
						if (frame >= (int)anim->frames.size()) {
							frame = 0;
						}

						if (frame != anim->curr_frame) {
							anim->curr_frame = frame;

							Skeleton::destroy_links(anim->work);
							anim->work = Skeleton::new_link();
							Skeleton::clone_link(anim->work, anim->frames[anim->curr_frame]);
							interpolate();
						}
					}
					else {
						if (selected_part != "") {
							if (transform_list->getLabels().size() > 0 && transform_list->getSelected() >= 0) {
								push_undo();
								int xx = 0;
								int yy = 0;
								if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
									xx = -1;
								}
								else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
									xx = 1;
								}
								if (event.keyboard.keycode == ALLEGRO_KEY_UP) {
									yy = -1;
								}
								else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
									yy = 1;
								}
								Skeleton::Transform *t = find_link_named(anim->frames[anim->curr_frame], selected_part)->part->get_transforms()[transform_list->getSelected()];
								switch (t->type) {
									case Skeleton::TRANSLATION:
										t->x += xx;
										t->y += yy;
										break;
									case Skeleton::ROTATION:
										t->angle += yy * (M_PI/180.0f);
										break;
									case Skeleton::SCALE:
										t->scale_x += 0.01f * xx;
										t->scale_y += 0.01f * yy;
										break;
									default:
										break;
								}
							}
						}
					}
				}
			}
			else if (event.type == ALLEGRO_EVENT_MOUSE_AXES && mouse_down) {
				float dx = event.mouse.x - mouse_down_point.x;
				float dy = event.mouse.y - mouse_down_point.y;
				Skeleton::Animation *anim = get_curr_anim();
				Skeleton::Transform *t = find_link_named(anim->frames[anim->curr_frame], selected_part)->part->get_transforms()[transform_list->getSelected()];
				switch (t->type) {
					case Skeleton::TRANSLATION:
						t->x += dx;
						t->y += dy;
						break;
					case Skeleton::ROTATION:
						t->angle += dy * (M_PI*2/200.0f);
						break;
					case Skeleton::SCALE:
						t->scale_x += dx * 0.02f;
						t->scale_y += dy * 0.02f;
						break;
					default:
						break;
				}
				mouse_down_point = General::Point<float>(
					event.mouse.x, event.mouse.y
				);
				Skeleton::interpolate(
					0.0f, anim->frames[anim->curr_frame], anim->frames[anim->curr_frame], anim->work
				);
			}
			else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && event.mouse.button == 1) {
 				if (tgui::determineTopLevelOwner(event.mouse.x, event.mouse.y) == NULL) {
					printf("Mouse: %d %d\n", event.mouse.x, event.mouse.y);
					Skeleton::Link *parent = find_link_with_point(
						General::Point<float>(event.mouse.x, event.mouse.y)
					);
					if (parent) {
						printf("PARENT!\n");
						selected_part = parent->part->get_name();
						transform_list->setSelected(0);
						update_transform_list();
					}
					else if (selected_part != "") {
						if (transform_list->getLabels().size() > 0 && transform_list->getSelected() >= 0) {
							push_undo();
							if (transform_list->getLabels()[transform_list->getSelected()] == "Bitmap") {
								if (selected_part != "") {
									while (1) {
										Skeleton::Animation *anim = get_curr_anim();
										int img = show_bitmap_select_dialog(find_link_named(anim->work, selected_part)->part);
										if (img >= 0) {
											Skeleton::Transform *t = find_link_named(anim->frames[anim->curr_frame], selected_part)->part->get_transforms()[transform_list->getSelected()];
											t->bitmap_index = img;
											t = find_link_named(anim->work, selected_part)->part->get_transforms()[transform_list->getSelected()];
											t->bitmap_index = img;
										}
										if (img == -2) {
											ALLEGRO_FILECHOOSER *c = al_create_native_file_dialog(
											   "", "Open Part Image", "*.png", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST
											);
											al_show_native_file_dialog(display, c);
											if (al_get_native_file_dialog_count(c) == 1) {
												char fn[1000];
												strncpy(fn, al_get_native_file_dialog_path(c, 0), 1000);
												ALLEGRO_PATH *p = al_create_path(fn);
												char file[100];
												strncpy(file, al_get_path_filename(p), 100);
												al_destroy_path(p);
												
												std::string imagename = std::string("skeletons/parts/") + file;

												ALLEGRO_BITMAP *bmp = al_load_bitmap(imagename.c_str());
												if (!bmp) {
													al_show_native_message_box(
														display,
														"Error",
														"Load failed",
														("Failed to load image " + imagename).c_str(),
														NULL,
														0
													);
												}
												else {
													std::string bones_filename = std::string("skeletons/parts/bones/") + file;
													bones_filename = bones_filename.replace(bones_filename.length()-3, 3, "xml");
													if (!check_for_bone(file)) {
														ensure_out_txt(imagename);
														rename("out.txt", bones_filename.c_str());
													}
													std::vector<Bones::Bone> bone;
													Bones::load(bone, al_get_bitmap_width(bmp), al_get_bitmap_height(bmp), bones_filename);

													add_bitmap_to_part(bmp, file, bone, selected_part);
													al_destroy_bitmap(bmp);
												}
											}
											al_destroy_native_file_dialog(c);
											continue;
										}
										break;
									}
								}
							}
							else {
								mouse_down = true;
								mouse_down_point = General::Point<float>(
									event.mouse.x, event.mouse.y
								);
							}
						}
					}
				}
			}
			else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
				mouse_down = false;
				/* Popup a context menu on a right click. */
				if (event.mouse.display == display && event.mouse.button == 2) {
					if (popup_menu) {
						right_click_point = General::Point<float>(
							event.mouse.x, event.mouse.y
						);
						al_popup_menu(popup_menu, display);
					}
				}
			}
			else if (event.type == ALLEGRO_EVENT_MENU_CLICK) {
				if (event.user.data1 == FILE_QUIT_ID) {
					goto done;
				}
				else if (event.user.data1 == FILE_OPEN_ID) {
					if (skeleton->get_animations().size() > 0) {
						push_undo();
					}
					ALLEGRO_FILECHOOSER *c = al_create_native_file_dialog(
					   "", "Open Skeleton", "*.xml", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST
					);
					al_show_native_file_dialog(display, c);
					if (al_get_native_file_dialog_count(c) == 1) {
						char fn[1000];
						strncpy(fn, al_get_native_file_dialog_path(c, 0), 1000);
						ALLEGRO_PATH *p = al_create_path(fn);
						char file[100];
						strncpy(file, al_get_path_filename(p), 100);
						al_destroy_path(p);
						delete skeleton;
						skeleton = new Skeleton::Skeleton(file);
						if (!skeleton->load()) {
							al_show_native_message_box(
								display,
								"Error",
								"Load failed",
								"Failed to load skeleton",
								NULL,
								0
							);
							delete skeleton;
							skeleton = new Skeleton::Skeleton();
						}
						else {
							std::map<Skeleton::Animation *, int>::iterator it;
							for (it = anim_menu_id_map.begin();
									it != anim_menu_id_map.end(); it++) {
								al_remove_menu_item(
									al_find_menu(menu, ANIMATION_ID),
									-5
								);
							}
							anim_menu_id_map.clear();
							std::vector<Skeleton::Animation *> &v =
								skeleton->get_animations();
							for (size_t i = 0; i < v.size(); i++) {
								Skeleton::Animation *a = v[i];
								al_append_menu_item(
									al_find_menu(menu, ANIMATION_ID),
									a->name.c_str(),
									curr_id,
									ALLEGRO_MENU_ITEM_CHECKBOX,
									NULL, NULL
								);
								al_rest(0.5);
								anim_menu_id_map[a] = curr_id;
								curr_id++;
							}
							check_only_current_anim();
						}
						timeline->setSkeleton(skeleton);
						selected_part = "";
					}
					al_destroy_native_file_dialog(c);
				}
				else if (event.user.data1 == ANIMATION_NEW_ID) {
					push_undo();
					std::string anim_name = "Rename Me";
					if (!new_animation(anim_name)) {
						std::string text = "Animation named " + anim_name + " already exists!";
						al_show_native_message_box(
							display,
							"Error",
							"Animation exists",
							text.c_str(),
							NULL,
							0
						);
					}
				}
				else if (event.user.data1 == NEW_PART_ID) {
					bool cant = false;
					bool msg = true;
					Skeleton::Link *parent = find_link_with_point(right_click_point);
					if (skeleton->get_animations().size() == 0) {
						cant = true;
					}
					else {
						Skeleton::Animation *anim = get_curr_anim();
						if (anim->frames.size() == 0) {
							cant = true;
						}
						else if (anim->frames[0]->part && parent == NULL) {
							show_yes_no_dialog("Careful! Replace root?");
							if (modal_result == no_button) {
								cant = true;
								msg = false;
							}
						}
					}
					if (cant) {
						if (msg) {
							show_ok_dialog("Please add an animation with at least 1 frame");
						}
					}
					else {
						push_undo();
						ALLEGRO_FILECHOOSER *c = al_create_native_file_dialog(
						   "", "Open Part Image", "*.png", ALLEGRO_FILECHOOSER_FILE_MUST_EXIST
						);
						al_show_native_file_dialog(display, c);
						if (al_get_native_file_dialog_count(c) == 1) {
							if (parent) {
								selected_part = parent->part->get_name();
								update_transform_list();
							}
							draw();
							char fn[1000];
							strncpy(fn, al_get_native_file_dialog_path(c, 0), 1000);
							ALLEGRO_PATH *p = al_create_path(fn);
							char file[100];
							strncpy(file, al_get_path_filename(p), 100);
							al_destroy_path(p);

							std::string imagename = std::string("skeletons/parts/") + file;

							ALLEGRO_BITMAP *bmp = al_load_bitmap(imagename.c_str());
							if (!bmp) {
								al_show_native_message_box(
									display,
									"Error",
									"Load failed",
									("Failed to load image " + imagename).c_str(),
									NULL,
									0
								);
							}
							else {
								std::vector<Wrap::Bitmap *> bitmaps;
								bitmaps.push_back(new Wrap::Bitmap(bmp, file));
								Skeleton::Part *p = new Skeleton::Part(General::itos(curr_part_id), std::vector<Skeleton::Transform *>(), bitmaps);
								std::string bones_filename = std::string("skeletons/parts/bones/") + file;
								bones_filename = bones_filename.replace(bones_filename.length()-3, 3, "xml");
								if (!check_for_bone(file)) {
									ensure_out_txt(imagename);
									rename("out.txt", bones_filename.c_str());
								}
								std::vector<Bones::Bone> bone;
								Bones::load(bone, al_get_bitmap_width(bmp), al_get_bitmap_height(bmp), bones_filename);
								p->add_bone(bone);
								p->get_bitmap_names().push_back(file);
								curr_part_id++;

								selected_part = p->get_name();

								insert_part(p, parent);
								update_transform_list();
							}
						}
						al_destroy_native_file_dialog(c);
					}
				}
				else if (event.user.data1 == FRAME_NEW_ID) {
					if (skeleton->get_animations().size() == 0) {
						show_ok_dialog("Create an animation first!");
					}
					else {
						push_undo();
						new_frame();
					}
				}
				else if (event.user.data1 == ANIMATION_PLAY_ID) {
					playing = !playing;
					if (!playing) {
						if (skeleton->get_animations().size() > 0) {
							Skeleton::Animation *anim = get_curr_anim();
							if (anim->frames.size() > 0) {
								interpolate();
							}
						}
					}
				}
				else if (event.user.data1 == FRAME_PROPERTIES_ID) {
					push_undo();
					show_frame_properties_dialog();
				}
				else if (event.user.data1 == ANIMATION_PROPERTIES_ID) {
					push_undo();
					show_animation_properties_dialog();
				}
				else if (event.user.data1 == FRAME_DELETE_ID) {
					push_undo();
					Skeleton::Animation *anim = get_curr_anim();
					if (anim->frames.size() > 0) {
						show_yes_no_dialog("Really delete frame?");
						if (modal_result == yes_button) {
							Skeleton::destroy_links(anim->frames[anim->curr_frame]);
							anim->frames.erase(anim->frames.begin()+anim->curr_frame);
							anim->delays.erase(anim->delays.begin()+anim->curr_frame);
							anim->curr_frame = 0;
							interpolate();
						}
					}
				}
				else if (event.user.data1 == ANIMATION_DELETE_ID) {
					push_undo();
					if (skeleton->get_animations().size() > 0) {
						show_yes_no_dialog("Really delete animation?");
						if (modal_result == yes_button) {
							int curr_anim = skeleton->get_curr_anim();
							int index = -(curr_anim + 5);
							al_remove_menu_item(al_find_menu(menu, ANIMATION_ID), index);
							check_only_current_anim(0);
							delete get_curr_anim();
							skeleton->get_animations().erase(skeleton->get_animations().begin()+curr_anim);
							if (skeleton->get_animations().size() == 0) {
								skeleton->set_curr_anim(-1);
							}
							else {
								skeleton->set_curr_anim(0);
							}
							selected_part = "";
							update_transform_list();
						}
					}
				}
				else if (event.user.data1 == FILE_SAVE_ID) {
					save();
				}
				else if (event.user.data1 == FILE_EXPORT_PNGS_ID) {
					show_png_export_dialog();
				}
				else if (event.user.data1 == PART_SELECT_NONE_ID) {
					selected_part = "";
					update_transform_list();
				}
				else if (event.user.data1 == PART_EDIT_OUTLINE_ID) {
					if (selected_part != "") {
						Skeleton::Animation *anim = get_curr_anim();
						Skeleton::Link *l = find_link_named(anim->work, selected_part);
						int bmp_num = l->part->get_curr_bitmap();
						ALLEGRO_BITMAP *bmp = l->part->get_bitmaps()[bmp_num]->bitmap;
						std::string bmp_name = l->part->get_bitmap_names()[bmp_num];
						std::string full_bmp_name = "skeletons/parts/" + bmp_name;
						std::string full_xml_name = "skeletons/parts/bones/" + bmp_name;
						full_xml_name = full_xml_name.replace(full_xml_name.length()-3, 3, "xml");
#ifdef ALLEGRO_WINDOWS
						std::string cmd = "boneed.exe";
#else
						std::string cmd = "./boneed";
#endif
						cmd += " " + full_bmp_name;
						system(cmd.c_str());
						if (al_filename_exists("out.txt")) {
							std::vector<Bones::Bone> bone;
							Bones::load(bone, al_get_bitmap_width(bmp), al_get_bitmap_height(bmp), "out.txt");
							l->part->get_bones()[bmp_num] = bone;
							l->part->get_transformed_bones()[bmp_num] = bone;
							rename("out.txt", full_xml_name.c_str());
						}
					}
				}
				else if (event.user.data1 == PART_PROPERTIES_ID) {
					push_undo();
					if (selected_part != "") {
						show_part_properties_dialog();
					}
				}
				else if (event.user.data1 == FRAME_VIEW_HIERARCHY_ID) {
					if (skeleton->get_animations().size() > 0) {
						Skeleton::Animation *anim = get_curr_anim();
						if (anim->frames.size() > 0 && anim->frames[anim->curr_frame]->part) {
							show_frame_hierarchy();
						}
					}
				}
				else if (event.user.data1 == PART_DELETE_ID) {
					push_undo();
					if (selected_part != "") {
						show_yes_no_dialog("Delete part and all children?!");
						if (modal_result == yes_button) {
							destroy_part();
						}
					}
				}
				else if (event.user.data1 == EDIT_UNDO_ID) {
					doUndo();
				}
				else if (event.user.data1 == EDIT_REDO_ID) {
					doRedo();
				}
				else {
					/* Check for click on animation name in Animation menu */
					int i = 0;
					std::map<Skeleton::Animation *, int>::iterator it;
					for (it = anim_menu_id_map.begin(); it != anim_menu_id_map.end(); it++) {
						std::pair<Skeleton::Animation * const, int> &p = *it;
						if (p.second == event.user.data1) {
							break;
						}
						i++;
					}
					if (it != anim_menu_id_map.end()) {
						if (skeleton->get_curr_anim() != i) {
							selected_part = "";
						}
						skeleton->set_curr_anim(i);
						check_only_current_anim();
						update_transform_list();
					}
				}
			}
			else if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
				int dh;

				/* The Windows implementation currently uses part of the client's height to
				 * render the window. This triggers a resize event, which can be trapped and
				 * used to upsize the window to be the size we expect it to be.
				 */

				al_acknowledge_resize(display);
				dh = height - al_get_display_height(display);

				/* On Windows, the menu steals space from our drawable area.
				 * The menu_height variable represents how much space is lost.
				 */
				if (dh > 0)
					menu_height = dh;
				else
					menu_height = 0;
				al_resize_display(display, width, height + menu_height);
			}
		}

		tgui::TGUIWidget *w = tgui::update();
		if (selected_part  != "") {
			if (w == plus_button) {
				show_transform_type_dialog();
				if (modal_result != cancel_button) {
					push_undo();
					Skeleton::Transform *t = new Skeleton::Transform;
					if (modal_result == add_translate_transform_button) {
						t->type = Skeleton::TRANSLATION;
						t->x = 0.0f;
						t->y = 0.0f;
					}
					else if (modal_result == add_rotate_transform_button) {
						t->type = Skeleton::ROTATION;
						t->angle = 0.0f;
					}
					else if (modal_result == add_scale_transform_button) {
						t->type = Skeleton::SCALE;
						t->scale_x = 1.0f;
						t->scale_y = 1.0f;
					}
					else { // bitmap transform
						t->type = Skeleton::BITMAP;
						t->bitmap_index = 0;
					}
					Skeleton::Animation *anim = get_curr_anim();
					for (size_t i = 0; i < anim->frames.size(); i++) {
						std::vector<Skeleton::Transform *> &transforms =
							find_link_named(
								anim->frames[i],
								selected_part
							)->part->get_transforms();
						transforms.push_back(clone_transform(t));
						if (i == 0) {
							transform_list->setSelected(transforms.size()-1);
						}
					}

					Skeleton::Link *l = find_link_named(anim->work, selected_part);
					if (l) {
						l->part->get_transforms().push_back(clone_transform(t));
					}
					delete t;
					update_transform_list();
				}
			}
			else if (w == minus_button) {
				show_yes_no_dialog("Really delete transform?");
				if (modal_result == yes_button) {
					push_undo();
					remove_transform();
				}
			}
			else if (w == up_button) {
				push_undo();
				move_transform_up();
				interpolate();
			}
			else if (w == down_button) {
				push_undo();
				move_transform_down();
				interpolate();
			}
		}

		if (draw_count > 0) {
			draw_count = 0;
			draw();
		}
	}

done:
	/* You must remove the menu before destroying the display to free resources */
	al_set_display_menu(display, NULL);

	tgui::shutdown();

	delete resource_manager;
	
	return 0;
}
