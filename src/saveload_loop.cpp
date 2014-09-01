#include <tgui2.hpp>

#include "saveload_loop.h"
#include "luainc.h"
#include "game_specific_globals.h"

bool SaveLoad_Loop::init()
{
	if (inited) {
		return true;
	}
	Loop::init();

	tgui::setNewWidgetParent(NULL);

	for (int i = 0; i < 3; i++) {
		std::string save_filename = General::get_save_filename(i);
		std::vector<std::string> players;
		double playtime = 0.0;
		std::string area_name = "-";
		FILE *f = fopen(save_filename.c_str(), "r");
		if (f) {
			char line[1000];
			while (true) {
				if (fgets(line, 1000, f) == NULL) {
					break;
				}
				if (!strncmp(line, "set_saved_players", 17)) {
					std::vector<const char *> v;
					v.push_back(strstr(line, "egbert"));
					v.push_back(strstr(line, "frogbert"));
					v.push_back(strstr(line, "bisou"));
					std::sort(v.begin(), v.end());
					if (v[0] == NULL) {
						v.erase(v.begin());
					}
					players.clear();
					for (size_t j = 0; j < v.size(); j++) {
						if (v[j][0] == 'e') { // egbert
							players.push_back("egbert");
						}
						else if (v[j][0] == 'f') { // frogbert
							players.push_back("frogbert");
						}
						else { // bisou
							players.push_back("bisou");
						}
					}
				}
				else if (!strncmp(line, "set_elapsed_time", 16)) {
					sscanf(line, "set_elapsed_time(%lf)", &playtime);
				}
				else if (!strncmp(line, "set_area_name", 13)) {
					char *p = line;
					char buf[1000];
					int n = 0;
					buf[0] = 0;
					while (*p && *p != '"') {
						p++;
					}
					if (*p == '"') {
						p++;
						while (*p != 0 && *p != '"' && n < 999) {
							buf[n++] = *p;
							p++;
						}
					}
					buf[n] = 0;
					area_name = buf;
				}
			}
			fclose(f);
		}
		if (strstr(area_name.c_str(), "map:")) {
			area_name = "MAP";
		}
		slots[i] = new W_SaveLoad_Button(0, 20+i*40, cfg.screen_w, 40, players, playtime, area_name == "-" ? "-" : t(area_name.c_str()));
		tgui::addWidget(slots[i]);
	}
	
	return_button = new W_Button("", t("RETURN"));
	return_button->setX(cfg.screen_w/2-return_button->getWidth()/2);
	return_button->setY(150-return_button->getHeight()/2);
	tgui::addWidget(return_button);

	tgui::setFocus(slots[0]);

	return true;
}

void SaveLoad_Loop::top()
{
}

bool SaveLoad_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
		if (
			event->keyboard.keycode == ALLEGRO_KEY_ESCAPE
#if defined FIRETV || (defined ALLEGRO_ANDROID && !defined OUYA)
#ifdef FIRETV
			|| event->keyboard.keycode == ALLEGRO_KEY_BUTTON_B
#endif
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

bool SaveLoad_Loop::logic()
{
	tgui::TGUIWidget *w = tgui::update();

	if (w) {
		for (int i = 0; i < 3; i++) {
			if (w == slots[i]) {
				if (saving) {
					engine->save_game(i);
					std::vector<std::string> v;
					v.push_back(t("GAME_SAVED"));
					std::vector<Loop *> loops;
					loops.push_back(this);
					engine->notify(v, &loops);
				}
				else {
					Game_Specific_Globals::elapsed_time = 0;
					engine->load_game(i);
				}
				engine->set_continued_or_saved(true);
			}
		}
		std::vector<Loop *> loops;
		loops.push_back(this);
		engine->fade_out(loops);
		engine->unblock_mini_loop();
		return true;
	}

	return false;
}

void SaveLoad_Loop::draw()
{
	al_clear_to_color(General::UI_GREEN);

	if (saving) {
		General::draw_text(t("CHOOSE_A_SAVE_SLOT"), cfg.screen_w/2, 10-General::get_font_line_height(General::FONT_LIGHT)/2, ALLEGRO_ALIGN_CENTER);
	}
	else {
		General::draw_text(t("CHOOSE_A_GAME_TO_LOAD"), cfg.screen_w/2, 10-General::get_font_line_height(General::FONT_LIGHT)/2, ALLEGRO_ALIGN_CENTER);
	}

	tgui::draw();
}

SaveLoad_Loop::SaveLoad_Loop(bool saving) :
	saving(saving)
{
}

SaveLoad_Loop::~SaveLoad_Loop()
{
	for (int i = 0; i < 3; i++) {
		slots[i]->remove();
		delete slots[i];
	}
	return_button->remove();
	delete return_button;

	tgui::pop(); // pushed beforehand
	tgui::unhide();
}

