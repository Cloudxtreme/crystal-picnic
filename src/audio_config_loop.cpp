#include "audio_config_loop.h"

bool Audio_Config_Loop::init()
{
	if (inited) {
		return true;
	}
	Loop::init();

	sfx_slider = new W_Slider(0, 0, 50);
	music_slider = new W_Slider(0, 0, 50);
	reverb_checkbox = new W_Checkbox(0, 0, cfg.reverb, t("REVERB"));
	save_button = new W_Audio_Settings_Button("misc_graphics/interface/fat_red_button.png", t("SAVE"));

	cancel_button = new W_Button("misc_graphics/interface/fat_red_button.png", t("CANCEL"));

	sfx_slider->setPosition(cfg.sfx_volume);
	music_slider->setPosition(cfg.music_volume);

	save_button->set_widgets(sfx_slider, music_slider, reverb_checkbox);

	int maxw = sfx_slider->getWidth() + 5 + General::get_text_width(General::FONT_LIGHT, t("SFX"));
	maxw = MAX(maxw, music_slider->getWidth() + 5 + General::get_text_width(General::FONT_LIGHT, t("MUSIC")));
	maxw = MAX(maxw, reverb_checkbox->getWidth());
	maxw = MAX(maxw, save_button->getWidth());
	maxw = MAX(maxw, cancel_button->getWidth());

	int one_h = General::get_font_line_height(General::FONT_LIGHT) + 4;
	int total_h = one_h * 5;

	sfx_slider->setX(cfg.screen_w/2+maxw/2-sfx_slider->getWidth());
	sfx_slider->setY(cfg.screen_h/2-total_h/2);

	music_slider->setX(cfg.screen_w/2+maxw/2-music_slider->getWidth());
	music_slider->setY(cfg.screen_h/2-total_h/2+one_h);

	reverb_checkbox->setX(cfg.screen_w/2-maxw/2);
	reverb_checkbox->setY(cfg.screen_h/2-total_h/2+one_h*2);

	int button_w = save_button->getWidth() + cancel_button->getWidth() + 5;

	save_button->setX(cfg.screen_w/2-button_w/2);
	save_button->setY(cfg.screen_h/2-total_h/2+one_h*4);

	cancel_button->setX(cfg.screen_w/2-button_w/2+save_button->getWidth()+5);
	cancel_button->setY(cfg.screen_h/2-total_h/2+one_h*4);

	tgui::addWidget(sfx_slider);
	tgui::addWidget(music_slider);
	tgui::addWidget(reverb_checkbox);
	tgui::addWidget(save_button);
	tgui::addWidget(cancel_button);

	tguiWidgetsSetColors(al_map_rgb(0xff, 0xff, 0x00), al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f));

	tgui::setFocus(cancel_button);

	return true;
}

void Audio_Config_Loop::top()
{
}

bool Audio_Config_Loop::handle_event(ALLEGRO_EVENT *event)
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

bool Audio_Config_Loop::logic()
{
	tgui::TGUIWidget *w = tgui::update();
	
	if (w == save_button) {
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

void Audio_Config_Loop::draw()
{
	al_clear_to_color(General::UI_GREEN);

	int maxw = General::get_text_width(General::FONT_LIGHT, t("SFX"));
	maxw = MAX(maxw, General::get_text_width(General::FONT_LIGHT, t("MUSIC")));

	General::draw_text(t("SFX"), sfx_slider->getX()-5-maxw, sfx_slider->getY()+2, 0);
	General::draw_text(t("MUSIC"), music_slider->getX()-5-maxw, music_slider->getY()+2, 0);

	tgui::draw();
}

Audio_Config_Loop::Audio_Config_Loop()
{
}

Audio_Config_Loop::~Audio_Config_Loop()
{
	sfx_slider->remove();
	delete sfx_slider;
	
	music_slider->remove();
	delete music_slider;

	reverb_checkbox->remove();
	delete reverb_checkbox;

	save_button->remove();
	delete save_button;

	cancel_button->remove();
	delete cancel_button;

	tgui::pop(); // pushed beforehand
	tgui::unhide();
}

