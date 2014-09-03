#include <tgui2.hpp>

#include "hqm.h"

#include "hqm_loop.h"

bool HQM_Loop::init()
{
	if (inited) {
		return true;
	}
	Loop::init();

	tgui::setNewWidgetParent(NULL);

	go_button = new W_Button("", t("START_RESUME"));
	pause_button = new W_Button("", t("STOP_PAUSE"));
	delete_button = new W_Button("", t("DELETE_HQM"));
	return_button = new W_Button("", t("RETURN"));

	max_w = go_button->getWidth();
	max_w = MAX(max_w, pause_button->getWidth());
	max_w = MAX(max_w, delete_button->getWidth());
	max_w = MAX(max_w, return_button->getWidth());
	int h = go_button->getHeight();

	go_button->setX(cfg.screen_w/2-max_w/2);
	pause_button->setX(cfg.screen_w/2-max_w/2);
	delete_button->setX(cfg.screen_w/2-max_w/2);
	return_button->setX(cfg.screen_w/2-max_w/2);

	go_button->setY(cfg.screen_h/2-h*2);
	pause_button->setY(cfg.screen_h/2-h);
	delete_button->setY(cfg.screen_h/2);
	return_button->setY(cfg.screen_h/2+h);

	tgui::addWidget(go_button);
	tgui::addWidget(pause_button);
	tgui::addWidget(delete_button);
	tgui::addWidget(return_button);

	tgui::setFocus(return_button);

	return true;
}

void HQM_Loop::top()
{
}

bool HQM_Loop::handle_event(ALLEGRO_EVENT *event)
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

bool HQM_Loop::logic()
{
	tgui::TGUIWidget *w = tgui::update();

	if (w) {
		if (w == go_button) {
			hqm_go();
		}
		else if (w == pause_button) {
			hqm_stop();
		}
		else if (w == delete_button) {
			hqm_delete();
		}
		else {
			std::vector<Loop *> loops;
			loops.push_back(this);
			engine->fade_out(loops);
			engine->unblock_mini_loop();
			return true;
		}
	}

	return false;
}

void HQM_Loop::draw()
{
	al_clear_to_color(General::UI_GREEN);
	
	std::string s = t("HQM_TITLE");
	int w = General::get_text_width(General::FONT_LIGHT, s);
	General::draw_text(s, cfg.screen_w/2-w/2, go_button->getY()-go_button->getHeight()*2, 0, General::FONT_LIGHT);

	float percent;
	int status = hqm_get_status(&percent);

	ALLEGRO_COLOR hilight = Graphics::change_brightness(General::UI_BLUE, 1.35f);
	Graphics::draw_gauge(go_button->getX(), go_button->getY()+go_button->getHeight()*5, max_w, true, percent, hilight, al_color_name("lime"));

	if (status == HQM_STATUS_PARTIAL) {
		int n = fmod(al_get_time(), 4.0);
		std::string s = "";
		for (int i = 0; i < n; i++) {
			s += ".";
		}
		General::draw_text(s, go_button->getX()+go_button->getWidth()+5, go_button->getY()+go_button->getHeight()*5-5, 0, General::FONT_HEAVY);
	}

	tgui::draw();
}

HQM_Loop::HQM_Loop()
{
}

HQM_Loop::~HQM_Loop()
{
	go_button->remove();
	delete go_button;
	pause_button->remove();
	delete pause_button;
	delete_button->remove();
	delete delete_button;
	return_button->remove();
	delete return_button;

	tgui::pop(); // pushed beforehand
	tgui::unhide();
}

