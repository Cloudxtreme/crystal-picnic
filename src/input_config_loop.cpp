#include <tgui2.hpp>

#include "input_config_loop.h"
#include "engine.h"
#include "widgets.h"

bool dont_process_dpad_events = false;

bool Input_Config_Loop::init()
{
	if (inited) {
		return true;
	}
	Loop::init();

	getting_key = -1;
	getting_button = -1;

	if (keyboard) {
		labels.push_back(t("CFG_LEFT"));
		labels.push_back(t("CFG_RIGHT"));
		labels.push_back(t("CFG_UP"));
		labels.push_back(t("CFG_DOWN"));
		labels.push_back(t("CFG_MENU"));
		labels.push_back(t("CFG_RUN"));
		labels.push_back(t("CFG_SWITCH"));
		labels.push_back(t("CFG_USE"));
		labels.push_back(t("CFG_ATTACK_SPECIAL"));
		labels.push_back(t("CFG_CUSTOM"));
		labels.push_back(t("CFG_JUMP_ACTIVATE"));

		keys[0] = cfg.key_left;
		keys[1] = cfg.key_right;
		keys[2] = cfg.key_up;
		keys[3] = cfg.key_down;
		keys[4] = cfg.key_menu;
		keys[5] = cfg.key_run;
		keys[6] = cfg.key_switch;
		keys[7] = cfg.key_ability[0];
		keys[8] = cfg.key_ability[1];
		keys[9] = cfg.key_ability[2];
		keys[10] = cfg.key_ability[3];
	}
	else {
		labels.push_back(t("CFG_USE"));
		labels.push_back(t("CFG_ATTACK_SPECIAL"));
		labels.push_back(t("CFG_CUSTOM"));
		labels.push_back(t("CFG_JUMP_ACTIVATE"));
		labels.push_back(t("CFG_MENU"));
		labels.push_back(t("CFG_SWITCH"));
		labels.push_back(t("CFG_ARRANGE_UP"));
		labels.push_back(t("CFG_ARRANGE_DOWN"));
		labels.push_back(t("CFG_DPAD_L"));
		labels.push_back(t("CFG_DPAD_R"));
		labels.push_back(t("CFG_DPAD_U"));
		labels.push_back(t("CFG_DPAD_D"));
	
		gamepad_buttons[0] = cfg.joy_ability[0];
		gamepad_buttons[1] = cfg.joy_ability[1];
		gamepad_buttons[2] = cfg.joy_ability[2];
		gamepad_buttons[3] = cfg.joy_ability[3];
		gamepad_buttons[4] = cfg.joy_menu;
		gamepad_buttons[5] = cfg.joy_switch;
		gamepad_buttons[6] = cfg.joy_arrange_up;
		gamepad_buttons[7] = cfg.joy_arrange_down;
		gamepad_buttons[8] = cfg.joy_dpad_l;
		gamepad_buttons[9] = cfg.joy_dpad_r;
		gamepad_buttons[10] = cfg.joy_dpad_u;
		gamepad_buttons[11] = cfg.joy_dpad_d;
	}
	
	tgui::setNewWidgetParent(NULL);

	if (keyboard) {
		for (int i = 0; i < 11; i++) {
			buttons[i] = new W_Button("", t("KEY"));
			buttons[i]->setX(90);
			buttons[i]->setY(5+12*i);
			buttons[i]->setHeight(buttons[i]->getHeight()-5);
			buttons[i]->set_text_yoffset(-1);
			tgui::addWidget(buttons[i]);
		}
	}
	else {
		for (int i = 0; i < 12; i++) {
			buttons[i] = new W_Button("", t("BUTTON"));
			buttons[i]->setX(90);
			buttons[i]->setY(5+12*i);
			buttons[i]->setHeight(buttons[i]->getHeight()-5);
			buttons[i]->set_text_yoffset(-1);
			tgui::addWidget(buttons[i]);
		}
	}

	defaults_button = new W_Button("misc_graphics/interface/fat_red_button.cpi", t("DEFAULTS"));
	defaults_button->setX(cfg.screen_w-defaults_button->getWidth()-5);
	defaults_button->setY(cfg.screen_h-defaults_button->getHeight()-10);
	tgui::addWidget(defaults_button);

	done_button = new W_Button("misc_graphics/interface/fat_red_button.cpi", t("SAVE"));
	done_button->setX(cfg.screen_w-done_button->getWidth()-5);
	done_button->setY(cfg.screen_h-defaults_button->getHeight()-done_button->getHeight()-15);
	tgui::addWidget(done_button);
	
	tgui::setFocus(done_button);

	return true;
}

void Input_Config_Loop::top()
{
}

bool Input_Config_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (getting_key >= 0) {
		if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			getting_key = -1;
		}
		else if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
			if (event->keyboard.keycode == ALLEGRO_KEY_ESCAPE || event->keyboard.keycode == ALLEGRO_KEY_ENTER || event->keyboard.keycode == ALLEGRO_KEY_PGUP || event->keyboard.keycode == ALLEGRO_KEY_PGDN) {
				// Do nothing (stop further actions below)
			}
			else if (event->keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
				keys[getting_key] = 0;
			}
			else {
				keys[getting_key] = event->keyboard.keycode;
			}
			getting_key = -1;
		}
	}
	else if (getting_button >= 0) {
		if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
			getting_button = -1;
			dont_process_dpad_events = false;
		}
		else if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
			if (event->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				// Do nothing (stop further actions below)
			}
			else if (event->keyboard.keycode == ALLEGRO_KEY_BACKSPACE) {
				gamepad_buttons[getting_button] = -1;
			}
			getting_button = -1;
			dont_process_dpad_events = false;
		}
		else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
			gamepad_buttons[getting_button] = event->joystick.button;
			getting_button = -1;
			dont_process_dpad_events = false;
		}
	}
	else if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
		if (
			event->keyboard.keycode == ALLEGRO_KEY_ESCAPE
#if defined ALLEGRO_ANDROID
			|| event->keyboard.keycode == ALLEGRO_KEY_BUTTON_B
			|| event->keyboard.keycode == ALLEGRO_KEY_BACK
#endif
		) {
			std::vector<Loop *> loops;
			loops.push_back(this);
			engine->fade_out(loops);
			engine->unblock_mini_loop();
			return true;
		}
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
		if (event->joystick.button == cfg.joy_ability[2]) {
			std::vector<Loop *> loops;
			loops.push_back(this);
			engine->fade_out(loops);
			engine->unblock_mini_loop();
			return true;
		}
	}

	return false;
}

bool Input_Config_Loop::logic()
{
	if (getting_key < 0 && getting_button < 0) {
		engine->set_send_tgui_events(true);
	}

	tgui::TGUIWidget *w = tgui::update();

	if (keyboard) {
		for (int i = 0; i < 11; i++) {
			if (w == buttons[i]) {
				getting_key = i;
				engine->set_send_tgui_events(false);
			}
		}
	}
	else {
		for (int i = 0; i < 12; i++) {
			if (w == buttons[i]) {
				getting_button = i;
				engine->set_send_tgui_events(false);
				dont_process_dpad_events = true;
			}
		}
	}

	if (w == done_button) {
		std::vector<Loop *> loops;
		loops.push_back(this);
		engine->fade_out(loops);
		engine->unblock_mini_loop();
		return true;
	}
	else if (w == defaults_button) {
		if (keyboard) {
			int tmp[11];
			tmp[0] = cfg.key_left;
			tmp[1] = cfg.key_right;
			tmp[2] = cfg.key_up;
			tmp[3] = cfg.key_down;
			tmp[4] = cfg.key_menu;
			tmp[5] = cfg.key_run;
			tmp[6] = cfg.key_switch;
			tmp[7] = cfg.key_ability[0];
			tmp[8] = cfg.key_ability[1];
			tmp[9] = cfg.key_ability[2];
			tmp[10] = cfg.key_ability[3];
			cfg.reset_keyboard_controls();
			keys[0] = cfg.key_left;
			keys[1] = cfg.key_right;
			keys[2] = cfg.key_up;
			keys[3] = cfg.key_down;
			keys[4] = cfg.key_menu;
			keys[5] = cfg.key_run;
			keys[6] = cfg.key_switch;
			keys[7] = cfg.key_ability[0];
			keys[8] = cfg.key_ability[1];
			keys[9] = cfg.key_ability[2];
			keys[10] = cfg.key_ability[3];
			cfg.key_left = tmp[0];
			cfg.key_right = tmp[1];
			cfg.key_up = tmp[2];
			cfg.key_down = tmp[3];
			cfg.key_menu = tmp[4];
			cfg.key_run = tmp[5];
			cfg.key_switch = tmp[6];
			cfg.key_ability[0] = tmp[7];
			cfg.key_ability[1] = tmp[8];
			cfg.key_ability[2] = tmp[9];
			cfg.key_ability[3] = tmp[10];
		}
		else {
			int tmp[12];
			tmp[0] = cfg.joy_ability[0];
			tmp[1] = cfg.joy_ability[1];
			tmp[2] = cfg.joy_ability[2];
			tmp[3] = cfg.joy_ability[3];
			tmp[4] = cfg.joy_menu;
			tmp[5] = cfg.joy_switch;
			tmp[6] = cfg.joy_arrange_up;
			tmp[7] = cfg.joy_arrange_down;
			tmp[8] = cfg.joy_dpad_l;
			tmp[9] = cfg.joy_dpad_r;
			tmp[10] = cfg.joy_dpad_u;
			tmp[11] = cfg.joy_dpad_d;
			cfg.reset_gamepad_controls();
			gamepad_buttons[0] = cfg.joy_ability[0];
			gamepad_buttons[1] = cfg.joy_ability[1];
			gamepad_buttons[2] = cfg.joy_ability[2];
			gamepad_buttons[3] = cfg.joy_ability[3];
			gamepad_buttons[4] = cfg.joy_menu;
			gamepad_buttons[5] = cfg.joy_switch;
			gamepad_buttons[6] = cfg.joy_arrange_up;
			gamepad_buttons[7] = cfg.joy_arrange_down;
			gamepad_buttons[8] = cfg.joy_dpad_l;
			gamepad_buttons[9] = cfg.joy_dpad_r;
			gamepad_buttons[10] = cfg.joy_dpad_u;
			gamepad_buttons[11] = cfg.joy_dpad_d;
			cfg.joy_ability[0] = tmp[0];
			cfg.joy_ability[1] = tmp[1];
			cfg.joy_ability[2] = tmp[2];
			cfg.joy_ability[3] = tmp[3];
			cfg.joy_menu = tmp[4];
			cfg.joy_switch = tmp[5];
			cfg.joy_arrange_up = tmp[6];
			cfg.joy_arrange_down = tmp[7];
			cfg.joy_dpad_l = tmp[8];
			cfg.joy_dpad_r = tmp[9];
			cfg.joy_dpad_u = tmp[10];
			cfg.joy_dpad_d = tmp[11];
		}
	}

	return false;
}

void Input_Config_Loop::draw()
{
	int num = keyboard ? 11 : 12;

	al_clear_to_color(General::UI_GREEN);

	for (int i = 0; i < num; i++) {
		General::draw_text(labels[i], buttons[i]->getX()-General::get_text_width(General::FONT_LIGHT, labels[i])-5, buttons[i]->getY()-1, 0);
		int val = keyboard ? keys[i] : gamepad_buttons[i];
		General::draw_text(keyboard ? al_keycode_to_name(val) : General::itos(val), buttons[i]->getX()+buttons[i]->getWidth()+5, buttons[i]->getY()-1, 0);
	}

	tgui::draw();
	
	if (getting_key >= 0 || getting_button >= 0) {
		int th = General::get_font_line_height(General::FONT_LIGHT);
		int w = 200;
		int h = 5*th+4;

		al_draw_filled_rectangle(0, 0, cfg.screen_w, cfg.screen_h, al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.5f));

		int x = cfg.screen_w/2-w/2;
		int y = cfg.screen_h/2-h/2;

		ALLEGRO_COLOR main_color = al_color_name("lightgrey");
		ALLEGRO_COLOR dark_color = Graphics::change_brightness(main_color, 0.5f);
		al_draw_filled_rectangle(
			x, y,
			w+x, h+y,
			main_color
		);
		al_draw_rectangle(
			0.5f+x, 0.5f+y,
			w-0.5f+x, h-0.5f+y,
			dark_color,
			1
		);
		General::draw_text(keyboard ? t("PRESS_A_KEY") : t("PRESS_A_BUTTON"), al_color_name("black"), cfg.screen_w/2, y+th, ALLEGRO_ALIGN_CENTER);
		General::draw_text(t("ESC_CANCEL"), al_color_name("black"), cfg.screen_w/2, y+th*2, ALLEGRO_ALIGN_CENTER);
		General::draw_text(t("BACKSPACE_CLEAR"), al_color_name("black"), cfg.screen_w/2, y+th*3, ALLEGRO_ALIGN_CENTER);
	}
}

Input_Config_Loop::Input_Config_Loop(bool keyboard) :
	keyboard(keyboard)
{
}

Input_Config_Loop::~Input_Config_Loop()
{
	int num = keyboard ? 11 : 12;

	for (int i = 0; i < num; i++) {
		buttons[i]->remove();
		delete buttons[i];
	}

	defaults_button->remove();
	delete defaults_button;

	done_button->remove();
	delete done_button;

	tgui::pop(); // pushed beforehand
	tgui::unhide();

	dont_process_dpad_events = false;

	if (keyboard) {
		cfg.key_left = keys[0];
		cfg.key_right = keys[1];
		cfg.key_up = keys[2];
		cfg.key_down = keys[3];
		cfg.key_menu = keys[4];
		cfg.key_run = keys[5];
		cfg.key_switch = keys[6];
		cfg.key_ability[0] = keys[7];
		cfg.key_ability[1] = keys[8];
		cfg.key_ability[2] = keys[9];
		cfg.key_ability[3] = keys[10];
	}
	else {
		cfg.joy_ability[0] = gamepad_buttons[0];
		cfg.joy_ability[1] = gamepad_buttons[1];
		cfg.joy_ability[2] = gamepad_buttons[2];
		cfg.joy_ability[3] = gamepad_buttons[3];
		cfg.joy_menu = gamepad_buttons[4];
		cfg.joy_switch = gamepad_buttons[5];
		cfg.joy_arrange_up = gamepad_buttons[6];
		cfg.joy_arrange_down = gamepad_buttons[7];
		cfg.joy_dpad_l = gamepad_buttons[8];
		cfg.joy_dpad_r = gamepad_buttons[9];
		cfg.joy_dpad_u = gamepad_buttons[10];
		cfg.joy_dpad_d = gamepad_buttons[11];
	}
}

