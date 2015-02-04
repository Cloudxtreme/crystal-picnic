#include <tgui2.hpp>

#include "crystalpicnic.h"
#include "video_config_loop.h"
#include "engine.h"
#include "widgets.h"

#if !defined ALLEGRO_WINDOWS
#include <unistd.h>
#endif

bool Video_Config_Loop::init()
{
	if (inited) {
		return true;
	}
	Loop::init();

	std::vector<std::string> mode_names;

	const int num_extra_modes = 6;

	int num_modes = al_get_num_display_modes();
	for (int i = 0; i < num_modes+num_extra_modes; i++) {
		ALLEGRO_DISPLAY_MODE mode;
		Mode my_mode;
		/* Here we insert some windowed only modes */
		if (i >= num_modes) {
			int mult = 0;
			if (i == num_modes) {
				mult = 1;
			}
			else if (i == num_modes+1) {
				mult = 2;
			}
			else if (i == num_modes+2) {
				mult = 3;
			}
			else if (i == num_modes+3) {
				mult = 4;
			}
			else if (i == num_modes+5) {
				mult = 5;
			}
			else if (i == num_modes+4) {
				mode.width = 1024;
				mode.height = 576;
			}
			my_mode.windowed_only = true;
			if (mult == 0) {
				my_mode.exact = false;
			}
			else {
				mode.width = mult * 285;
				mode.height = mult * 160;
				my_mode.exact = true;
			}
		}
		else {
			al_get_display_mode(i, &mode);
			my_mode.windowed_only = false;
			my_mode.exact = (mode.height % 160 == 0);
		}
		float low = 4.0f / 3.0f * 0.95f;
		float high = 16.0f / 9.0f * 1.05f;
		float aspect = mode.width / (float)mode.height;
		if (aspect < low || aspect > high) {
			continue;
		}
		int insert = 0;
		bool found = false;
		for (size_t j = 0; j < modes.size(); j++) {
			if (modes[j].width == mode.width && modes[j].height == mode.height) {
				found = true;
				break;
			}
			if (modes[j].width > mode.width) {
				break;
			}
			if (modes[j].width == mode.width && modes[j].height > mode.height) {
				break;
			}
			insert++;
		}
		if (found) {
			continue;
		}
		my_mode.width = mode.width;
		my_mode.height = mode.height;
		modes.insert(modes.begin()+insert, my_mode);
		mode_names.insert(mode_names.begin()+insert, General::itos(mode.width) + "x" + General::itos(mode.height) + (my_mode.exact ? "*" : "") + (my_mode.windowed_only ? " WINDOWED" : ""));
	}

	int current = 0;
	for (int i = 0; i < (int)modes.size(); i++) {
		if (modes[i].width == cfg.save_screen_w && modes[i].height == cfg.save_screen_h) {
			current = i;
			break;
		}
	}
	
	mode_list = new W_Scrolling_List(mode_names, std::vector<std::string>(), std::vector<std::string>(), std::vector<bool>(), General::FONT_LIGHT, true);
	mode_list->setWidth(100);
	mode_list->setHeight(64);
	mode_list->set_selected(current);

	fs_checkbox = new W_Checkbox(0, 0, cfg.fullscreen, t("CONFIG_FULLSCREEN"));
	linear_checkbox = new W_Checkbox(0, 0, cfg.linear_filtering, t("LINEAR_FILTERING"));

	save_button = new W_Button("misc_graphics/interface/fat_red_button.png", t("SAVE"));
	cancel_button = new W_Button("misc_graphics/interface/fat_red_button.png", t("CANCEL"));

	int maxw = mode_list->getWidth();
	maxw = MAX(maxw, fs_checkbox->getWidth());
	maxw = MAX(maxw, linear_checkbox->getWidth());
	maxw = MAX(maxw, save_button->getWidth());
	maxw = MAX(maxw, cancel_button->getWidth());

	mode_list->setX(cfg.screen_w/2-maxw/2);
	mode_list->setY(cfg.screen_h/2-mode_list->getHeight());

	scrollbar = new W_Vertical_Scrollbar(mode_list->get_scrollbar_tab_size());
	scrollbar->setX(mode_list->getX()+mode_list->getWidth()+1);
	scrollbar->setY(mode_list->getY());
	scrollbar->setHeight(mode_list->getHeight());
	mode_list->setSyncedWidget(scrollbar);
	scrollbar->setSyncedWidget(mode_list);
	mode_list->show_selected();

	fs_checkbox->setX(cfg.screen_w/2-maxw/2);
	fs_checkbox->setY(cfg.screen_h/2+5);

	linear_checkbox->setX(cfg.screen_w/2-maxw/2);
	linear_checkbox->setY(cfg.screen_h/2+(General::get_font_line_height(General::FONT_LIGHT)+4));

	int button_width = save_button->getWidth() + cancel_button->getWidth() + 5;

	save_button->setX(cfg.screen_w/2-button_width/2);
	save_button->setY(cfg.screen_h/2+(General::get_font_line_height(General::FONT_LIGHT)+4)*2+5);
	cancel_button->setX(cfg.screen_w/2-button_width/2+save_button->getWidth()+5);
	cancel_button->setY(cfg.screen_h/2+(General::get_font_line_height(General::FONT_LIGHT)+4)*2+5);

	tgui::addWidget(mode_list);
	tgui::addWidget(scrollbar);
	tgui::addWidget(fs_checkbox);
	tgui::addWidget(linear_checkbox);
	tgui::addWidget(save_button);
	tgui::addWidget(cancel_button);

	tguiWidgetsSetColors(al_map_rgb(0xff, 0xff, 0x00), al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f));

	tgui::setFocus(cancel_button);

	return true;
}

void Video_Config_Loop::top()
{
}

bool Video_Config_Loop::handle_event(ALLEGRO_EVENT *event)
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

bool Video_Config_Loop::logic()
{
	tgui::TGUIWidget *w = tgui::update();
	
	if (mode_list->is_activated()) {
		list_was_activated = true;
	}
	else {
		list_was_activated = false;
	}

	if (w == save_button) {
		if (modes.size() > 0) {
			cfg.loaded_w = modes[mode_list->get_selected()].width;
			cfg.loaded_h = modes[mode_list->get_selected()].height;
			cfg.loaded_fullscreen = modes[mode_list->get_selected()].windowed_only ? false : fs_checkbox->getChecked();
			cfg.loaded_linear_filtering = linear_checkbox->getChecked();

			engine->unblock_mini_loop();
			restart_game = true;
			return true;
		}
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

void Video_Config_Loop::draw()
{
	al_clear_to_color(General::UI_GREEN);

	tgui::draw();
}

Video_Config_Loop::Video_Config_Loop()
{
}

Video_Config_Loop::~Video_Config_Loop()
{
	mode_list->remove();
	delete mode_list;

	scrollbar->remove();
	delete scrollbar;

	fs_checkbox->remove();
	delete fs_checkbox;

	linear_checkbox->remove();
	delete linear_checkbox;

	save_button->remove();
	delete save_button;

	cancel_button->remove();
	delete cancel_button;

	tgui::pop(); // pushed beforehand
	tgui::unhide();
}

