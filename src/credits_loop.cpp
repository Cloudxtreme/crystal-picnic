#include <tgui2.hpp>

#include "credits_loop.h"
#include "engine.h"

bool Credits_Loop::init()
{
	engine->clear_touches();

	if (inited) {
		return true;
	}
	Loop::init();

	bitmaps.push_back(Wrap::load_bitmap("misc_graphics/credits/1.cpi"));
	bitmaps.push_back(Wrap::load_bitmap("misc_graphics/credits/2.cpi"));
	bitmaps.push_back(Wrap::load_bitmap("misc_graphics/credits/3.cpi"));
	bitmaps.push_back(Wrap::load_bitmap("misc_graphics/credits/4.cpi"));
	bitmaps.push_back(Wrap::load_bitmap("misc_graphics/credits/indiegogo.cpi"));
	bitmaps.push_back(Wrap::load_bitmap("misc_graphics/credits/nooskewl.cpi"));

	std::vector<std::string> v1;
	v1.push_back(t("CREDITS_CONCEPT"));
	v1.push_back("Tony Huisman");
	v1.push_back("Trent Gamblin");
	v1.push_back("");
	v1.push_back("");
	v1.push_back(t("CREDITS_STORY"));
	v1.push_back("John Bardinelli"); 
	strings.push_back(v1);
	v1.clear();
	v1.push_back(t("CREDITS_GRAPHICS"));
	v1.push_back("Tony Huisman");
	strings.push_back(v1);
	v1.clear();
	v1.push_back(t("CREDITS_AUDIO"));
	v1.push_back("Zoe Coleman");
	strings.push_back(v1);
	v1.clear();
	v1.push_back(t("CREDITS_CODE"));
	v1.push_back("Trent Gamblin");
	v1.push_back("");
	v1.push_back("");
	v1.push_back(t("CREDITS_ADDITIONAL_LEVEL_DESIGN"));
	v1.push_back("Bradley Stapleton");
	v1.push_back("");
	v1.push_back("");
	v1.push_back(t("CREDITS_TRANSLATIONS"));
	v1.push_back("Thomas Faust");
	strings.push_back(v1);
	v1.clear();
	v1.push_back("Richard Pett");
	v1.push_back("Aaron Bolyard");
	v1.push_back("Josh Larouche");
	v1.push_back("cdoty");
	v1.push_back("Carl Olsson");
	v1.push_back("William Browne");
	v1.push_back("");
	strings.push_back(v1);
	v1.clear();
	v1.push_back("\xc2\xa9 2014 Nooskewl");
	strings.push_back(v1);

	total_offset = cfg.screen_h * 1.5f;

	for (size_t i = 0; i < bitmaps.size(); i++) {
		if (bitmaps[i] == NULL) continue;
		total_offset += al_get_bitmap_height(bitmaps[i]->bitmap);
	}

	for (size_t i = 0; i < strings.size(); i++) {
		total_offset += 50 * 2;
		for (size_t j = 0; j < strings[i].size(); j++) {
			total_offset += General::get_font_line_height(General::FONT_LIGHT);
			printf("%s\n", strings[i][j].c_str());
		}
	}

	offset = 0;

	return true;
}

void Credits_Loop::top()
{
}

bool Credits_Loop::handle_event(ALLEGRO_EVENT *event)
{
	return false;
}

bool Credits_Loop::logic()
{
	engine->set_touch_input_type(TOUCHINPUT_GUI);

	offset += 0.25f;

	if (offset >= total_offset) {
		engine->unblock_mini_loop();
		return true;
	}

	return false;
}

void Credits_Loop::draw()
{
	al_clear_to_color(al_color_name("black"));
	float y = -offset;

	y += cfg.screen_h;

	for (int i = 0; i < 6; i++) {
		y += draw_bitmap(i, y);
		y += draw_strings(i, y);
	}
}

int Credits_Loop::draw_bitmap(int index, int y)
{
	if (bitmaps[index] == NULL) return 0;
	al_draw_bitmap(
		bitmaps[index]->bitmap,
		cfg.screen_w/2 - al_get_bitmap_width(bitmaps[index]->bitmap)/2,
		y,
		0
	);

	return al_get_bitmap_height(bitmaps[index]->bitmap);
}

int Credits_Loop::draw_strings(int index, int y)
{
	int inc = 50;

	for (size_t i = 0; i < strings[index].size(); i++) {
		int w = General::get_text_width(General::FONT_LIGHT, strings[index][i]);
		General::draw_text(
			strings[index][i],
			cfg.screen_w/2-w/2,
			y+inc,
			0
		);
		inc += General::get_font_line_height(General::FONT_LIGHT);
	}

	return inc + 50;
}

Credits_Loop::Credits_Loop()
{
	Music::play("music/heroes.mid");
}

Credits_Loop::~Credits_Loop()
{
	for (size_t i = 0; i < bitmaps.size(); i++) {
		if (bitmaps[i]) {
			Wrap::destroy_bitmap(bitmaps[i]);
		}
	}
	
	engine->fade(0, 0, al_map_rgba(0, 0, 0, 255));

	engine->clear_touches();
}

