#include <tgui2.hpp>

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

	int current = 0;
	int count = 0;
	std::vector<std::string> mode_names;

#ifdef ALLEGRO_RASPBERRYPI
	modes.push_back(std::pair<int, int>(1024, 576));
	mode_names.push_back("1024x576");
#endif

	int num_modes = al_get_num_display_modes();
	for (int i = 0; i < num_modes; i++) {
		ALLEGRO_DISPLAY_MODE mode;
		al_get_display_mode(i, &mode);
		float low = 3.0f / 2.0f * 0.95f;
		float high = 16.0f / 9.0f * 1.05f;
		float aspect = mode.width / (float)mode.height;
		if (aspect < low || aspect > high) {
			continue;
		}
		int insert = 0;
		bool found = false;
		for (size_t j = 0; j < modes.size(); j++) {
			if (modes[j].first == mode.width && modes[j].second == mode.height) {
				found = true;
				break;
			}
			if (modes[j].first > mode.width) {
				break;
			}
			if (modes[j].first == mode.width && modes[j].second > mode.height) {
				break;
			}
			insert++;
		}
		if (found) {
			continue;
		}
		modes.insert(modes.begin()+insert, std::pair<int, int>(mode.width, mode.height));
		mode_names.insert(mode_names.begin()+insert, General::itos(mode.width) + "x" + General::itos(mode.height));
		if (mode.width == cfg.save_screen_w && mode.height == cfg.save_screen_h) {
			current = count;
		}
		count++;
	}
	
	mode_list = new W_Scrolling_List(mode_names, std::vector<std::string>(), std::vector<std::string>(), std::vector<bool>(), General::FONT_LIGHT, true);
	mode_list->setWidth(100);
	mode_list->setHeight(64);
	mode_list->set_selected(current);
	checkbox = new W_Checkbox(cfg.fullscreen);

	save_button = new W_Button("", t("SAVE_AND_RESTART"));
	cancel_button = new W_Button("", t("CANCEL"));

	int maxw = mode_list->getWidth();
	maxw = MAX(maxw, checkbox->getWidth());
	maxw = MAX(maxw, save_button->getWidth());
	maxw = MAX(maxw, cancel_button->getWidth());

	mode_list->setX(cfg.screen_w/2-maxw/2);
	mode_list->setY(cfg.screen_h/2-mode_list->getHeight());
	checkbox->setX(cfg.screen_w/2-maxw/2);
	checkbox->setY(cfg.screen_h/2+5);
	save_button->setX(cfg.screen_w/2-maxw/2);
	save_button->setY(cfg.screen_h/2+General::get_font_line_height(General::FONT_LIGHT)*2);
	cancel_button->setX(cfg.screen_w/2-maxw/2);
	cancel_button->setY(cfg.screen_h/2+General::get_font_line_height(General::FONT_LIGHT)*3);

	tgui::addWidget(mode_list);
	tgui::addWidget(checkbox);
	tgui::addWidget(save_button);
	tgui::addWidget(cancel_button);

	tguiWidgetsSetColors(al_color_name("yellow"), al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f));

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
			cfg.loaded_w = modes[mode_list->get_selected()].first;
			cfg.loaded_h = modes[mode_list->get_selected()].second;
			cfg.loaded_fullscreen = checkbox->getChecked();
			cfg.save();

#ifdef ALLEGRO_WINDOWS
			STARTUPINFO sui;
			PROCESS_INFORMATION pi;

			memset(&sui, 0, sizeof(sui));
			sui.cb = sizeof(sui);

			CreateProcess(
				NULL,
				"CrystalPicnic.exe",
				NULL,
				NULL,
				FALSE,
				0,
				NULL,
				NULL,
				&sui,
				&pi
			);
			exit(0);
#else
			ALLEGRO_PATH *exe = al_get_standard_path(ALLEGRO_EXENAME_PATH);
			char path[5000];
#ifdef ALLEGRO_MACOSX
			for (int i = 0; i < 2; i++) {
				al_drop_path_tail(exe);
			}
			al_set_path_filename(exe, "");
			snprintf(path, 5000, "open \"%s\"", al_path_cstr(exe, '/'));
			if (path[strlen(path)-1] == '/') {
				path[strlen(path)-1] = 0;
			}
#else
			strncpy(path, al_path_cstr(exe, '/'), 5000);
#endif
			al_destroy_path(exe);
			if (fork() == 0) {
				system(path);
			}
			else {
				exit(0);
			}
#endif
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

	General::draw_text(t("CONFIG_FULLSCREEN"), checkbox->getX()+checkbox->getWidth()+2, checkbox->getY()-2, 0);

	tgui::draw();
}

Video_Config_Loop::Video_Config_Loop()
{
}

Video_Config_Loop::~Video_Config_Loop()
{
	mode_list->remove();
	delete mode_list;

	checkbox->remove();
	delete checkbox;

	save_button->remove();
	delete save_button;

	cancel_button->remove();
	delete cancel_button;

	tgui::pop(); // pushed beforehand
	tgui::unhide();
}

