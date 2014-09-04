#include <tgui2.hpp>

#include "settings_loop.h"
#include "engine.h"
#include "widgets.h"
#include "input_config_loop.h"
#include "video_config_loop.h"

bool Settings_Loop::init()
{
	if (inited) {
		return true;
	}
	Loop::init();

	video_button = new W_Button("", t("CONFIG_VIDEO"));
	keyboard_button = new W_Button("", t("CONFIG_KEYBOARD"));
	gamepad_button = new W_Button("", t("CONFIG_GAMEPAD"));
	return_button = new W_Button("", t("RETURN"));

	int maxw = video_button->getWidth();
	maxw = MAX(maxw, keyboard_button->getWidth());
	maxw = MAX(maxw, gamepad_button->getWidth());
	maxw = MAX(maxw, return_button->getWidth());

	video_button->setX(cfg.screen_w/2-maxw/2);
	video_button->setY(cfg.screen_h/2-General::get_font_line_height(General::FONT_LIGHT)*2);
	keyboard_button->setX(cfg.screen_w/2-maxw/2);
	keyboard_button->setY(cfg.screen_h/2-General::get_font_line_height(General::FONT_LIGHT));
	gamepad_button->setX(cfg.screen_w/2-maxw/2);
	gamepad_button->setY(cfg.screen_h/2);
	return_button->setX(cfg.screen_w/2-maxw/2);
	return_button->setY(cfg.screen_h/2+General::get_font_line_height(General::FONT_LIGHT));

	tgui::addWidget(video_button);
	tgui::addWidget(keyboard_button);
	tgui::addWidget(gamepad_button);
	tgui::addWidget(return_button);

	tgui::setFocus(return_button);

	return true;
}

void Settings_Loop::top()
{
}

bool Settings_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
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

	return false;
}

bool Settings_Loop::logic()
{
	tgui::TGUIWidget *w = tgui::update();

	if (w == video_button) {
		std::vector<Loop *> this_loop;
		this_loop.push_back(this);
		engine->fade_out(this_loop);
		Video_Config_Loop *l = new Video_Config_Loop();
		tgui::hide();
		tgui::push(); // popped in ~Video_Config_Loop()
		std::vector<Loop *> loops;
		l->init();
		loops.push_back(l);
		engine->fade_in(loops);
		engine->do_blocking_mini_loop(loops, NULL);
		engine->fade_in(this_loop);
	}
	else if (w == keyboard_button) {
		std::vector<Loop *> this_loop;
		this_loop.push_back(this);
		engine->fade_out(this_loop);
		Input_Config_Loop *l = new Input_Config_Loop(true);
		tgui::hide();
		tgui::push(); // popped in ~Input_Config_Loop()
		std::vector<Loop *> loops;
		l->init();
		loops.push_back(l);
		engine->fade_in(loops);
		engine->do_blocking_mini_loop(loops, NULL);
		engine->fade_in(this_loop);
	}
	else if (w == gamepad_button) {
		std::vector<Loop *> this_loop;
		this_loop.push_back(this);
		engine->fade_out(this_loop);
		Input_Config_Loop *l = new Input_Config_Loop(false);
		tgui::hide();
		tgui::push(); // popped in ~Input_Config_Loop()
		std::vector<Loop *> loops;
		l->init();
		loops.push_back(l);
		engine->fade_in(loops);
		engine->do_blocking_mini_loop(loops, NULL);
		engine->fade_in(this_loop);
	}
	else if (w == return_button) {
		std::vector<Loop *> loops;
		loops.push_back(this);
		engine->fade_out(loops);
		engine->unblock_mini_loop();
		return true;
	}

	return false;
}

void Settings_Loop::draw()
{
	al_clear_to_color(General::UI_GREEN);

	tgui::draw();
}

Settings_Loop::Settings_Loop()
{
}

Settings_Loop::~Settings_Loop()
{
	video_button->remove();
	delete video_button;
	keyboard_button->remove();
	delete keyboard_button;
	gamepad_button->remove();
	delete gamepad_button;
	return_button->remove();
	delete return_button;

	tgui::pop(); // pushed beforehand
	tgui::unhide();
}

