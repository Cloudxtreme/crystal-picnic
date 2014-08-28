#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>
#include <tgui2.hpp>
#include <tgui2_widgets.hpp>
#include "sound-bass.h"
#include <cstdio>

static ALLEGRO_PATH *loadPath;
ALLEGRO_DISPLAY *disp;
ALLEGRO_TIMER *timer;

std::string get_filename()
{
	ALLEGRO_FILECHOOSER *diag;
	diag = al_create_native_file_dialog(
		al_path_cstr(loadPath, '/'),
		"Load MIDI",
		"*.MID",
		ALLEGRO_FILECHOOSER_FILE_MUST_EXIST
	);

	al_stop_timer(timer);
	al_show_native_file_dialog(disp, diag);
	al_start_timer(timer);

	if (al_get_native_file_dialog_count(diag) != 1)
		return "";

	const ALLEGRO_PATH *result = al_create_path(al_get_native_file_dialog_path(diag, 0));
	al_destroy_path(loadPath);
	loadPath = al_clone_path(result);
	al_set_path_filename(loadPath, NULL);

	std::string path = al_path_cstr(result, ALLEGRO_NATIVE_PATH_SEP);

	al_destroy_native_file_dialog(diag);

	return path;
}

int main(int argc, char **argv)
{
	al_init();
	al_init_font_addon();
	al_init_primitives_addon();
	al_install_mouse();

	loadPath = al_create_path("");

	al_set_new_display_flags(ALLEGRO_OPENGL);
	disp = al_create_display(640, 480);
	al_set_window_title(disp, "midiplayer");
	ALLEGRO_FONT *font = al_create_builtin_font();
	ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
	timer = al_create_timer(1.0/60.0);
	al_start_timer(timer);
	al_register_event_source(queue, al_get_display_event_source(disp));
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_timer_event_source(timer));

	tgui::init(disp);
	tgui::setFont(font);

	TGUI_Button *open = new TGUI_Button("Open", 10, 10, 100, 20);
	TGUI_Button *play = new TGUI_Button("Play", 10, 40, 100, 20);
	TGUI_Button *stop = new TGUI_Button("Stop", 10, 70, 100, 20);

	TGUI_Slider *s1 = new TGUI_Slider(300, 10, 100, TGUI_HORIZONTAL);
	TGUI_Slider *s2 = new TGUI_Slider(300, 40, 100, TGUI_HORIZONTAL);
	TGUI_Slider *s3 = new TGUI_Slider(300, 70, 100, TGUI_HORIZONTAL);
	TGUI_Slider *s4 = new TGUI_Slider(300, 100, 100, TGUI_HORIZONTAL);

	TGUI_Label *l1 = new TGUI_Label("InGain", al_color_name("white"), 150, 15, 0);
	TGUI_Label *l2 = new TGUI_Label("ReverbMix", al_color_name("white"), 150, 45, 0);
	TGUI_Label *l3 = new TGUI_Label("ReverbTime", al_color_name("white"), 150, 75, 0);
	TGUI_Label *l4 = new TGUI_Label("HighFreqRTRatio", al_color_name("white"), 150, 105, 0);

	TGUI_Checkbox *checkbox = new TGUI_Checkbox(10, 100, 10, 10, true);
	TGUI_Label *checkbox_label = new TGUI_Label("Reverb", al_color_name("white"), 30, 100, 0);
	TGUI_Checkbox *loop_checkbox = new TGUI_Checkbox(10, 130, 10, 10, true);
	TGUI_Label *loop_checkbox_label = new TGUI_Label("Loop (applies to next load)", al_color_name("white"), 30, 130, 0);

	tgui::setNewWidgetParent(0);
	tgui::addWidget(open);
	tgui::addWidget(play);
	tgui::addWidget(stop);
	tgui::addWidget(s1);
	tgui::addWidget(s2);
	tgui::addWidget(s3);
	tgui::addWidget(s4);
	tgui::addWidget(l1);
	tgui::addWidget(l2);
	tgui::addWidget(l3);
	tgui::addWidget(l4);
	tgui::addWidget(checkbox);
	tgui::addWidget(checkbox_label);
	tgui::addWidget(loop_checkbox);
	tgui::addWidget(loop_checkbox_label);
		
	tgui::draw();
	al_flip_display();

	bool playing = false;
	HSTREAM stream = 0;
	HFX fx = 0;
	BASS_DX8_REVERB reverb_params;
	float val1 = 0;
	float val2 = 0;
	float val3 = 0;
	float val4 = 0;
	bool reverb_on = true;
	bool looping = true;

	init_sound();

	while (true) {
		ALLEGRO_EVENT event;
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			exit(0);
		}
		tgui::handleEvent(&event);
		tgui::TGUIWidget *w = tgui::update();
		if (w == open) {
			std::string fn = get_filename();
			if (fn != "") {
				looping = loop_checkbox->getChecked();
				stream = load_midi(fn, looping);
				fx = add_reverb(stream);
				reverb_on = true;
				BASS_FXGetParameters(fx, (void *)&reverb_params);
				val1 = (reverb_params.fInGain + 96) / 96;
				s1->setPosition(val1);
				val2 = (reverb_params.fReverbMix + 96) / 96;
				s2->setPosition(val2);
				val3 = (reverb_params.fReverbTime * 1000 - 1) / 2999999;
				s3->setPosition(val3);
				val4 = (reverb_params.fHighFreqRTRatio * 1000 - 1) / 998;
				s4->setPosition(val4);
			}
		}
		else if (w == play && !playing && stream) {
			play_midi(stream, 1);
			if (looping) {
				playing = true;
			}
		}
		else if (w == stop && stream) {
			stop_midi(stream);
			playing = false;
		}
		if (playing) {
			float val;
			bool changed = false;
			val = s1->getPosition();
			if (val != val1) {
				val = 96 - (val * 96);
				reverb_params.fInGain = val;
				val1 = val;
				changed = true;
			}
			val = s2->getPosition();
			if (val != val2) {
				val = 96 - (val * 96);
				reverb_params.fReverbMix = val;
				val2 = val;
				changed = true;
			}
			val = s3->getPosition();
			if (val != val3) {
				val *= 2999999;
				val = val + 1;
				val /= 1000;
				reverb_params.fReverbTime = val;
				val3 = val;
				changed = true;
			}
			val = s4->getPosition();
			if (val != val4) {
				val *= 998;
				val = val + 1;
				val /= 1000;
				reverb_params.fHighFreqRTRatio = val;
				val4 = val;
				changed = true;
			}
			if (changed) {
				BASS_FXSetParameters(fx, (void *)&reverb_params);
			}
		}
		if (stream) {
			if (reverb_on) {
				if (checkbox->getChecked() == false) {
					remove_fx(stream, fx);
					reverb_on = false;
				}
			}
			else {
				if (checkbox->getChecked() == true) {
					fx = add_reverb(stream);
					reverb_on = true;
				}
			}
		}
		al_clear_to_color(al_color_name("black"));
		tgui::draw();
		al_flip_display();
	}
}

