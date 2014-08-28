#ifndef WIDGETS_H
#define WIDGETS_H

#include <vector>
#include <list>

#include <tgui2.hpp>

#include "general.h"
#include "engine.h"
#include "graphics.h"
#include "equipment.h"
#include "resource_manager.h"

typedef void (*W_Notifier)(void *data, float row_y_pixel);

class W_I_Scrollable {
public:
	virtual void set_value(float v) = 0;

	void setSyncedWidget(W_I_Scrollable *w) {
		synced_widget = w;
	}

	W_I_Scrollable() :
		synced_widget(NULL)
	{
	}

protected:
	W_I_Scrollable *synced_widget;
};

class W_I_Toggleable {
public:
	void setOn(bool on) {
		this->on = on;
	}

	bool getOn() {
		return on;
	}

protected:
	W_I_Toggleable() :
		on(false)
	{
	}

	bool on;
};

class CrystalPicnic_Widget : public tgui::TGUIWidget {
public:
	virtual void setOffset(General::Point<float> offset) {
		this->offset = offset;
	}

	virtual General::Point<float> getOffset() {
		return offset;
	}

	CrystalPicnic_Widget() :
		offset(General::Point<float>(0, 0))
	{
	}

	virtual ~CrystalPicnic_Widget() {}

protected:
	General::Point<float> offset;
};

class W_Button : public CrystalPicnic_Widget {
public:
	virtual void draw(int abs_x, int abs_y);
	bool acceptsFocus();
	void keyDown(int keycode);
	void joyButtonDown(int button);
	void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	TGUIWidget *update();
	void setSize(int w, int h);
	Wrap::Bitmap *getImage();
	void setImageFlags(int f);
	void set_sample_name(std::string name);
	void set_enabled(bool enabled);
	void set_text_color(ALLEGRO_COLOR text_color);

	W_Button(int x, int y, int width, int height);
	W_Button(std::string filename);
	W_Button(std::string filename, std::string text);
	virtual ~W_Button();

protected:
	std::string filename;
	bool pressed;
	Wrap::Bitmap *image;
	int flags;
	std::string sample_name;
	Wrap::Bitmap *disabled_image;
	bool enabled;
	std::string text;
	ALLEGRO_COLOR text_color;
};

class W_SaveLoad_Button : public W_Button {
public:
	void draw(int abs_x, int abs_y);

	W_SaveLoad_Button(int x, int y, int width, int height, std::vector<std::string> players, double playtime, std::string area_name);
	virtual ~W_SaveLoad_Button();

protected:
	int num_players;
	double playtime;
	std::string area_name;
	Wrap::Bitmap *player_bmps[3];
	std::vector<std::string> players;
};

class W_Title_Screen_Button : public W_Button
{
public:
	void losingFocus();
	void draw(int abs_x, int abs_y);
	W_Title_Screen_Button(std::string text);
	virtual ~W_Title_Screen_Button();
};

class W_Icon : public CrystalPicnic_Widget {
public:
	void draw(int abs_x, int abs_y) {
		al_draw_bitmap(bitmap->bitmap, abs_x, abs_y, 0);
	}

	W_Icon(Wrap::Bitmap *bitmap) :
		bitmap(bitmap)
	{
		width = al_get_bitmap_width(bitmap->bitmap);
		height = al_get_bitmap_height(bitmap->bitmap);
	}

	virtual ~W_Icon() {
	}

protected:
	Wrap::Bitmap *bitmap;
};

class W_Integer : public CrystalPicnic_Widget {
public:
	void draw(int abs_x, int abs_y) {
		std::string s = General::itos(number);
		int s_w = General::get_text_width(General::FONT_LIGHT, s);
		int s_h = General::get_font_line_height(General::FONT_LIGHT);
		General::draw_text(s, color, abs_x-s_w/2, abs_y-s_h/2, 0);
	}

	int get_number()
	{
		return number;
	}

	void set_number(int n)
	{
		number = n;
	}

	W_Integer(int start) :
		number(start)
	{
		color = al_color_name("black");
		width = 1;
		height = 1;
	}

	virtual ~W_Integer()
	{
	}

protected:
	int number;
	ALLEGRO_COLOR color;
};

class W_Icon_Button : public W_Button {
public:
	void draw(int abs_x, int abs_y) {
		W_Button::draw(abs_x, abs_y);

		if (icon) {
			int icon_w = al_get_bitmap_width(icon->bitmap);
			int total_w = icon_w + General::get_text_width(General::FONT_LIGHT, caption) + 2;
			int draw_x = x + width/2 - total_w/2 + offset.x;
			al_draw_bitmap(
				icon->bitmap,
				draw_x,
				y + height/2 - icon_h/2 + offset.y,
				0
			);
			General::draw_text(
				caption,
				al_color_name("white"),
				draw_x + icon_w + 2,
				y+height/2-General::get_font_line_height(General::FONT_LIGHT)/2+offset.y,
				ALLEGRO_ALIGN_LEFT,
				General::FONT_LIGHT
			);
		}
		else {
			General::draw_text(
				caption,
				al_color_name("white"),
				x+width/2+offset.x,
				y+height/2-General::get_font_line_height(General::FONT_LIGHT)/2+offset.y,
				ALLEGRO_ALIGN_CENTER,
				General::FONT_LIGHT
			);
		}
	}

	W_Icon_Button(std::string caption, std::string bg_name, std::string icon_name) :
		W_Button(bg_name),
		caption(caption),
		icon_filename(icon_name)
	{
		if (icon_name != "") {
			icon = resource_manager->reference_bitmap(icon_name);
		}
		else {
			icon = NULL;
		}
		
		if (icon) {
			icon_w = al_get_bitmap_width(icon->bitmap);
			icon_h = al_get_bitmap_height(icon->bitmap);
		}
		else {
			icon_w = 0;
			icon_h = 0;
		}
	}

	virtual ~W_Icon_Button() {
		if (icon_filename != "") {
			resource_manager->release_bitmap(icon_filename);
		}
	}

protected:
	std::string caption;
	std::string icon_filename;

	Wrap::Bitmap *icon;
		
	int icon_w;
	int icon_h;
};

class W_Scrolling_List : public CrystalPicnic_Widget, public W_I_Scrollable {
public:
	static const int LEFT = 1;
	static const int RIGHT= 2;
	static const int UP = 3;
	static const int DOWN = 4;
	static const int A = 5;
	static const int B = 6;

	bool acceptsFocus();

	virtual void use_button(int button);
	virtual bool keyChar(int keycode, int unichar);
	virtual void joyButtonDownRepeat(int button);
	virtual bool joyAxisRepeat(int stick, int axis, float value);
	int get_num_items();
	void set_translate_item_names(bool translate);
	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	virtual void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y);
	virtual void draw(int abs_x, int abs_y);
	virtual TGUIWidget *update();
	void set_value(float v);
	int get_selected();
	void set_selected(int sel);
	void setHeight(int h);
	void setNotifier(W_Notifier n, void *data);
	// returns percent of max height
	float get_scrollbar_tab_size();
	void set_tint_icons(bool tint);
	virtual void remove_item(int index);
	void move(int index, int dir);
	void move_up();
	void move_down();
	void update_item(int index, bool update_name, std::string name, bool update_icon, std::string icon_filename, bool update_right_justified_text, std::string _right_justified_text);
	std::string get_item_name(int index);
	void set_right_icon_filenames(std::vector<std::string> right_icon_filenames);
	bool get_right_icon_clicked();
	bool is_activated();
	void set_can_arrange(bool can_arrange);
	void set_item_images(std::vector<Wrap::Bitmap *> *images);
	W_Scrolling_List(std::vector<std::string> item_names, std::vector<std::string> icon_filenames, std::vector<std::string> right_justified_text, std::vector<bool> disabled, General::Font_Type font, bool draw_box);
	virtual ~W_Scrolling_List();

protected:
	static const int POINTS_TO_TRACK = 10;
	static const float DECELLERATION;
	static const int MAX_VELOCITY = 500;

	void set_velocity() {
		if (move_points.size() > 1) {
			std::pair<double, General::Point<int> > p_start = move_points[move_points.size()-1];
			std::pair<double, General::Point<int> > p_end = move_points[0];
			double elapsed = p_end.first - p_start.first;
			float travelled = p_end.second.y - p_start.second.y;
			vy = -travelled / elapsed;
			if (vy > MAX_VELOCITY)
				vy = MAX_VELOCITY;
			else if (vy < -MAX_VELOCITY)
				vy = -MAX_VELOCITY;
		}
	}

	int selected;
	std::vector<std::string> item_names;
	General::Font_Type font;
	
	std::vector<std::string> icon_filenames;
	std::vector<Wrap::Bitmap *> icons;
	std::vector<std::string> right_justified_text;
	std::vector<bool> disabled;

	float y_offset;
	float max_y_offset;
	float vy; // pixels/second

	bool mouse_is_down;
	General::Point<int> real_mouse_down_point;
	General::Point<int> mouse_down_point;
	std::vector< std::pair<double, General::Point<int> > > move_points;

	bool can_notify;
	W_Notifier notifier;
	void *notifier_data;

	bool draw_box;
	bool tint_icons;

	std::vector<std::string> right_icon_filenames;
	std::vector<Wrap::Bitmap *> right_icons;

	bool right_icon_clicked;

	bool activated;
	int active_column;

	bool translate_item_names;

	bool can_arrange;

	std::vector<Wrap::Bitmap *> *item_images;
};

class W_Equipment_List : public W_Scrolling_List {
public:
	void use_button(int button);
	bool keyChar(int keycode, int unichar);
	void joyButtonDownRepeat(int button);
	bool joyAxisRepeat(int stick, int axis, float value);
	void update_description(int index, std::string desc);
	void set_active_column(int c);
	void deactivate();
	void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y);
	void draw(int abs_x, int abs_y);
	void postDraw(int abs_x, int abs_y);
	void insert_item(int pos, std::string icon_filename, std::string text, std::string right_icon_filename, Equipment::Equipment_Type equip_type, std::string desc, bool disabled);
	void remove_item(int index);
	Equipment::Equipment_Type get_type(int index);
	std::string get_item_name(int index);
	void set_draw_outline(bool draw_outline);

	W_Equipment_List(std::vector<std::string> item_names, std::vector<std::string> icon_filenames, std::vector<Equipment::Equipment_Type> equipment_type, std::vector<std::string> descriptions, std::vector<bool> disabled, General::Font_Type font, bool draw_box);
	virtual ~W_Equipment_List();

protected:

	std::vector<Equipment::Equipment_Type> equipment_type;
	std::vector<std::string> descriptions;
	Wrap::Bitmap *info_bmp;

	bool draw_info_box;
	float info_box_x;
	float info_box_y;
	std::string info_box_text;
	bool draw_outline;
};

class W_Vertical_Scrollbar : public CrystalPicnic_Widget, public W_I_Scrollable {
public:
	static const int WIDTH = 5;

	void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (rel_x >= 0 && rel_y >= 0) {
			Wrap::Bitmap *bmp;
			trough_anim->set_sub_animation("top");
			bmp = trough_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();

			set(rel_y - al_get_bitmap_height(bmp->bitmap)-get_tab_size()/2);
			down_point = General::Point<int>(abs_x, abs_y);
			down = true;
		}
	}

	void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (down) {
			down = false;
		}
	}

	void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
		if (down) {
			Wrap::Bitmap *bmp;
			trough_anim->set_sub_animation("top");
			bmp = trough_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();
			set(abs_y-y-al_get_bitmap_height(bmp->bitmap)-get_tab_size()/2);
		}
	}

	void draw(int abs_x, int abs_y) {
		Wrap::Bitmap *tab_top, *tab_middle, *tab_bottom;
		tab_anim->set_sub_animation("top");
		tab_top = tab_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();
		tab_anim->set_sub_animation("middle");
		tab_middle = tab_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();
		tab_anim->set_sub_animation("bottom");
		tab_bottom = tab_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();
		
		Wrap::Bitmap *trough_top, *trough_middle;
		trough_anim->set_sub_animation("top");
		trough_top = trough_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();
		trough_anim->set_sub_animation("middle");
		trough_middle = trough_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();
		
		al_draw_bitmap(trough_top->bitmap, abs_x+offset.x, abs_y+offset.y, 0);
		al_draw_scaled_bitmap(trough_middle->bitmap, 0, 0, WIDTH, 1, abs_x+offset.x, abs_y+al_get_bitmap_height(trough_top->bitmap)+offset.y,
			WIDTH, height-al_get_bitmap_height(trough_top->bitmap)*2, 0);
		al_draw_bitmap(
			trough_top->bitmap,
			abs_x+offset.x,
			abs_y+height-al_get_bitmap_height(trough_top->bitmap)+offset.y,
			ALLEGRO_FLIP_VERTICAL
		);

		int tab_x = abs_x + WIDTH/2 - al_get_bitmap_width(tab_top->bitmap)/2 + offset.x;
		int tab_y = abs_y + al_get_bitmap_height(trough_top->bitmap) + value * travel + offset.y;
		al_draw_bitmap(
			tab_top->bitmap,
			tab_x, tab_y,
			0
		);
		al_draw_scaled_bitmap(
			tab_middle->bitmap,
			0, 0, al_get_bitmap_width(tab_middle->bitmap), al_get_bitmap_height(tab_middle->bitmap),
			tab_x, tab_y+al_get_bitmap_height(tab_top->bitmap),
			al_get_bitmap_width(tab_middle->bitmap),
			get_tab_size()-al_get_bitmap_height(tab_top->bitmap)-al_get_bitmap_height(tab_bottom->bitmap),
			0
		);
		al_draw_bitmap(
			tab_bottom->bitmap,
			tab_x, tab_y+get_tab_size()-al_get_bitmap_height(tab_bottom->bitmap),
			0
		);
	}

	TGUIWidget *update() {
		return NULL;
	}

	float get_value() {
		return value;
	}

	void set_value(float v) {
		value = v;
	}

	void setHeight(int h) {
		TGUIWidget::setHeight(h);

		Wrap::Bitmap *bmp;
		trough_anim->set_sub_animation("top");
		bmp = trough_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();
		travel = height - al_get_bitmap_height(bmp->bitmap)*2 - get_tab_size();
	}

	int get_tab_size()
	{
		int size = height;

		Wrap::Bitmap *bmp;
		trough_anim->set_sub_animation("top");
		bmp = trough_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();
		size -= al_get_bitmap_height(bmp->bitmap);

		trough_anim->set_sub_animation("middle");
		bmp = trough_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();
		size -= al_get_bitmap_height(bmp->bitmap);

		size *= f_tab_size;

		if (size < 8) size = 8;
		return size;
	}

	void set_tab_size(float tab_size)
	{
		f_tab_size = tab_size;
		setHeight(height);
		if (synced_widget) {
			synced_widget->set_value(value);
		}
	}
	
	#define TOP_NAME "misc_graphics/interface/vertical_scrollbar_top.cpi"
	#define MID_NAME "misc_graphics/interface/vertical_scrollbar_middle.cpi"

	W_Vertical_Scrollbar(float tab_size) :
		down(false),
		value(0.0f),
		f_tab_size(tab_size)
	{
		this->width = WIDTH;
	
		trough_anim = new Animation_Set();
		trough_anim->load("misc_graphics/interface/vertical_scrollbar_trough");

		tab_anim = new Animation_Set();
		tab_anim->load("misc_graphics/interface/vertical_scrollbar_tab");
	}

	virtual ~W_Vertical_Scrollbar() {
		delete trough_anim;
		delete tab_anim;
	}
	
	#undef TOP_NAME
	#undef MID_NAME

protected:
	void set(int trough_pos) {
		Wrap::Bitmap *bmp;
		trough_anim->set_sub_animation("top");
		bmp = trough_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();
		int ignore_size = al_get_bitmap_height(bmp->bitmap)*2;
		if (get_tab_size()+ignore_size >= height) {
			value = 0;
		}
		else if (trough_pos >= 0 && trough_pos < travel+get_tab_size()-ignore_size) {
			value = (float)trough_pos / travel;
			if (value < 0)
				value = 0;
			else if (value > 1)
				value = 1;
		}
		else if (trough_pos < 0)
			value = 0;
		else if (trough_pos >= travel+get_tab_size()) {
			value = 1;
		}
		if (synced_widget) {
			synced_widget->set_value(value);
		}
	}

	bool down;
	General::Point<int> down_point;
	int travel;
	float value;

	Animation_Set *trough_anim;
	Animation_Set *tab_anim;

	float f_tab_size;
};

class Button_Group {
public:
	Button_Group(std::list<W_I_Toggleable *> group) :
		group(group)
	{
	}

	void toggle(W_I_Toggleable *widget) {
		std::list<W_I_Toggleable *>::iterator it;
		for (it = group.begin(); it != group.end(); it++) {
			W_I_Toggleable *w = *it;
			if (w == widget)
				w->setOn(true);
			else
				w->setOn(false);
		}
	}

protected:
	std::list<W_I_Toggleable *> group;
};

#endif // WIDGETS_H
