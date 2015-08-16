#include <tgui2.hpp>

#include "language_config_loop.h"
#include "engine.h"
#include "widgets.h"

#if !defined ALLEGRO_WINDOWS
#include <unistd.h>
#endif

bool Language_Config_Loop::init()
{
	if (inited) {
		return true;
	}
	Loop::init();

	std::vector<std::string> language_names;

	untranslated_languages.push_back("English");
	untranslated_languages.push_back("French");
	untranslated_languages.push_back("German");

	language_names.push_back(t("ENGLISH"));
	language_names.push_back(t("FRENCH"));
	language_names.push_back(t("GERMAN"));

	int current = 0;

	for (size_t i = 0; i < language_names.size(); i++) {
		if (untranslated_languages[i] == cfg.language) {
			current = i;
			break;
		}
	}

	language_list = new W_Scrolling_List(language_names, std::vector<std::string>(), std::vector<std::string>(), std::vector<bool>(), General::FONT_LIGHT, true);
	language_list->setWidth(100);
	language_list->setHeight(64);
	language_list->set_selected(current);

	save_button = new W_Button("misc_graphics/interface/fat_red_button.png", t("SAVE"));

	cancel_button = new W_Button("misc_graphics/interface/fat_red_button.png", t("CANCEL"));

	int maxw = language_list->getWidth();
	maxw = MAX(maxw, save_button->getWidth());
	maxw = MAX(maxw, cancel_button->getWidth());

	language_list->setX(cfg.screen_w/2-maxw/2);
	language_list->setY(cfg.screen_h/2-language_list->getHeight());

	scrollbar = new W_Vertical_Scrollbar(language_list->get_scrollbar_tab_size());
	scrollbar->setX(language_list->getX()+language_list->getWidth()+1);
	scrollbar->setY(language_list->getY());
	scrollbar->setHeight(language_list->getHeight());
	language_list->setSyncedWidget(scrollbar);
	scrollbar->setSyncedWidget(language_list);
	language_list->show_selected();

	int button_w = save_button->getWidth() + cancel_button->getWidth() + 5;

	save_button->setX(cfg.screen_w/2-button_w/2);
	save_button->setY(cfg.screen_h/2+General::get_font_line_height(General::FONT_LIGHT));

	cancel_button->setX(cfg.screen_w/2-button_w/2+save_button->getWidth() + 5);
	cancel_button->setY(cfg.screen_h/2+General::get_font_line_height(General::FONT_LIGHT));

	tgui::addWidget(language_list);
	tgui::addWidget(scrollbar);
	tgui::addWidget(save_button);
	tgui::addWidget(cancel_button);

	tguiWidgetsSetColors(al_map_rgb(0xff, 0xff, 0x00), al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f));

	tgui::setFocus(save_button);
	tgui::setFocus(cancel_button);

	return true;
}

void Language_Config_Loop::top()
{
}

bool Language_Config_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
		if (
			event->keyboard.keycode == ALLEGRO_KEY_ESCAPE
#if defined ALLEGRO_ANDROID
			|| event->keyboard.keycode == ALLEGRO_KEY_BUTTON_B
			|| event->keyboard.keycode == ALLEGRO_KEY_BACK
#endif
		) {
			if (!list_was_activated) {
				std::vector<Loop *> loops;
				loops.push_back(this);
				engine->fade_out(loops);
				engine->unblock_mini_loop();
				return true;
			}
		}
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
		if (event->joystick.button == cfg.joy_ability[2]) {
			if (!list_was_activated) {
				std::vector<Loop *> loops;
				loops.push_back(this);
				engine->fade_out(loops);
				engine->unblock_mini_loop();
				return true;
			}
		}
	}

	return false;
}

bool Language_Config_Loop::logic()
{
	tgui::TGUIWidget *w = tgui::update();

	if (language_list->is_activated()) {
		list_was_activated = true;
	}
	else {
		list_was_activated = false;
	}

	if (w == save_button) {
		cfg.language = untranslated_languages[language_list->get_selected()];
		engine->load_translation();
		std::vector<Loop *> loops;
		loops.push_back(this);
		engine->fade_out(loops);
		engine->unblock_mini_loop();
		return true;
	}
	else if (w == cancel_button) {
		std::vector<Loop *> loops;
		loops.push_back(this);
		engine->fade_out(loops);
		engine->unblock_mini_loop();
		return true;
	}

	return false;
}

void Language_Config_Loop::draw()
{
	al_clear_to_color(General::UI_GREEN);

	tgui::draw();
}

Language_Config_Loop::Language_Config_Loop()
{
}

Language_Config_Loop::~Language_Config_Loop()
{
	language_list->remove();
	delete language_list;

	scrollbar->remove();
	delete scrollbar;

	save_button->remove();
	delete save_button;

	cancel_button->remove();
	delete cancel_button;

	tgui::pop(); // pushed beforehand
	tgui::unhide();
}

