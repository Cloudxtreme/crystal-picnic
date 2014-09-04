#ifndef MAIN_MENU_LOOP_H
#define MAIN_MENU_LOOP_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <wrap.h>

#include "loop.h"
#include "widgets.h"
#include "general.h"
#include "player.h"
#include "sound.h"

class Main_Menu_Loop : public Loop {
public:
	static const int GREEN_WIN_BASE_W = 163;
	static const int GREEN_WIN_BASE_H = 119;

	enum Menu_ID {
		MAIN = 0,
		ABILITIES,
		ITEMS,
		EQUIP
	};

	enum State {
		S_TRANSITIONING_IN = 0,
		S_IN,
		S_TRANSITIONING_OUT,
		S_OUT
	};

	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();
	void post_draw();
	void destroy_graphics();
	void reload_graphics();

	bool get_main_menu_loop_button_pressed();
	void set_main_menu_loop_button_pressed(bool pressed);
	
	std::vector<Player *> get_players();
	int get_selected_player();
	State get_state();
	float get_transition_percent();
	int get_extra_x();
	int get_extra_y();
	int get_green_x();
	int get_green_y();
	tgui::TGUIWidget *get_tgui_update_result();
	int get_win_w();
	int get_win_h();

	tgui::TGUIWidget *get_return_button() { return return_button; }

	void start_transition_out(std::string sample_name = "");

	void set_esc_pressed(bool esc_pressed);

	Main_Menu_Loop(std::vector<Player *> players, std::vector<Loop *> background_draw_loops);
	virtual ~Main_Menu_Loop();

protected:
	void setup_current_menu();
	void set_widget_offsets();
	void start_transition_in(std::string sample_name = "");

	Loop *sub_loop;
	Menu_ID current_menu;
	bool skip_draw;

	std::vector<Player *> players;
	
	State state;
	double transition_start;
	float transition_percent;

	tgui::TGUIWidget *tgui_update_result;
	
	int selected_player;
	
	int extra_x, extra_y;
	int green_x, green_y;
	int remaining_height;
	
	Wrap::Bitmap *character_preview_box;
	Wrap::Bitmap *money_time_box;
	Wrap::Bitmap *nine_crystal;
	Wrap::Bitmap *nine_coin;
	Wrap::Bitmap *nine_clock;
	W_Icon_Button *options_button;
	W_Icon_Button *save_button;
	W_Icon_Button *quit_button;
 	W_Icon_Button *return_button;

	W_Button *player_buttons[3];

	W_Button *crystal_button;

	bool do_not_transition_in_again;

	int next_player;
	bool main_menu_loop_button_pressed;

	std::vector<Loop *> background_draw_loops;

	bool esc_pressed;

	Wrap::Bitmap *bg;
};

class I_Main_Menu_Parent {
public:
	virtual int get_selected() = 0;
};

class Main_Menu_Main_Loop : public Loop, public I_Main_Menu_Parent {
public:
	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();

	int get_selected() {
		return selected;
	}

	void set_transition_type(int transition_type);
	
	Main_Menu_Main_Loop(Main_Menu_Loop *main_menu_loop, int transitioin_type);
	virtual ~Main_Menu_Main_Loop();

protected:
	void set_widget_offsets();

	Main_Menu_Loop *main_menu_loop;

	int selected;
	
	int profile_x, profile_y;
	int profile_w, profile_h;
	
	int text_x;
	int text_w;
	
	std::vector<Wrap::Bitmap *> profiles;

 	W_Icon_Button *equip_button;
 	W_Icon_Button *items_button;
 	W_Icon_Button *abilities_button;

	bool done_first_transition;
	int transition_type;

	bool in;
};

class Main_Menu_Abilities_Loop : public Loop {
public:
	struct Notification {
		Main_Menu_Abilities_Loop *loop;
		Player *player;
	};

	int get_selected();
	int get_list_selected();

	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();

	Main_Menu_Abilities_Loop(Main_Menu_Loop *main_menu_loop);
	virtual ~Main_Menu_Abilities_Loop();

protected:
	void set_widget_offsets();

	Main_Menu_Loop *main_menu_loop;

	Wrap::Bitmap *abilities_button;
	Wrap::Bitmap *abilities_button_grey;

	W_Scrolling_List *list;
	W_Vertical_Scrollbar *scrollbar;

	std::vector<std::string> abilities;
	
	int abilities_button_w;
	int abilities_button_h;
	int dark_green_h;
	int buttons_y;

	int selected;

	W_Button *ability_buttons[3];

	int widths[4];
	int xstarts[4];
	int xends[4];
	int button_starts[4];
	int win_w;
	int green_x;

	Notification *notification;

	bool in;
	
	bool list_was_activated;
};

class Main_Menu_Items_Loop : public Loop {
public:
	struct Notification
	{
		W_Scrolling_List *list;
		W_Vertical_Scrollbar *scrollbar;
		Main_Menu_Items_Loop *loop;
	};

	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();
	void post_draw();

	Main_Menu_Items_Loop(Main_Menu_Loop *main_menu_loop);
	virtual ~Main_Menu_Items_Loop();
	
	bool something_dropped;
	std::string dropped_string;
	General::Point<float> dropped_string_pos;
	float dropped_string_angle;
	General::Point<float> dropped_string_velocity;

	void set_do_use(bool do_use);

protected:
	void set_widget_offsets();

	Main_Menu_Loop *main_menu_loop;

	W_Scrolling_List *list;
	W_Vertical_Scrollbar *scrollbar;

	int info_panel_h;
	int win_w;
	int win_h;

	std::vector<Wrap::Bitmap *> item_images;

	int item_button_x;
	int item_button_y;
	int item_button_w;
	int item_button_h;
	W_Button *item_button;
	
	Notification *notification;

	bool in;

	bool do_use;

	bool list_was_activated;
};

class Main_Menu_Equip_Loop : public Loop {
public:
	struct Notification
	{
		W_Equipment_List *list;
		W_Equipment_List *list2;
		W_Vertical_Scrollbar *scrollbar;
		Main_Menu_Equip_Loop *loop;
		std::vector<Player *> players;
		int selected_player;
	};

	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();
	void post_draw();

	void start_swap(int selected, float ystart);

	Main_Menu_Equip_Loop(Main_Menu_Loop *main_menu_loop);
	virtual ~Main_Menu_Equip_Loop();

	bool something_dropped;
	std::string dropped_string;
	General::Point<float> dropped_string_pos;
	float dropped_string_angle;
	General::Point<float> dropped_string_velocity;

protected:
	void set_widget_offsets();

	Main_Menu_Loop *main_menu_loop;

	W_Equipment_List *list;
	W_Equipment_List *list2;
	W_Vertical_Scrollbar *scrollbar;

	int win_w;
	int win_h;

	int top_box_w;
	int top_box_h;

	Wrap::Bitmap *weapon_icon;
	Wrap::Bitmap *armor_icon;
	Wrap::Bitmap *accessory_icon;
	Wrap::Bitmap *info_icon;
	Wrap::Bitmap *x_icon;

	std::vector<std::string> item_names;
	Notification *notification;
	Notification *notification2;
	std::string swap_string;
	std::string orig_string;
	bool swapping;
	float y_swap_start;
	float y_swap_dest;
	Equipment::Equipment_Type swap_type;
	double swap_start_time;
	double swap_now;
	bool swap_update_icon;

	bool draw_info_box;
	float info_box_x;
	float info_box_y;
	std::string info_box_text;
	int last_mouse_x, last_mouse_y;
	int info_clicked_x, info_clicked_y;

	bool in;
	
	bool list_was_activated;
};

#endif // MAIN_MENU_H

