#include <tgui2.hpp>

#include "difficulty_loop.h"
#include "engine.h"
#include "widgets.h"
#include "config.h"

bool Difficulty_Loop::init()
{
	if (inited) {
		return true;
	}
	Loop::init();

	easy_button = new W_Translated_Button("EASY");
	normal_button = new W_Translated_Button("NORMAL");
	hard_button = new W_Translated_Button("HARD");

	int maxw = easy_button->getWidth();
	maxw = MAX(maxw, normal_button->getWidth());
	maxw = MAX(maxw, hard_button->getWidth());

	tgui::TGUIWidget *w[] = {
		easy_button,
		normal_button,
		hard_button
	};
	int nw = 3;

	for (int i = 0; i < nw; i++) {
		w[i]->setX(cfg.screen_w/2-maxw/2);
		w[i]->setY(cfg.screen_h/2-((General::get_font_line_height(General::FONT_LIGHT)+4)*nw)/2+(General::get_font_line_height(General::FONT_LIGHT)+4)*i+2);
		tgui::addWidget(w[i]);
	}

	tgui::setFocus(normal_button);

	return true;
}

void Difficulty_Loop::top()
{
}

bool Difficulty_Loop::handle_event(ALLEGRO_EVENT *event)
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

bool Difficulty_Loop::logic()
{
	tgui::TGUIWidget *w = tgui::update();

	if (w == easy_button) {
		cfg.difficulty = Configuration::EASY;
		std::vector<Loop *> loops;
		loops.push_back(this);
		engine->fade_out(loops);
		engine->unblock_mini_loop();
		cfg.cancelled = false;
		return true;
	}
	else if (w == normal_button) {
		cfg.difficulty = Configuration::NORMAL;
		std::vector<Loop *> loops;
		loops.push_back(this);
		engine->fade_out(loops);
		engine->unblock_mini_loop();
		cfg.cancelled = false;
		return true;
	}
	else if (w == hard_button) {
		cfg.difficulty = Configuration::HARD;
		std::vector<Loop *> loops;
		loops.push_back(this);
		engine->fade_out(loops);
		engine->unblock_mini_loop();
		cfg.cancelled = false;
		return true;
	}

	return false;
}

void Difficulty_Loop::draw()
{
	al_clear_to_color(General::UI_GREEN);

	tgui::draw();
}

Difficulty_Loop::Difficulty_Loop()
{
}

Difficulty_Loop::~Difficulty_Loop()
{
	easy_button->remove();
	delete easy_button;
	normal_button->remove();
	delete normal_button;
	hard_button->remove();
	delete hard_button;

	tgui::pop(); // pushed beforehand
	tgui::unhide();
}

