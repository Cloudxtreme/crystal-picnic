#include "battle_loop.h"

#include "main_menu_loop.h"
#include "battle_pathfind.h"
#include "graphics.h"
#include "runner_loop.h"
#include "collision_detection.h"
#include "general.h"
#include "skeleton.h"
#include "player.h"
#include "battle_transition_in.h"
#include "enemy_avatar.h"
#include "game_specific_globals.h"

#include <cfloat>

#include <allegro5/allegro.h>

ALLEGRO_DEBUG_CHANNEL("CrystalPicnic")

int PSX;
int PSY;
int PEX;
int PEY;
Battle_Pathfinder *pathfinder;
Battle_Pathfinder_Node *path;

General::Size<int> Battle_Loop::size;
float Battle_Loop::x_offset_ratio;

const int MINI_BATTLE_STATS_W = 64;
static void draw_mini_battle_stats(Battle_Player *p, int x, int y)
{
	Wrap::Bitmap *profile = p->get_profile_bitmap();
	al_draw_bitmap(profile->bitmap, x, y, 0);

	Battle_Attributes &attr = p->get_attributes();

	ALLEGRO_COLOR hilight = al_map_rgb_f(0.4f, 0.4f, 0.4f);

	int profile_w = al_get_bitmap_width(profile->bitmap);
	int profile_h = al_get_bitmap_height(profile->bitmap);
	int w = MINI_BATTLE_STATS_W - profile_w - 2;

	int max_hp = attr.max_hp;
	int max_mp = attr.max_mp;
	Game_Specific_Globals::get_accessory_effects(attr.equipment.accessory.name, &max_hp, &max_mp, NULL, NULL);

	Graphics::draw_gauge(
		x + profile_w + 2,
		y+profile_h/2-6,
		w,
		true,
		(float)(attr.hp/cfg.difficulty_mult())/(max_hp/cfg.difficulty_mult()), hilight,
		al_color_name("lime")
	);
	Graphics::draw_gauge(
		x + profile_w + 2,
		y+profile_h/2,
		w,
		false,
		(float)attr.mp/max_mp,
		hilight,
		al_color_name("cyan")
	);
}

void Battle_Loop::draw_bullet_time(General::Point<float> start, General::Point<float> end, float width)
{
	if (cfg.low_graphics || al_get_target_bitmap() == screen_copy_bmp->bitmap) {
		return;
	}

	float offset = al_get_time() * (0.75 / (1.0 / 60.0));

	float X1 = start.x;
	float Y1 = start.y;
	float X2 = end.x;
	float Y2 = end.y;

	Shader::use(bullet_time_shader);
	al_set_shader_float("x1", X1);
	al_set_shader_float("y1", Y1);
	al_set_shader_float("x2", X2);
	al_set_shader_float("y2", Y2);
	al_set_shader_float("offset", offset);
	al_set_shader_float("target_h", al_get_bitmap_height(al_get_target_bitmap()));
	al_draw_bitmap_region(screen_copy_bmp->bitmap, X1, Y1, X2-X1, Y2-Y1, X1, Y1, 0);
	Shader::use(NULL);
}

void Battle_Loop::draw_bullet_time_v(General::Point<float> start, General::Point<float> end, float width)
{
	if (cfg.low_graphics || al_get_target_bitmap() == screen_copy_bmp->bitmap) {
		return;
	}

	float offset = al_get_time() * (0.75 / (1.0 / 60.0));

	float X1 = start.x;
	float Y1 = start.y;
	float X2 = end.x;
	float Y2 = end.y;

	printf("%f %f %f %f\n", X1, Y1, X2, Y2);

	Shader::use(bullet_time_v_shader);
	al_set_shader_float("x1", X1);
	al_set_shader_float("y1", Y1);
	al_set_shader_float("x2", X2);
	al_set_shader_float("y2", Y2);
	al_set_shader_float("offset", offset);
	al_set_shader_float("target_w", al_get_bitmap_width(al_get_target_bitmap()));
	al_draw_bitmap_region(screen_copy_bmp->bitmap, X1, Y1, X2-X1, Y2-Y1, X1, Y1, 0);
	Shader::use(NULL);
}

General::Point<float> Battle_Loop::get_top(void)
{
	int dx, dy;
	get_area_offset(&dx, &dy);

	// adjust for scripted camera
	dx -= _offset.x;
	dy -= _offset.y;

	return General::Point<float>(-dx, -dy);
}

bool Battle_Loop::init(void)
{
	engine->clear_touches();

	if (inited) {
		return true;
	}
	Loop::init();

	if (dynamic_cast<Runner_Loop *>(this)) {
		do_not_make_screen_copy = true;
		runner_loop = true;
	}
	else {
		runner_loop = false;
	}

	bullet_time_shader = Shader::get("bullet_time");
	bullet_time_v_shader = Shader::get("bullet_time_v");

	active_players = 0;
	for (size_t i = 0; i < players.size(); i++) {
		bool add;
		if (cart_battle && players[i]->get_name() == "frogbert") {
			add = true;
		}
		else if (dynamic_cast<Runner_Loop *>(this)) {
			add = true;
		}
		else if (!cart_battle && players[i]->get_battle_attributes().hp > 0) {
			add = true;
		}
		else {
			add = false;
		}
		if (add) {
			Battle_Player *p = new Battle_Player(this, players[i], players[i]->get_name(), i);
			p->construct();
			p->set_stops_battle_end(false);
			p->set_id(curr_id++);
			p->get_animation_set()->set_sub_animation("battle-idle");
			if (p->get_weapon_animation_set()) {
				p->get_weapon_animation_set()->set_sub_animation("battle-idle");
			}
			entities.push_back(p);
			if (active_player == -1) {
				active_player = p->get_player_id();
			}
			active_players++;
		}
	}

	if (enemy_avatar) {
		for (unsigned int i = 0; i < enemy_avatar->get_enemies().size(); i++) {
			Battle_Enemy *enemy =
				new Battle_Enemy(this, enemy_avatar->get_enemies()[i]);
			enemy->construct();
			enemy->set_id(curr_id++);
			entities.push_back(enemy);
		}
	}

	cart_pixels_travelled = 0;
	set_cart_transition_start = false;
	added_antboss = false;
	cart_battle_enemy_dead = false;
	cart_crashed = false;
	rumble_offset = General::Point<float>(0.0f, 0.0f);
	played_wheel_sample = false;

	battle_start_time = al_get_time();

	return true;
}

void Battle_Loop::top(void)
{
	if (first_run) {
		first_run = false;
		init_lua();
		engine->play_sample("sfx/enter_battle.ogg");
		battle_transition_done = false;
	}
}

bool Battle_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (!battle_transition_done) {
		return false;
	}

	if (event->type == ALLEGRO_EVENT_KEY_DOWN && (
#ifdef ALLEGRO_ANDROID
		event->keyboard.keycode == ALLEGRO_KEY_BUTTON_L1 ||
#endif
		event->keyboard.keycode == cfg.key_switch
	)) {
		switch_players = true;
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN && event->joystick.button == cfg.joy_switch) {
		switch_players = true;
	}

	if (!cart_battle || !cart_battle_enemy_dead) {
		int sz = entities.size();
		for (int i = 0; i < sz; i++) {
			entities[i]->handle_event(event);
		}
	}

	return true;
}

bool Battle_Loop::logic(void)
{
	engine->set_touch_input_type(TOUCHINPUT_BATTLE);

	for (size_t i = 0; i < entities_to_add.size(); i++) {
		entities.push_back(entities_to_add[i]);
		if (entities_to_add[i]->get_name() == "antboss") {
			added_antboss = true;
		}
	}
	entities_to_add.clear();

	if (cart_battle_enemy_dead) {
		cart_pixels_travelled = cart_pixels_travelled_final;
	}
	else {
		cart_pixels_travelled += 600 / General::LOGIC_RATE; // 600 pixels/second
	}

	if (script_has_logic) {
		Lua::call_lua(lua_state, "logic", ">");
	}

	if (cart_battle) {
		pumping_cart->update(General::LOGIC_MILLIS);
		cart_wheel->update(General::LOGIC_MILLIS);
	}

	size_t bhi;
	for (bhi = 0; bhi < burrow_holes.size(); bhi++) {
		if (burrow_holes[bhi].end_time <= al_get_time()) {
			destroy_burrow_hole(burrow_holes, bhi);
		}
		else {
			burrow_particle_count++;
			if (burrow_particle_count % 5 == 0) {
				engine->play_sample("sfx/plant_shovel.ogg");
				Particle::Particle *p = Particle::add_particle(burrow_holes[bhi].pgid, 1, 1, al_map_rgba_f(0, 0, 0, 0), 0, 0, true, false);
				p->set_position(burrow_holes[bhi].pos);
				p->data[0] = burrow_holes[bhi].pos.x;
				p->data[1] = burrow_holes[bhi].pos.y;
				float angle = (General::rand()%1000/1000.0f) * (M_PI-M_PI/4) + M_PI + M_PI/8;
				float vx = cos(angle) * 4;
				float vy = sin(angle) * 4;
				p->data[2] = vx;
				p->data[3] = vy;
				p->data[4] = 6;
				p->data[5] = 0;
			}
			bhi++;
		}
	}
	for (bhi = 0; bhi < static_burrow_holes.size(); bhi++) {
		if (static_burrow_holes[bhi].end_time <= al_get_time()) {
			destroy_burrow_hole(static_burrow_holes, bhi);
		}
		else {
			bhi++;
		}
	}

	if (switch_players) {
		switch_players = false;
		/* Zero players input so they don't keep running or something */
		Battle_Player *p = get_active_player();
		p->close_circles();
		int ap = active_player;
		General::Point<float> start = get_top();
		bool found = false;
		for (int i = 0; i < 2; i++) {
			active_player++;
			active_player %= players.size();
			if (get_active_player() != NULL && get_active_player()->get_attributes().hp > 0) {
				found = true;
				break;
			}
		}
		if (found) {
			std::map<const int, float> &input = get_player(active_player)->get_input();
			input.clear();
			General::Point<float> end = get_top();
			// FIXME: add it or set it?
			_offset.x = start.x - end.x;
			_offset.y = start.y - end.y;
			move_camera_to_zero = true;
			engine->play_sample("sfx/switch_to_" + get_active_player()->get_name() + ".ogg", 1.0f, 0.0f, 1.0f);
		}
		else {
			active_player = ap;
		}
	}

	int sz = entities.size();
	for (int i = 0; i < sz; i++) {
		entities[i]->logic();
	}
	sz = particles.size();
	for (int i = 0; i < sz; i++) {
		particles[i]->logic();
	}
	for (int i = 0; i < sz; i++) {
		if (particles[i]->get_delete_me()) {
			delete particles[i];
			ERASE_FROM_UNSORTED_VECTOR(particles, i);
			i--;
			sz--;
		}
	}

	if (!enemy_avatar)
		return false;

	// Move camera to zero?
	if (move_camera_to_zero) {
		int sign_x = General::sign(_offset.x);
		int sign_y = General::sign(_offset.y);
		if (_offset.x > 0) {
			_offset.x -= 16.0f;
		}
		else if (_offset.x < 0) {
			_offset.x += 16.0f;
		}
		if (_offset.y > 0) {
			_offset.y -= 16.0f;
		}
		else if (_offset.y < 0) {
			_offset.y += 16.0f;
		}
		int sign_x2 = General::sign(_offset.x);
		int sign_y2 = General::sign(_offset.y);
		if (sign_x2 != sign_x) {
			_offset.x = 0;
		}
		if (sign_y2 != sign_y) {
			_offset.y = 0;
		}
		if (_offset.x == 0 && _offset.y == 0) {
			move_camera_to_zero = false;
		}
	}

	// Remove dead entities
	sz = entities.size();
	for (int i = 0; i < sz; i++) {
		Entity *entity = entities[i];
		if (entity->get_delete_me()) {
			ERASE_FROM_UNSORTED_VECTOR(entities, i);
			delete entity;
			i--;
			sz--;
		}
	}

	bool no_battle_stoppers = true;
	sz = entities.size();
	for (int i = 0; i < sz; i++) {
		Battle_Entity *e = entities[i];
		if (e) {
			if (e->get_stops_battle_end()) {
				no_battle_stoppers = false;
				break;
			}
		}
		else {
			no_battle_stoppers = false;
			break;
		}
	}

	if ((!cart_battle || added_antboss) && game_over_time < 0 && (get_active_player() == NULL || no_battle_stoppers)) {
		if (!cart_battle_enemy_dead) {
			cart_battle_enemy_dead = true;
			cart_battle_enemy_dead_time = al_get_time();
			cart_pixels_travelled_final = cart_pixels_travelled;
		}
		else {
			double t = al_get_time() - cart_battle_enemy_dead_time;
			if (t > 5 && !cart_crashed) {
				cart_crashed = true;
				engine->stop_sample("sfx/cart_running.ogg");
				Lua::call_lua(lua_state, "crash", "");
			}
			if (t > 3) {
				t = 3;
				if (!cart_music_ramped) {
					cart_music_ramped = true;
					Music::ramp_down(1.0);
				}
			}
			_offset.x = -(80 * t * (600.0f/80.0f));
		}
		if (!cart_battle || al_get_time() > cart_battle_enemy_dead_time+15) {
			engine->set_loops(std::vector<Loop *>(), true);
			return false;
		}
	}

	if (game_over_time >= 0.0 && al_get_time() > game_over_time + 15.0) {
		engine->set_game_over(true);
		if (boss_battle) {
			engine->set_lost_boss_battle(true);
		}
		else {
			engine->set_lost_boss_battle(false);
		}
		engine->fade_out();
		engine->set_loops(std::vector<Loop *>(), true);
		return false;
	}
	
	sz = entities.size();
	for (int i = 0; i < sz; i++) {
		// Regenerate magic
		Battle_Player *p = dynamic_cast<Battle_Player *>(entities[i]);
		if (p == NULL) {
			continue;
		}
		float count = p->get_magic_regen_count();
		count += Game_Specific_Globals::regenerate_magic(p->get_attributes(), true, count);
		p->set_magic_regen_count(count-((int)count));
		// Apply status effects
		Game_Specific_Globals::apply_status(p->get_attributes());
	}

	return false;
}

void Battle_Loop::get_area_offset(int *dx, int *dy)
{
	General::Point<float> pos = get_active_player()->get_position(); // player pos

	int sw = cfg.screen_w;
	float offset = (sw/2) * x_offset_ratio;

	*dx = -(pos.x - offset);
	if (*dx < -(size.w-sw))
		*dx = -(size.w-sw);
	else if (*dx > 0)
		*dx = 0;

	if (enemy_avatar) {
		int hh = cfg.screen_h - General::TILE_SIZE*3;
		int sz = size.h;
		if (cfg.screen_h > sz) {
			*dy = (cfg.screen_h - sz) / 2;
		}
		else {
			*dy = -(pos.y - hh);
			if (*dy < -(size.h-(hh+General::TILE_SIZE*3)))
				*dy = -(size.h-(hh+General::TILE_SIZE*3));
			else if (*dy > 0)
				*dy = 0;
		}
	}
	else {
		*dy = -(pos.y-cfg.screen_h/2);
	}
}

void Battle_Loop::get_player_screen_pos(General::Point<float> pos, int *px, int *py)
{
	int sw = cfg.screen_w;
	float offset = (sw/2) * x_offset_ratio;

	if (pos.x < offset)
		*px = pos.x;
	else if (pos.x > size.w-offset)
		*px = pos.x - (size.w - sw);
	else
		*px = offset;

	// this feels a bit hackish
	Runner_Loop *rl = General::find_in_vector<Runner_Loop *, Loop *>(engine->get_mini_loops());
	if (!rl) {
		int hh = cfg.screen_h - General::TILE_SIZE*3;
		int sz = size.h;
		if (pos.y < hh) {
			*py = pos.y;
		}
		else if (pos.y < sz-General::TILE_SIZE*3) {
			*py = hh;
		}
		else {
			*py = hh + (pos.y - (sz-General::TILE_SIZE*3));
		}
		if (cfg.screen_h > sz) {
			*py += (cfg.screen_h - sz) / 2;
		}
	}
	else {
		*py = cfg.screen_h / 2;
	}
}

void Battle_Loop::get_entity_draw_pos(General::Point<float> pos, Animation_Set *anim_set, int *tlx, int *tly)
{
	int px, py;
	Battle_Loop::get_player_screen_pos(pos, &px, &py);
	int w = anim_set->get_current_animation()->get_current_frame()->get_bitmap()->get_width();
	int h = anim_set->get_current_animation()->get_current_frame()->get_bitmap()->get_height();
	*tlx = px - w/2;
	*tly = py - h + General::BOTTOM_SPRITE_PADDING;
}

void Battle_Loop::get_entity_coordinates(General::Point<float> pos, Animation_Set *anim_set, int *tlx, int *tly)
{
	*tlx = pos.x;
	*tly = pos.y;
}

void Battle_Loop::draw_entities(int layer)
{
	int dx, dy;
	get_area_offset(&dx, &dy);

	// adjust for scripted camera
	dx -= _offset.x;
	dy -= _offset.y;

	if (cart_battle) {
		Battle_Entity *p = get_player(active_player);
		if (p && p->get_attributes().hp <= 0) {
			pumping_cart->set_sub_animation("without");
		}
		Animation *a = pumping_cart->get_current_animation();
		a->draw(size.w/2 + 192/2 - 72 + dx, size.h - 24 - a->get_current_frame()->get_height() + dy, 0);
	}

	int sz = entities.size();
	for (int i = 0; i < sz; i++) {
		Battle_Entity *entity = entities[i];

		if (!entity->is_visible()) {
			entity->script_pre_draw();
			entity->script_post_draw();
			continue;
		}
		if ((entity->get_layer() == -1 && layer != entity_layer) && (entity->get_layer() != layer)) {
			continue;
		}

		int tlx, tly;
		General::Point<float> pos = entity->get_position();
		Animation *a = NULL;
		Skeleton::Skeleton *skeleton;
		int tmp_w, tmp_h;

		skeleton = entity->get_skeleton();
		if (skeleton) {
			tmp_w = 100;
			tmp_h = 100;
		}
		else {
			a = entity->get_animation_set()->get_current_animation();
			tmp_w = a->get_current_frame()->get_bitmap()->get_width();
			tmp_h = a->get_current_frame()->get_bitmap()->get_height();
		}
		
		if (entity == get_active_player()) {
			get_entity_draw_pos(pos, entity->get_animation_set(), &tlx, &tly);
			tlx -= _offset.x;
			tly -= _offset.y;
		}
		else {
			tlx = dx + pos.x - tmp_w/2;
			tly = dy + pos.y - tmp_h + General::TILE_SIZE;
		}

		// Try a quick check if they're on the screen. Note: tlx/tly aren't used below for skeletons
		if (tlx < -150 || tly < -150 || tlx > cfg.screen_w || tly > cfg.screen_h) {
			continue;
		}

		entity->pre_draw();

		int cx, cy, cw, ch;
		al_get_clipping_rectangle(&cx, &cy, &cw, &ch);
		if (entity->is_burrowing()) {
			General::Point<float> pos = entity->get_burrow_start_position();
			General::Point<float> top = get_top();
			General::set_clipping_rectangle(pos.x-32-top.x, pos.y-64-top.y, 64, 64);
		}

		Battle_Player *player = dynamic_cast<Battle_Player *>(entity);
		if (player) {
			Animation *wa;
			if (player->get_attributes().hp > 0 && !player->get_lost_weapon() && entity->get_weapon_animation_set())
				wa = entity->get_weapon_animation_set()->get_current_animation();
			else
				wa = NULL;

			a->draw_tinted_rotated(
				entity->get_tint(),
				tlx,
				tly,
				entity->is_facing_right() ? 0 : ALLEGRO_FLIP_HORIZONTAL
			);
			if (wa) {
				wa->draw_tinted_rotated(
					entity->get_tint(),
					tlx,
					tly,
					entity->is_facing_right() ? 0 : ALLEGRO_FLIP_HORIZONTAL
				);
			}
				
			#if 0
			int px = dx + pos.x;
			int py = dy + pos.y;

			// KEEPME: draw players collision point in cyan
			al_draw_circle(
				px, py, 4, al_color_name("cyan"), 1
			);

			// KEEPME: Same as below but using the pathfinding info (so some extra lines too for jump points)
			for (int i = 0; i < num_pathfinding_edges; i++) {
				Battle_Pathfinder_Edge &e = pathfinding_edges[i];
				al_draw_line(
					dx+e.start->x, dy+e.start->y,
					dx+e.end->x, dy+e.end->y,
					al_color_name("lime"), 1
				);
			}

			// KEEPME: draw platform lines in yellow
			for (unsigned int j = 0; j < geometry.size(); j++) {
				for (unsigned int i = 1; i < geometry[j].size(); i++) {
					al_draw_line(
						dx+geometry[j][i-1].x,
						dy+geometry[j][i-1].y,
						dx+geometry[j][i].x,
						dy+geometry[j][i].y,
						al_color_name("yellow"),
						4 // FIXME: 1
					);
				}
			}

			// KEEPME: draw big purple circles on the geometry points
			for (unsigned int j = 0; j < geometry.size(); j++) {
				for (unsigned int i = 0; i < geometry[j].size(); i++) {
					al_draw_circle(
						dx+geometry[j][i].x,
						dy+geometry[j][i].y,
						6, al_color_name("magenta"), 1
					);
				}
			}

			// KEEPME: draw jump_points in red
			for (unsigned int j = 0; j < jump_points.size(); j++) {
				for (unsigned int i = 0; i < jump_points[j].size(); i++) {
					if (j == i)
						continue;
					Jump_Point &jp = jump_points[j][i];
					al_draw_filled_circle(dx+jp.x, dy+jp.y, 4, al_color_name("red"));
				}
			}

			// KEEPME: draw start and end pos of pathfinding (testing)
			al_draw_rectangle(
				dx+PSX-5, dy+PSY-5, dx+PSX+5, dy+PSY+5, al_color_name("black"), 1
			);
			al_draw_rectangle(
				dx+PEX-5, dy+PEY-5, dx+PEX+5, dy+PEY+5, al_color_name("white"), 1
			);

			if (path) {
				Battle_Pathfinder_Node *n = path;
				Battle_Pathfinder_Node *prev = NULL;
				while (n) {
					if (prev) {
						al_draw_line(
							dx+n->x, dy+n->y,
							dx+prev->x, dy+prev->y,
							al_color_name("blue"), 1
						);
					}
					prev = n;
					n = n->parent;
				}
			}
			#endif

			#if 0
			// KEEPME: draw player bones
			int anim_w = a->get_current_frame()->get_width();
			int anim_h = a->get_current_frame()->get_height();

			std::map< std::pair<std::string, int>, std::vector<Bones::Bone> > &bones = entity->get_bones();
			std::map< std::pair<std::string, int>, std::vector<Bones::Bone> > &weapon_bones = entity->get_weapon_bones();
			std::string name = a->get_name();
			int frame = a->get_current_frame_num();

			std::pair<std::string, int> p;
			p.first = name;
			p.second = frame;

			// draw weapons too!
			for (int j = 0; j < 2; j++) {
				std::vector<Bones::Bone> &v = j == 0 ? bones[p] : weapon_bones[p];
				if (v.size() == 0) {
					p.first = std::string("(rnd)") + name;
					v = j == 0 ? bones[p] : weapon_bones[p];
				}
				for (unsigned int i = 0; i < v.size(); i++) {
					Bones::Bone &b = v[i];
					ALLEGRO_COLOR c;
					switch (b.type) {
						case Bones::BONE_NORMAL:
							c = al_map_rgb(180, 180, 180);
							// FIXME:
							continue;
							break;
						case Bones::BONE_ATTACK:
							c = al_map_rgb(255, 0, 0);
							break;
						case Bones::BONE_RESISTANT:
							c = al_map_rgb(70, 70, 70);
							break;
						case Bones::BONE_WEAK:
							c = al_map_rgb(255, 255, 0);
							break;
					}
					std::vector<Triangulate::Triangle> triangles;
					if (entity->is_facing_right())
						triangles = b.get();
					else
						triangles = b.get_mirrored();
					std::vector<Triangulate::Triangle>::iterator it;
					it = triangles.begin();
					for (; it != triangles.end(); it++) {
						Triangulate::Triangle &t = *it;
						al_draw_line(
							tlx+t.points[0].x+anim_w/2, tly+t.points[0].y+anim_h,
							tlx+t.points[1].x+anim_w/2, tly+t.points[1].y+anim_h,
							c, 1
						);
						al_draw_line(
							tlx+t.points[1].x+anim_w/2, tly+t.points[1].y+anim_h,
							tlx+t.points[2].x+anim_w/2, tly+t.points[2].y+anim_h,
							c, 1
						);
						al_draw_line(
							tlx+t.points[2].x+anim_w/2, tly+t.points[2].y+anim_h,
							tlx+t.points[0].x+anim_w/2, tly+t.points[0].y+anim_h,
							c, 1
						);
					}
				}
			}
			#endif

			#if 0
			// KEEPME: draw player bones outline
			std::map< std::pair<std::string, int>, std::vector<Bones::Bone> > &bones = entity->get_bones();
			std::string name = a->get_name();
			int frame = a->get_current_frame_num();

			std::pair<std::string, int> p;
			p.first = name;
			p.second = frame;

			std::vector<Bones::Bone> &v = bones[p];

			for (size_t i = 0; i < v.size(); i++) {
				Bones::Bone b = v[i];
				std::vector< General::Point<float> > p = entity->is_facing_right() ? b.get_outline() : b.get_outline_mirrored();
				for (size_t j = 0; j < p.size(); j++) {
					int k = (j + 1) % p.size();
					al_draw_line(
						tlx+p[j].x+anim_w/2,
						tly+p[j].y+anim_h,
						tlx+p[k].x+anim_w/2,
						tly+p[k].y+anim_h,
						al_map_rgb_f(1, 1, 1),
						1
					);
				}
			}
			#endif
		}
		else {
			General::Point<float> pos = entity->get_position();

			ALLEGRO_COLOR tint;
			int ndraws;

			if (entity->get_attributes().hp <= 0) {
				double t = al_get_time() - entity->get_death_animation_start();

				if (t < Battle_Entity::DEATH_ANIM_LENGTH/2) {
					ndraws = 3;
					float p = t / (Battle_Entity::DEATH_ANIM_LENGTH/2);
					tint = al_map_rgba_f(
						p, p, p, p
					);
				}
				else if (t < Battle_Entity::DEATH_ANIM_LENGTH*3/4) {
					t -= Battle_Entity::DEATH_ANIM_LENGTH/2;
					ndraws = 3;
					float p = 1.0 - (t / (Battle_Entity::DEATH_ANIM_LENGTH/4));
					tint = al_map_rgba_f(
						p, p, p, p
					);
				}
				else {
					t -= Battle_Entity::DEATH_ANIM_LENGTH*3/4;
					ndraws = 1;
					float p = 1.0 - (t / (Battle_Entity::DEATH_ANIM_LENGTH/4));
					tint = al_map_rgba_f(
						p, p, p, p
					);
				}
				if (entity->get_name() == "oldoak") {
					ndraws = 1;
				}
			}
			else {
				ndraws = 1;
				tint = al_map_rgb_f(1, 1, 1);
			}

			if (skeleton) {
				ALLEGRO_STATE blend_state;
				al_store_state(&blend_state, ALLEGRO_STATE_BLENDER);

				for (int i = 0; i < ndraws; i++) {
					ALLEGRO_COLOR t;

					if (i == 0 && ndraws > 1) {
						t = al_map_rgba_f(1, 1, 1, 1);
					}
					else
						t = tint;

					bool is_hurt = entity->get_attributes().hp > 0 && entity->is_hurt();
					if (is_hurt) {
						Wrap::Shader *tinter = Graphics::get_add_tint_shader();
						Shader::use(tinter);
						al_set_shader_float("p", 0.5f);
						al_set_shader_float("color_r", 1.0f);
						al_set_shader_float("color_g", 1.0f);
						al_set_shader_float("color_b", 1.0f);
					}

					skeleton->draw(General::Point<float>(pos.x+dx, pos.y+dy), !entity->is_facing_right(), t);

					if (is_hurt) {
						Shader::use(NULL);
					}

					al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
				}
				
				al_restore_state(&blend_state);
			}
			else {
				Animation *a = entity->get_animation_set()->get_current_animation();
				Animation *wa;
				if (entity->get_weapon_animation_set())
					wa = entity->get_weapon_animation_set()->get_current_animation();
				else
					wa = NULL;
				int w = a->get_current_frame()->get_bitmap()->get_width();
				int h = a->get_current_frame()->get_bitmap()->get_height();

				ALLEGRO_STATE blend_state;
				al_store_state(&blend_state, ALLEGRO_STATE_BLENDER);

				if ((entity->get_name() == "oldoak" && entity->get_attributes().hp <= 0) || (entity->get_attributes().hp > 0 && entity->is_hurt())) {
					ALLEGRO_COLOR t = al_map_rgb_f(1.0, 1.0, 1.0);
					a->draw_add_tinted_rotated(
						t,
						0.5f,
						dx+pos.x-w/2,
						dy+pos.y-h+General::TILE_SIZE,
						entity->is_facing_right() ? 0 : ALLEGRO_FLIP_HORIZONTAL
					);
				}
				else {
					for (int i = 0; i < ndraws; i++) {
						ALLEGRO_COLOR t;

						if (i == 0 && ndraws > 1) {
							t = al_map_rgba_f(1, 1, 1, 1);
						}
						else
							t = tint;

						if (entity->get_name() == "oldoak") {
							Wrap::Bitmap *b = a->get_current_frame()->get_bitmap()->get_bitmap();
							Graphics::draw_tinted_bitmap_region_depth_yellow_glow(
								b,
								al_map_rgb_f(1, 1, 1),
								0, 0,
								al_get_bitmap_width(b->bitmap),
								al_get_bitmap_height(b->bitmap),
								dx+pos.x-w/2,
								dy+pos.y-h+General::TILE_SIZE,
								entity->is_facing_right() ? 0 : ALLEGRO_FLIP_HORIZONTAL,
								1.0f,
								183, 207, 243,
								183, 207, 243
							);
						}
						else {
							a->draw_tinted_rotated(
								t,
								dx+pos.x-w/2,
								dy+pos.y-h+General::TILE_SIZE,
								entity->is_facing_right() ? 0 : ALLEGRO_FLIP_HORIZONTAL
							);
						}
						if (wa) {
							wa->draw_tinted_rotated(
								t,
								dx+pos.x-w/2,
								dy+pos.y-h+General::TILE_SIZE,
								entity->is_facing_right() ? 0 : ALLEGRO_FLIP_HORIZONTAL
							);
						}

						al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
					}
				}

				al_restore_state(&blend_state);

				// KEEPME: draw enemy bones
				#if 0
				int anim_w = a->get_current_frame()->get_width();
				int anim_h = a->get_current_frame()->get_height() + General::TILE_SIZE;
				int tlx = dx+pos.x-w/2;
				int tly = dy+pos.y-h;
				std::map< std::pair<std::string, int>, std::vector<Bones::Bone> > &bones = entity->get_bones();

				std::string name = a->get_name();
				int frame = a->get_current_frame_num();

				std::pair<std::string, int> p;
				p.first = name;
				p.second = frame;
				std::vector<Bones::Bone> &v = bones[p];
				if (v.size() == 0) {
					p.first = std::string("(rnd)") + name;
					v = bones[p];
				}
				for (unsigned int i = 0; i < v.size(); i++) {
					Bones::Bone &b = v[i];
					ALLEGRO_COLOR c;
					switch (b.type) {
						case Bones::BONE_NORMAL:
							c = al_map_rgb(180, 180, 180);
							break;
						case Bones::BONE_ATTACK:
							c = al_map_rgb(255, 0, 0);
							break;
						case Bones::BONE_RESISTANT:
							c = al_map_rgb(70, 70, 70);
							break;
						case Bones::BONE_WEAK:
							c = al_map_rgb(255, 255, 0);
							break;
					}
					std::vector<Triangulate::Triangle> triangles;
					if (entity->is_facing_right())
						triangles = b.get();
					else
						triangles = b.get_mirrored();
					std::vector<Triangulate::Triangle>::iterator it;
					it = triangles.begin();
					for (; it != triangles.end(); it++) {
						Triangulate::Triangle &t = *it;
						al_draw_line(
							tlx+t.points[0].x+anim_w/2, tly+t.points[0].y+anim_h,
							tlx+t.points[1].x+anim_w/2, tly+t.points[1].y+anim_h,
							c, 1
						);
						al_draw_line(
							tlx+t.points[1].x+anim_w/2, tly+t.points[1].y+anim_h,
							tlx+t.points[2].x+anim_w/2, tly+t.points[2].y+anim_h,
							c, 1
						);
						al_draw_line(
							tlx+t.points[2].x+anim_w/2, tly+t.points[2].y+anim_h,
							tlx+t.points[0].x+anim_w/2, tly+t.points[0].y+anim_h,
							c, 1
						);
					}
				}
				#endif
			}
		}
		
		entity->draw();

		entity->post_draw();
		
		if (entity->is_burrowing()) {
			al_set_clipping_rectangle(cx, cy, cw, ch);
		}
	}

	extra_drawing_hook(layer);
}

void Battle_Loop::draw()
{
	if (!cart_battle && !dynamic_cast<Runner_Loop *>(this) && !battle_transition_done) {
		// We need to run logic() once to make sure enemy AI "start" is called
		// which can set position etc of enemies
		logic();

		battle_transition_done = true;

		Wrap::Bitmap *second = Wrap::create_bitmap_no_preserve(cfg.screen_w, cfg.screen_h);
		ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
		al_set_target_bitmap(second->bitmap);
		al_clear_to_color(al_map_rgb(0, 0, 0));
		bool di = draw_interface;
		draw_interface = false;
		do_not_make_screen_copy = true;
		draw();
		draw_interface = di;
		do_not_make_screen_copy = false;

		battle_transition_in(pre_battle_screen_bmp, second);
		
		al_set_target_bitmap(old_target);
		
		Wrap::destroy_bitmap(pre_battle_screen_bmp);
		Wrap::destroy_bitmap(second);
	
		engine->reset_logic_count();
	}

	if (cfg.low_graphics) {
		do_not_make_screen_copy = true;
	}

	ALLEGRO_BITMAP *old_target = NULL;
	if (!do_not_make_screen_copy && making_screen_copy) {
		old_target = al_get_target_bitmap();
		al_set_target_bitmap(screen_copy_bmp->bitmap);
	}

	int dx, dy;
	get_area_offset(&dx, &dy);

	// adjust for scripted camera
	dx -= _offset.x;
	dy -= _offset.y;

	// draw parallax background
	if (cart_battle) {
		lua_getglobal(lua_state, "seventh_hit_time");
		double seventh_hit_time = lua_tonumber(lua_state, 1);
		lua_pop(lua_state, 1);

		Wrap::Bitmap *b = cart_parallax[3];
		int bmp_w = al_get_bitmap_width(b->bitmap);
		int bmp_h = al_get_bitmap_height(b->bitmap);
		int x = (cart_pixels_travelled+(int)rumble_offset.x) % bmp_w;
		int y = -(dy+rumble_offset.y) * ((double)bmp_h / size.h);

		al_hold_bitmap_drawing(true);
		al_draw_bitmap_region(
			b->bitmap,
			0,
			y,
			al_get_bitmap_width(b->bitmap),
			cfg.screen_h,
			-x-al_get_bitmap_width(b->bitmap),
			0,
			0
		);
		al_draw_bitmap_region(
			b->bitmap,
			0,
			y,
			al_get_bitmap_width(b->bitmap),
			cfg.screen_h,
			-x,
			0,
			0
		);
		al_draw_bitmap_region(
			b->bitmap,
			0,
			y,
			al_get_bitmap_width(b->bitmap),
			cfg.screen_h,
			-x+bmp_w,
			0,
			0
		);
		al_hold_bitmap_drawing(false);

		b = cart_parallax[0];
		bmp_w = al_get_bitmap_width(b->bitmap);
		bmp_h = al_get_bitmap_height(b->bitmap);
		x = (cart_pixels_travelled+(int)rumble_offset.x) % bmp_w;
		y = -(dy+rumble_offset.y) * ((double)bmp_h / size.h);

		if (seventh_hit_time > 0 && !set_cart_transition_start) {
			cart_transition_start = cart_pixels_travelled + (bmp_w - x);
			set_cart_transition_start = true;
			engine->add_flash(1.8, 0.25, 0.0, 0.25, al_color_name("white"));
		}

		Wrap::Bitmap *bmp1, *bmp2;

		if (set_cart_transition_start && cart_pixels_travelled >= cart_transition_start) {
			int bmp_w2 = al_get_bitmap_width(cart_parallax[1]->bitmap);
			int bmp_w3 = al_get_bitmap_width(cart_parallax[2]->bitmap);

			if (cart_pixels_travelled < cart_transition_start+bmp_w) {
				bmp1 = b;
				bmp2 = cart_parallax[1];
			}
			else if (cart_pixels_travelled < cart_transition_start+bmp_w+bmp_w2) {
				bmp1 = cart_parallax[1];
				bmp2 = cart_parallax[2];

				x = cart_pixels_travelled - cart_transition_start - bmp_w;
			}
			else {
				bmp1 = cart_parallax[2];
				bmp2 = cart_parallax[2];

				x = (cart_pixels_travelled - cart_transition_start - bmp_w - bmp_w2) % bmp_w3;
			}
		}
		else {
			bmp1 = bmp2 = b;
		}
		
		al_draw_bitmap_region(
			cart_parallax[2]->bitmap,
			0,
			y,
			al_get_bitmap_width(cart_parallax[2]->bitmap),
			cfg.screen_h,
			-x-al_get_bitmap_width(cart_parallax[2]->bitmap),
			0,
			0
		);
		al_draw_bitmap_region(
			bmp1->bitmap,
			0,
			y,
			al_get_bitmap_width(bmp1->bitmap),
			cfg.screen_h,
			-x,
			0,
			0
		);
		al_draw_bitmap_region(
			bmp2->bitmap,
			0,
			y,
			al_get_bitmap_width(bmp2->bitmap),
			cfg.screen_h,
			-x+al_get_bitmap_width(bmp1->bitmap),
			0,
			0
		);

		if (cart_crashed) {
			double t = al_get_time() - cart_battle_enemy_dead_time;
			if (t > 7) {
				if (!played_wheel_sample) {
					played_wheel_sample = true;
					engine->play_sample("sfx/cart_wheel.ogg");
				}
				t = t - 7;
				Animation *a = cart_wheel->get_current_animation();
				a->draw(cfg.screen_w+16-t*100, cfg.screen_h-32, 0);
			}
		}
	}
	for (unsigned int i = 0; i < parallax_bmps.size(); i++) {
		if (!parallax_is_foreground[i]) {
			Wrap::Bitmap *b = parallax_bmps[i];
			int bmp_w = al_get_bitmap_width(b->bitmap);
			int bmp_h = al_get_bitmap_height(b->bitmap);
			int level_w = size.w;
			int diff1 = bmp_w - cfg.screen_w;
			int diff2 = level_w - cfg.screen_w;
			int xx = (float)dx/diff2 * diff1;
			int x = -xx;
			int y = -dy * ((float)bmp_h / size.h);
			al_draw_bitmap_region(
				b->bitmap,
				x,
				y,
				cfg.screen_w,
				cfg.screen_h,
				0,
				0,
				0
			);
		}
	}
	
	bool draw_layer_0_tinted = false;
	int sz = entities.size();
	for (int i = 0; i < sz; i++) {
		Battle_Enemy *e = dynamic_cast<Battle_Enemy *>(entities[i]);
		if (e && e->get_name() == "oldoak") {
			if (e->get_attributes().hp <= 0 || e->is_hurt()) {
				draw_layer_0_tinted = true;
			}
			break;
		}
	}
	
	int num_sheets = atlas_get_num_sheets(atlas);

	for (size_t layer = 0; layer < tiles.size(); layer++) {
		if (layer == 0 && draw_layer_0_tinted) {
			Wrap::Shader *tinter = Graphics::get_add_tint_shader();
			Shader::use(tinter);
			al_set_shader_float("p", 0.5f);
			al_set_shader_float("color_r", 1.0f);
			al_set_shader_float("color_g", 1.0f);
			al_set_shader_float("color_b", 1.0f);
		}
		for (int sheet = 0; sheet < num_sheets; sheet++) {
			al_hold_bitmap_drawing(true);
			int sz = tiles[layer].size();
			for (int t = 0; t < sz; t++) {
				Tile &tile = tiles[layer][t];
				int id;
				if (tile.ids.size() > 0) {
					int frame = fmod((al_get_time() / (tile.delay/1000.0f)), tile.ids.size());
					id = tile.ids[frame];
				}
				else {
					id = tile.id;
				}
				ATLAS_ITEM *item = atlas_get_item_by_id(atlas, id);
				if (atlas_get_item_sheet(item) != sheet) {
					continue;
				}
				Wrap::Bitmap *sub = atlas_get_item_sub_bitmap(item);
				int w = al_get_bitmap_width(sub->bitmap);
				int h = al_get_bitmap_height(sub->bitmap);
				float x = tile.x + dx;
				float y = tile.y + dy;
				if (x+w >= 0 && y+h >= 0 && x < cfg.screen_w && y < cfg.screen_h) {
					float x1, y1, x2, y2, _dx, _dy;
					_dx = x;
					_dy = y;
					if (_dx+w >= cfg.screen_w) {
						x2 = w - ((_dx+w) - cfg.screen_w - 1);
					}
					else {
						x2 = w;
					}
					if (_dy+h >= cfg.screen_h) {
						y2 = h - ((_dy+h) - cfg.screen_h - 1);
					}
					else {
						y2 = h;
					}
					if (_dx < 0) {
						x1 = -_dx;
						_dx = 0;
					}
					else {
						x1 = 0;
					}
					if (_dy < 0) {
						y1 = -_dy;
						_dy = 0;
					}
					else {
						y1 = 0;
					}
					al_draw_bitmap_region(sub->bitmap, x1, y1, x2-x1, y2-y1, _dx, _dy, 0);
				}
			}
			al_hold_bitmap_drawing(false);
		}
		if (draw_layer_0_tinted) {
			Shader::use(NULL);
		}
		draw_entities(layer);
		if ((int)layer == entity_layer) {
			// draw burrow holes
			for (size_t i = 0; i < burrow_holes.size(); i++) {
				int frame = (al_get_time() - burrow_holes[i].start_time) / (burrow_holes[i].end_time - burrow_holes[i].start_time) * 8;
				if (frame > 3) frame = 3;
				al_draw_bitmap(burrow_hole_bmps[frame]->bitmap, burrow_holes[i].pos.x-al_get_bitmap_width(burrow_hole_bmps[0]->bitmap)/2-get_top().x, burrow_holes[i].pos.y-al_get_bitmap_height(burrow_hole_bmps[0]->bitmap)-get_top().y+4, 0);
			}
			for (size_t i = 0; i < static_burrow_holes.size(); i++) {
				al_draw_bitmap(burrow_hole_bmps[3]->bitmap, static_burrow_holes[i].pos.x-al_get_bitmap_width(burrow_hole_bmps[0]->bitmap)/2-get_top().x, static_burrow_holes[i].pos.y-al_get_bitmap_height(burrow_hole_bmps[0]->bitmap)-get_top().y+4, 0);
			}
			
			// Draw low particles
			int sz = particles.size();
			al_hold_bitmap_drawing(true);
			for (int i = 0; i < sz; i++) {
				Particle::Particle *p = particles[i];

				if (!p->hidden && !p->high) {
					p->draw();
				}
			}
			al_hold_bitmap_drawing(false);
		}
	}

	// Draw high particles
	sz = particles.size();
	al_hold_bitmap_drawing(true);
	for (int i = 0; i < sz; i++) {
		Particle::Particle *p = particles[i];

		if (!p->hidden && p->high) {
			p->draw();
		}
	}
	al_hold_bitmap_drawing(false);

	// draw parallax foreground
	for (unsigned int i = 0; i < parallax_bmps.size(); i++) {
		if (parallax_is_foreground[i]) {
			Wrap::Bitmap *b = parallax_bmps[i];
			int bmp_w = al_get_bitmap_width(b->bitmap);
			int bmp_h = al_get_bitmap_height(b->bitmap);
			int level_w = size.w;
			int diff1 = bmp_w - cfg.screen_w;
			int diff2 = level_w - cfg.screen_w;
			int xx = (float)dx/diff2 * diff1;
			int x = -xx;
			int y = -dy * ((float)bmp_h / size.h);
			al_draw_bitmap_region(
				b->bitmap,
				x,
				y,
				cfg.screen_w,
				cfg.screen_h,
				0,
				0,
				0
			);
		}
	}

	sz = entities.size();
	for (int i = 0; i < sz; i++) {
		entities[i]->ui_draw();
	}

	int count = 0;

	if (!draw_interface) {
		goto end_interface;
	}

	{
		Battle_Player *p = NULL;
		int sz = entities.size();
		for (int i = 0; i < sz; i++) {
			p = dynamic_cast<Battle_Player *>(entities[i]);
			if (p) {
				break;
			}
		}
		Wrap::Bitmap *profile = p->get_profile_bitmap();
		int stats_h = al_get_bitmap_height(profile->bitmap) + 6;
		Wrap::Bitmap *work_bitmap = engine->get_work_bitmap();
		ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
		al_set_target_bitmap(work_bitmap->bitmap);
		al_clear_to_color(al_map_rgba(0, 0, 0, 0));
		for (size_t i = 0; i < players.size(); i++) {
			Battle_Player *p = get_player(i);
			if (p) {
				draw_mini_battle_stats(p, MINI_BATTLE_STATS_W*count+5*(count+1), 5);
				count++;
			}
		}
		al_set_target_bitmap(old_target);
		al_draw_tinted_bitmap_region(
			work_bitmap->bitmap,
			al_color_name("black"),
			0, 0, cfg.screen_w, stats_h,
			1, 1,
			0
		);
		al_draw_bitmap_region(
			work_bitmap->bitmap,
			0, 0, cfg.screen_w, stats_h,
			0, 0,
			0
		);
	}

end_interface:
	if ((cart_battle || dynamic_cast<Runner_Loop *>(this)) && !battle_transition_done) {
		double p = al_get_time() - battle_start_time;
		if (p > 1) {
			battle_transition_done = true;
			if (!dynamic_cast<Runner_Loop *>(this)) {
				draw_interface = true;
			}
		}
		else {
			p = 1 - p;

			al_draw_filled_rectangle(0, 0, cfg.screen_w, cfg.screen_h, al_map_rgba_f(0, 0, 0, p));
		}
	}

	if (!do_not_make_screen_copy) {
		if (making_screen_copy) {
			making_screen_copy = false;
			al_set_target_bitmap(old_target);
		}
		else {
			making_screen_copy = true;
			draw();
		}
	}
}

void Battle_Loop::set_entity_layer(int layer)
{
	entity_layer = layer;
}

int Battle_Loop::get_entity_layer()
{
	return entity_layer;
}

void Battle_Loop::construct(void)
{
	do_not_make_screen_copy = false;

	draw_interface = true;

	x_offset_ratio = 1.0f;

	if (enemy_avatar) {
		level_name = enemy_avatar->get_level();
		script_name = enemy_avatar->get_script();
		boss_battle = enemy_avatar->is_boss_battle();

		reload_graphics();
	}
	else {
		script_name = "runner";
	}

	cart_battle = (level_name == "cart");

	for (int i = 0; i < 4; i++) {
		burrow_hole_bmps[i] = Wrap::load_bitmap("battle/misc_graphics/gopher_hole/" + General::itos(i+1) + ".cpi");
	}

	if (cart_battle) {
		cart_parallax[0] = Wrap::load_bitmap("battle/parallax/caverns-tracks-repeating.cpi");
		cart_parallax[1] = Wrap::load_bitmap("battle/parallax/caverns-jungle-transition.cpi");
		cart_parallax[2] = Wrap::load_bitmap("battle/parallax/jungle-tracks-repeating.cpi");
		cart_parallax[3] = Wrap::load_bitmap("battle/parallax/jungle-back.cpi");
	}

	entity_layer = 0;

	// load sfx
	engine->load_sample("sfx/enter_battle.ogg");
	engine->load_sample("sfx/enemy_die.ogg");
	engine->load_sample("sfx/hit.ogg");
	engine->load_sample("sfx/swing_heavy_metal.ogg");
	engine->load_sample("sfx/hit_heavy_metal.ogg");
	engine->load_sample("sfx/double_jump.ogg");
	engine->load_sample("sfx/enemy_jump.ogg");
	engine->load_sample("sfx/bomb.ogg");
	engine->load_sample("sfx/bow_and_arrow.ogg");
	engine->load_sample("sfx/open_circle.ogg");
	engine->load_sample("sfx/circle_scroll_left.ogg");
	engine->load_sample("sfx/circle_scroll_right.ogg");
	engine->load_sample("sfx/slash.ogg");
	engine->load_sample("sfx/throw_ability.ogg");
	engine->load_sample("sfx/kick.ogg");
	engine->load_sample("sfx/kick_hit.ogg");
	engine->load_sample("sfx/plant_shovel.ogg");
	engine->load_sample("sfx/plant_seed.ogg");
	engine->load_sample("sfx/plant_pop_up.ogg");
	engine->load_sample("sfx/plant_pop_down.ogg");
	engine->load_sample("sfx/plant_fire.ogg");
	engine->load_sample("sfx/ice_blast.ogg");
	engine->load_sample("sfx/fire.ogg");
	engine->load_sample("sfx/fire_hit.ogg");
	engine->load_sample("sfx/roll.ogg");
	engine->load_sample("sfx/heal_cast.ogg");
	engine->load_sample("sfx/heal_drop.ogg");
	engine->load_sample("sfx/poison_initial.ogg");
	if (cart_battle) {
		engine->load_sample("sfx/cart_wheel.ogg", false);
		engine->load_sample("sfx/cart_running.ogg", true);
		engine->play_sample("sfx/cart_running.ogg");
		pumping_cart = new Animation_Set();
		pumping_cart->load("misc_graphics/pumping_cart");
		cart_wheel = new Animation_Set();
		cart_wheel->load("misc_graphics/cart_wheel");
	}
}

Battle_Loop::Battle_Loop(std::vector<Player *> players, Enemy_Avatar *enemy_avatar, bool delete_enemy_avatar, Wrap::Bitmap *pre_battle_screen_bmp, Wrap::Bitmap **end_screenshot, std::string *end_player) :
	pathfinding_nodes(NULL),
	pathfinding_edges(NULL),
	pre_battle_screen_bmp(pre_battle_screen_bmp),
	first_run(true),
	enemy_avatar(enemy_avatar),
	active_player(-1),
	move_camera_to_zero(false),
	players(players),
	end_screenshot(end_screenshot),
	end_player(end_player),
	curr_id(0),
	switch_players(false),
	game_over_time(-1.0),
	delete_enemy_avatar(delete_enemy_avatar)
{
	construct();
}

Battle_Loop::~Battle_Loop(void)
{
	if (cart_battle || runner_loop) {
		engine->fade_out();
	}

	// Capture screenshot for transition
	if (cfg.low_graphics) {
		ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
		al_set_target_bitmap(screen_copy_bmp->bitmap);
		draw();
		al_set_target_bitmap(old_target);
	}
	*end_screenshot = screen_copy_bmp;
	
	if (engine->game_is_over()) {
		Wrap::destroy_bitmap(screen_copy_bmp);
	}

	*end_player = get_active_player()->get_name();

	Shader::destroy(bullet_time_shader);
	Shader::destroy(bullet_time_v_shader);

	// Transfer stats back to Players
	for (size_t i = 0; i < players.size(); i++) {
		Battle_Player *p = get_player(i);
		if (p) {
			Battle_Attributes &a1 = players[i]->get_battle_attributes();
			Battle_Attributes &a2 = p->get_attributes();
			a1 = a2;
		}
	}

	int sz = entities.size();
	for (int i = 0; i < sz; i++) {
		delete entities[i];
	}

	for (size_t i = 0; i < parallax_bmps.size(); i++) {
		Wrap::destroy_bitmap(parallax_bmps[i]);
	}

	delete[] pathfinding_nodes;
	delete[] pathfinding_edges;

	lua_close(lua_state);
	
	for (int i = 0; i < 4; i++) {
		Wrap::destroy_bitmap(burrow_hole_bmps[i]);
	}

	while (burrow_holes.size() > 0) {
		destroy_burrow_hole(burrow_holes, 0);
	}
	while (static_burrow_holes.size() > 0) {
		destroy_burrow_hole(static_burrow_holes, 0);
	}

	if (cart_battle) {
		for (int i = 0; i < 4; i++) {
			Wrap::destroy_bitmap(cart_parallax[i]);
		}
	}

	engine->destroy_sample("sfx/enter_battle.ogg");
	engine->destroy_sample("sfx/enemy_die.ogg");
	engine->destroy_sample("sfx/hit.ogg");
	engine->destroy_sample("sfx/swing_heavy_metal.ogg");
	engine->destroy_sample("sfx/hit_heavy_metal.ogg");
	engine->destroy_sample("sfx/double_jump.ogg");
	engine->destroy_sample("sfx/enemy_jump.ogg");
	engine->destroy_sample("sfx/bomb.ogg");
	engine->destroy_sample("sfx/bow_and_arrow.ogg");
	engine->destroy_sample("sfx/open_circle.ogg");
	engine->destroy_sample("sfx/circle_scroll_left.ogg");
	engine->destroy_sample("sfx/circle_scroll_right.ogg");
	engine->destroy_sample("sfx/slash.ogg");
	engine->destroy_sample("sfx/throw_ability.ogg");
	engine->destroy_sample("sfx/kick.ogg");
	engine->destroy_sample("sfx/kick_hit.ogg");
	engine->destroy_sample("sfx/plant_shovel.ogg");
	engine->destroy_sample("sfx/plant_seed.ogg");
	engine->destroy_sample("sfx/plant_pop_up.ogg");
	engine->destroy_sample("sfx/plant_pop_down.ogg");
	engine->destroy_sample("sfx/plant_fire.ogg");
	engine->destroy_sample("sfx/ice_blast.ogg");
	engine->destroy_sample("sfx/fire.ogg");
	engine->destroy_sample("sfx/fire_hit.ogg");
	engine->destroy_sample("sfx/roll.ogg");
	engine->destroy_sample("sfx/heal_cast.ogg");
	engine->destroy_sample("sfx/heal_drop.ogg");
	engine->destroy_sample("sfx/poison_initial.ogg");
	if (cart_battle) {
		engine->destroy_sample("sfx/cart_wheel.ogg");
		engine->destroy_sample("sfx/cart_running.ogg");
		delete pumping_cart;
		delete cart_wheel;
	}

	for (size_t i = 0; i < battle_data.size(); i++) {
		delete[] battle_data[i].data;
	}

	atlas_destroy(atlas);

	if (delete_enemy_avatar) {
		delete enemy_avatar;
	}

	sz = particles.size();
	for (int i = 0; i < sz; i++) {
		delete particles[i];
	}
	
	engine->clear_touches();
}

void Battle_Loop::init_lua(void)
{
	lua_state = luaL_newstate();

	Lua::open_lua_libs(lua_state);

	Lua::register_c_functions(lua_state);

	Lua::load_global_scripts(lua_state);

	unsigned char *bytes;
	
	bytes = General::slurp("battle/global.lua");
	if (bytes) {
		if (luaL_loadstring(lua_state, (char *)bytes)) {
			Lua::dump_lua_stack(lua_state);
			throw Error("Error loading global battle script.\n");
		}
	}
	delete[] bytes;

	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running global battle script.");
	}

	bytes = General::slurp("battle/scripts/" + script_name + ".lua");
	if (luaL_loadstring(lua_state, (char *)bytes)) {
		Lua::dump_lua_stack(lua_state);
		throw Error("Error loading battle script.\n");
	}
	delete[] bytes;

	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running battle script.");
	}

	Lua::call_lua(lua_state, "start", "s>", level_name.c_str());
	
	lua_getglobal(lua_state, "logic");
	script_has_logic = !lua_isnil(lua_state, -1);
	lua_pop(lua_state, 1);
}

// find close points between platforms and make "jump points"
void Battle_Loop::make_jump_points(void)
{
	General::Point<float> result;

	// add geometry as necessary
	for (size_t platform1 = 0; platform1 < geometry.size(); platform1++) {
		for (size_t platform2 = 0; platform2 < geometry.size(); platform2++) {
			if (platform2 <= platform1)
				continue;

			std::vector<int> finds;
			std::vector< General::Point<float> > find_pos;
			std::vector<int> insertion_points;

			for (size_t point1 = 0; point1 < geometry[platform1].size(); point1++) {
				for (size_t point2 = 0; point2 < geometry[platform2].size(); point2++) {
					if (point1 > 0) {
						General::Point<float> p1, p2;
						General::Point<float> p3(0, General::SMALL_FLOAT);
						General::Point<float> p4(0, General::BIG_FLOAT);
						p1 = geometry[platform1][point1-1];
						p2 = geometry[platform1][point1];
						p3.x = geometry[platform2][point2].x;
						p4.x = geometry[platform2][point2].x;
						if (checkcoll_line_line(&p1, &p2, &p3, &p4, &result)) {
							if (fabs(result.y-geometry[platform2][point2].y) <= 75) {
								finds.push_back(1);
								insertion_points.push_back(point1);
								find_pos.push_back(result);
							}
						}
					}
					if (point2 > 0) {
						General::Point<float> p1(0, General::SMALL_FLOAT);
						General::Point<float> p2(0, General::BIG_FLOAT);
						General::Point<float> p3, p4;
						p1.x = geometry[platform1][point1].x;
						p2.x = geometry[platform1][point1].x;
						p3 = geometry[platform2][point2-1];
						p4 = geometry[platform2][point2];
						if (checkcoll_line_line(&p1, &p2, &p3, &p4, &result)) {
							if (fabs(result.y-geometry[platform1][point1].y) <= 75) {
								finds.push_back(2);
								insertion_points.push_back(point2);
								find_pos.push_back(result);
							}
						}
					}
				}
			}

			int one_count = 0;
			int two_count = 0;

			for (size_t i = 0; i < insertion_points.size(); i++) {
				int found = finds[i];
				float x = find_pos[i].x;
				float y = find_pos[i].y;
				int insertion_point = insertion_points[i];
				// split is on platform1
				if (found == 1) {
					insertion_point += one_count;
					geometry[platform1].insert(geometry[platform1].begin()+insertion_point, General::Point<int>(x, y));
					one_count++;
				}
				// split is on platform2
				else {
					insertion_point += two_count;
					geometry[platform2].insert(geometry[platform2].begin()+insertion_point, General::Point<int>(x, y));
					two_count++;
				}
			}
		}
	}
	
	for (size_t platform1 = 0; platform1 < geometry.size(); platform1++) {
		jump_points.push_back(std::vector<Jump_Point>());
	}

	for (size_t platform1 = 0; platform1 < geometry.size(); platform1++) {
		for (size_t platform2 = 0; platform2 < geometry.size(); platform2++) {
			if (platform2 <= platform1)
				continue;
			int closest_point1;
			int closest_point2;
			float closest_x1;
			float closest_x2;
			float closest_y1;
			float closest_y2;
			bool first_higher = false;
			for (size_t point1 = 0; point1 < geometry[platform1].size(); point1++) {
				for (size_t point2 = 0; point2 < geometry[platform2].size(); point2++) {
					General::Point<float> p1 = geometry[platform1][point1];
					General::Point<float> p2 = geometry[platform2][point2];
					if (fabs(p1.y-p2.y) > 75) {
						continue;
					}

					if (p1.y < p2.y)
						first_higher = true;
					else
						first_higher = false;

					closest_point1 = point1;
					closest_point2 = point2;
					closest_x1 = p1.x;
					closest_y1 = p1.y;
					closest_x2 = p2.x;
					closest_y2 = p2.y;

					if (fabs(p1.x-p2.x) > 0.1) {
						if ((point1 == 0 && point2 == geometry[platform2].size()-1 && p1.x > p2.x) ||
								(point2 == 0 && point1 == geometry[platform1].size()-1 && p2.x > p1.x)) {
							General::Point<float> p1 = geometry[platform1][0];
							General::Point<float> p2 = geometry[platform1][geometry[platform1].size()-1];
							General::Point<float> p3 = geometry[platform2][0];
							General::Point<float> p4 = geometry[platform2][geometry[platform2].size()-1];
							General::Point<float> tmp;
							bool swapped1 = false, swapped2 = false;
							if (p1.x > p2.x) {
								tmp = p1;
								p1 = p2;
								p2 = tmp;
								swapped1 = true;
							}
							if (p3.x > p4.x) {
								tmp = p3;
								p3 = p4;
								p4 = tmp;
								swapped2 = true;
							}
							// check far end of one, near end of other and vice versa
							float dist1 = General::distance(p1.x, p1.y, p4.x, p4.y);
							float dist2 = General::distance(p2.x, p2.y, p3.x, p3.y);
							Jump_Point c;
							c.type = JUMP_HORIZONTAL;
							int pt1, pt2;
							float x1, y1;
							float x2, y2;
							if (dist1 < dist2) {
								if (dist1 > 75) {
									continue;
								}
								if (swapped1) {
									pt1 = geometry[platform1].size()-1;
									x1 = p2.x;
									y1 = p2.y;
								}
								else {
									pt1 = 0;
									x1 = p1.x;
									y1 = p1.y;
								}
								if (swapped2) {
									pt2 = 0;
									x2 = p3.x;
									y2 = p3.y;
								}
								else {
									pt2 = geometry[platform2].size()-1;
									x2 = p4.x;
									y2 = p4.y;
								}
							}
							else {
								if (dist2 > 100) {
									continue;
								}
								if (swapped1) {
									pt1 = 0;
									x1 = p1.x;
									y1 = p1.y;
								}
								else {
									pt1 = geometry[platform1].size()-1;
									x1 = p2.x;
									y1 = p2.y;
								}
								if (swapped2) {
									pt2 = geometry[platform2].size()-1;
									x2 = p4.x;
									y2 = p4.y;
								}
								else {
									pt2 = 0;
									x2 = p3.x;
									y2 = p3.y;
								}
							}
							c.x = x1;
							c.y = y1;
							c.point = pt1;
							c.dest_platform = platform2;
							c.dest_point = pt2;
							jump_points[platform1].push_back(c);
							//
							c.x = x2;
							c.y = y2;
							c.point = pt2;
							c.dest_platform = platform1;
							c.dest_point = pt1;
							jump_points[platform2].push_back(c);
						}
					}
					else {
						Jump_Point c;
						c.type = first_higher ? JUMP_DOWN : JUMP_UP;
						c.x = closest_x1;
						c.y = closest_y1;
						c.point = closest_point1;
						c.dest_platform = platform2;
						c.dest_point = closest_point2;
						jump_points[platform1].push_back(c);
						//
						c.type = first_higher ? JUMP_UP : JUMP_DOWN;
						c.x = closest_x2;
						c.y = closest_y2;
						c.point = closest_point2;
						c.dest_platform = platform1;
						c.dest_point = closest_point1;
						jump_points[platform2].push_back(c);
					}
				}
			}
		}
	}
}

void Battle_Loop::make_pathfinding_info(void)
{
	num_pathfinding_nodes = 0;
	num_pathfinding_edges = 0;
	for (size_t i = 0; i < geometry.size(); i++) {
		num_pathfinding_edges += 2 + (geometry[i].size()-2)*2;
		num_pathfinding_edges += jump_points[i].size();
		num_pathfinding_nodes += geometry[i].size();
	}
	// FIXME?!??!?!
	//num_pathfinding_edges -= geometry.size();
	pathfinding_nodes = new Battle_Pathfinder_Node[num_pathfinding_nodes];
	pathfinding_edges = new Battle_Pathfinder_Edge[num_pathfinding_edges];

	int n_count = 0;
	int e_count = 0;

	for (size_t platform = 0; platform < geometry.size(); platform++) {
		for (size_t point = 0; point < geometry[platform].size(); point++) {
			General::Point<int> &p = geometry[platform][point];
			Battle_Pathfinder_Node n;
			n.x = p.x;
			n.y = p.y;
			n.platform = platform;
			if (point > 0) {
				Battle_Pathfinder_Edge e;
				e.start = &pathfinding_nodes[n_count];
				e.end = &pathfinding_nodes[n_count-1];
				pathfinding_edges[e_count] = e;
				Battle_Pathfinder_Node::List_Data data;
				data.edge = &pathfinding_edges[e_count];
				data.jump_point = NULL;
				n.links.push_back(data);
				e_count++;
			}
			if (point < geometry[platform].size()-1) {
				Battle_Pathfinder_Edge e;
				e.start = &pathfinding_nodes[n_count];
				e.end = &pathfinding_nodes[n_count+1];
				pathfinding_edges[e_count] = e;
				Battle_Pathfinder_Node::List_Data data;
				data.edge = &pathfinding_edges[e_count];
				data.jump_point = NULL;
				n.links.push_back(data);
				e_count++;
			}
			for (size_t i = 0; i < jump_points[platform].size(); i++) {
				Jump_Point &jp = jump_points[platform][i];
				if ((unsigned)jp.point != point) {
					continue;
				}
				int dest_index = 0;
				for (int z = 0; z < jp.dest_platform; z++) {
					dest_index += geometry[z].size();;
				}
				dest_index += jp.dest_point;
				Battle_Pathfinder_Edge e;
				e.start = &pathfinding_nodes[n_count];
				e.end = &pathfinding_nodes[dest_index];
				pathfinding_edges[e_count] = e;
				Battle_Pathfinder_Node::List_Data data;
				data.edge = &pathfinding_edges[e_count];
				data.jump_point = &jp;
				n.links.push_back(data);
				e_count++;
			}
			pathfinding_nodes[n_count++] = n;
		}
	}
}

std::vector<Battle_Pathfinder_Node *> Battle_Loop::find_closest_nodes(General::Point<float> pos, int platform)
{
	std::vector<Battle_Pathfinder_Node *> nodes;
	float closest_dist = General::BIG_FLOAT;

	for (int i = 0; i < num_pathfinding_nodes; i++) {
		Battle_Pathfinder_Node &n = pathfinding_nodes[i];
		if (platform != -1 && n.platform != platform)
			continue;
		float dist = General::distance(pos.x, pos.y, n.x, n.y);
		if (dist < closest_dist) {
			closest_dist = dist;
			nodes.clear();
			nodes.push_back(&n);
		}
		else if (dist == closest_dist) {
			nodes.push_back(&n);
		}
	}

	return nodes;
}

std::vector<Battle_Pathfinder_Node> Battle_Loop::find_path(
	int start_platform, General::Point<float> start, int end_platform, General::Point<float> end)
{
	int closest1 = 0;
	float closest_dist1 = General::BIG_FLOAT;
	int closest2 = 0;
	float closest_dist2 = General::BIG_FLOAT;

	PSX = start.x;
	PSY = start.y;
	PEX = end.x;
	PEY = end.y;

	for (int i = 0; i < num_pathfinding_nodes; i++) {
		Battle_Pathfinder_Node &n = pathfinding_nodes[i];
		n.parent = NULL;
		n.cost_from_start = General::BIG_FLOAT/2;
		n.cost_to_goal = General::BIG_FLOAT/2;
		n.total_cost = General::BIG_FLOAT/2;

		float dist;
		if (start_platform < 0 || start_platform == n.platform) {
			dist = General::distance(start.x, start.y, n.x, n.y);
			if (dist < closest_dist1) {
				closest_dist1 = dist;
				closest1 = i;
			}
		}
		if (end_platform < 0 || end_platform == n.platform) {
			dist = General::distance(end.x, end.y, n.x, n.y);
			if (dist < closest_dist2) {
				closest_dist2 = dist;
				closest2 = i;
			}
		}
	}

	int start_node = closest1;
	int end_node = closest2;

	pathfinder = new Battle_Pathfinder(&pathfinding_nodes[start_node]);
	path = pathfinder->find_path(&pathfinding_nodes[end_node]);

	std::vector<Battle_Pathfinder_Node> result;

	if (path) {
		Battle_Pathfinder_Node *n = path;
		while (n) {
			result.push_back(*n);
			n = n->parent;
		}
	}

	delete pathfinder;

	return result;
}

Battle_Entity *Battle_Loop::get_entity(int entity_id)
{
	int sz = entities.size();
	for (int i = 0; i < sz; i++) {
		if (entities[i]->get_id() == entity_id)
			return entities[i];
	}
	sz = entities_to_add.size();
	for (int i = 0; i < sz; i++) {
		if (entities_to_add[i]->get_id() == entity_id)
			return entities_to_add[i];
	}

	return NULL;
}

Battle_Player *Battle_Loop::get_player(int player_id)
{
	int sz = entities.size();
	for (int i = 0; i < sz; i++) {
		Battle_Player *p = dynamic_cast<Battle_Player *>(entities[i]);
		if (p) {
			if (p->get_player_id() == player_id) {
				return p;
			}
		}
	}

	return NULL;
}

Battle_Player *Battle_Loop::get_active_player()
{
	return get_player(active_player);
}

std::vector<Battle_Entity *> &Battle_Loop::get_entities(void)
{
	return entities;
}

void Battle_Loop::add_parallax_bitmap(Wrap::Bitmap *bitmap, bool foreground)
{
	parallax_bmps.push_back(bitmap);
	parallax_is_foreground.push_back(foreground);
}

lua_State *Battle_Loop::get_lua_state(void)
{
	return lua_state;
}

int Battle_Loop::ai_get(lua_State *stack, int entity_id, std::string cmd)
{
	std::vector<std::string> c = General::split(cmd);

	int i = 0;
	if (c[i] == "entity_ids_by_name") {
		i++;
		std::string name = c[i++];
		int count = 0;
		int sz = entities.size();
		for (int j = 0; j < sz; j++) {
			Battle_Entity *be = entities[j];
			if (be && be->get_name() == name) {
				lua_pushnumber(stack, be->get_id());
				count++;
			}
		}
		return count;
	}
	else if (c[i] == "entity_positions_by_id") {
		i++;
		int count = 0;
		while (i < (int)c.size()) {
			int this_id = atoi(c[i++].c_str());
			Battle_Entity *e = get_entity(this_id);
			if (e) {
				General::Point<float> pos = e->get_position();
				lua_pushnumber(stack, pos.x);
				lua_pushnumber(stack, pos.y);
			}
			else {
				lua_pushnumber(stack, -1);
				lua_pushnumber(stack, -1);
			}
			count += 2;
		}
		return count;
	}
	else if (c[i] == "entity_distance") {
		i++;
		int to_id = atoi(c[i++].c_str());
		Battle_Entity *to = get_entity(to_id);
		Battle_Entity *this_entity = get_entity(entity_id);
		General::Point<float> to_pos = to->get_position();
		General::Point<float> entity_pos = this_entity->get_position();
		lua_pushnumber(
			stack,
			General::distance(to_pos.x, to_pos.y,
				entity_pos.x, entity_pos.y
			)
		);
		return 1;
	}
	else if (c[i] == "battle_area_size") {
		i++;
		lua_pushnumber(stack, size.w);
		lua_pushnumber(stack, size.h);
		return 2;
	}
	else if (c[i] == "hp") {
		i++;
		int eid = atoi(c[i++].c_str());
		Battle_Entity *be = get_entity(eid);
		if (be) {
			lua_pushnumber(stack, be->get_attributes().hp);
			return 1;
		}
	}
	else if (c[i] == "A_PLAYER") {
		i++;
		Battle_Entity *this_entity = get_entity(entity_id);
		if (this_entity) {
			Battle_AI *ai = this_entity->get_ai();
			if (ai) {
				int player_id = ai->get_A_PLAYER();
				if (player_id >= 0) {
					lua_pushnumber(stack, get_player(player_id)->get_id());
					return 1;
				}
			}
		}
	}
	else if (c[i] == "A_ENEMY") {
		i++;
		Battle_Entity *this_entity = get_entity(entity_id);
		if (this_entity) {
			Battle_AI *ai = this_entity->get_ai();
			if (ai) {
				int enemy_id = ai->get_A_ENEMY();
				if (enemy_id >= 0) {
					lua_pushnumber(stack, enemy_id);
					return 1;
				}
			}
		}
	}
	else if (c[i] == "nearest_enemy") {
		i++;
		Battle_Entity *this_entity = get_entity(entity_id);
		if (this_entity) {
			int nearest = -1;
			float dist = FLT_MAX;
			General::Point<float> pos = this_entity->get_position();
			int sz = entities.size();
			for (int j = 0; j < sz; j++) {
				Battle_Enemy *e = dynamic_cast<Battle_Enemy *>(entities[j]);
				if (e && !General::is_item(e->get_name())) {
					General::Point<float> epos = e->get_position();
					float d = General::distance(pos.x, pos.y, epos.x, epos.y);
					if (d < dist) {
						nearest = e->get_id();
						dist = d;
					}
				}
			}
			if (nearest >= 0) {
				lua_pushnumber(stack, nearest);
				return 1;
			}
		}
	}
	else if (c[i] == "right") {
		i++;
		int eid = atoi(c[i++].c_str());		
		Battle_Entity *be = get_entity(eid);
		if (be) {
			lua_pushboolean(stack, be->is_facing_right());
			return 1;
		}
	}

	return 0;
}

void start_battle(Enemy_Avatar *enemy_avatar, bool delete_enemy_avatar, Wrap::Bitmap **end_screenshot, std::string *end_player)
{
	engine->stop_timers();

	ALLEGRO_STATE st;
	al_store_state(&st, ALLEGRO_STATE_TARGET_BITMAP);
	Wrap::Bitmap *pre_battle_screen_bmp = Wrap::create_bitmap(
		cfg.screen_w, cfg.screen_h
	);
	al_set_target_bitmap(pre_battle_screen_bmp->bitmap);
	engine->set_draw_touch_controls(false);
	engine->draw_all(engine->get_loops(), true);
	engine->set_draw_touch_controls(true);
	al_restore_state(&st);

	Area_Loop *al = GET_AREA_LOOP;

	Battle_Loop *bl = new Battle_Loop(al->get_players(), enemy_avatar, delete_enemy_avatar, pre_battle_screen_bmp, end_screenshot, end_player);
	std::vector<Loop *> loops;
	loops.push_back(bl);
	engine->set_loops(loops, false);

	engine->start_timers();
}

void Battle_Loop::make_sectors()
{
	sectors.clear();
	sector_platforms.clear();

	for (int i = 0; i < size.w / SECTOR_SIZE; i++) {
		sectors.push_back(std::vector< std::pair< General::Point<float>, General::Point<float> > >());
		sector_platforms.push_back(std::vector<int>());
	}

	for (size_t plat = 0; plat < geometry.size(); plat++) {
		for (size_t i = 1; i < geometry[plat].size(); i++) {
			std::pair< General::Point<float>, General::Point<float> > p(
				geometry[plat][i-1],
				geometry[plat][i]
			);
			for (int j = 0; j < size.w / SECTOR_SIZE; j++) {
				General::Point<float> topleft(j * SECTOR_SIZE, 0);
				General::Point<float> bottomright((j+1) * SECTOR_SIZE, size.h);
				if (checkcoll_box_box(topleft, bottomright, p.first, p.second)) {
					sectors[j].push_back(p);
					sector_platforms[j].push_back(plat);
				}
			}
		}
	}
}

int Battle_Loop::get_next_id()
{
	return curr_id++;
}

General::Point<int> Battle_Loop::choose_random_start_platform(int entity_spacing, int edge_spacing)
{
	int x, y;
	int *sizes = new int[geometry.size()];
	int grand_total = 0;
	for (size_t i = 0; i < geometry.size(); i++) {
		int total = 0;
		for (size_t j = 1; j < geometry[i].size(); j++) {
			General::Point<int> &p1 = geometry[i][j-1];
			General::Point<int> &p2 = geometry[i][j];
			total += General::distance(p1.x, p1.y, p2.x, p2.y);
		}
		sizes[i] = total;
		grand_total += total;
	}
	
	int start = General::rand() % (int)grand_total;

	while (true) {
		bool found = false;

		int plat = 0;
		int n = start;
		for (; plat < (int)geometry.size()-1; plat++) {
			if (n < sizes[plat]) {
				break;
			}
			n -= sizes[plat];
		}

		int lowest = INT_MAX;
		for (size_t i = 0; i < geometry[plat].size(); i++) {
			if (geometry[plat][i].x < lowest) {
				lowest = geometry[plat][i].x;
			}
		}


		int plat_size = sizes[plat];
		int good_size = plat_size - edge_spacing*2;

		if (good_size > 0) {
			int tmp = General::rand() % good_size + edge_spacing;
			x = tmp + lowest;
			y = -1;
			for (size_t i = 1; i < geometry[plat].size(); i++) {
				General::Point<int> &p1 = geometry[plat][i-1];
				General::Point<int> &p2 = geometry[plat][i];
				float dist = General::distance(p1.x, p1.y, p2.x, p2.y);
				if (dist >= tmp) {
					float p = tmp / dist;
					y = p1.y + p * (p2.y - p1.y);
					break;
				}
				tmp -= dist;
			}
			if (y >= 0) {
				// Check spacing with entities
				found = true;
				for (size_t i = 0; i < entities.size(); i++) {
					Battle_Entity *be = entities[i];
					if (be) {
						General::Point<float> entity_pos = be->get_position();
						if (General::distance(x, y, entity_pos.x, entity_pos.y) < entity_spacing) {
							found = false;
							break;
						}
					}
				}
			}
		}

		if (!found) {
			start += 10;
			start %= grand_total;
		}
		else {
			break;
		}
	}

	delete[] sizes;

	return General::Point<int>(x, y);
}

void Battle_Loop::set_switch_players(bool switch_players)
{
	this->switch_players = switch_players;
}

bool Battle_Loop::battle_transition_is_done()
{
	return battle_transition_done;
}

void Battle_Loop::set_game_over_time(double game_over_time)
{
	this->game_over_time = game_over_time;
}

void Battle_Loop::destroy_burrow_hole(std::vector<Burrow_Hole> &v, int index)
{
	v.erase(v.begin()+index);
}

void Battle_Loop::add_burrow_hole(General::Point<float> pos, double end_time)
{
	Burrow_Hole bh;
	bh.pos = pos;
	bh.start_time = al_get_time();
	bh.end_time = end_time;
	std::vector<std::string> v;
	v.push_back("dirt_chunk");
	bh.pgid = engine->add_particle_group("fading_outward_shooting", 0, PARTICLE_HURT_NONE, v);
	burrow_holes.push_back(bh);
}

void Battle_Loop::add_static_burrow_hole(General::Point<float> pos, double end_time)
{
	Burrow_Hole bh;
	bh.pos = pos;
	bh.end_time = end_time;
	bh.pgid = -1;
	static_burrow_holes.push_back(bh);
}

float Battle_Loop::get_distance_from_nearest_edge(int id)
{
	Battle_Entity *be = get_entity(id);
	if (be) {
		int plat = be->get_platform();
		if (plat < 0) {
			return -1;
		}
		else {
			std::vector< std::vector< General::Point<int> > > &geo = get_geometry();
			General::Point<int> p1 = geo[plat][0];
			General::Point<int> p2 = geo[plat][geo[plat].size()-1];
			General::Point<float> pos = be->get_position();
			float d1 = General::distance(pos.x, pos.y, p1.x, p1.y);
			float d2 = General::distance(pos.x, pos.y, p2.x, p2.y);
			return MIN(d1, d2);
		}
	}
	return -1;
}

int Battle_Loop::find_battle_data(std::string name)
{
	for (size_t i = 0; i < battle_data.size(); i++) {
		if (name == battle_data[i].name) {
			return i;
		}
	}

	return -1;
}

void Battle_Loop::add_battle_data(std::string name, int size, double *data)
{
	Battle_Data d;
	d.name = name;
	d.size = size;
	d.data = data;
	battle_data.push_back(d);
}

int Battle_Loop::get_battle_data_size(std::string name)
{
	int i = find_battle_data(name);
	if (i >= 0) {
		return battle_data[i].size;
	}
	else {
		return 0;
	}
}

double Battle_Loop::get_battle_data(std::string name, int index)
{
	int i = find_battle_data(name);
	if (i >= 0 && index < battle_data[i].size) {
		return battle_data[i].data[index];
	}
	return 0.0;
}

bool Battle_Loop::reposition_entity_in_vector(int id, int before)
{
	Battle_Entity *e = get_entity(id);
	if (e) {
		std::vector<Battle_Entity *>::iterator it = std::find(entities.begin(), entities.end(), e);
		if (it != entities.end()) {
			entities.erase(it);
			if (before == -1) {
				entities.push_back(e);
			}
			else {
				entities.insert(entities.begin()+before, e);
			}
			return true;
		}
	}

	return false;
}

std::vector<bool> &Battle_Loop::get_platform_solid()
{
	return platform_solid;
}

void Battle_Loop::destroy_graphics()
{
	atlas_destroy(atlas);

	Wrap::destroy_bitmap(screen_copy_bmp);
}

void Battle_Loop::reload_graphics()
{
	ALLEGRO_FILE *f = engine->get_cpa()->load("battle/levels/" + level_name);

	size.w = General::read32bits(f);
	size.h = General::read32bits(f);

	int ntiles = General::read32bits(f);
	std::vector<std::string> names;
	for (int i = 0; i < ntiles; i++) {
		names.push_back(General::readNString(f));
	}

	std::vector<Wrap::Bitmap *> bmps;
	atlas = atlas_create(
		1024, 1024, ATLAS_REPEAT_EDGES, 1, false, false
	);
	for (int i = 0; i < ntiles; i++) {
		Wrap::Bitmap *bmp = Wrap::load_bitmap("battle/tiles/" + names[i] + ".cpi");
		atlas_add(atlas, bmp, i);
		bmps.push_back(bmp);
	}
	for (int i = 0; i < ntiles; i++) {
		bool add;
		std::string name;
		int id;
		if (names[i] == "cart_left") {
			add = true;
			name = "cart_left2";
			id = 1000;
		}
		else if (names[i] == "cart_right") {
			add = true;
			name = "cart_right2";
			id = 1001;
		}
		else {
			add = false;
		}
		if (add) {
			Wrap::Bitmap *bmp = Wrap::load_bitmap("battle/tiles/" + name + ".cpi");
			atlas_add(atlas, bmp, id);
			bmps.push_back(bmp);
		}
	}
	atlas_finish(atlas);

	General::log_message("Num atlas sheets = " + General::itos(atlas_get_num_sheets(atlas)));

	for (size_t i = 0; i < bmps.size(); i++) {
		Wrap::destroy_bitmap(bmps[i]);
	}

	int nlayers = General::read32bits(f);

	tiles.clear();

	for (int l = 0; l < nlayers; l++) {
		tiles.push_back(std::vector<Tile>());
		int num = General::read32bits(f);
		for (int i = 0; i < num; i++) {
			int id = General::read32bits(f);
			int x = General::read32bits(f);
			int y = General::read32bits(f);
			Tile tile;
			if (names[id] == "cart_left") {
				tile.ids.push_back(id);
				tile.ids.push_back(1000);
				tile.delay = 100;
			}
			else if (names[id] == "cart_right") {
				tile.ids.push_back(id);
				tile.ids.push_back(1001);
				tile.delay = 100;
			}
			else {
				tile.id = id;
			}
			tile.x = x;
			tile.y = y;
			tiles[l].push_back(tile);
		}
	}

	int nlines = General::read32bits(f);
	std::vector< std::vector< General::Point<int> > > geo;
	for (int i = 0; i < nlines; i++) {
		geo.push_back(std::vector< General::Point<int> >());
		int pts = General::read32bits(f);
		for (int p = 0; p < pts; p++) {
			int x = General::read32bits(f);
			int y = General::read32bits(f);
			geo[i].push_back(General::Point<int>(x, y));
		}
		std::sort(geo[i].begin(), geo[i].end(), General::compare_points_x<int>);
		platform_solid.push_back(true);
	}
	set_geometry(geo);

	al_fclose(f);

	screen_copy_bmp = Wrap::create_bitmap_no_preserve(cfg.screen_w, cfg.screen_h);
	making_screen_copy = true;
}

void Battle_Loop::add_particle(Particle::Particle *p)
{
	particles.push_back(p);
}

void Battle_Loop::remove_particle(int id)
{
	int sz = particles.size();
	for (int i = 0; i < sz; i++) {
		if (particles[i]->get_id() == id) {
			particles[i]->set_delete_me(true);
		}
	}
}

Particle::Particle *Battle_Loop::get_particle(int id)
{
	int sz = particles.size();
	for (int i = 0; i < sz; i++) {
		if (particles[i]->get_id() == id) {
			return particles[i];
		}
	}

	return NULL;
}

void Battle_Loop::add_entity(Battle_Entity *entity)
{
	entities_to_add.push_back(entity);
}

bool Battle_Loop::is_cart_battle()
{
	return cart_battle;
}
