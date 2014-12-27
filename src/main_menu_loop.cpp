#include "main_menu_loop.h"
#include "speech_types.h"
#include "game_specific_globals.h"
#include "battle_entity.h"
#include "crystal_loop.h"
#include "main.h"
#include "saveload_loop.h"
#include "resource_manager.h"

const float TOTAL_TRANSITION_TIME = 0.4f;

const float DROPPED_TEXT_SPEED = 5.0f;
#define DROP(s, x, y) { \
		engine->play_sample("sfx/drop_item.ogg"); \
		loop->something_dropped = true; \
		loop->dropped_string = s; \
		int len = General::get_text_width(General::FONT_LIGHT, loop->dropped_string); \
		loop->dropped_string_pos = General::Point<float>(x+len/2, y); \
		loop->dropped_string_angle = 0.0f; \
		float a = (General::rand() % 1000) / 1000.0f * M_PI * 2.0f; \
		loop->dropped_string_velocity = General::Point<float>(cos(a)*DROPPED_TEXT_SPEED, sin(a)*DROPPED_TEXT_SPEED); \
	}

const int TRANSITION_DIAGONAL = 0;
const int TRANSITION_HORIZONTAL = 1;

//-----------------------------------------------------

static void draw_character_preview_box(Player *p, bool selected, Wrap::Bitmap *box, int x, int y)
{
	al_draw_bitmap(box->bitmap, x, y, 0);
	
	if (p == NULL) {
		return;
	}
	
	Wrap::Bitmap *icon = resource_manager->reference_bitmap("misc_graphics/interface/" + p->get_name() + "_normal_icon.cpi");
	
	al_draw_bitmap(
		icon->bitmap,
		x+4+10-al_get_bitmap_width(icon->bitmap)/2, y+al_get_bitmap_height(box->bitmap)/2-al_get_bitmap_height(icon->bitmap)/2,
		0
	);
	
	const int ICON_W = 20;
	int remain_w = al_get_bitmap_width(box->bitmap) - 10 - ICON_W;
	int x1 = ICON_W + 6;
	int gauge_span = (al_get_bitmap_height(box->bitmap)-6-General::get_font_line_height(General::FONT_LIGHT)-1) / 3;
	int y1 = y + 3 + gauge_span/2 - 2 + General::get_font_line_height(General::FONT_LIGHT) + 1;
	
	ALLEGRO_COLOR name_color;
	if (selected) {
		name_color = al_color_name("yellow");
	}
	else {
		name_color = al_color_name("white");
	}
	
	General::draw_text(General::get_hero_printable_name(p->get_name()), name_color, x+x1, y+3, 0);
	
	ALLEGRO_COLOR hilight = Graphics::change_brightness(General::UI_BLUE, 1.35f);

	Battle_Attributes &attr = p->get_battle_attributes();
	int max_hp = attr.max_hp;
	int max_mp = attr.max_mp;
	Game_Specific_Globals::get_accessory_effects(attr.equipment.accessory.name, &max_hp, &max_mp, NULL, NULL);

	float percent;
	percent = (attr.hp/cfg.difficulty_mult()) / (float)(max_hp/cfg.difficulty_mult());
	Graphics::draw_gauge(x+x1, y1, remain_w, true, percent, hilight, al_color_name("lime"));
	percent = attr.mp / (float)max_mp;
	Graphics::draw_gauge(x+x1, y1+gauge_span, remain_w, false, percent, hilight, al_color_name("cyan"));
	
	resource_manager->release_bitmap("misc_graphics/interface/" + p->get_name() + "_normal_icon.cpi");
}

static void get_green_window_offsets(Main_Menu_Loop::State state, int transition_type, float transition_percent, float *ox, float *oy)
{
	if (state == Main_Menu_Loop::S_TRANSITIONING_IN || state == Main_Menu_Loop::S_TRANSITIONING_OUT) {
		if (transition_type == TRANSITION_DIAGONAL) {
			if (state == Main_Menu_Loop::S_TRANSITIONING_IN) {
				*ox = cfg.screen_w * (1.0 - transition_percent);
				*oy = -cfg.screen_h * (1.0 - transition_percent);
			}
			else {
				*ox = cfg.screen_w * transition_percent;
				*oy = -cfg.screen_h * transition_percent;
			}
		}
		else {
			if (state == Main_Menu_Loop::S_TRANSITIONING_IN) {
				*ox = cfg.screen_w * (1.0 - transition_percent);
			}
			else {
				*ox = cfg.screen_w * transition_percent;
			}
			*oy = 0;
		}
	}
	else {
		*ox = 0;
		*oy = 0;
	}
}

void draw_green_window(Main_Menu_Loop *main_menu_loop, float ox, float oy)
{
	General::draw_speech_window(
		SPEECH_NORMAL,
		ox+main_menu_loop->get_green_x(), oy+main_menu_loop->get_green_y(),
		Main_Menu_Loop::GREEN_WIN_BASE_W + main_menu_loop->get_extra_x(),
		Main_Menu_Loop::GREEN_WIN_BASE_H + main_menu_loop->get_extra_y(),
		false,
		al_color_name("white"),
		1.0f
	);
}

//-----------------------------------------------------

void Main_Menu_Loop::set_esc_pressed(bool esc_pressed)
{
	this->esc_pressed = esc_pressed;
	if (esc_pressed) {
		tgui::setFocus(return_button);
	}
}

bool Main_Menu_Loop::init()
{
	engine->clear_touches();

	if (inited) {
		return true;
	}
	Loop::init();

	// Load sound effects
	engine->load_sample("sfx/drop_item.ogg");
	engine->load_sample("sfx/enter_submenu.ogg");
	engine->load_sample("sfx/exit_submenu.ogg");

	character_preview_box = resource_manager->reference_bitmap("misc_graphics/interface/character_preview_box.cpi");
	money_time_box = resource_manager->reference_bitmap("misc_graphics/interface/money_time_box.cpi");
	nine_crystal = resource_manager->reference_bitmap("misc_graphics/interface/9crystal.cpi");
	nine_coin = resource_manager->reference_bitmap("misc_graphics/interface/9coin.cpi");
	nine_clock = resource_manager->reference_bitmap("misc_graphics/interface/9clock.cpi");

	extra_x = cfg.screen_w - General::RENDER_W;
	extra_y = cfg.screen_h - General::RENDER_H;
	green_x = cfg.screen_w - GREEN_WIN_BASE_W - 1 - extra_x;
	green_y = 1;
	
	// lowest point from these two things, center everything else from bottom to this point
	int lowest = MAX(3+al_get_bitmap_height(character_preview_box->bitmap)*3, 1+GREEN_WIN_BASE_H+extra_y);
	remaining_height = cfg.screen_h - lowest;

	if ((float)cfg.screen_w/cfg.screen_h < 3.0f/2.0f) {
		save_button = new W_Icon_Button(t("SAVE"), "misc_graphics/interface/fat_blue_button_narrow.cpi", "");	
		quit_button = new W_Icon_Button(t("QUIT"), "misc_graphics/interface/fat_blue_button_narrow.cpi", "");	
		return_button = new W_Icon_Button(t("RETURN"), "misc_graphics/interface/fat_red_button_narrow.cpi", "");
	}
	else {
		save_button = new W_Icon_Button(t("SAVE"), "misc_graphics/interface/fat_blue_button.cpi", "misc_graphics/interface/save_icon.cpi");	
		quit_button = new W_Icon_Button(t("QUIT"), "misc_graphics/interface/fat_blue_button.cpi", "misc_graphics/interface/x_icon.cpi");	
		return_button = new W_Icon_Button(t("RETURN"), "misc_graphics/interface/fat_red_button.cpi", "misc_graphics/interface/return_icon.cpi");
	}
	
	int width = GREEN_WIN_BASE_W + extra_x;
	
	quit_button->setX(cfg.screen_w-1-width/2-save_button->getWidth()/2);
	save_button->setX(quit_button->getX()-quit_button->getWidth()-1);
	return_button->setX(quit_button->getX()+quit_button->getWidth()+1);
	
	int y = cfg.screen_h-remaining_height/2-save_button->getHeight()/2;

	save_button->setY(y);
	quit_button->setY(y);
	return_button->setY(y);
	
	tgui::setNewWidgetParent(NULL);
	tgui::addWidget(save_button);
	tgui::addWidget(quit_button);
	tgui::addWidget(return_button);
	
	for (int i = 0; i < 3; i++) {
		player_buttons[i] = new W_Button(1, 1+i+al_get_bitmap_height(character_preview_box->bitmap)*i,
			al_get_bitmap_width(character_preview_box->bitmap),
			al_get_bitmap_height(character_preview_box->bitmap)
		);
		tgui::addWidget(player_buttons[i]);
	}

	if (engine->can_use_crystals()) {
		int w1 = General::get_text_width(General::FONT_LIGHT, General::itos(99));
		int w2 = General::get_text_width(General::FONT_LIGHT, General::itos(9999));
		int w4 = al_get_bitmap_width(nine_crystal->bitmap);
		int money_y = cfg.screen_h-al_get_bitmap_height(money_time_box->bitmap)-1;
		crystal_button = new W_Button(
			al_get_bitmap_width(money_time_box->bitmap)/2-(w1+w4+5+w2+w4)/2 - 2,
			money_y + 7 - 2,
			w1+w4+4,
			General::get_font_line_height(General::FONT_LIGHT)+4
		);
		tgui::addWidget(crystal_button);
	}
	else {
		crystal_button = NULL;
	}

	start_transition_in("sfx/main_menu.ogg");
	setup_current_menu();

	transition_start = al_get_time();
	transition_percent = 0.0f;

	create_bg();

	return true;
}

void Main_Menu_Loop::set_widget_offsets()
{
	if (state == S_TRANSITIONING_IN && !do_not_transition_in_again) {
		float ox, oy;
		ox = -cfg.screen_w * (1.0 - transition_percent);
		oy = cfg.screen_h * (1.0 - transition_percent);
		save_button->setOffset(General::Point<float>(ox, oy));
		quit_button->setOffset(General::Point<float>(ox, oy));
	 	return_button->setOffset(General::Point<float>(ox, oy));
	}
	else if (state == S_TRANSITIONING_OUT && current_menu == MAIN && dynamic_cast<I_Main_Menu_Parent *>(sub_loop)->get_selected() < 0) {
		float ox, oy;
		ox = -cfg.screen_w * transition_percent;
		oy = cfg.screen_h * transition_percent;
		save_button->setOffset(General::Point<float>(ox, oy));
		quit_button->setOffset(General::Point<float>(ox, oy));
	 	return_button->setOffset(General::Point<float>(ox, oy));
	}
	else if (state == S_IN) {
		save_button->setOffset(General::Point<float>(0, 0));
		quit_button->setOffset(General::Point<float>(0, 0));
	 	return_button->setOffset(General::Point<float>(0, 0));
		do_not_transition_in_again = true;
	}
}

void Main_Menu_Loop::top()
{
	sub_loop->top();
}

bool Main_Menu_Loop::handle_event(ALLEGRO_EVENT *event)
{
	return sub_loop->handle_event(event);
}

bool Main_Menu_Loop::logic()
{
	engine->set_touch_input_type(TOUCHINPUT_GUI);

	if (state != S_IN && state != S_OUT) {
		double elapsed = al_get_time() - transition_start;
		if (elapsed > TOTAL_TRANSITION_TIME)
			transition_percent = 1;
		else
			transition_percent = elapsed / TOTAL_TRANSITION_TIME;
		
		if (state == S_TRANSITIONING_IN) {
		}
		else if (state == S_TRANSITIONING_OUT) {
		}
		
		if (transition_percent == 1.0) {
			if (state == S_TRANSITIONING_IN) {
				state = S_IN;
				engine->set_send_tgui_events(true);
			}
			else if (state == S_TRANSITIONING_OUT) {
				state = S_OUT;
				if (current_menu == MAIN) {
					I_Main_Menu_Parent *m = dynamic_cast<I_Main_Menu_Parent *>(sub_loop);
					int choice = m->get_selected();
					if (choice < 0) {
						std::vector<Loop *> v;
						engine->set_loops(v, true);
						delete sub_loop;
						sub_loop = NULL;
						return true;
					}
					current_menu = (Menu_ID)choice;
				}
				else {
					current_menu = MAIN;
				}
				if (next_player != -1) {
					selected_player = next_player;
					next_player = -1;
				}
				setup_current_menu();
				skip_draw = true;
				start_transition_in("sfx/enter_submenu.ogg");
			}
		}
	}

	set_widget_offsets();
	
	tgui_update_result = tgui::update();
	int pressed_player_button = -1;
	for (size_t i = 0; i < players.size(); i++) {
		if (tgui_update_result && tgui_update_result == player_buttons[i]) {
			pressed_player_button = i;
			break;
		}
	}
	if ((tgui_update_result == return_button || (pressed_player_button >= 0 && current_menu != MAIN) || esc_pressed) && state != S_TRANSITIONING_OUT) {
		if (current_menu == MAIN) {
			if (tgui_update_result == return_button || esc_pressed) {
				tgui::setFocus(NULL);
			}
			Main_Menu_Main_Loop *l = dynamic_cast<Main_Menu_Main_Loop *>(sub_loop);
			l->set_transition_type(TRANSITION_DIAGONAL);
		}
		esc_pressed = false;
		main_menu_loop_button_pressed = true;
		start_transition_out(current_menu == MAIN ? "sfx/main_menu.ogg" : "sfx/exit_submenu.ogg");
		next_player = pressed_player_button;
		pressed_player_button = -1;
	}
	if (pressed_player_button >= 0) {
		selected_player = pressed_player_button;
	}
	if (tgui_update_result == save_button) {
#if defined OUYA || defined FIRETV
		if (!engine->get_purchased()) {
			std::string music = Music::get_playing();
			std::vector<std::string> texts;
			texts.push_back(t("CANT_SAVE_PURCHASE1"));
			texts.push_back(t("CANT_SAVE_PURCHASE2"));
			bool buy = engine->yes_no_prompt(texts);
			if (buy) {
				if (try_purchase(engine->get_event_queue())) {
					engine->set_purchased(true);
				}
				engine->switch_in();
				Music::play(music);
				tgui::setFocus(save_button);
			}
		}
		else
#elif defined DEMO
		if (true) {
			std::vector<std::string> texts;
			texts.push_back(t("NO_SAVE_GAME_IN_DEMO"));
			engine->notify(texts);
			tgui::setFocus(save_button);
		}
		else
#endif
		{
			engine->fade_out();
			tgui::hide();
			tgui::push(); // popped in ~SaveLoad_Loop()
			std::vector<Loop *> loops;
			SaveLoad_Loop *l = new SaveLoad_Loop(true);
			l->init();
			loops.push_back(l);
			engine->fade_in(loops);
			engine->do_blocking_mini_loop(loops, NULL);
			engine->fade_in();
		}
	}
	else if (tgui_update_result == quit_button) {
		std::vector<std::string> texts;
		texts.push_back(t("REALLY_QUIT_TO_TITLE"));
		if (engine->prompt(texts, t("YES"), t("NO")) == 0) {
			engine->set_done(true);
		}
	}
	else if (engine->can_use_crystals() && tgui_update_result == crystal_button) {
		engine->fade_out();

		tgui::hide();
		tgui::push(); // popped in ~Crystal_Loop()

		Crystal_Loop *l = new Crystal_Loop(players, selected_player);
		std::vector<Loop *> loops;
		loops.push_back(l);
		l->init();

		engine->fade_in(loops);

		engine->do_blocking_mini_loop(loops, "");
		
		engine->fade_in();
	}

	return sub_loop->logic();
}

void Main_Menu_Loop::draw()
{
	if (skip_draw) {
		return;
	}

	ALLEGRO_TRANSFORM backup_transform, t;
	al_copy_transform(&backup_transform, al_get_current_transform());
	al_identity_transform(&t);
	al_use_transform(&t);
	Wrap::Shader *tinter = Graphics::get_tint_shader();
	Shader::use(tinter);
	al_set_shader_float("color_r", 112.0f/112.0f);
	al_set_shader_float("color_g", 66.0f/112.0f);
	al_set_shader_float("color_b", 20.0f/112.0f);
	al_draw_bitmap(bg, 0, 0, 0);
	Shader::use(NULL);
	al_use_transform(&backup_transform);

	int ox, oy;

	if (state == S_TRANSITIONING_IN && !do_not_transition_in_again) {
		ox = -cfg.screen_w * (1.0 - transition_percent);
		oy = cfg.screen_h * (1.0 - transition_percent);
	}
	else if (state == S_TRANSITIONING_OUT && current_menu == MAIN && dynamic_cast<I_Main_Menu_Parent *>(sub_loop)->get_selected() < 0) {
		ox = -cfg.screen_w * transition_percent;
		oy = cfg.screen_h * transition_percent;
	}
	else {
		ox = oy = 0;
	}
	
	for (size_t i = 0; i < 3; i++) {	
		Player *p;
		if (i >= players.size()) {
			p = NULL;
		}
		else {
			p = players[i];
		}
		draw_character_preview_box(p, (int)i == selected_player, character_preview_box, ox+1, oy+1+i+al_get_bitmap_height(character_preview_box->bitmap)*i);
	}
	for (size_t i = 0; i < 3; i++) {	
		Player *p;
		if (i >= players.size()) {
			p = NULL;
		}
		else {
			p = players[i];
		}
		if (p && p->get_battle_attributes().status.name == "POISON") {
			Wrap::Bitmap *icon = resource_manager->reference_bitmap("misc_graphics/interface/" + p->get_name() + "_normal_icon.cpi");
			General::draw_poison_bubbles(General::Point<float>(
				ox+5+al_get_bitmap_width(icon->bitmap)/2,
				oy+1+i+al_get_bitmap_height(character_preview_box->bitmap)*i+al_get_bitmap_height(character_preview_box->bitmap)/2-al_get_bitmap_height(icon->bitmap)/2
			));
			resource_manager->release_bitmap("misc_graphics/interface/" + p->get_name() + "_normal_icon.cpi");
		}
	}
	
	int money_y = cfg.screen_h-al_get_bitmap_height(money_time_box->bitmap)-1;
	
	al_draw_bitmap(
		money_time_box->bitmap,
		ox+1, oy+money_y,
		0
 	);

	money_y += 7;

	ALLEGRO_COLOR crystals_color = engine->can_use_crystals() ? al_color_name("cyan") : al_color_name("lightgrey");

	int w1 = General::get_text_width(General::FONT_LIGHT, General::itos(99));
	int w2 = General::get_text_width(General::FONT_LIGHT, General::itos(9999));
	int w3 = General::get_text_width(General::FONT_LIGHT, General::get_time_string(Game_Specific_Globals::elapsed_time));
	int w4 = al_get_bitmap_width(nine_crystal->bitmap);

	General::draw_text(
		General::itos(Game_Specific_Globals::crystals),
		crystals_color,
		ox+al_get_bitmap_width(money_time_box->bitmap)/2-(w1+w2+w4*2+5)/2+w4,
		oy+money_y,
		0
	);
	if (engine->can_use_crystals()) {
		al_draw_line(
			ox+al_get_bitmap_width(money_time_box->bitmap)/2-(w1+w2+w4*2+5)/2,
			oy+money_y+11,
			ox+al_get_bitmap_width(money_time_box->bitmap)/2-(w1+w2+w4*2+5)/2+w1+w4,
			oy+money_y+11,
			crystals_color,
			1
		);
	}
	General::draw_text(
		General::itos(Game_Specific_Globals::cash),
		al_color_name("lightgrey"),
		ox+al_get_bitmap_width(money_time_box->bitmap)/2-(w1+w2+w4*2+5)/2+w1+w4+5+w4,
		oy+money_y,
		0
	);
	General::draw_text(
		General::get_time_string(Game_Specific_Globals::elapsed_time),
		al_color_name("lightgrey"),
		ox+al_get_bitmap_width(money_time_box->bitmap)/2-(w3+w4)/2+w4,
		oy+money_y+General::get_font_line_height(General::FONT_LIGHT),
		0
	);

	al_draw_bitmap(
		nine_crystal->bitmap,
		ox+al_get_bitmap_width(money_time_box->bitmap)/2-(w1+w2+w4*2+5)/2,
		oy+money_y,
		0
	);
	al_draw_bitmap(
		nine_coin->bitmap,
		ox+al_get_bitmap_width(money_time_box->bitmap)/2-(w1+w2+w4*2+5)/2+w1+w4+5,
		oy+money_y,
		0
	);
	al_draw_bitmap(
		nine_clock->bitmap,
		ox+al_get_bitmap_width(money_time_box->bitmap)/2-(w3+w4)/2,
		oy+money_y+General::get_font_line_height(General::FONT_LIGHT),
		0
	);

	sub_loop->draw();
 	
	tgui::draw();
}

void Main_Menu_Loop::post_draw()
{
	if (skip_draw) {
		skip_draw = false;
		return;
	}

	sub_loop->post_draw();
}

bool Main_Menu_Loop::get_main_menu_loop_button_pressed()
{
	return main_menu_loop_button_pressed;
}

void Main_Menu_Loop::set_main_menu_loop_button_pressed(bool pressed)
{
	main_menu_loop_button_pressed = pressed;
}

Main_Menu_Loop::Main_Menu_Loop(std::vector<Player *> players, std::vector<Loop *> background_draw_loops) :
	sub_loop(NULL),
	current_menu(MAIN),
	skip_draw(false),
	players(players),
	selected_player(0),
	do_not_transition_in_again(false),
	next_player(-1),
	main_menu_loop_button_pressed(false),
	background_draw_loops(background_draw_loops),
	esc_pressed(false),
	bg(NULL)
{
	crystal_button = NULL;

	for (size_t i = 0; i < players.size(); i++) {
		resource_manager->reference_bitmap("misc_graphics/interface/" + players[i]->get_name() + "_normal_icon.cpi");
	}
}

Main_Menu_Loop::~Main_Menu_Loop()
{
	resource_manager->release_bitmap("misc_graphics/interface/character_preview_box.cpi");
	resource_manager->release_bitmap("misc_graphics/interface/money_time_box.cpi");
	resource_manager->release_bitmap("misc_graphics/interface/9crystal.cpi");
	resource_manager->release_bitmap("misc_graphics/interface/9coin.cpi");
	resource_manager->release_bitmap("misc_graphics/interface/9clock.cpi");

	save_button->remove();
	quit_button->remove();
	return_button->remove();
	delete save_button;
	delete quit_button;
	delete return_button;
	
	if (crystal_button) {
		crystal_button->remove();
		delete crystal_button;
	}

	for (int i = 0; i < 3; i++) {
		player_buttons[i]->remove();
		delete player_buttons[i];
	}

	engine->destroy_sample("sfx/drop_item.ogg");
	engine->destroy_sample("sfx/enter_submenu.ogg");
	engine->destroy_sample("sfx/exit_submenu.ogg");

	delete sub_loop;
	
	for (size_t i = 0; i < players.size(); i++) {
		resource_manager->release_bitmap("misc_graphics/interface/" + players[i]->get_name() + "_normal_icon.cpi");
	}

	al_destroy_bitmap(bg);

	engine->set_send_tgui_events(true);
	
	engine->clear_touches();
}

std::vector<Player *> Main_Menu_Loop::get_players()
{
	return players;
}

int Main_Menu_Loop::get_selected_player()
{
	return selected_player;
}

void Main_Menu_Loop::setup_current_menu()
{
	bool first = sub_loop == NULL;

	delete sub_loop;

	switch (current_menu) {
		case MAIN:
			sub_loop = new Main_Menu_Main_Loop(this, first ? TRANSITION_DIAGONAL : TRANSITION_HORIZONTAL);
			break;
		case ABILITIES:
			sub_loop = new Main_Menu_Abilities_Loop(this);
			break;
		case ITEMS:
			sub_loop = new Main_Menu_Items_Loop(this);
			break;
		case EQUIP:
			sub_loop = new Main_Menu_Equip_Loop(this);
			break;
	}
	
	sub_loop->init();
}

void Main_Menu_Loop::start_transition_out(std::string sample_name)
{
	if (!main_menu_loop_button_pressed) {
		tgui::setFocus(NULL);
	}

	if (sample_name != "") {
		engine->play_sample(sample_name);
	}
	
	state = S_TRANSITIONING_OUT;
	transition_start = al_get_time();
	transition_percent = 0.0f;
	set_widget_offsets();

	engine->set_send_tgui_events(false);
}

void Main_Menu_Loop::start_transition_in(std::string sfx_name)
{
	if (sfx_name != "") {
		engine->play_sample(sfx_name);
	}

	state = S_TRANSITIONING_IN;
	transition_start = al_get_time();
	transition_percent = 0.0f;
	set_widget_offsets();
}

Main_Menu_Loop::State Main_Menu_Loop::get_state()
{
	return state;
}

float Main_Menu_Loop::get_transition_percent()
{
	return transition_percent;
}

int Main_Menu_Loop::get_green_x()
{
	return green_x;
}

int Main_Menu_Loop::get_green_y()
{
	return green_y;
}

int Main_Menu_Loop::get_extra_x()
{
	return extra_x;
}

int Main_Menu_Loop::get_extra_y()
{
	return extra_y;
}

tgui::TGUIWidget *Main_Menu_Loop::get_tgui_update_result()
{
	return tgui_update_result;
}

int Main_Menu_Loop::get_win_w()
{
	return GREEN_WIN_BASE_W + extra_x;
}

int Main_Menu_Loop::get_win_h()
{
	return GREEN_WIN_BASE_H + extra_y;
}

void Main_Menu_Loop::destroy_graphics()
{
}

void Main_Menu_Loop::reload_graphics()
{
}

void Main_Menu_Loop::create_bg()
{
	ALLEGRO_BITMAP *rb = engine->get_render_buffer()->bitmap;
	bool preserve;
#ifdef ALLEGRO_ANDROID
	preserve = true;
#else
	if (al_get_display_flags(engine->get_display()) & ALLEGRO_DIRECT3D) {
		preserve = true;
	}
	else {
		preserve = false;
	}
#endif
	preserve = true;
	int flags = al_get_new_bitmap_flags();
	if (preserve) {
		al_set_new_bitmap_flags(flags & ~ALLEGRO_NO_PRESERVE_TEXTURE);
	}
	bg = al_create_bitmap(al_get_bitmap_width(rb), al_get_bitmap_height(rb));
	if (preserve) {
		al_set_new_bitmap_flags(flags);
	}
	ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
	al_set_target_bitmap(bg);
	al_draw_bitmap(rb, 0, 0, 0);
	al_set_target_bitmap(old_target);
}

//-----------------------------------------------------

void Main_Menu_Main_Loop::set_widget_offsets()
{
	float ox, oy;
	get_green_window_offsets(main_menu_loop->get_state(), transition_type, main_menu_loop->get_transition_percent(), &ox, &oy);

	if (ox == 0 && oy == 0 && !done_first_transition) {
		done_first_transition = true;
		transition_type = TRANSITION_HORIZONTAL;
	}

	equip_button->setOffset(General::Point<float>(ox, oy));
	items_button->setOffset(General::Point<float>(ox, oy));
	abilities_button->setOffset(General::Point<float>(ox, oy));
}

bool Main_Menu_Main_Loop::init()
{
	if (inited) {
		return true;
	}
	Loop::init();

	std::vector<Player *> players = main_menu_loop->get_players();

	for (size_t i = 0; i < players.size(); i++) {
		std::string name = players[i]->get_name();
		profiles.push_back(Wrap::load_bitmap("misc_graphics/interface/" + name + "_profile.cpi"));
	}
	
	equip_button = new W_Icon_Button(t("EQUIP"), "misc_graphics/interface/medium_blue_button.cpi", "misc_graphics/interface/equip_icon.cpi");
	items_button = new W_Icon_Button(t("ITEMS"), "misc_graphics/interface/medium_blue_button.cpi", "misc_graphics/interface/items_icon.cpi");
	abilities_button = new W_Icon_Button(t("ABILITIES"), "misc_graphics/interface/medium_blue_button.cpi", "misc_graphics/interface/abilities_icon.cpi");

	int button_w = equip_button->getWidth();
	int button_h = equip_button->getHeight();
	
	profile_w = al_get_bitmap_width(profiles[0]->bitmap);
	profile_h = al_get_bitmap_height(profiles[0]->bitmap);
	int profile_mid = 7 + button_w/2;
	profile_x = profile_mid - profile_w/2;
	profile_y = 9;
	
	int button_x = profile_mid - button_w/2 + main_menu_loop->get_green_x();
	int button_y = profile_y + profile_h + 7;

	int extra_x = main_menu_loop->get_extra_x();
	text_x = button_x + button_w + 2 + (extra_x < 0 ? 0 : main_menu_loop->get_extra_x()/2);
	text_w = Main_Menu_Loop::GREEN_WIN_BASE_W + (extra_x < 0 ? extra_x : 0) - 7 - ((button_x + button_w + 1) - main_menu_loop->get_green_x());

	bool line_buttons_up = main_menu_loop->get_extra_y() > equip_button->getHeight() + 2;

	equip_button->setX(button_x);
	items_button->setX(button_x);
	abilities_button->setX(button_x+(line_buttons_up ? 0 : button_w+1));
	
	equip_button->setY(button_y);
	items_button->setY(button_y+button_h+2);
	abilities_button->setY(button_y+button_h+2+(line_buttons_up ? button_h+2 : 0));
	
	tgui::addWidget(equip_button);
	tgui::addWidget(items_button);
	tgui::addWidget(abilities_button);
	
	set_widget_offsets();

	return true;
}

void Main_Menu_Main_Loop::top()
{
}

bool Main_Menu_Main_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
		if (
			event->keyboard.keycode == ALLEGRO_KEY_ESCAPE
#if defined ALLEGRO_ANDROID
			|| event->keyboard.keycode == ALLEGRO_KEY_BUTTON_B
			|| event->keyboard.keycode == ALLEGRO_KEY_BACK
#endif
		) {
			main_menu_loop->set_esc_pressed(true);
			return true;
		}
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
		if (event->joystick.button == cfg.joy_ability[2]) {
			main_menu_loop->set_esc_pressed(true);
			return true;
		}
	}
	return false;
}

bool Main_Menu_Main_Loop::logic()
{
	if (!in) {
		if (main_menu_loop->get_state() == Main_Menu_Loop::S_IN) {
			in = true;
			if (!main_menu_loop->get_main_menu_loop_button_pressed()) {
				tgui::setFocus(main_menu_loop->get_return_button());
			}
		}
	}

	set_widget_offsets();

	if (main_menu_loop->get_state() == Main_Menu_Loop::S_IN) {
		tgui::TGUIWidget *w = main_menu_loop->get_tgui_update_result();
		if (w == abilities_button) {
			selected = (int)Main_Menu_Loop::ABILITIES;
			main_menu_loop->set_main_menu_loop_button_pressed(false);
			main_menu_loop->start_transition_out("sfx/exit_submenu.ogg");
		}
		else if (w == items_button) {
			selected = (int)Main_Menu_Loop::ITEMS;
			main_menu_loop->set_main_menu_loop_button_pressed(false);
			main_menu_loop->start_transition_out("sfx/exit_submenu.ogg");
		}
		else if (w == equip_button) {
			selected = (int)Main_Menu_Loop::EQUIP;
			main_menu_loop->set_main_menu_loop_button_pressed(false);
			main_menu_loop->start_transition_out("sfx/exit_submenu.ogg");
		}
	}

	return false;
}

void Main_Menu_Main_Loop::draw()
{
	std::vector<Player *> players = main_menu_loop->get_players();

	float ox, oy;

	get_green_window_offsets(main_menu_loop->get_state(), transition_type, main_menu_loop->get_transition_percent(), &ox, &oy);

	draw_green_window(main_menu_loop, ox, oy);
	
 	// Draw selected player info
 	
	al_draw_filled_rectangle(
		ox+main_menu_loop->get_green_x()+profile_x, oy+main_menu_loop->get_green_y()+profile_y,
		ox+main_menu_loop->get_green_x()+profile_x+profile_w, oy+main_menu_loop->get_green_y()+profile_y+profile_h,
		al_color_name("black")
	);
	// FIXME: draw right profile pic
	al_draw_bitmap(profiles[main_menu_loop->get_selected_player()]->bitmap, ox+main_menu_loop->get_green_x()+profile_x, oy+main_menu_loop->get_green_y()+profile_y, 0);

	// Draw stats
	Player *p = players[main_menu_loop->get_selected_player()];
	Battle_Attributes &attr = p->get_battle_attributes();
	
	int text_y;
	int text_h = General::get_font_line_height(General::FONT_LIGHT) - 2;
	int gauge_h = 3;
	ALLEGRO_COLOR hilight = Graphics::change_brightness(General::UI_GREEN, 1.35f);

	// Draw gauges first
	text_y = 10 + text_h*2;
	int max_hp = attr.max_hp;
	int max_mp = attr.max_mp;
	int attack = attr.attack;
	int defense = attr.defense;
	Game_Specific_Globals::get_accessory_effects(attr.equipment.accessory.name, &max_hp, &max_mp, &attack, &defense);
	attack += attr.equipment.weapon.name == "" ? 0 : attr.equipment.weapon.attack;
	defense += attr.equipment.armor.name == "" ? 0 : attr.equipment.armor.defense;
	float percent;
	percent = (float)(attr.hp/cfg.difficulty_mult()) / (max_hp/cfg.difficulty_mult());
	Graphics::draw_gauge(ox+text_x, oy+text_y, text_w, true, percent, hilight, al_color_name("lime"));
	text_y += text_h + gauge_h + 1;
	percent = (float)attr.mp / max_mp;
	Graphics::draw_gauge(ox+text_x, oy+text_y, text_w, false, percent, hilight, al_color_name("cyan"));
	text_y += text_h + gauge_h;
	percent = (float)attack / 7;
	Graphics::draw_gauge(ox+text_x, oy+text_y, text_w, false, percent, hilight, al_color_name("yellow"));
	text_y += text_h + gauge_h;
	percent = (float)defense / 5;
	Graphics::draw_gauge(ox+text_x, oy+text_y, text_w, false, percent, hilight, al_color_name("yellow"));
	
	// Then text
	text_y = 10 + text_h;
	General::draw_text(t("HEALTH"), ox+text_x, oy+text_y, 0);
	General::draw_text(General::itos(attr.hp/cfg.difficulty_mult()), ox+text_x+text_w, oy+text_y, ALLEGRO_ALIGN_RIGHT);
	text_y += text_h + gauge_h + 1;
	General::draw_text(t("MAGIC"), ox+text_x, oy+text_y, 0);
	General::draw_text(General::itos(attr.mp), ox+text_x+text_w, oy+text_y, ALLEGRO_ALIGN_RIGHT);
	text_y += text_h + gauge_h;
	General::draw_text(t("ATTACK"), ox+text_x, oy+text_y, 0);
	General::draw_text(General::itos(attack), ox+text_x+text_w, oy+text_y, ALLEGRO_ALIGN_RIGHT);
	text_y += text_h + gauge_h;
	General::draw_text(t("DEFENSE"), ox+text_x, oy+text_y, 0);
	General::draw_text(General::itos(defense), ox+text_x+text_w, oy+text_y, ALLEGRO_ALIGN_RIGHT);
}

Main_Menu_Main_Loop::Main_Menu_Main_Loop(Main_Menu_Loop *main_menu_loop, int transition_type) :
	main_menu_loop(main_menu_loop),
	done_first_transition(false),
	transition_type(transition_type),
	in(false)
{
	selected = -1;
}

Main_Menu_Main_Loop::~Main_Menu_Main_Loop()
{
	for (size_t i = 0; i < profiles.size(); i++) {
		Wrap::destroy_bitmap(profiles[i]);
	}

	equip_button->remove();
	items_button->remove();
	abilities_button->remove();
	delete equip_button;
	delete items_button;
	delete abilities_button;
}

void Main_Menu_Main_Loop::set_transition_type(int transition_type)
{
	this->transition_type = transition_type;
}

//-----------------------------------------------------

static void abilities_notifier(void *data, float row_y_pixel)
{
	Main_Menu_Abilities_Loop::Notification *n = (Main_Menu_Abilities_Loop::Notification *)data;

	int button = n->loop->get_selected();

	if (button < 0) {
		return;
	}

	int ability = n->loop->get_list_selected();

	std::vector<std::string> abilities = n->player->get_abilities();

	std::string name;

	if (ability >= (int)abilities.size()) {
		name = "NONE_ABILITY";
	}
	else {
		name = abilities[ability];
	}

	n->player->set_selected_ability(true, button, name == "NONE_ABILITY" ? "" : name);
	engine->play_sample("sfx/use_item.ogg");
}

void Main_Menu_Abilities_Loop::set_widget_offsets()
{
	float ox, oy;
	get_green_window_offsets(main_menu_loop->get_state(), TRANSITION_HORIZONTAL, main_menu_loop->get_transition_percent(), &ox, &oy);

	list->setOffset(General::Point<float>(ox, oy));
	scrollbar->setOffset(General::Point<float>(ox, oy));

	for (int i = 0; i < 3; i++) {
		ability_buttons[i]->setOffset(General::Point<float>(ox, oy));
	}
}

bool Main_Menu_Abilities_Loop::init()
{
	if (inited) {
		return true;
	}
	Loop::init();

	float ox, oy;

	get_green_window_offsets(main_menu_loop->get_state(), TRANSITION_HORIZONTAL, main_menu_loop->get_transition_percent(), &ox, &oy);
	abilities_button = Wrap::load_bitmap("misc_graphics/interface/abilities_button.cpi");
	abilities_button_grey = Wrap::load_bitmap("misc_graphics/interface/abilities_button_grey.cpi");

	abilities_button_w = al_get_bitmap_width(abilities_button->bitmap);
	abilities_button_h = al_get_bitmap_height(abilities_button->bitmap);
	dark_green_h = abilities_button_h + 2;
	buttons_y = main_menu_loop->get_green_y() + Main_Menu_Loop::GREEN_WIN_BASE_H + main_menu_loop->get_extra_y() - dark_green_h*3 - 3 - 6;
	win_w = main_menu_loop->get_win_w();
	green_x = main_menu_loop->get_green_x();
	widths[0] = win_w/2 - 6;
	widths[1] = win_w/2 - abilities_button_w - 6;
	widths[2] = win_w/2 - abilities_button_w - 6;
	widths[3] = win_w/2 - 6;
	xstarts[0] = green_x+8;
	xstarts[1] = green_x+8;
	xstarts[2] = green_x+win_w-6-widths[2];
	xstarts[3] = green_x+win_w-6-widths[3];
	xends[0] = green_x + 6 + widths[0];
	xends[1] = green_x + 6 + widths[1];
	xends[2] = green_x + win_w - 7;
	xends[3] = green_x + win_w - 7;
	button_starts[0] = xends[0] - abilities_button_w/2;
	button_starts[1] = xends[1] - abilities_button_w/2;
	button_starts[2] = xstarts[2] - abilities_button_w/2;
	button_starts[3] = xstarts[3] - abilities_button_w/2;
	int clickable_starts[4];
	clickable_starts[0] = xstarts[0];
	clickable_starts[1] = xstarts[1];
	clickable_starts[2] = xstarts[2] - abilities_button_w/2;
	clickable_starts[3] = xstarts[3] - abilities_button_w/2;

	abilities = main_menu_loop->get_players()[main_menu_loop->get_selected_player()]->get_abilities();
	abilities.push_back("NONE_ABILITY");

	std::vector<std::string> icon_filenames;

	for (size_t i = 0; i < abilities.size(); i++) {
		icon_filenames.push_back("");
	}

	int green_x = main_menu_loop->get_green_x();
	int green_y = main_menu_loop->get_green_y();
	int list_w = Main_Menu_Loop::GREEN_WIN_BASE_W + main_menu_loop->get_extra_x() - 16 - W_Vertical_Scrollbar::WIDTH;
	int list_h = buttons_y - green_y - 12;
	list = new W_Scrolling_List(abilities, icon_filenames, std::vector<std::string>(), std::vector<bool>(), General::FONT_LIGHT, true);
	list->set_translate_item_names(true);
	list->setX(green_x+8);
	list->setY(green_y+9);
	list->setWidth(list_w);
	list->setHeight(list_h);
	list->setNotifier(abilities_notifier, notification);
	list->set_selected(abilities.size()-1);
	list->set_tint_icons(false);

	scrollbar = new W_Vertical_Scrollbar(list->get_scrollbar_tab_size());
	scrollbar->setX(list->getX()+list->getWidth()+1);
	scrollbar->setY(list->getY());
	scrollbar->setHeight(list->getHeight());

	list->setSyncedWidget(scrollbar);
	scrollbar->setSyncedWidget(list);

	// create buttons for red buttons
	int by = buttons_y;
	for (int i = 0; i < 3; i++) {
		ability_buttons[i] = new W_Button(clickable_starts[i], by, widths[i]+abilities_button_w/2, dark_green_h);
		if (i != 1) {
			by += dark_green_h + 1;
		}
	}

	tgui::addWidget(list);
	tgui::addWidget(scrollbar);

	for (int i = 0; i < 3; i++) {
		tgui::addWidget(ability_buttons[i]);
	}

	set_widget_offsets();

	return true;
}

void Main_Menu_Abilities_Loop::top()
{
}

bool Main_Menu_Abilities_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (!list_was_activated) {
		if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
			if (
				event->keyboard.keycode == ALLEGRO_KEY_ESCAPE
#if defined ALLEGRO_ANDROID
				|| event->keyboard.keycode == ALLEGRO_KEY_BUTTON_B
				|| event->keyboard.keycode == ALLEGRO_KEY_BACK
#endif
			) {
				main_menu_loop->set_esc_pressed(true);
				return true;
			}
		}
		else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
			if (event->joystick.button == cfg.joy_ability[2]) {
				main_menu_loop->set_esc_pressed(true);
				return true;
			}
		}
	}
	return false;
}

bool Main_Menu_Abilities_Loop::logic()
{
	if (!in) {
		if (main_menu_loop->get_state() == Main_Menu_Loop::S_IN) {
			in = true;
			if (!main_menu_loop->get_main_menu_loop_button_pressed()) {
				tgui::setFocus(main_menu_loop->get_return_button());
			}
		}
	}

	set_widget_offsets();

	if (list->is_activated()) {
		list_was_activated = true;
	}
	else {
		list_was_activated = false;
	}

	tgui::TGUIWidget *w;
	if (main_menu_loop->get_state() == Main_Menu_Loop::S_IN) {
		w = main_menu_loop->get_tgui_update_result();
		for (int i = 0; i < 3; i++) {
			if (w == ability_buttons[i]) {
				selected = i;
				std::vector<Player *> players = main_menu_loop->get_players();
				Player *p = players[main_menu_loop->get_selected_player()];
				std::vector<std::string> selected_abilities = p->get_selected_abilities(true, false, false);
				std::vector<std::string> abilities = p->get_abilities();
				abilities.push_back("NONE_ABILITY");
				std::string name = selected_abilities[selected];
				if (name == "") {
					name = "NONE_ABILITY";
				}
				size_t j = 0;
				for (; j < abilities.size(); j++) {
					if (abilities[j] == name) {
						break;
					}
				}
				if (j == abilities.size()) {
					j = abilities.size()-1;
				}
				list->set_selected(j);
			}
		}
	}

	return false;
}

void Main_Menu_Abilities_Loop::draw()
{
	float ox, oy;

	get_green_window_offsets(main_menu_loop->get_state(), TRANSITION_HORIZONTAL, main_menu_loop->get_transition_percent(), &ox, &oy);

	draw_green_window(main_menu_loop, ox, oy);

	Wrap::Bitmap *bmps[4] = {
		abilities_button,
		abilities_button,
		abilities_button,
		abilities_button_grey
	};

	int text_pos[4] = {
		xends[0] - abilities_button_w/2 - 3,
		xends[1] - abilities_button_w/2 - 3,
		xstarts[2] + abilities_button_w/2 + 4,
		xstarts[3] + abilities_button_w/2 + 4
	};

	int align[4] = {
		ALLEGRO_ALIGN_RIGHT,
		ALLEGRO_ALIGN_RIGHT,
		0,
		0
	};

	std::vector<Player *> players = main_menu_loop->get_players();
	Player *p = players[main_menu_loop->get_selected_player()];
	std::vector<std::string> selected_abilities = p->get_selected_abilities(true, false, false);

	std::string texts[4] = {
		t(selected_abilities[0].c_str()),
		t(selected_abilities[1].c_str()),
		t(selected_abilities[2].c_str()),
		t(selected_abilities[3].c_str())
	};

	int by = buttons_y;
	int by_save = 0;
	for (int i = 0; i < 4; i++) {
		al_draw_filled_rectangle(ox+xstarts[i], oy+by, ox+xends[i], oy+by + dark_green_h, al_map_rgb(18, 43, 24));
		al_draw_bitmap(bmps[i]->bitmap, ox+button_starts[i], oy+by + 1, 0);
		General::draw_text(texts[i], ox+text_pos[i], oy+by-1, align[i]);
		if (i == selected) {
			by_save = by;
		}
		if (i != 1) {
			by += dark_green_h + 1;
		}
	}
	if (selected >= 0) {
		int radius = (abilities_button_w + 6) / 2;
		Graphics::draw_focus_circle(ox+button_starts[selected]+abilities_button_w/2.0, oy+by_save+abilities_button_h/2.0+1, radius, al_color_name("yellow"));
	}
}

Main_Menu_Abilities_Loop::Main_Menu_Abilities_Loop(Main_Menu_Loop *main_menu_loop) :
	main_menu_loop(main_menu_loop),
	selected(-1),
	in(false),
	list_was_activated(false)
{
	std::vector<Player *> players = main_menu_loop->get_players();
	Player *p = players[main_menu_loop->get_selected_player()];
	notification = new Notification;
	notification->loop = this;
	notification->player = p;
}

Main_Menu_Abilities_Loop::~Main_Menu_Abilities_Loop()
{
	Wrap::destroy_bitmap(abilities_button);
	Wrap::destroy_bitmap(abilities_button_grey);

	list->remove();
	scrollbar->remove();
	delete list;
	delete scrollbar;
	
	for (int i = 0; i < 3; i++) {
		ability_buttons[i]->remove();
		delete ability_buttons[i];
	}

	delete notification;
}

int Main_Menu_Abilities_Loop::get_selected()
{
	return selected;
}

int Main_Menu_Abilities_Loop::get_list_selected()
{
	return list->get_selected();
}

//-----------------------------------------------------

void items_notifier(void *data, float row_y_pixel)
{
	Main_Menu_Items_Loop::Notification *d = (Main_Menu_Items_Loop::Notification *)data;
	W_Scrolling_List *list = d->list;
	W_Vertical_Scrollbar *scrollbar = d->scrollbar;
	Main_Menu_Items_Loop *loop = d->loop;

	if (list->get_right_icon_clicked()) {
		int sel = list->get_selected();
		std::vector<Game_Specific_Globals::Item> &items = Game_Specific_Globals::get_items();
		int quantity = items[sel].quantity;
		int num_to_drop;
		std::vector<std::string> v;
		v.push_back(t(items[sel].name.c_str()));
		if (quantity == 1) {
			v.push_back(t("REALLY_DROP"));
			num_to_drop = engine->yes_no_prompt(v);
		}
		else {
			v.push_back(t("DROP_HOW_MANY"));
			num_to_drop = engine->get_number(v, 0, quantity, 0);
		}
		if (num_to_drop == quantity) {
			DROP(list->get_item_name(sel) + "x" + General::itos(num_to_drop), list->getX()+12, row_y_pixel)
			list->remove_item(sel);
			scrollbar->set_tab_size(list->get_scrollbar_tab_size());
			items.erase(items.begin() + sel);
			if (sel >= (int)items.size()) {
				list->set_selected(sel-1);
			}
		}
		else if (num_to_drop > 0) {
			DROP(list->get_item_name(sel) + "x" + General::itos(num_to_drop), list->getX()+12, row_y_pixel)
			items[sel].quantity -= num_to_drop;
			list->update_item(sel, false, "", false, "", true, General::itos(items[sel].quantity));
		}
	}
	else {
		loop->set_do_use(true);
	}
}

bool Main_Menu_Items_Loop::init()
{
	if (inited) {
		return true;
	}
	Loop::init();

	notification = new Notification;

	int green_x = main_menu_loop->get_green_x();
	int green_y = main_menu_loop->get_green_y();

	win_w = main_menu_loop->get_win_w();
	win_h = main_menu_loop->get_win_h();

	std::vector<Game_Specific_Globals::Item> &items = Game_Specific_Globals::get_items();

	std::vector<std::string> item_names;
	std::vector<std::string> icon_filenames;
	std::vector<std::string> right_justified_text;
	std::vector<std::string> right_icon_filenames;

	for (size_t i = 0; i < items.size(); i++) {
		item_names.push_back(t(items[i].name.c_str()));
		std::string icon_filename = "misc_graphics/interface/items_icon_white.cpi";
		icon_filenames.push_back(icon_filename);
		Wrap::Bitmap *bmp = resource_manager->reference_bitmap(items[i].image_filename);
		item_images.push_back(bmp);
		right_justified_text.push_back(General::itos(items[i].quantity));
		right_icon_filenames.push_back("misc_graphics/interface/x_icon.cpi");
	}

	info_panel_h = General::get_font_line_height(General::FONT_LIGHT)*2 + 2;

	list = new W_Scrolling_List(item_names, icon_filenames, right_justified_text, std::vector<bool>(), General::FONT_LIGHT, true);
	list->set_can_arrange(true);
	list->setX(green_x+8+55+1);
	list->setY(green_y+9);
	list->setWidth(win_w-16-55-1-W_Vertical_Scrollbar::WIDTH);
	list->setHeight(win_h - 18 - 1 - info_panel_h);
	list->set_right_icon_filenames(right_icon_filenames);
	list->set_item_images(&item_images);

	scrollbar = new W_Vertical_Scrollbar(list->get_scrollbar_tab_size());
	scrollbar->setX(list->getX()+list->getWidth()+1);
	scrollbar->setY(list->getY());
	scrollbar->setHeight(list->getHeight());

	notification->list = list;
	notification->scrollbar = scrollbar;
	notification->loop = this;
	list->setNotifier(items_notifier, notification);

	list->setSyncedWidget(scrollbar);
	scrollbar->setSyncedWidget(list);

	item_button_x = green_x+8;
	item_button_y = list->getY();
	item_button_w = 55;
	item_button_h = list->getHeight();
	item_button = new W_Button(item_button_x, item_button_y, item_button_w, item_button_h);
	item_button->set_sample_name("");

	tgui::addWidget(list);
	tgui::addWidget(scrollbar);
	tgui::addWidget(item_button);

	set_widget_offsets();

	return true;
}

void Main_Menu_Items_Loop::top()
{
}

bool Main_Menu_Items_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (!list_was_activated) {
		if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
			if (
				event->keyboard.keycode == ALLEGRO_KEY_ESCAPE
#if defined ALLEGRO_ANDROID
				|| event->keyboard.keycode == ALLEGRO_KEY_BUTTON_B
				|| event->keyboard.keycode == ALLEGRO_KEY_BACK
#endif
			) {
				main_menu_loop->set_esc_pressed(true);
				return true;
			}
		}
		else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
			if (event->joystick.button == cfg.joy_ability[2]) {
				main_menu_loop->set_esc_pressed(true);
				return true;
			}
		}
	}
	return false;
}

bool Main_Menu_Items_Loop::logic()
{
	if (!in) {
		if (main_menu_loop->get_state() == Main_Menu_Loop::S_IN) {
			in = true;
			if (!main_menu_loop->get_main_menu_loop_button_pressed()) {
				tgui::setFocus(main_menu_loop->get_return_button());
			}
		}
	}

	set_widget_offsets();

	if (list->is_activated()) {
		list_was_activated = true;
	}
	else {
		list_was_activated = false;
	}

	tgui::TGUIWidget *w;
	if (main_menu_loop->get_state() == Main_Menu_Loop::S_IN) {
		w = main_menu_loop->get_tgui_update_result();
		if (w == item_button && list->get_num_items() > 0) {
			do_use = true;
		}
	}

	if (do_use) {
		do_use = false;
		// FIXME: properly use item, not just deplete
		std::vector<Game_Specific_Globals::Item> &items = Game_Specific_Globals::get_items();
		int sel = list->get_selected();
		std::vector<Player *> players = main_menu_loop->get_players();
		Game_Specific_Globals::use_item(items[sel].name, players[main_menu_loop->get_selected_player()]->get_battle_attributes(), true);
		items[sel].quantity--;
		if (items[sel].quantity <= 0) {
			// remove from list
			resource_manager->release_bitmap(items[sel].image_filename);
			items.erase(items.begin()+sel);
			item_images.erase(item_images.begin()+sel);
			list->remove_item(sel);
			scrollbar->set_tab_size(list->get_scrollbar_tab_size());
			if (sel >= (int)items.size()) {
				list->set_selected(sel-1);
			}
		}
		else {
			// update list
			list->update_item(sel, false, "", false, "", true, General::itos(items[sel].quantity));
		}
	}

	if (something_dropped) {
		dropped_string_pos.x += dropped_string_velocity.x;
		dropped_string_pos.y += dropped_string_velocity.y;
		int len = General::get_text_width(General::FONT_LIGHT, dropped_string) / 2;
		if (dropped_string_pos.x < -len || dropped_string_pos.y < -len || dropped_string_pos.x > cfg.screen_w+len || dropped_string_pos.y > cfg.screen_h+len) {
			something_dropped = false;
		}
		else {
			dropped_string_angle -= 0.1f;
		}
	}

	return false;
}

void Main_Menu_Items_Loop::draw()
{
	float ox, oy;

	get_green_window_offsets(main_menu_loop->get_state(), TRANSITION_HORIZONTAL, main_menu_loop->get_transition_percent(), &ox, &oy);

	draw_green_window(main_menu_loop, ox, oy);

	int green_x = main_menu_loop->get_green_x();
	int green_y = main_menu_loop->get_green_y();

	if (list->get_num_items() > 0) {
		ALLEGRO_COLOR dark = General::UI_GREEN;
		dark = Graphics::change_brightness(dark, 0.65f);

		int sel = list->get_selected();

		al_draw_filled_rectangle(ox+item_button_x, oy+item_button_y, ox+item_button_x+item_button_w, oy+item_button_y+item_button_h, dark);
		Wrap::Bitmap *bmp = item_images[sel];
		if (bmp) {
			al_draw_bitmap(
				bmp->bitmap,
				ox+item_button_x+item_button_w/2-al_get_bitmap_width(bmp->bitmap)/2,
				oy+item_button_y+item_button_h/2-al_get_bitmap_height(bmp->bitmap)/2,
				0
			);
		}

		al_draw_filled_rectangle(green_x+ox+8, green_y+oy+win_h-9-info_panel_h, green_x+ox+win_w-7, green_y+oy+win_h-7, dark);
		std::vector<Game_Specific_Globals::Item> &items = Game_Specific_Globals::get_items();
		General::draw_wrapped_text(
			Game_Specific_Globals::get_item_description(items[sel].name),
			al_color_name("white"), green_x+ox+9, green_y+oy+win_h-9-info_panel_h+1, win_w-18, General::FONT_LIGHT
		);
	}
	else {
		General::draw_wrapped_text(
			t("NO_ITEMS"), al_color_name("white"), green_x+ox+9, green_y+oy+win_h-9-info_panel_h+1, win_w-18, General::FONT_LIGHT
		);
	}
}

void Main_Menu_Items_Loop::post_draw()
{
	if (something_dropped) {
		ALLEGRO_TRANSFORM t, backup;
		al_copy_transform(&backup, al_get_current_transform());
		al_identity_transform(&t);
		al_rotate_transform(&t, dropped_string_angle);
		al_translate_transform(&t, dropped_string_pos.x, dropped_string_pos.y);
		al_compose_transform(&t, &backup);
		al_use_transform(&t);
		General::draw_text(dropped_string, al_color_name("red"), 0, 0, ALLEGRO_ALIGN_CENTER);
		al_use_transform(&backup);
	}
}

Main_Menu_Items_Loop::Main_Menu_Items_Loop(Main_Menu_Loop *main_menu_loop) :
	something_dropped(false),
	main_menu_loop(main_menu_loop),
	in(false),
	do_use(false),
	list_was_activated(false)
{
}

Main_Menu_Items_Loop::~Main_Menu_Items_Loop()
{
	list->remove();
	scrollbar->remove();
	item_button->remove();
	delete list;
	delete scrollbar;
	delete item_button;
	
	std::vector<Game_Specific_Globals::Item> &items = Game_Specific_Globals::get_items();
	for (size_t i = 0; i < items.size(); i++) {
		resource_manager->release_bitmap(items[i].image_filename);
	}
	
	delete notification;
}

void Main_Menu_Items_Loop::set_widget_offsets()
{
	float ox, oy;

	get_green_window_offsets(main_menu_loop->get_state(), TRANSITION_HORIZONTAL, main_menu_loop->get_transition_percent(), &ox, &oy);

	General::Point<float> p(ox, oy);

	list->setOffset(p);
	scrollbar->setOffset(p);
}

void Main_Menu_Items_Loop::set_do_use(bool do_use)
{
	this->do_use = do_use;
}

//-----------------------------------------------------

static void equipped_equip_notifier(void *data, float row_y_pixel)
{
	Main_Menu_Equip_Loop::Notification *d = (Main_Menu_Equip_Loop::Notification *)data;
	W_Equipment_List *list = d->list;
	W_Equipment_List *list2 = d->list2;

	std::vector<Player *> players = d->players;
	int player = d->selected_player;
	Battle_Attributes &attr = players[player]->get_battle_attributes();
	Equipment::Equipment &equip = attr.equipment;

	int sel = list2->get_selected();

	if (list2->get_item_name(sel) != "") {
		if (list2->get_right_icon_clicked()) {
			std::vector<std::string> v;
			if (sel == 0) {
				std::vector<Equipment::Weapon> &v = Game_Specific_Globals::get_weapons();
				if (equip.weapon.attachments.size() > 0) {
					list2->update_item(sel, false, "", true, "misc_graphics/interface/weapon_icon.cpi", false, "");
					list2->update_description(sel, "");
					for (int i = equip.weapon.attachments.size()-1; i >= 0; i--) {
						v.insert(v.begin(), equip.weapon.attachments[i]);
						list->insert_item(0, "misc_graphics/interface/weapon_icon.cpi", equip.weapon.attachments[i].name, "misc_graphics/interface/x_icon.cpi", Equipment::WEAPON, std::string(t("QUANTITY")) + ": " + General::itos(equip.weapon.attachments[i].quantity), false);
					}
					equip.weapon.attachments.clear();
					// NOTE: HACK!!! Trick the code below so the weapon stays focussed (there will always be one!)
					sel--;
				}
				else {
					v.insert(v.begin(), equip.weapon);
					list2->update_item(sel, true, "", false, "", false, "");
					list->insert_item(0, "misc_graphics/interface/weapon_icon.cpi", equip.weapon.name, "misc_graphics/interface/x_icon.cpi", Equipment::WEAPON, "", false);
					equip.weapon.name = "";
					equip.weapon.attack = 0;
					equip.weapon.element = Equipment::ELEMENT_NONE;
					equip.weapon.usable_by = players[sel]->get_name();
					players[player]->load_weapon("");
				}
			}
			else if (sel == 1) {
				int idx = Game_Specific_Globals::get_weapons().size();
				std::vector<Equipment::Armor> &v = Game_Specific_Globals::get_armor();
				v.insert(v.begin(), equip.armor);
				list2->update_item(sel, true, "", false, "", false, "");
				list->insert_item(idx, "misc_graphics/interface/armor_icon.cpi", equip.armor.name, "misc_graphics/interface/x_icon.cpi", Equipment::ARMOR, "", false);
				equip.armor.name = "";
				equip.armor.defense = 0;
				equip.armor.element = Equipment::ELEMENT_NONE;
			}
			else if (sel == 2) {
				int idx = Game_Specific_Globals::get_weapons().size() + Game_Specific_Globals::get_armor().size();
				std::vector<Equipment::Accessory> &v = Game_Specific_Globals::get_accessories();
				v.insert(v.begin(), equip.accessory);
				list2->update_item(sel, true, "", false, "", false, "");
				list->insert_item(idx, "misc_graphics/interface/accessory_icon.cpi", equip.accessory.name, "misc_graphics/interface/x_icon.cpi", Equipment::ACCESSORY, Game_Specific_Globals::get_item_description(equip.accessory.name), false);
				equip.accessory.name = "";
			}
			int first = sel + 1;
			int found = -1;
			for (int i = 0; i < 3; i++) {
				int j = (first + i) % 3;
				if (list2->get_item_name(j) != "") {
					found = j;
					break;
				}
			}
			if (found != -1) {
				list2->set_selected(found);
				list2->set_active_column(0);
			}
			else {
				list2->deactivate();
			}
			engine->play_sample("sfx/drop_item.ogg");
		}
	}
}

static void equip_notifier(void *data, float row_y_pixel)
{
	Main_Menu_Equip_Loop::Notification *d = (Main_Menu_Equip_Loop::Notification *)data;
	W_Equipment_List *list = d->list;
	//W_Equipment_List *list2 = d->list2;
	W_Vertical_Scrollbar *scrollbar = d->scrollbar;
	Main_Menu_Equip_Loop *loop = d->loop;
	std::vector<Player *> players = d->players;
	int sel_player = d->selected_player;
	Battle_Attributes &attr = players[sel_player]->get_battle_attributes();
	Equipment::Equipment &equip = attr.equipment;

	if (list->get_right_icon_clicked()) {
		std::vector<std::string> test;
		test.push_back(t(list->get_item_name(list->get_selected()).c_str()));
		test.push_back(t("REALLY_DROP"));
		bool drop = engine->yes_no_prompt(test);
		if (drop) {
			// FIXME: throw + smashed glass sound
			int selected = list->get_selected();
			Equipment::Equipment_Type equip_type = list->get_type(selected);
			if (equip_type == Equipment::ARMOR) {
				int this_idx = selected - Game_Specific_Globals::get_weapons().size();
				Game_Specific_Globals::get_armor().erase(Game_Specific_Globals::get_armor().begin()+this_idx);
			}
			else if (equip_type == Equipment::ACCESSORY) {
				int this_idx = selected - Game_Specific_Globals::get_weapons().size() - Game_Specific_Globals::get_armor().size();
				Game_Specific_Globals::get_accessories().erase(Game_Specific_Globals::get_accessories().begin()+this_idx);
			}
			else {
				int this_idx = selected;
				Game_Specific_Globals::get_weapons().erase(Game_Specific_Globals::get_weapons().begin()+this_idx);
			}
			DROP(list->get_item_name(selected) + "x1", list->getX()+12, row_y_pixel)
			list->remove_item(selected);
			scrollbar->set_tab_size(list->get_scrollbar_tab_size());
			int total_size = Game_Specific_Globals::get_weapons().size() + Game_Specific_Globals::get_armor().size() + Game_Specific_Globals::get_accessories().size();
			if (total_size == 0) {
				list->deactivate();
			}
			else if (selected >= total_size) {
				list->set_selected(selected-1);
			}
		}
	}
	else {
		int sel = list->get_selected();
		std::string name = list->get_item_name(sel);
		if (Game_Specific_Globals::weapon_is_attachment(name)) {
			if (Game_Specific_Globals::weapon_attaches_to(name, equip.weapon.name)) {
				if (equip.weapon.attachments.size() == 0) {
					loop->start_swap(sel, row_y_pixel);
					engine->play_sample("sfx/use_item.ogg");
					int total_size = Game_Specific_Globals::get_weapons().size() + Game_Specific_Globals::get_armor().size() + Game_Specific_Globals::get_accessories().size();
					if (total_size == 0) {
						list->deactivate();
					}
					else if (sel >= total_size) {
						list->set_selected(sel-1);
					}
				}
				else if (equip.weapon.attachments[0].name == name) {
					if (equip.weapon.attachments[0].quantity < 99) {
						loop->start_swap(sel, row_y_pixel);
						engine->play_sample("sfx/use_item.ogg");
						int total_size = Game_Specific_Globals::get_weapons().size() + Game_Specific_Globals::get_armor().size() + Game_Specific_Globals::get_accessories().size();
						if (total_size == 0) {
							list->deactivate();
						}
						else if (sel >= total_size) {
							list->set_selected(sel-1);
						}
					}
					else {
						std::vector<std::string> v;
						v.push_back(t("TOO_MANY_ATTACHMENTS"));
						engine->notify(v);
					}
				}
				else {
					std::vector<std::string> v;
					v.push_back(t("UNEQUIP_ATTACHMENT_FIRST"));
					engine->notify(v);
				}
			}
			else {
				std::vector<std::string> v;
				v.push_back(t("DOESNT_ATTACH"));
				engine->notify(v);
			}
		}
		else {
			loop->start_swap(sel, row_y_pixel);
			engine->play_sample("sfx/use_item.ogg");
			int total_size = Game_Specific_Globals::get_weapons().size() + Game_Specific_Globals::get_armor().size() + Game_Specific_Globals::get_accessories().size();
			if (total_size == 0) {
				list->deactivate();
			}
			else if (sel >= total_size) {
				list->set_selected(sel-1);
			}
		}
	}
}

bool Main_Menu_Equip_Loop::init()
{
	if (inited) {
		return true;
	}
	Loop::init();

	notification = new Notification;
	notification2 = new Notification;

	int green_x = main_menu_loop->get_green_x();
	int green_y = main_menu_loop->get_green_y();

	win_w = main_menu_loop->get_win_w();
	win_h = main_menu_loop->get_win_h();

	top_box_w = win_w - 15;
	top_box_h = General::get_font_line_height(General::FONT_LIGHT) * 3;

	std::vector<Equipment::Weapon> weapons = Game_Specific_Globals::get_weapons();
	std::vector<Equipment::Armor> armor = Game_Specific_Globals::get_armor();
	std::vector<Equipment::Accessory> accessories = Game_Specific_Globals::get_accessories();

	std::vector<std::string> item_names;
	std::vector<std::string> icon_filenames;
	std::vector<std::string> right_icon_filenames;
	std::vector<Equipment::Equipment_Type> equipment_type;
	std::vector<std::string> descriptions;
	std::vector<bool> disabled;

	std::vector<Player *> players = main_menu_loop->get_players();
	int player = main_menu_loop->get_selected_player();

	for (size_t i = 0; i < weapons.size(); i++) {
		item_names.push_back(weapons[i].name);
		icon_filenames.push_back("misc_graphics/interface/weapon_icon.cpi");
		right_icon_filenames.push_back("misc_graphics/interface/x_icon.cpi");
		equipment_type.push_back(Equipment::WEAPON);
		if (Game_Specific_Globals::weapon_is_attachment(weapons[i].name)) {
			descriptions.push_back(std::string(t("QUANTITY")) + ": " + General::itos(weapons[i].quantity));
		}
		else {
			descriptions.push_back("");
		}
		if (weapons[i].usable_by != players[player]->get_name()) {
			disabled.push_back(true);
		}
		else {
			disabled.push_back(false);
		}
	}
	for (size_t i = 0; i < armor.size(); i++) {
		item_names.push_back(armor[i].name);
		icon_filenames.push_back("misc_graphics/interface/armor_icon.cpi");
		right_icon_filenames.push_back("misc_graphics/interface/x_icon.cpi");
		equipment_type.push_back(Equipment::ARMOR);
		descriptions.push_back("");
		disabled.push_back(false);
	}
	for (size_t i = 0; i < accessories.size(); i++) {
		item_names.push_back(accessories[i].name);
		icon_filenames.push_back("misc_graphics/interface/accessory_icon.cpi");
		right_icon_filenames.push_back("misc_graphics/interface/x_icon.cpi");
		equipment_type.push_back(Equipment::ACCESSORY);
		descriptions.push_back(Game_Specific_Globals::get_item_description(accessories[i].name));
		disabled.push_back(false);
	}

	int list_w = win_w * 2 / 3 - 16 - 6;
	int list_h = win_h-16-1-top_box_h;

	list = new W_Equipment_List(item_names, icon_filenames, equipment_type, descriptions, disabled, General::FONT_LIGHT, true);
	list->setX(green_x+win_w-list_w-7-6);
	list->setY(green_y+9+1+top_box_h);
	list->setWidth(list_w);
	list->setHeight(list_h);
	list->set_right_icon_filenames(right_icon_filenames);

	scrollbar = new W_Vertical_Scrollbar(list->get_scrollbar_tab_size());
	scrollbar->setX(list->getX()+list->getWidth()+1);
	scrollbar->setY(list->getY());
	scrollbar->setHeight(list->getHeight());

	item_names.clear();
	icon_filenames.clear();
	right_icon_filenames.clear();
	equipment_type.clear();
	descriptions.clear();
	disabled.clear();

	Battle_Attributes &attr = players[player]->get_battle_attributes();
	Equipment::Equipment &equip = attr.equipment;

	item_names.push_back(equip.weapon.name);
	item_names.push_back(equip.armor.name);
	item_names.push_back(equip.accessory.name);
	if (equip.weapon.attachments.size() > 0) {
		icon_filenames.push_back("misc_graphics/interface/attached_weapon_icon.cpi");
	}
	else {
		icon_filenames.push_back("misc_graphics/interface/weapon_icon.cpi");
	}
	icon_filenames.push_back("misc_graphics/interface/armor_icon.cpi");
	icon_filenames.push_back("misc_graphics/interface/accessory_icon.cpi");
	right_icon_filenames.push_back("misc_graphics/interface/x_icon.cpi");
	right_icon_filenames.push_back("misc_graphics/interface/x_icon.cpi");
	right_icon_filenames.push_back("misc_graphics/interface/x_icon.cpi");
	equipment_type.push_back(Equipment::WEAPON);
	equipment_type.push_back(Equipment::ARMOR);
	equipment_type.push_back(Equipment::ACCESSORY);
	if (equip.weapon.attachments.size() > 0) {
		descriptions.push_back(std::string(t(equip.weapon.attachments[0].name.c_str())) + ", " + std::string(t("QUANTITY")) + ": " + General::itos(equip.weapon.attachments[0].quantity));
	}
	else {
		descriptions.push_back("");
	}
	descriptions.push_back("");
	descriptions.push_back(Game_Specific_Globals::get_item_description(equip.accessory.name));
	disabled.push_back(false);
	disabled.push_back(false);
	disabled.push_back(false);

	int equip_y = green_y + 8;

	list2 = new W_Equipment_List(item_names, icon_filenames, equipment_type, descriptions, disabled, General::FONT_LIGHT, true);
	list2->set_draw_outline(false);
	list2->setX(list->getX());
	list2->setY(equip_y);
	list2->setWidth(list_w);
	list2->setHeight(top_box_h);
	list2->set_right_icon_filenames(right_icon_filenames);

	notification->list = list;
	notification->list2 = list2;
	notification->scrollbar = scrollbar;
	notification->loop = this;
	notification->players = players;
	notification->selected_player = main_menu_loop->get_selected_player();
	list->setNotifier(equip_notifier, notification);

	notification2->list = list;
	notification2->list2 = list2;
	notification2->loop = this;
	notification2->players = players;
	notification2->selected_player = main_menu_loop->get_selected_player();
	list2->setNotifier(equipped_equip_notifier, notification2);

	list->setSyncedWidget(scrollbar);
	scrollbar->setSyncedWidget(list);

	tgui::addWidget(list2);
	tgui::addWidget(list);
	tgui::addPostDrawWidget(list);
	tgui::addPostDrawWidget(list2);
	tgui::addWidget(scrollbar);

	set_widget_offsets();

	return true;
}

void Main_Menu_Equip_Loop::top()
{
}

bool Main_Menu_Equip_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
		if (!list_was_activated) {
			if (
				event->keyboard.keycode == ALLEGRO_KEY_ESCAPE
#if defined ALLEGRO_ANDROID
				|| event->keyboard.keycode == ALLEGRO_KEY_BUTTON_B
				|| event->keyboard.keycode == ALLEGRO_KEY_BACK
#endif
			) {
				main_menu_loop->set_esc_pressed(true);
				return true;
			}
		}
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
		if (!list_was_activated) {
			if (event->joystick.button == cfg.joy_ability[2]) {
				main_menu_loop->set_esc_pressed(true);
				return true;
			}
		}
	}
	else if (event->type == ALLEGRO_EVENT_MOUSE_AXES) {
		last_mouse_x = event->mouse.x;
		last_mouse_y = event->mouse.y;
		if (draw_info_box) {
			float dx = last_mouse_x - info_clicked_x;
			float dy = last_mouse_y - info_clicked_y;
			float dist = sqrt(dx*dx + dy*dy);
			if (dist > 5) {
				draw_info_box = false;
			}
		}
	}
	else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
		draw_info_box = false;
	}

	return false;
}

bool Main_Menu_Equip_Loop::logic()
{
	if (!in) {
		if (main_menu_loop->get_state() == Main_Menu_Loop::S_IN) {
			in = true;
			if (!main_menu_loop->get_main_menu_loop_button_pressed()) {
				tgui::setFocus(main_menu_loop->get_return_button());
			}
		}
	}

	set_widget_offsets();
	
	if (list->is_activated() || list2->is_activated()) {
		list_was_activated = true;
	}
	else {
		list_was_activated = false;
	}

	if (main_menu_loop->get_state() == Main_Menu_Loop::S_IN) {
		main_menu_loop->get_tgui_update_result();
	}

	swap_now = al_get_time();
	if (swapping && swap_now - swap_start_time > 0.5) {
		int n;
		if (swap_type == Equipment::WEAPON) {
			n = 0;
		}
		else if (swap_type == Equipment::ARMOR) {
			n = 1;
		}
		else {
			n = 2;
		}
		if (swap_update_icon) {
			list2->update_item(n, false, "", true, "misc_graphics/interface/attached_weapon_icon.cpi", false, "");
			std::vector<Player *> players = main_menu_loop->get_players();
			int sel = main_menu_loop->get_selected_player();
			Battle_Attributes &attr = players[sel]->get_battle_attributes();
			Equipment::Equipment &equip = attr.equipment;
			list2->update_description(0, std::string(t(swap_string.c_str())) + ", " + std::string(t("QUANTITY")) + ": " + General::itos(equip.weapon.attachments[0].quantity));
		}
		else {
			list2->update_item(n, true, swap_string, false, "", false, "");
		}
		swapping = false;
	}

	if (something_dropped) {
		dropped_string_pos.x += dropped_string_velocity.x;
		dropped_string_pos.y += dropped_string_velocity.y;
		int len = General::get_text_width(General::FONT_LIGHT, dropped_string) / 2;
		if (dropped_string_pos.x < -len || dropped_string_pos.y < -len || dropped_string_pos.x > cfg.screen_w+len || dropped_string_pos.y > cfg.screen_h+len) {
			something_dropped = false;
		}
		else {
			dropped_string_angle -= 0.1f;
		}
	}

	return false;
}

void Main_Menu_Equip_Loop::draw()
{
	float ox, oy;

	get_green_window_offsets(main_menu_loop->get_state(), TRANSITION_HORIZONTAL, main_menu_loop->get_transition_percent(), &ox, &oy);

	draw_green_window(main_menu_loop, ox, oy);

	int green_x = main_menu_loop->get_green_x();
	int green_y = main_menu_loop->get_green_y();

	ALLEGRO_COLOR dark = General::UI_GREEN;
	dark = Graphics::change_brightness(dark, 0.65f);

	al_draw_filled_rectangle(
		ox+green_x+8,
		oy+green_y+9,
		ox+green_x+8+top_box_w,
		oy+green_y+9+top_box_h,
		dark
	);

	
	std::vector<Player *> players = main_menu_loop->get_players();
	int sel = main_menu_loop->get_selected_player();

	std::string start_anim = players[sel]->get_animation_set()->get_sub_animation_name();

	// draw player
	Animation_Set *anim_set = players[sel]->get_animation_set();
	anim_set->set_sub_animation("attack");
	anim_set->get_current_animation()->set_frame(1);
	Wrap::Bitmap *player_bmp = anim_set->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();
	int anim_center_x = green_x + win_w / 6 * 2 / 3 + 8;
	int anim_center_y = green_y + 9 + top_box_h / 2;
	al_draw_bitmap(
		player_bmp->bitmap,
		ox+anim_center_x-al_get_bitmap_width(player_bmp->bitmap)/2,
		oy+anim_center_y-al_get_bitmap_height(player_bmp->bitmap)/2,
		0
	);
	anim_set->set_sub_animation(start_anim);
	// draw weapon
	anim_set = players[sel]->get_weapon_animation_set();
	if (anim_set) {
		anim_set->set_sub_animation("attack");
		anim_set->get_current_animation()->set_frame(1);
		player_bmp = anim_set->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();
		al_draw_bitmap(
			player_bmp->bitmap,
			ox+anim_center_x-al_get_bitmap_width(player_bmp->bitmap)/2,
			oy+anim_center_y-al_get_bitmap_height(player_bmp->bitmap)/2,
			0
		);
		anim_set->set_sub_animation(start_anim);
	}

	Battle_Attributes &attr = players[sel]->get_battle_attributes();

	int attr_x = green_x + 9;
	int attr_y = green_y + 10 + top_box_h;

	std::string labels[4] = {
		t("MAX_HP"),
		t("MAX_MP"),
		t("ATTACK"),
		t("DEFENSE")
	};

	int max_hp = attr.max_hp / cfg.difficulty_mult();
	int max_mp = attr.max_mp;
	int attack = attr.attack;
	int defense = attr.defense;
	Game_Specific_Globals::get_accessory_effects(attr.equipment.accessory.name, &max_hp, &max_mp, &attack, &defense);
	attack += attr.equipment.weapon.name == "" ? 0 : attr.equipment.weapon.attack;
	defense += attr.equipment.armor.name == "" ? 0 : attr.equipment.armor.defense;

	int amounts[4] = {
		max_hp,
		max_mp,
		attack,
		defense
	};

	float percents[4] = {
		(float)max_hp / 22,
		(float)max_mp / 10,
		(float)attack / 7,
		(float)defense / 5
	};

	ALLEGRO_COLOR colors[4] = {
		al_color_name("lime"),
		al_color_name("cyan"),
		al_color_name("yellow"),
		al_color_name("yellow")
	};
	
	ALLEGRO_COLOR gauge_color = Graphics::change_brightness(General::UI_GREEN, 1.35f);
	int th = General::get_font_line_height(General::FONT_LIGHT);

	for (int i = 0; i < 4; i++) {
		General::draw_text(labels[i], ox+attr_x, oy+attr_y, 0);
		General::draw_text(General::itos(amounts[i]), ox+list->getX()-1, oy+attr_y, ALLEGRO_ALIGN_RIGHT);
		Graphics::draw_gauge(ox+attr_x, oy+attr_y+th, (list->getX()-1)-attr_x, i == 0, percents[i], gauge_color, colors[i]);
		attr_y += 4 + th;
	}

	if (swapping) {
		double percent = (swap_now - swap_start_time) / 0.5;
		float swap_y = y_swap_start + ((y_swap_dest-y_swap_start) * percent);
		General::draw_text(t(swap_string.c_str()), al_color_name("yellow"), ox+list2->getX()+12, oy+swap_y, 0);
	}
}

void Main_Menu_Equip_Loop::post_draw()
{
	if (draw_info_box) {
		Graphics::draw_info_box(info_box_x, info_box_y, 100, 40, info_box_text);
	}

	if (something_dropped) {
		ALLEGRO_TRANSFORM t, backup;
		al_copy_transform(&backup, al_get_current_transform());
		al_identity_transform(&t);
		al_rotate_transform(&t, dropped_string_angle);
		al_translate_transform(&t, dropped_string_pos.x, dropped_string_pos.y);
		al_compose_transform(&t, &backup);
		al_use_transform(&t);
		General::draw_text(dropped_string, al_color_name("red"), 0, 0, ALLEGRO_ALIGN_CENTER);
		al_use_transform(&backup);
	}
}

Main_Menu_Equip_Loop::Main_Menu_Equip_Loop(Main_Menu_Loop *main_menu_loop) :
	something_dropped(false),
	main_menu_loop(main_menu_loop),
	swapping(false),
	draw_info_box(false),
	in(false),
	list_was_activated(false)
{
}

Main_Menu_Equip_Loop::~Main_Menu_Equip_Loop()
{
	list->remove();
	list2->remove();
	scrollbar->remove();
	delete list;
	delete list2;
	delete scrollbar;
	
	delete notification;
	delete notification2;
}

void Main_Menu_Equip_Loop::set_widget_offsets()
{
	float ox, oy;

	get_green_window_offsets(main_menu_loop->get_state(), TRANSITION_HORIZONTAL, main_menu_loop->get_transition_percent(), &ox, &oy);

	General::Point<float> p(ox, oy);

	list->setOffset(p);
	list2->setOffset(p);
	scrollbar->setOffset(p);
}

void Main_Menu_Equip_Loop::start_swap(int selected, float ystart)
{
	std::vector<Player *> players = main_menu_loop->get_players();
	int sel = main_menu_loop->get_selected_player();
	Battle_Attributes &attr = players[sel]->get_battle_attributes();
	Equipment::Equipment &equip = attr.equipment;

	swapping = true;
	y_swap_start = ystart;
	y_swap_dest = main_menu_loop->get_green_y() + 10;
	swap_type = list->get_type(selected);

	if (swap_type == Equipment::ARMOR) {
		swap_string = list->get_item_name(selected);
		swap_update_icon = false;
		y_swap_dest += General::get_font_line_height(General::FONT_LIGHT);
		orig_string = equip.armor.name;
		int this_idx = selected - Game_Specific_Globals::get_weapons().size();
		Equipment::Armor armor = Game_Specific_Globals::get_armor()[this_idx];
		if (orig_string != "") {
			list->insert_item(selected, "misc_graphics/interface/armor_icon.cpi", orig_string.c_str(), "misc_graphics/interface/x_icon.cpi", Equipment::ARMOR, "", false);
			Game_Specific_Globals::get_armor()[this_idx] = equip.armor;
		}
		else {
			Game_Specific_Globals::get_armor().erase(Game_Specific_Globals::get_armor().begin() + this_idx);
		}
		equip.armor = armor;
		list->remove_item(selected+(orig_string == "" ? 0 : 1));
	}
	else if (swap_type == Equipment::ACCESSORY) {
		swap_string = list->get_item_name(selected);
		swap_update_icon = false;
		y_swap_dest += General::get_font_line_height(General::FONT_LIGHT) * 2;
		orig_string = equip.accessory.name;
		int this_idx = selected - Game_Specific_Globals::get_weapons().size() - Game_Specific_Globals::get_armor().size();
		Equipment::Accessory accessory = Game_Specific_Globals::get_accessories()[this_idx];
		if (orig_string != "") {
			list->insert_item(selected, "misc_graphics/interface/accessory_icon.cpi", orig_string.c_str(), "misc_graphics/interface/x_icon.cpi", Equipment::ACCESSORY, Game_Specific_Globals::get_item_description(accessory.name), false);
			Game_Specific_Globals::get_accessories()[this_idx] = equip.accessory;
		}
		else {
			Game_Specific_Globals::get_accessories().erase(Game_Specific_Globals::get_accessories().begin() + this_idx);
		}
		equip.accessory = accessory;
		list2->update_description(2, Game_Specific_Globals::get_item_description(accessory.name));
		list->remove_item(selected+(orig_string == "" ? 0 : 1));
	}
	else {
		Equipment::Weapon weapon = Game_Specific_Globals::get_weapons()[selected];
		if (Game_Specific_Globals::weapon_is_attachment(weapon.name)) {
			swap_string = weapon.name;
			swap_update_icon = true;
			if (equip.weapon.attachments.size() == 0) {
				equip.weapon.attachments.push_back(Game_Specific_Globals::get_weapons()[selected]);
				Game_Specific_Globals::get_weapons().erase(Game_Specific_Globals::get_weapons().begin() + selected);
				list->remove_item(selected);
			}
			else {
				int take = 99 - equip.weapon.attachments[0].quantity;
				if (take >= Game_Specific_Globals::get_weapons()[selected].quantity) {
					take = Game_Specific_Globals::get_weapons()[selected].quantity;
					Game_Specific_Globals::get_weapons().erase(Game_Specific_Globals::get_weapons().begin() + selected);
					list->remove_item(selected);
				}
				equip.weapon.attachments[0].quantity += take;
			}
		}
		else {
			swap_string = list->get_item_name(selected);
			swap_update_icon = false;
			orig_string = equip.weapon.name;
			int this_idx = selected;
			if (orig_string != "") {
				for (int i = equip.weapon.attachments.size()-1; i >= 0; i--) {
					list->insert_item(selected, "misc_graphics/interface/weapon_icon.cpi", equip.weapon.attachments[i].name.c_str(), "misc_graphics/interface/x_icon.cpi", Equipment::WEAPON, std::string(t("QUANTITY")) + ": " + General::itos(equip.weapon.attachments[i].quantity), false);
					std::vector<Equipment::Weapon> &v = Game_Specific_Globals::get_weapons();
					v.insert(v.begin()+this_idx, equip.weapon.attachments[i]);
				}
				list->insert_item(selected, "misc_graphics/interface/weapon_icon.cpi", orig_string.c_str(), "misc_graphics/interface/x_icon.cpi", Equipment::WEAPON, "", false);
				Game_Specific_Globals::get_weapons()[this_idx] = equip.weapon;
			}
			else {
				Game_Specific_Globals::get_weapons().erase(Game_Specific_Globals::get_weapons().begin() + this_idx);
			}
			equip.weapon = weapon;
			players[sel]->load_weapon(weapon.name);
			list->remove_item(selected+(orig_string == "" ? 0 : 1));
		}
	}

	scrollbar->set_tab_size(list->get_scrollbar_tab_size());

	swap_start_time = al_get_time();
}

