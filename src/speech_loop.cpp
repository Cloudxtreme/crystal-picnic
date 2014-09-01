#include "crystalpicnic.h"
#include "speech_loop.h"

#include <cfloat>
#include <cctype>

static int32_t get_int32t_at_index(ALLEGRO_USTR *u, int i)
{
	int pos = al_ustr_offset(u, i);
	return al_ustr_get(u, pos);
}

static bool is_eow(const char *c)
{
	return isspace(*(c+1));
}

bool Speech_Loop::init()
{
	engine->clear_touches();

	if (inited) {
		return true;
	}
	Loop::init();

	return true;
}

void Speech_Loop::top()
{
}

bool Speech_Loop::handle_event(ALLEGRO_EVENT *event)
{
	bool prev_next = next;

	if (cfg.use_mouse) {
		if (event->type == ALLEGRO_EVENT_TOUCH_BEGIN && !autoscroll) {
			next = true;
			next_terminates = true;
			next_touch_id = event->touch.id;
		}
		else if (event->type == ALLEGRO_EVENT_TOUCH_END &&
				event->touch.id == next_touch_id) {
			next = false;
		}
	}
	else {
		if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
			if (event->keyboard.keycode == cfg.key_ability[3] && !autoscroll) {
				next = true;
				next_terminates = true;
			}
		}
		else if (event->type == ALLEGRO_EVENT_KEY_UP) {
			if (event->keyboard.keycode == cfg.key_ability[3]) {
				next = false;
			}
		}
		else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
			if (event->joystick.button == cfg.joy_ability[3] && !autoscroll) {
				next = true;
				next_terminates = true;
			}
		}
		else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP) {
			if (event->joystick.button == cfg.joy_ability[3]) {
				next = false;
			}
		}
	}

	handle_next(prev_next);

	return true;
}

bool Speech_Loop::logic()
{
	if (autoscroll) {
		if (block_num < (int)text.size()-1 && al_get_time() > end_block_time+5) {
			end_block_time = DBL_MAX;
			next = true;
			next_terminates = true;
			handle_next(false);
		}
	}

	for (size_t i = 0; i < children.size(); i++) {
		if (children[i]->logic()) {
			children.clear();
			break;
		}
	}

	engine->set_touch_input_type(TOUCHINPUT_SPEECH);

	if (wstate == FADING_IN) {
		in_count += General::LOGIC_MILLIS;
		if (in_count > FADE_TIME) {
			wstate = FADED_IN;
			in_count = FADE_TIME;
		}
	}
	else if (wstate == FADING_OUT) {
		out_count += General::LOGIC_MILLIS;
		int extra = autoscroll ? 5000 : 0;
		if (out_count > FADE_TIME+extra) {
			out_count = FADE_TIME+extra;
			if (blocking) {
				engine->unblock_mini_loop();
			}
			else if (autoscroll) {
				engine->remove_loop(this, true);
			}
			else {
				engine->set_loops(std::vector<Loop *>(), true);
			}
			return false;
		}
	}

	if (pstate == PRINTING) {
		char_count += General::LOGIC_MILLIS * (autoscroll ? 0.75 : (next ? 4 : 1));

		std::vector<Meta_Data *> md = get_metadata(char_num);

		while (char_count > char_delay) {
			char_count -= char_delay;

			char_num++;
			std::vector<Meta_Data *> md2 = get_metadata(char_num);

			if (md.size() == md2.size()) {
				bool same = true;
				for (size_t i = 0; i < md.size(); i++) {
					if (md[i] != md2[i]) {
						same = false;
						break;
					}
				}
				if (!same) {
					break;
				}
			}

			/* search for delays and speed settings */
			char_delay = DEFAULT_CHAR_DELAY;
			for (size_t i = 0; i < md2.size(); i++) {
				if (md2[i]->type == DELAY) {
					if (!md2[i]->used) {
						md2[i]->used = true;
						char_count = -md2[i]->milliseconds;
					}
				}
				else if (md2[i]->type == SPEED) {
					char_delay = md2[i]->milliseconds;
				}
			}

			int32_t ch = get_int32t_at_index(text[block_num], char_num-1);
			if (ch != ' ') {
				if (type != SPEECH_SPECIAL) {
					if (dream_keys) {
						if (char_num % 3 == 0) {
							//Sound::play(dream_keys);
						}
					}
					else {
						//General::play_type_writer_key();
					}
				}
			}
			if (char_num == newlines[line_num]) {
				line_num++;
				if (line_num == (int)newlines.size() || line_num-begin_line >= lines_available) {
					if (autoscroll && block_num >= (int)text.size()-1 && line_num == (int)newlines.size()) {
						pstate = DONE;
						wstate = FADING_OUT;
					}
					pstate = WAITING;
					end_block_time = al_get_time();
				}
				break;
			}
		}
	}

	return false;
}

void Speech_Loop::draw()
{
	for (size_t i = 0; i < children.size(); i++) {
		children[i]->draw();
	}
}

void Speech_Loop::post_draw()
{
	float alpha = 1;

	if (wstate == FADING_IN) {
		alpha = (float)in_count / FADE_TIME;
	}
	else if (wstate == FADED_IN) {
		alpha = 1;
	}
	else {
		if (autoscroll) {
			if (out_count < 5000) {
				alpha = 1.0;
			}
			else {
				alpha = 1.0 -
					((float)(out_count-5000) / FADE_TIME);
			}
		}
	}

	bool draw_star;
	if (autoscroll)
		draw_star = false;
	else
		draw_star = true;

	ALLEGRO_COLOR color;
	if (type == SPEECH_NORMAL) {
		color.r = alpha;
		color.g = alpha;
		color.b = alpha;
		color.a = alpha;
	}
	else if (type == SPEECH_SPECIAL) {
		color = al_map_rgba_f(0.0f, 0.0f, 0.0f, 0.0f);
	}
	else if (type == SPEECH_THOUGHTS) {
		color = al_map_rgba_f(
			0.7f*alpha, 0.7f*alpha, 0.7f*alpha, 0.7f*alpha
		);
	}
	else {
		color = bgcolor;
		color.r *= alpha;
		color.g *= alpha;
		color.b *= alpha;
		color.a *= alpha;
	}

	General::draw_speech_window(type, 0, y, cfg.screen_w, height, type == SPEECH_SPECIAL ? false : draw_star, color, 0.85f);

	// draw text
	int char_y = y+INSET;
	int line = begin_line;
	int start_x = INSET;

	al_hold_bitmap_drawing(true);
	for (int i = char_start; i < char_num; i++) {
		if (i == newlines[line]) {
			char_y += General::get_font_line_height(General::FONT_LIGHT)+1;
			line++;
			start_x = INSET;
		}

		int o = al_ustr_offset(text[block_num], i);
		int o2 = al_ustr_offset(text[block_num], i+1);
		int32_t ch = get_int32t_at_index(text[block_num], o);
		char str[7] = { '\0', };
		const char *cstr = al_cstr(text[block_num]);
		memcpy(str, cstr+o, o2-o);

		std::vector<Meta_Data *> md = get_metadata(i);

		if (ch != '\n')
			draw_char(str, start_x, char_y, md, alpha);

		start_x += get_char_width(str, md, is_eow(cstr+o));
	}
	al_hold_bitmap_drawing(false);
}

void Speech_Loop::set_height(int height)
{
	height = height == -1 ? cfg.screen_h-10 : height;
	int h = height;
	h -= INSET*2;
	if (type == SPEECH_NORMAL) {
		int sub = 0;
		sub -= 10; // overlap of name box and main box
		h -= sub;
		height -= sub;
	}
	h /= (General::get_font_line_height(General::FONT_LIGHT)+1);
	lines_available = h;
	set_y(height);
}

void Speech_Loop::set_y(int height)
{
	this->height = height;
	if (at_top) {
		y = 5;
	}
	else {
		y = cfg.screen_h-5-height;
	}
}

void Speech_Loop::set_colors(ALLEGRO_COLOR fgcolor, ALLEGRO_COLOR bgcolor)
{
	this->fgcolor = fgcolor;
	this->bgcolor = bgcolor;
}

Speech_Loop::Speech_Loop(
	bool blocking, std::vector<int> ids, std::vector<std::string> gestures, std::vector<ALLEGRO_USTR *> text, Speech_Location loc, Speech_Type type, bool autoscroll, std::vector<Loop *> children, int entity_to_set_facing_of, General::Direction direction_entity_should_face) :
	wstate(FADING_IN),
	pstate(PRINTING),
	in_count(0),
	out_count(0),
	char_count(0),
	block_num(0),
	char_num(0),
	char_start(0),
	line_num(0),
	begin_line(0),
	loc(loc),
	char_delay(DEFAULT_CHAR_DELAY),
	ids(ids),
	text(text),
	gestures(gestures),
	next(false),
	next_terminates(true),
	next_touch_id(-1),
	type(type),
	autoscroll(autoscroll),
	blocking(blocking),
	end_block_time(DBL_MAX),
	children(children),
	entity_to_set_facing_of(entity_to_set_facing_of),
	direction_entity_should_face(direction_entity_should_face),
	prev_entity_for_sound("")
{
	Area_Loop *al = General::find_in_vector<Area_Loop *, Loop *>(children);
	if (al) {
		al->get_area()->set_in_speech_loop(true);
	}

	if (type == SPEECH_SPECIAL) {
		lines_available = 10;
	}
	else {
		lines_available = LINES_AVAILABLE;
	}

	if (type == SPEECH_THOUGHTS) {
		dream_keys = NULL;
	}
	else {
		dream_keys = NULL;
	}

	bool top;

	if (loc == SPEECH_LOC_TOP)
		top = true;
	else if (loc == SPEECH_LOC_BOTTOM) {
		top = false;
	}
	else {
		Area_Loop *l = GET_AREA_LOOP;
		if (l) {
			Area_Manager *area = l->get_area();
			// dynamic_cast can't fail here
			Map_Entity *player = area->get_entity(0);
			General::Point<float> pos = player->get_position();
			General::Point<float> area_top = area->get_top();
			int y = pos.y - area_top.y;
			if ((y > (cfg.screen_h/2 - 50)) && (y < (cfg.screen_h/2 + 50))) {
				General::Direction d = player->get_direction();
				if (d == General::DIR_S || d == General::DIR_SW || d == General::DIR_SE) {
					top = true;
				}
				else {
					top = false;
				}
			}
			else if (y < cfg.screen_h/2) {
				top = false;
			}
			else {
				top = true;
			}
		}
		else {
			top = true;
		}
	}

	/* Stop players moving */
	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		Area_Manager *area = l->get_area();

		int nplayers = l->get_players().size();
		for (int i = 0; i < nplayers; i++) {
			int id;
			if (i == 0) {
				id = 0;
			}
			else {
				id = l->get_player_npcs()[i-1]->get_id();
			}
			Map_Entity *e = area->get_entity(id);
			float *input = e->get_inputs();
			if (input[Map_Entity::X] != 0 || input[Map_Entity::Y] != 0) {
				input[Map_Entity::X] = input[Map_Entity::Y] = 0;
				e->update_direction(false);
			}
		}
	}

	at_top = top;

	set_y((General::get_font_line_height(General::FONT_LIGHT)+1)*3+INSET*2);

	process_lines();

	gesture(true);
}

Speech_Loop::~Speech_Loop()
{
	Area_Loop *al = General::find_in_vector<Area_Loop *, Loop *>(children);
	if (al) {
		al->get_area()->set_in_speech_loop(false);
	}

	// If an entity faced the player, set their direction back
	if (entity_to_set_facing_of != -1) {
		Area_Loop *l = GET_AREA_LOOP;
		if (l) {
			Area_Manager *area = l->get_area();
			Map_Entity *e = area->get_entity(entity_to_set_facing_of);
			if (e) {
				e->set_direction(direction_entity_should_face);
				e->update_direction(false);
			}
		}
	}

	if (dream_keys) {
		Sound::destroy(dream_keys);
	}

	for (size_t i = 0; i < text.size(); i++) {
		al_ustr_free(text[i]);
	}

	for (size_t i = 0; i < fonts.size(); i++) {
		al_destroy_font(fonts[i].font);
	}
	fonts.clear();
	
	engine->clear_touches();
}

void Speech_Loop::gesture(bool first)
{
	if ((int)ids.size() <= block_num) {
		return;
	}

	int id = ids[block_num];

	Area_Loop *l = General::find_in_vector<Area_Loop *, Loop *>(children);
	if (l) {
		Area_Manager *area = l->get_area();
		Map_Entity *ent = area->get_entity(id);
		if (ent) {
			if (id != entity_to_set_facing_of) {
				float *input = ent->get_inputs();
				input[Map_Entity::X] = input[Map_Entity::Y] = 0;
				ent->update_direction(false);
			}
			ent->set_role_paused(true);

			prev_gesture = ent->get_animation_set()->get_sub_animation_name();
			ent->get_animation_set()->set_sub_animation(gestures[block_num]);
			ent->get_animation_set()->reset();
			float y = ent->get_position().y - ent->get_z();
			y -= area->get_top().y + area->_offset.y;
			if ((y > (cfg.screen_h/2 - 50)) && (y < (cfg.screen_h/2 + 50))) {
				General::Direction d = ent->get_direction();
				if (d == General::DIR_S || d == General::DIR_SW || d == General::DIR_SE) {
					at_top = true;
				}
				else {
					at_top = false;
				}
			}
			else if (y < cfg.screen_h/2) {
				at_top = false;
			}
			else {
				at_top = true;
			}
			set_y((General::get_font_line_height(General::FONT_LIGHT)+1)*3+INSET*2);

			if (type == SPEECH_NORMAL && prev_entity_for_sound != ent->get_name()) {
				prev_entity_for_sound = ent->get_name();
				if (prev_entity_for_sound == "egbert") {
					engine->play_sample("sfx/quack.ogg");
				}
				else if (prev_entity_for_sound == "frogbert") {
					if (ent->get_animation_set()->get_sub_animation_name() == "blush") {
						engine->play_sample("sfx/blush.ogg");
					}
					else {
						engine->play_sample("sfx/ribbit.ogg");
					}
				}
				else if (prev_entity_for_sound == "bisou") {
					engine->play_sample("sfx/bisou.ogg");
				}
				else if (prev_entity_for_sound == "pyou") {
					engine->play_sample("sfx/pyou.ogg");
				}
				else if (prev_entity_for_sound == "amaysa" && !engine->milestone_is_complete("beat_game")) {
					engine->play_sample("sfx/amaysa.ogg");
				}
				else if (prev_entity_for_sound == "king") {
					engine->play_sample("sfx/roar.ogg");
				}
				else if (prev_entity_for_sound == "flea_minigames") {
					engine->play_sample("sfx/flea_minigames.ogg");
				}
				else if (prev_entity_for_sound == "flea_items") {
					engine->play_sample("sfx/flea_items.ogg");
				}
				else if (prev_entity_for_sound == "flea_equipment") {
					engine->play_sample("sfx/flea_equipment.ogg");
				}
				else if (prev_entity_for_sound != "polygon") {
					engine->play_sample("sfx/npc.ogg");
				}
			}
		}
		else {
			if (loc == SPEECH_LOC_TOP) {
				at_top = true;
			}
			else if (loc == SPEECH_LOC_BOTTOM) {
				at_top = false;
			}
			set_y((General::get_font_line_height(General::FONT_LIGHT)+1)*3+INSET*2);
		}
	}
}

void Speech_Loop::ungesture()
{
	int id = ids[block_num];

	Area_Loop *l = General::find_in_vector<Area_Loop *, Loop *>(children);
	if (l) {
		Area_Manager *area = l->get_area();
		Map_Entity *ent = area->get_entity(id);
		if (ent) {
			ent->get_animation_set()->set_sub_animation(prev_gesture);
			ent->set_role_paused(false);
		}
	}
}

void Speech_Loop::process_lines()
{
	newlines.clear();
	meta_data.clear();

	ALLEGRO_USTR *str = text[block_num];
	bool done = false;
	int width = 0;
	int i = 0;
	int last_word = 0;

	while (true) {
		if ((unsigned)i >= al_ustr_length(str)) {
			break;
		}

		int top_i = i;

		int32_t ch = get_int32t_at_index(str, i);

		if (ch == '<') {
			i++;
			char keyword[100];
			if (get_int32t_at_index(str, i) == '/') {
				char *ptr = keyword;
				i++;
				while (get_int32t_at_index(str, i) != '>')
					*ptr++ = get_int32t_at_index(str, i++);
				*ptr = '\0';
				int m = meta_data.size()-1;
				Meta_Data_Type type = get_metadata_type(keyword);
				for (; m >= 0; m--) {
					if (meta_data[m].type == type) {
						meta_data[m].end = top_i-1;
						break;
					}
				}
			}
			else {
				Meta_Data md;
				md.start = i-1;
				char *ptr = keyword;
				while (get_int32t_at_index(str, i) != ' '
						&& get_int32t_at_index(str, i) != '>'
						&& get_int32t_at_index(str, i) != '/') {
					*ptr++ = get_int32t_at_index(str, i++);
				}
				*ptr = '\0';
				md.type = get_metadata_type(keyword);
				char cstr[10000];
				int o = al_ustr_offset(str, i);
				al_ustr_to_buffer(str, cstr, 10000);
				if (get_int32t_at_index(str, i) == '/') {
					i++;
				}
				if (get_int32t_at_index(str, i) != '>') {
					switch (md.type) {
					case SPEED:
					case DELAY:
						sscanf(cstr+o, "%d",
							&md.milliseconds);
						if (md.type == DELAY) {
							md.used = false;
							md.end = md.start;
							md.start--;
						}
						break;
					case SIZE:
						sscanf(cstr+o, "%d", &md.font_size);
						load_font(md.font_size, get_bold(get_metadata(i)));
						break;
					case COLOR: {
						int r, g, b;
						sscanf(cstr+o,
							"%d %d %d>",
							&r, &g, &b
						);
						md.color = al_map_rgb(
							r, g, b
						);
						break;
					}
					case BOLD:
					case ITALICS:
						md.end = INT_MAX;
						break;
					default:
						break;
					}
					while (get_int32t_at_index(str, i) != '>')
						i++;
				}
				meta_data.push_back(md);
			}
			int offs1 = al_ustr_offset(str, top_i);
			int offs2 = al_ustr_offset(str, i+1);
			al_ustr_remove_range(str, offs1, offs2);
			/*
			if (!strcmp(keyword, "i")) {
				al_ustr_insert_chr(str, top_i, ' ');
			}
			*/
			i = top_i;
			continue;
		}

		i++;
	}

	i = 0;

	while (!done) {
		int32_t ch = get_int32t_at_index(str, i);
		char buf[7] = { '\0', };
		int o = al_ustr_offset(str, i);
		int o2 = al_ustr_offset(str, i+1);
		const char *cstr = al_cstr(str);
		memcpy(buf, cstr+o, o2-o);

		int this_w = get_char_width(buf, get_metadata(i),
			is_eow(cstr+o));

		bool eol = false;
		if ((unsigned)i == al_ustr_length(str)-1) {
			done = true;
			eol = true;
		}
		else if (get_int32t_at_index(str, i) == '\n') {
			eol = true;
		}
		if (eol || (width+this_w) >= (cfg.screen_w-(INSET*2))) {
			int this_end_codepoints;
			int this_end_bytes;
			if (done)
				this_end_codepoints = i+1;
			else if (eol)
				this_end_codepoints = i;
			else
				this_end_codepoints = last_word;
			this_end_bytes = al_ustr_offset(str, this_end_codepoints);
			ALLEGRO_USTR *s = al_ustr_dup_substr(str, 0, this_end_bytes);
			newlines.push_back(al_ustr_length(s));
			al_ustr_free(s);
			width = 0;
			i = this_end_codepoints;
			if (eol && !done)
				i++;
			continue;
		}
		else {
			width += this_w;
			if (ch == ' ') {
				last_word = i;
				while (get_int32t_at_index(str, last_word) == ' ') {
					width += this_w;
					last_word++;
				}
			}
		}

		i++;
	}
}

Speech_Loop::Meta_Data_Type Speech_Loop::get_metadata_type(std::string keyword)
{
	if (keyword == "delay") {
		return DELAY;
	}
	else if (keyword == "speed") {
		return SPEED;
	}
	else if (keyword == "shake") {
		return SHAKE;
	}
	else if (keyword == "b") {
		return BOLD;
	}
	else if (keyword == "i") {
		return ITALICS;
	}
	else if (keyword == "size") {
		return SIZE;
	}
	else if (keyword == "color") {
		return COLOR;
	}

	return UNDEFINED;
}

std::vector<Speech_Loop::Meta_Data *> Speech_Loop::get_metadata(int index)
{
	std::vector<Meta_Data *> attrs;

	for (size_t i = 0; i < meta_data.size(); i++) {
		if (meta_data[i].start <= index && meta_data[i].end >= index) {
			attrs.push_back(&meta_data[i]);
		}
	}

	return attrs;
}

void Speech_Loop::load_font(int size, bool bold)
{
	for (size_t i = 0; i < fonts.size(); i++) {
		if (fonts[i].size == size)
			return;
	}

	Font f;
	f.size = size;
	f.font = General::load_font(bold ? "fonts/heavy.ttf" : "fonts/light.ttf", size);
	fonts.push_back(f);
}

int Speech_Loop::get_char_width(const char *cstr, std::vector<Meta_Data *> md,
	bool end_of_word)
{
	ALLEGRO_FONT *font = get_font(md);

	int width = General::get_text_width(font, cstr);

	if (get_shake(md))
		width += 2;
	if (get_italic(md)) {
		width += (end_of_word && type != SPEECH_THOUGHTS) ? 6 : 0;
	}

	return width;
}

void Speech_Loop::draw_char(const char *str, int x, int y, std::vector<Meta_Data *> md, float alpha)
{
	ALLEGRO_FONT *font = get_font(md);
	bool have_italic_font = false;

	if (get_shake(md)) {
		x += General::rand() % 3 - 1;
		y += General::rand() % 3 - 1;
	}

	ALLEGRO_COLOR color;

	switch (type) {
	case SPEECH_THOUGHTS:
		color = al_map_rgba(64*alpha, 64*alpha, 64*alpha, 255*alpha);
		break;
	case SPEECH_NORMAL:
		color = al_map_rgba(200*alpha, 200*alpha, 200*alpha, 255*alpha);
		break;
	case SPEECH_COLORED:
	case SPEECH_COLORED_ROUNDED:
		color = al_map_rgba_f(
			fgcolor.r*alpha,
			fgcolor.g*alpha,
			fgcolor.b*alpha,
			alpha
		);
		break;
	default:
		color = al_map_rgba(255*alpha, 255*alpha, 255*alpha, 255*alpha);
		break;
	}
	for (size_t i = 0; i < md.size(); i++) {
		if (md[i]->type == COLOR) {
			color = md[i]->color;
			color.r *= alpha;
			color.g *= alpha;
			color.b *= alpha;
			color.a = alpha;
			break;
		}
	}

	// FIXME: this is a skew -- probably replace with Allegro's
	ALLEGRO_TRANSFORM backup;
	if (get_italic(md) && !have_italic_font) {
		ALLEGRO_TRANSFORM t;
		al_copy_transform(&backup, al_get_current_transform());
		al_copy_transform(&t, al_get_current_transform());
		t.m[1][0] = -0.4f;
		t.m[3][0] = 0.4f * al_get_font_line_height(font)+y;
		al_use_transform(&t);
	}

	General::draw_text(font, str, color, x, y, 0);

	if (get_italic(md) && !have_italic_font) {
		al_use_transform(&backup);
	}
}

ALLEGRO_FONT *Speech_Loop::get_font(std::vector<Meta_Data *> md)
{
	ALLEGRO_FONT *font;

	if (get_bold(md)) {
		if (get_italic(md)) {
			font = General::get_font(General::FONT_HEAVY);
		}
		else {
			font = General::get_font(General::FONT_HEAVY);
		}
	}
	else {
		if (get_italic(md)) {
			font = General::get_font(General::FONT_LIGHT);
		}
		else {
			font = General::get_font(General::FONT_LIGHT);
		}
	}

	for (size_t i = 0;  i < md.size(); i++) {
		if (md[i]->type == SIZE) {
			for (size_t j = 0; j < fonts.size(); j++) {
				if (fonts[j].size == md[i]->font_size) {
					font = fonts[j].font;
					break;
				}
			}
			break;
		}
	}

	return font;
}

bool Speech_Loop::get_shake(std::vector<Meta_Data *> md)
{
	for (size_t i = 0; i < md.size(); i++) {
		if (md[i]->type == SHAKE) {
			return true;
		}
	}
	return false;
}

bool Speech_Loop::get_bold(std::vector<Meta_Data *> md)
{
	for (size_t i = 0; i < md.size(); i++) {
		if (md[i]->type == BOLD) {
			return true;
		}
	}
	return false;
}

bool Speech_Loop::get_italic(std::vector<Meta_Data *> md)
{
	for (size_t i = 0; i < md.size(); i++) {
		if (md[i]->type == ITALICS) {
			return true;
		}
	}
	return false;
}

void Speech_Loop::handle_next(bool prev_next)
{
	if (next) {
		if (pstate == WAITING && next_terminates) {
			next = false;
			engine->play_sample("sfx/use_item.ogg");
			char_start = char_num;
			begin_line = line_num;
			bool setnl = false;
			if (line_num >= (int)newlines.size()) {
				ungesture();
				block_num++;
				gesture(false);
				char_num = 0;
				line_num = 0;
				char_start = 0;
				begin_line = 0;
				setnl = true;
			}
			if (block_num >= (int)text.size()) {
				pstate = DONE;
				wstate = FADING_OUT;
			}
			else {
				pstate = PRINTING;
				if (setnl) {
					process_lines();
				}
			}
		}
		else if (prev_next == false) {
			next_terminates = false;
		}
	}
}
