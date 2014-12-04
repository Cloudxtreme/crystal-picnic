#include "crystalpicnic.h"
#include "whack_a_skunk_loop.h"

static void draw_centered_text(Animation_Set *font, ALLEGRO_COLOR c, const char *s, int x, int y)
{
	int one_w = font->get_current_animation()->get_current_frame()->get_width();
	int tot_w = one_w * strlen(s);
	int xx = x - tot_w/2;

	for (int i = 0; s[i]; i++) {
		char buf[10];
		if (s[i] == ':') {
			snprintf(buf, 10, "%s", "colon");
		}
		else {
			snprintf(buf, 10, "%c", s[i]);
		}
		font->set_sub_animation(buf);
		font->get_current_animation()->draw_tinted(
			c, xx+i*one_w, y, 0
		);
	}
}

const int Whack_a_Skunk_Loop::STAR_LIFETIME = 1000;
const int Whack_a_Skunk_Loop::STARS_PER = 20;
const float Whack_a_Skunk_Loop::SHOOT_SPEED = 1.75;
const int Whack_a_Skunk_Loop::POW_LIFETIME = 1000;
const int Whack_a_Skunk_Loop::GAME_LENGTH = 60; // seconds

bool Whack_a_Skunk_Loop::init(void)
{
	engine->clear_touches();

	if (inited) {
		return true;
	}
	Loop::init();

	regular_skunk = new Animation_Set();
	regular_skunk->load("mini_games/whack_a_skunk/skunk");
	silver_skunk = new Animation_Set();
	silver_skunk->load("mini_games/whack_a_skunk/silver_skunk");
	gold_skunk = new Animation_Set();
	gold_skunk->load("mini_games/whack_a_skunk/gold_skunk");
	fake_skunk = new Animation_Set();
	fake_skunk->load("mini_games/whack_a_skunk/mallard");
	skunk_size.w = regular_skunk->get_current_animation()->get_current_frame()->get_bitmap()->get_width();
	skunk_size.h = regular_skunk->get_current_animation()->get_current_frame()->get_bitmap()->get_height();

	bg_bitmap = Wrap::load_bitmap("mini_games/whack_a_skunk/board.cpi");

	hand = new Animation_Set();
	hand->load("mini_games/whack_a_skunk/hands");
	hand_size.w = hand->get_current_animation()->get_current_frame()->get_bitmap()->get_width();
	hand_size.h = hand->get_current_animation()->get_current_frame()->get_bitmap()->get_height();

	font_w = 15;
	font_h = 19;
	font = new Animation_Set();
	font->load("mini_games/whack_a_skunk/font");

	XMLData *xml = new XMLData("mini_games/whack_a_skunk/board.xml");

	for (int i = 0; i < 9; i++) {
		char buf[2];
		buf[0] = '0'+i;
		buf[1] = 0;
		XMLData *node = xml->find(std::string(buf));
		XMLData *x_node = node->find("x");
		XMLData *y_node = node->find("y");
		Skunk_Hole h;
		h.x = atoi(x_node->get_value().c_str());
		h.y = atoi(y_node->get_value().c_str());
		h.status = NIL;
		h.miss_count = -1;
		holes.push_back(h);
	}

	delete xml;

	bg_w = al_get_bitmap_width(bg_bitmap->bitmap);
	bg_h = al_get_bitmap_height(bg_bitmap->bitmap);

	top_offset.x = (cfg.screen_w-bg_w)/2;
	top_offset.y = cfg.screen_h-bg_h;

	next_diff = max_next - min_next;
	next_count = 0;

	misses = 0;
	hits = 0;

	star_bmp = Wrap::load_bitmap("mini_games/whack_a_skunk/star.cpi");
	pow_bmp = Wrap::load_bitmap("mini_games/whack_a_skunk/pow.cpi");
	kratch_bmp = Wrap::load_bitmap("mini_games/whack_a_skunk/kratch.cpi");
	mask_front = Wrap::load_bitmap("mini_games/whack_a_skunk/mask-front.cpi");
	mask_middle = Wrap::load_bitmap("mini_games/whack_a_skunk/mask-middle.cpi");
	mask_back = Wrap::load_bitmap("mini_games/whack_a_skunk/mask-back.cpi");
	highlight_front = Wrap::load_bitmap("mini_games/whack_a_skunk/highlight-front.cpi");
	highlight_middle = Wrap::load_bitmap("mini_games/whack_a_skunk/highlight-middle.cpi");
	highlight_back = Wrap::load_bitmap("mini_games/whack_a_skunk/highlight-back.cpi");
	mask_fronthighlight = Wrap::load_bitmap("mini_games/whack_a_skunk/mask-fronthighlight.cpi");
	mask_middlehighlight = Wrap::load_bitmap("mini_games/whack_a_skunk/mask-middlehighlight.cpi");
	mask_backhighlight = Wrap::load_bitmap("mini_games/whack_a_skunk/mask-backhighlight.cpi");

	start_time = al_current_time();

	// Load SFX
	hit_sample = Sound::load("mini_games/whack_a_skunk/sfx/bash.ogg");
	miss_sample = Sound::load("mini_games/whack_a_skunk/sfx/bash_no_skunk.ogg");
	taunt_sample = Sound::load_set("mini_games/whack_a_skunk/sfx/taunt", "ogg");

	return true;
}

void Whack_a_Skunk_Loop::top(void)
{
}

bool Whack_a_Skunk_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (done) {
		return true;
	}

	if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
		if (event->keyboard.keycode == cfg.key_left) {
			axes[0] = -1;
		}
		else if (event->keyboard.keycode == cfg.key_right) {
			axes[0] = 1;
		}
		else if (event->keyboard.keycode == cfg.key_down) {
			axes[1] = 1;
		}
		else if (event->keyboard.keycode == cfg.key_up) {
			axes[1] = -1;
		}
		else if (event->keyboard.keycode == cfg.key_ability[3]) {
			if (!bashing) {
				bashing = true;
			}
		}
	}
	else if (event->type == ALLEGRO_EVENT_KEY_UP) {
		if (event->keyboard.keycode == cfg.key_left || event->keyboard.keycode == cfg.key_right) {
			axes[0] = 0;
		}
		else if (event->keyboard.keycode == cfg.key_up || event->keyboard.keycode == cfg.key_down) {
			axes[1] = 0;
		}
	}

	if (cfg.use_joy) {
		if (event->type == ALLEGRO_EVENT_JOYSTICK_AXIS && event->joystick.stick == 0) {
			axes[event->joystick.axis] = fabs(event->joystick.pos) > 0.25 ? General::sign(event->joystick.pos) : 0;
		}
		else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
			if (event->joystick.button == cfg.joy_ability[3])
				if (!bashing)
					bashing = true;
		}
	}

#if defined ALLEGRO_ANDROID || defined ALLEGRO_IPHONE
	if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
		int closest_index = -1;
		float closest_distance = FLT_MAX;
		for (int i = 0; i < 9; i++) {
			float dist = General::distance(holes[i].x, holes[i].y, event->mouse.x, event->mouse.y);
			if (dist < closest_distance) {
				closest_distance = dist;
				closest_index = i;
			}
		}
		curr_hole = closest_index;
		bashing = true;
	}
#endif

	int hole_numbers[3][3] = {
		{ 0, 1, 2 },
		{ 3, 4, 5 },
		{ 6, 7, 8 }
	};

	if (!bashing) {
		curr_hole = hole_numbers[axes[1]+1][axes[0]+1];
	}
	else if ((holes[curr_hole].status == ALIVE || holes[curr_hole].status == TAUNTING) && holes[curr_hole].count < regular_skunk->get_length("popup") * 0.7f) {
		holes[curr_hole].miss_count = 600;
		holes[curr_hole].status = DYING;
		holes[curr_hole].dying_count = 0;
		if (holes[curr_hole].type == FAKE) {
			engine->play_sample("sfx/error.ogg");
			misses++;
			hits -= 3;
			hits_in_a_row = 0;
			add_pow_effects(curr_hole, true);
		}
		else {
			Sound::play(hit_sample);
			hits_in_a_row++;
			int mul;
			if (hits_in_a_row >= 7) {
				mul = 3;
			}
			else if (hits_in_a_row >= 3) {
				mul = 2;
			}
			else {
				mul = 1;
			}
			int points;
			if (holes[curr_hole].type == GOLD) {
				points = 3;
			}
			else if (holes[curr_hole].type == SILVER) {
				points = 2;
			}
			else {
				points = 1;
			}
			hits += mul * points;
			add_pow_effects(curr_hole, false);
		}
	}
	else {
		if (holes[curr_hole].miss_count < 0) {
			Sound::play(miss_sample);
			hits_in_a_row = 0;
			holes[curr_hole].miss_count = 600;
		}
	}

	return true;
}

bool Whack_a_Skunk_Loop::logic(void)
{
	engine->set_touch_input_type(TOUCHINPUT_GUI);

	for (int i = 0; i < 9; i++) {
		if (holes[i].miss_count >= 0) {
			holes[i].miss_count -= General::LOGIC_MILLIS;
		}
	}

	if (done) {
		done_count += General::LOGIC_MILLIS;
		if (done_count > 2000) {
			/* Stop automatic comeback to this game */
			Area_Loop *a = General::find_in_vector<Area_Loop *, Loop *>(engine->get_loops());
			if (a) {
				std::vector<Player *> players = a->get_players();
				for (size_t i = 0; i < players.size(); i++) {
					players[i]->reset();
				}
			}

			if (hits < 0) {
				hits = 0;
			}
			area_loop->set_whack_a_skunk_played(true);
			area_loop->set_whack_a_skunk_score(hits);

			engine->set_loops(std::vector<Loop *>(), true);
		}
		return false;
	}

	double now = al_current_time();
	timer = (now - start_time) * 1000;

	for (int i = 0; i < 9; i++) {
		if (holes[i].status == ALIVE || holes[i].status == GOING_DOWN) {
			int old_count = holes[i].count;
			holes[i].count += General::LOGIC_MILLIS;
			int len = regular_skunk->get_length("popup");
			if (old_count < len/2 && holes[i].count >= len/2) {
				if (holes[i].type != FAKE && General::rand() % 3 == 0) {
					Sound::play(taunt_sample);
					holes[i].status = TAUNTING;
					holes[i].taunting_count = 0;
				}
			}
		}
		else if (holes[i].status == DYING) {
			holes[i].dying_count += General::LOGIC_MILLIS;
			if (holes[i].dying_count >= regular_skunk->get_length("hit")) {
				holes[i].status = GOING_DOWN;
			}
		}
		else if (holes[i].status == TAUNTING) {
			holes[i].taunting_count += General::LOGIC_MILLIS;
			if (holes[i].taunting_count >= regular_skunk->get_length("taunt")) {
				holes[i].status = GOING_DOWN;
			}
		}
	}

	std::list<Star>::iterator star_it;
	for (star_it = stars.begin(); star_it != stars.end();) {
		Star &s = *star_it;
		s.x += s.dx;
		s.y += s.dy;
		s.angle += s.da;
		s.count += General::LOGIC_MILLIS;
		if (s.count > STAR_LIFETIME)
			star_it = stars.erase(star_it);
		else
			star_it++;
	}
	std::list<Pow>::iterator pow_it;
	for (pow_it = pows.begin(); pow_it != pows.end();) {
		Pow &p = *pow_it;
		p.count += General::LOGIC_MILLIS;
		if (p.count > POW_LIFETIME)
			pow_it = pows.erase(pow_it);
		else
			pow_it++;
	}

	next_count += General::LOGIC_MILLIS;

	if (next_count > min_next) {
		bool go = ((int)(((float)(General::rand()%1000)/1000 * next_diff)) < General::LOGIC_MILLIS) ||
			next_count >= max_next;
		if (go) {
			std::vector<int> available;
			for (int i = 0; i < 9; i++) {
				if (holes[i].status == NIL) {
					available.push_back(i);
				}
			}
			if (available.size() > 0) {
				int i = available[General::rand()%available.size()];
				holes[i].status = ALIVE;
				holes[i].count = 0;
				next_count = 0;
				int r = General::rand() % 100;
				if (r < 10) {
					holes[i].type = FAKE;
				}
				else if (r < 25) {
					holes[i].type = GOLD;
				}
				else if (r < 40) {
					holes[i].type = SILVER;
				}
				else {
					holes[i].type = REGULAR;
				}
			}
		}
	}

	if (timer >= GAME_LENGTH*1000) {
		done = true;
	}

	if (bashing) {
		hand->update();
		if (hand->get_current_animation()->is_finished()) {
			bashing = false;
			hand->reset();
#if defined ALLEGRO_IPHONE || defined ALLEGRO_ANDROID
			curr_hole = -1;
#endif
		}
	}

	return false;
}

void Whack_a_Skunk_Loop::draw(void)
{
	al_clear_to_color(al_color_name("black"));

	al_draw_bitmap(
		bg_bitmap->bitmap,
		top_offset.x,
		top_offset.y,
		0
	);

	float scales[3] = { 0.83f, 0.9f, 1.0f };
	Wrap::Bitmap *hole_bmps[3] = {
		mask_back,
		mask_middle,
		mask_front
	};
	int hole_yoffs[3] = { 6, 8, 10 };
	Wrap::Bitmap *highlight_bmps[3] = {
		highlight_back,
		highlight_middle,
		highlight_front
	};
	int highlight_yoffs[3] = { 5, 6, 8 };
	Wrap::Bitmap *maskhighlight_bmps[3] = {
		mask_backhighlight,
		mask_middlehighlight,
		mask_fronthighlight
	};
	
	for (int i = 0; i < 9; i++) {
		int row = i / 3;

		if (i == curr_hole) {
			Wrap::Bitmap *highlight = highlight_bmps[row];
			int highlight_w = al_get_bitmap_width(highlight->bitmap);
			int highlight_h = al_get_bitmap_height(highlight->bitmap);
			al_draw_bitmap(
				highlight->bitmap,
				holes[i].x-highlight_w/2+top_offset.x,
				holes[i].y-highlight_h+highlight_yoffs[row]+top_offset.y,
				0
			);
		}
		
		Animation_Set *anim_set;
		bool done = get_skunk_info(i, &anim_set);

		int x = holes[i].x - skunk_size.w/2 + top_offset.x;
		int y = holes[i].y - skunk_size.h + top_offset.y;
		float scale = scales[row];
		int ox = (skunk_size.w * (1.0f-scale)) / 2;
		int oy = 10;
		oy += (skunk_size.h * (1.0f-scale)) / 2;
		int len = anim_set->get_length(anim_set->get_sub_animation_name());
		if (holes[i].count > len/2) {
			oy += ((float)(holes[i].count-(len/2)) / (len/2)) * 60 /* 60 = base skunk height */;
		}
		else {
			oy += (1.0 - ((float)holes[i].count / (len/2))) * 60;
		}

		if (!done) {
			int cx, cy, cw, ch;
			al_get_clipping_rectangle(&cx, &cy, &cw, &ch);
			General::set_clipping_rectangle(x, y, skunk_size.w, skunk_size.h);

			if (holes[i].status == ALIVE || holes[i].status == TAUNTING || holes[i].status == GOING_DOWN) {
				anim_set->get_current_animation()->draw_scaled(
					0, 0, skunk_size.w, skunk_size.h,
					x+ox, y+oy, skunk_size.w*scale, skunk_size.h*scale,
					0
				);
			}
			else if (holes[i].status == DYING) {
				std::string name = anim_set->get_sub_animation_name();
				int frame = anim_set->get_current_animation()->get_current_frame_num();
				anim_set->set_sub_animation("popup");
				anim_set->set_frame(holes[i].type == FAKE ? 0 : 3);
				anim_set->get_current_animation()->draw_scaled(
					0, 0, skunk_size.w, skunk_size.h,
					x+ox, y+oy, skunk_size.w*scale, skunk_size.h*scale,
					0
				);
				anim_set->set_sub_animation(name);
				anim_set->set_frame(frame);
			}

			al_set_clipping_rectangle(cx, cy, cw, ch);
		}

		Wrap::Bitmap *mask = i == curr_hole ? maskhighlight_bmps[row] : hole_bmps[row];
		int mask_w = al_get_bitmap_width(mask->bitmap);
		int mask_h = al_get_bitmap_height(mask->bitmap);
		al_draw_bitmap(
			mask->bitmap,
			holes[i].x-mask_w/2+top_offset.x,
			holes[i].y-mask_h+hole_yoffs[row]+top_offset.y,
			0
		);
		
		if (!done) {
			if (holes[i].status == DYING) {
				anim_set->get_current_animation()->draw_scaled(
					0, 0, skunk_size.w, skunk_size.h,
					x+ox, y+oy, skunk_size.w*scale, skunk_size.h*scale,
					0
				);
			}
		}
	}

	// Draw timer
	int digits;
	if (timer/1000 >= 10) {
		digits = 2;
	}
	else {
		digits = 1;
	}
	
	int font_w = font->get_current_animation()->get_current_frame()->get_width();
	float xx = (cfg.screen_w-font_w*(digits+1))/2;
	float yy = 5;
	int tmp = timer;
	for (int i = 0; i < digits; i++) {
		int p = powf(10, (digits-i)+2);
		int frame = tmp / p;
		font->set_sub_animation(General::itos(frame));
		font->get_current_animation()->draw(
			xx, yy, 0
		);
		tmp -= frame * p;
		xx += font_w;
	}
	font->set_sub_animation("s");
	font->get_current_animation()->draw(
		xx, yy, 0
	);

	// Draw score/hitx
	if (hits < 0) {
		hits = 0;
	}
	char buf[100];
	snprintf(buf, 100, "%d", hits);
	draw_centered_text(font, al_color_name("lime"), buf, cfg.screen_w/4-30, 5);

	int hitx;
	if (hits_in_a_row >= 7) {
		hitx = 3;
	}
	else if (hits_in_a_row >= 3) {
		hitx = 2;
	}
	else {
		hitx = 1;
	}
	snprintf(buf, 100, "x%d", hitx);
	draw_centered_text(font, al_color_name("gold"), buf, cfg.screen_w*3/4+15, 5);

	std::list<Pow>::iterator pow_it;
	for (pow_it = pows.begin(); pow_it != pows.end(); pow_it++) {
		Pow &p = *pow_it;
		ALLEGRO_COLOR tint;
		float a;
		if (p.count < POW_LIFETIME/2) {
			a = 1.0;
		}
		else {
			a = ((float)p.count-POW_LIFETIME/2) / (POW_LIFETIME/2);
			if (a > 1.0) a = 1.0;
			a = 1.0 - a;
		}
		tint = al_map_rgba_f(1.0, 1.0, 1.0, a);
		int r = (cfg.screen_w / 120) * 2 + 1;
		al_draw_tinted_bitmap(
			p.kratch ? kratch_bmp->bitmap : pow_bmp->bitmap,
			tint,
			p.x-al_get_bitmap_width(pow_bmp->bitmap)/2+General::rand()%r-(r/2)+top_offset.x,
			p.y-al_get_bitmap_height(pow_bmp->bitmap)/2+General::rand()%r-(r/2)+top_offset.y,
			0
		);
	}
	std::list<Star>::iterator star_it;
	for (star_it = stars.begin(); star_it != stars.end(); star_it++) {
		Star &s = *star_it;
		ALLEGRO_COLOR tint;
		float b = (float)s.count / STAR_LIFETIME;
		if (b > 1.0) b = 1.0;
		tint = al_map_rgb_f(1.0, 1.0, b);
		al_draw_tinted_rotated_bitmap(
			star_bmp->bitmap,
			tint,
			al_get_bitmap_width(star_bmp->bitmap)/2,
			al_get_bitmap_height(star_bmp->bitmap)/2,
			s.x+top_offset.x, s.y+top_offset.y,
			s.angle,
			0
		);
	}

	if (bashing) {
		float xx = holes[curr_hole].x;
		float yy = holes[curr_hole].y - hand_size.h;
		xx += top_offset.x;
		yy += top_offset.y;

		xx += 90; // trial and error
		yy += 110;

		if  (curr_hole == 0 || curr_hole == 3 || curr_hole == 6) {
			hand->set_sub_animation("left");
		}
		else if (curr_hole == 1 || curr_hole == 4 || curr_hole == 7) {
			hand->set_sub_animation("middle");
			xx -= 68;
		}
		else {
			hand->set_sub_animation("right");
			xx -= 68*2;
		}

		hand->get_current_animation()->draw(
			xx-hand_size.w/2,
			yy,
			0
		);
	}
}

Whack_a_Skunk_Loop::Whack_a_Skunk_Loop(int min_next, int max_next) :
	min_next(min_next),
	max_next(max_next),
	bashing(false),
	done(false),
	done_count(0),
	hits_in_a_row(0),
	timer(0)
{
	old_music = Music::get_playing();
	Music::play("music/whack_a_skunk.mid", 1.0f, true);

	axes[0] = axes[1] = 0;
	hits = misses = 0;

#if defined ALLEGRO_IPHONE || defined ALLEGRO_ANDROID
	curr_hole = -1;
#else
	curr_hole = 4;
#endif
}

Whack_a_Skunk_Loop::~Whack_a_Skunk_Loop(void)
{
	delete regular_skunk;
	delete silver_skunk;
	delete gold_skunk;
	delete fake_skunk;
	Wrap::destroy_bitmap(bg_bitmap);
	delete hand;
	Wrap::destroy_bitmap(star_bmp);
	Wrap::destroy_bitmap(pow_bmp);
	Wrap::destroy_bitmap(kratch_bmp);

	Sound::destroy(hit_sample);
	Sound::destroy(miss_sample);
	Sound::destroy(taunt_sample);

	Music::play(old_music, 1.0f, true);
	
	engine->clear_touches();
}

// NOTE: sets anim_set sub animation and frame
bool Whack_a_Skunk_Loop::get_skunk_info(int hole, Animation_Set **anim_set)
{	
	if (holes[hole].type == REGULAR) {
		*anim_set = regular_skunk;
	}
	else if (holes[hole].type == SILVER) {
		*anim_set = silver_skunk;
	}
	else if (holes[hole].type == GOLD) {
		*anim_set = gold_skunk;
	}
	else {
		*anim_set = fake_skunk;
	}

	int count;

	if (holes[hole].status == ALIVE || holes[hole].status == GOING_DOWN) {
		(*anim_set)->set_sub_animation("popup");
		count = holes[hole].count;
	}
	else if (holes[hole].status == DYING) {
		(*anim_set)->set_sub_animation("hit");
		count = holes[hole].dying_count;
	}
	else {
		(*anim_set)->set_sub_animation("taunt");
		count = holes[hole].taunting_count;
	}

	int frame = -1;
	int total = 0;
	int nframes = (int)((*anim_set)->get_current_animation()->get_num_frames());
	int end = (holes[hole].status == ALIVE || holes[hole].status == GOING_DOWN) ? nframes*2-1 : nframes;

	for (int i = 0; i < end; i++) {
		int fr = (i >= nframes) ? (nframes - (i % 4 + 2)) : i;
		total += (*anim_set)->get_current_animation()->get_frame(fr)->get_delay();
		if (count < total) {
			frame = fr;
			break;
		}
	}
	
	if (frame == -1 && (holes[hole].status == GOING_DOWN || holes[hole].status == ALIVE)) {
		holes[hole].status = NIL;
		return true;
	}
	else {
		(*anim_set)->get_current_animation()->set_frame(frame);
		return false;
	}
}

void Whack_a_Skunk_Loop::add_pow_effects(int hole, bool kratch)
{
	Pow p;
	p.x = holes[hole].x;
	p.y = holes[hole].y-25;
	p.count = 0;
	p.kratch = kratch;
	pows.push_back(p);

	for (int i = 0; i < STARS_PER; i++) {
		Star s;
		s.x = holes[hole].x;
		s.y = holes[hole].y-25;
		float shoot_angle = (float)(General::rand()%1000)/1000*M_PI+M_PI;
		float r = (float)(General::rand()%1000)/1000*SHOOT_SPEED;
		s.dx = cos(shoot_angle)*(SHOOT_SPEED+r);
		s.dy = sin(shoot_angle)*(SHOOT_SPEED+r);
		s.count = 0;
		s.angle =  (float)(General::rand()%1000)/1000*M_PI+M_PI;
		s.da = (float)(General::rand()%1000)/1000*0.1-0.05;
		s.da += 0.05*General::sign(s.da);
		stars.push_back(s);
	}
}

void Whack_a_Skunk_Loop::set_area_loop(Area_Loop *area_loop)
{
	this->area_loop = area_loop;
}
