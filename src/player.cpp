#include "crystalpicnic.h"
#include "player.h"
#include "area_manager.h"
#include "game_specific_globals.h"
#include "noop_character_role.h"

const float Player::PAN_SCALE = 10.0;

void Player::set_position(General::Point<float> pos)
{
	if (role) {
		character_set_position(pos);
	}
	else {
		Map_Entity::set_position(pos);
	}
}

void Player::reset(void)
{
	Map_Entity::reset();
}

void Player::draw(void)
{
	if (role) {
		character_draw();
	}
	else {
		Map_Entity::draw();
	}
}

void Player::handle_event(ALLEGRO_EVENT *event)
{
	if (input_disabled || role) {
		return;
	}
	
	bool change_axis = true;
	if (no_direction_change_end_time > al_current_time()) {
		change_axis = false;
	}
	/* Make sure directionals and run key stay across areas */
	else if (polled_joystick == false) {
		polled_joystick = true;
		if (al_is_joystick_installed()) {
			int numjoy = al_get_num_joysticks();
			for (int i = 0; i < numjoy; i++) {
				ALLEGRO_JOYSTICK *joy = al_get_joystick(i);
				if (al_get_joystick_num_buttons(joy) != 0) {
					ALLEGRO_JOYSTICK_STATE joystate;
					al_get_joystick_state(joy, &joystate);
					float x = joystate.stick[0].axis[0];
					float y = joystate.stick[0].axis[1];
					if (joystate.button[cfg.joy_dpad_l]) {
						x = -1;
					}
					if (joystate.button[cfg.joy_dpad_r]) {
						x = 1;
					}
					if (joystate.button[cfg.joy_dpad_u]) {
						y = -1;
					}
					if (joystate.button[cfg.joy_dpad_d]) {
						y = 1;
					}
					ALLEGRO_EVENT e;
					e.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
					e.joystick.stick = 0;
					e.joystick.axis = 0;
					e.joystick.pos = x;
					handle_event(&e);
					e.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
					e.joystick.stick = 0;
					e.joystick.axis = 1;
					e.joystick.pos = y;
					handle_event(&e);
				}
			}
		}
		if (al_is_keyboard_installed()) {
			ALLEGRO_KEYBOARD_STATE kbstate;
			al_get_keyboard_state(&kbstate);
			ALLEGRO_EVENT e;
			if (al_key_down(&kbstate, cfg.key_run)) {
				e.type = ALLEGRO_EVENT_KEY_DOWN;
				e.keyboard.keycode = cfg.key_run;
				handle_event(&e);
			}
			if (al_key_down(&kbstate, cfg.key_left)) {
				e.type = ALLEGRO_EVENT_KEY_DOWN;
				e.keyboard.keycode = cfg.key_left;
				handle_event(&e);
			}
			if (al_key_down(&kbstate, cfg.key_right)) {
				e.type = ALLEGRO_EVENT_KEY_DOWN;
				e.keyboard.keycode = cfg.key_right;
				handle_event(&e);
			}
			if (al_key_down(&kbstate, cfg.key_up)) {
				e.type = ALLEGRO_EVENT_KEY_DOWN;
				e.keyboard.keycode = cfg.key_up;
				handle_event(&e);
			}
			if (al_key_down(&kbstate, cfg.key_down)) {
				e.type = ALLEGRO_EVENT_KEY_DOWN;
				e.keyboard.keycode = cfg.key_down;
				handle_event(&e);
			}
		}
	}

	float pan_factor;

	if (panning) {
		pan_factor = PAN_SCALE;
	}
	else {
		pan_factor = 1.0;
	}

	switch (event->type) {
		case ALLEGRO_EVENT_KEY_UP:
			if (event->keyboard.keycode == cfg.key_run) {
				input[BUTTON1] = 0;
				input[X] = General::sign(input[X]) * get_run_factor() * pan_factor;
				input[Y] = General::sign(input[Y]) * get_run_factor() * pan_factor;
			}
			if (event->keyboard.keycode == cfg.key_left) {
				if (input[X] < 0.0f)
					input[X] = 0.0f;
				input[Y] = General::sign(input[Y]) * get_run_factor() * pan_factor;
				if (input[Y] < 0.0f) {
					direction = General::DIR_N;
				}
				else if (input[Y] > 0.0f) {
					direction = General::DIR_S;
				}
			}
			if (event->keyboard.keycode == cfg.key_right) {
				if (input[X] > 0.0f)
					input[X] = 0.0f;
				input[Y] = General::sign(input[Y]) * get_run_factor() * pan_factor;
				if (input[Y] < 0.0f) {
					direction = General::DIR_N;
				}
				else if (input[Y] > 0.0f) {
					direction = General::DIR_S;
				}
			}
			if (event->keyboard.keycode == cfg.key_up) {
				if (input[Y] < 0.0f)
					input[Y] = 0.0f;
				input[X] = General::sign(input[X]) * get_run_factor() * pan_factor;
				if (input[X] < 0.0f) {
					direction = General::DIR_W;
				}
				else if (input[X] > 0.0f) {
					direction = General::DIR_E;
				}
			}
			if (event->keyboard.keycode == cfg.key_down) {
				if (input[Y] > 0.0f)
					input[Y] = 0.0f;
				input[X] = General::sign(input[X]) * get_run_factor() * pan_factor;
				if (input[X] < 0.0f) {
					direction = General::DIR_W;
				}
				else if (input[X] > 0.0f) {
					direction = General::DIR_E;
				}
			}
			for (int i = 0; i < 4; i++) {
				if (event->keyboard.keycode == cfg.key_ability[i]) {
					execute_ability(get_selected_abilities(false, false, false)[i], false);
				}
			}
			break;
		case ALLEGRO_EVENT_KEY_DOWN:
			if (event->keyboard.keycode == cfg.key_run) {
				input[BUTTON1] = 1;
				input[X] = General::sign(input[X]) * get_run_factor() * pan_factor;
				input[Y] = General::sign(input[Y]) * get_run_factor() * pan_factor;
			}
			if (event->keyboard.keycode == cfg.key_left) {
				if (input[Y] != 0.0f) {
					input[Y] = General::sign(input[Y]) * 0.71f * get_run_factor() * pan_factor;
					input[X] = -0.71f * get_run_factor() * pan_factor;
					if (input[Y] < 0.0f) {
						direction = General::DIR_N;
					}
					else {
						direction = General::DIR_S;
					}
				}
				else {
					input[X] = -1.0f * get_run_factor() * pan_factor;
					direction = General::DIR_W;
				}
			}
			if (event->keyboard.keycode == cfg.key_right) {
				if (input[Y] != 0.0f) {
					input[Y] = General::sign(input[Y]) * 0.71f * get_run_factor() * pan_factor;
					input[X] = 0.71f * get_run_factor() * pan_factor;
					if (input[Y] < 0.0f) {
						direction = General::DIR_N;
					}
					else {
						direction = General::DIR_S;
					}
				}
				else {
					input[X] = 1.0f * get_run_factor() * pan_factor;
					direction = General::DIR_E;
				}
			}
			if (event->keyboard.keycode == cfg.key_up) {
				if (input[X] != 0.0f) {
					input[X] = General::sign(input[X]) * 0.71f * get_run_factor() * pan_factor;
					input[Y] = -0.71f * get_run_factor() * pan_factor;
					if (input[X] < 0.0f) {
						direction = General::DIR_W;
					}
					else {
						direction = General::DIR_E;
					}
				}
				else {
					input[Y] = -1.0f * get_run_factor() * pan_factor;
					direction = General::DIR_N;
				}
			}
			if (event->keyboard.keycode == cfg.key_down) {
				if (input[X] != 0.0f) {
					input[X] = General::sign(input[X]) * 0.71f * get_run_factor() * pan_factor;
					input[Y] = 0.71f * get_run_factor() * pan_factor;
					if (input[X] < 0.0f) {
						direction = General::DIR_W;
					}
					else {
						direction = General::DIR_E;
					}
				}
				else {
					input[Y] = 1.0f * get_run_factor() * pan_factor;
					direction = General::DIR_S;
				}
			}
			for (int i = 0; i < 4; i++) {
				if (event->keyboard.keycode == cfg.key_ability[i]) {
					execute_ability(get_selected_abilities(false, false, false)[i], true);
				}
			}
			break;
	}

	if (event->type == ALLEGRO_EVENT_JOYSTICK_AXIS && event->joystick.stick == 0) {
		if (event->joystick.axis == 0) {
			input[X] = event->joystick.pos * pan_factor;
		}
		else {
			input[Y] = event->joystick.pos * pan_factor;
		}

		if (fabs(input[X]) > 0.2f || fabs(input[Y]) > 0.2f) {
			if (fabs(input[Y]) > fabs(input[X])) {
				if (input[Y] < 0.0f) {
					direction = General::DIR_N;
				}
				else {
					direction = General::DIR_S;
				}
			}
			else {
				if (input[X] < 0.0f) {
					direction = General::DIR_W;
				}
				else {
					direction = General::DIR_E;
				}
			}
		}
		if (General::distance(0, 0, input[X], input[Y]) > 0.71f) {
			input[BUTTON1] = 1;
		}
		else {
			input[BUTTON1] = 0;
		}
	}
	if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP) {
		for (int i = 0; i < 4; i++) {
			if (event->joystick.button == cfg.joy_ability[i]) {
				execute_ability(get_selected_abilities(false, false, false)[i], false);
			}
		}
	}
	if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
		for (int i = 0; i < 4; i++) {
			if (event->joystick.button == cfg.joy_ability[i]) {
				execute_ability(get_selected_abilities(false, false, false)[i], true);
			}
		}
	}

	if (!change_axis)
		input[X] = input[Y] = 0;

	if (fabs(input[X]) < 0.2) input[X] = 0;
	if (fabs(input[Y]) < 0.2) input[Y] = 0;

	set_facing_right(direction);

	if (!attacking && !panning && back_home && no_direction_change_end_time <= al_current_time()) {
		std::string base;
		std::string suffix;
		if (direction == General::DIR_N) {
			suffix = "-up";
		}
		else if (direction == General::DIR_S) {
			suffix = "-down";
		}
		else {
			suffix = "";
		}
		if (input[X] != 0.0f || input[Y] != 0.0f) {
			float magnitude = sqrt(input[X]*input[X] + input[Y]*input[Y]);
			if (input[BUTTON1] && magnitude > 0.5f) {
				base = "run";
			}
			else {
				base = "walk";
			}
		}
		else {
			base = "idle";
		}
		anim_set->set_sub_animation(base + suffix);
		if (weapon_anim_set) {
			weapon_anim_set->set_sub_animation(anim_set->get_sub_animation_name());
		}
	}
}

void Player::logic(void)
{
	if (!input_disabled && !role) {
		engine->set_can_move(true);
	}

	if (role && dynamic_cast<NoOp_Character_Role *>(role) == NULL) {
		character_logic();
		return;
	}

	if (attacking) {
		if (anim_set->get_current_animation()->is_finished()) {
			attacking = false;
			anim_set->reset();
			update_direction(false);
			if (weapon_anim_set) {
				weapon_anim_set->reset();
				weapon_anim_set->set_sub_animation(anim_set->get_sub_animation_name());
			}
			called_attacked = false;
		}
		else if (!called_attacked && weapon_anim_set && strstr(weapon_anim_set->get_sub_animation_name().c_str(), "attack")) {
			std::list<Map_Entity *> collided;
			collided = area->entity_is_colliding(this, pos, true);
			if (collided.size() > 0) {
				called_attacked = true;
				std::list<Map_Entity *>::iterator it;
				for (it = collided.begin(); it != collided.end(); it++) {
					Map_Entity *e = *it;
					int e_id = e->get_id();
					lua_State *lua_state = area->get_lua_state();
					Lua::call_lua(lua_state, "attacked", "ii>", id, e_id);
				}
			}
		}
	}

	if (panning) {
		Area_Loop *loop = General::find_in_vector<Area_Loop *, Loop *>(engine->get_loops());

		if (loop) {
			float new_x  = player_pan_pos.x + input[X];
			float new_y  = player_pan_pos.y + input[Y];

			float change_x = 0;
			float change_y = 0;

			General::Size<int> pixel_size = loop->get_area()->get_pixel_size();
			int area_w = pixel_size.w;
			int area_h = pixel_size.h;

			if (((new_x > cfg.screen_w/2) || input[X] >= 0) &&
			((new_x < (area_w-cfg.screen_w/2)) || input[X] <= 0)) {
				change_x = input[X];
			}
			if (((new_y > cfg.screen_h/2) || input[Y] >= 0) &&
			((new_y < (area_h-cfg.screen_h/2)) || input[Y] <= 0)) {
				change_y = input[Y];
			}

			Camera *camera = cameras[0]; // area window
			General::Point<float> tmp(change_x, change_y);
			camera->move_camera(tmp, player_pan_pos, true);
			pan_movement_sum.x += change_x;
			pan_movement_sum.y += change_y;
			player_pan_pos.x += change_x;
			player_pan_pos.y += change_y;
		}
	}
	else if (!back_home) {
		bool home_x;
		bool home_y;
		float dx = pan_movement_sum.x-pan_amount_moved_back.x;
		if (fabs(dx) > PAN_SCALE*2) {
			dx = General::sign(dx)*(PAN_SCALE*2);
			home_x = false;
		}
		else {
			home_x = true;
		}
		float dy = pan_movement_sum.y-pan_amount_moved_back.y;
		if (fabs(dy) > PAN_SCALE*2) {
			dy = General::sign(dy)*(PAN_SCALE*2);
			home_y = false;
		}
		else {
			home_y = true;
		}
		Camera *camera = cameras[0]; // area window
		General::Point<float> tmp(-dx, -dy);
		camera->move_camera(tmp, player_pan_pos, true);
		player_pan_pos.x -= dx;
		player_pan_pos.y -= dy;
		pan_amount_moved_back.x += dx;
		pan_amount_moved_back.y += dy;

		if (home_x && home_y) {
			back_home = true;
		}
	}
	else {
		Map_Entity::logic();
		if (weapon_anim_set) {
			weapon_anim_set->update();
		}
	}
}

bool Player::load_weapon(std::string weapon_name)
{
	if (weapon_anim_set) {
		delete weapon_anim_set;
		weapon_anim_set = NULL;
	}
	// FIXME: bg / fgfx delete
	
	battle_attributes.equipment.weapon.name = weapon_name;

	if (weapon_name == "") {
		return true;
	}
	
	std::string path = "map_entities/" + name;
	std::string weapon_path = "battle/weapons/" + weapon_name + "/main";

	bool success = load_weapon_animations(weapon_path, path);
 	if (!success) {
		return false;
 	}

	weapon_bones.clear();

	Bones::load(weapon_anim_set, path + "/info.xml", weapon_path, weapon_bones);

	return true;
}

bool Player::load(void)
{
	Lua::restore_battle_attributes(this);

	bool success = Map_Entity::load();
	if (!success) {
		return false;
	}

	std::string weapname = battle_attributes.equipment.weapon.name;
	if (!load_weapon(weapname)) {
		return false;
	}

	update_direction(false);
	if (weapon_anim_set) {
		weapon_anim_set->set_sub_animation(anim_set->get_sub_animation_name());
	}

	return true;
}

Player::Player(std::string name) :
	Character_Map_Entity(name),
	panning(false),
	back_home(true),
	no_direction_change_end_time(0),
	called_attacked(false),
	attacking(false),
	magic_regen_count(0.0f),
	polled_joystick(false)
{
	for (int i = 0; i < 4; i++) {
		battle_abilities.push_back("");
	}
}

Player::~Player(void)
{
	delete weapon_anim_set;
}

void Player::set_panning(bool panning)
{
	input[X] = input[Y] = 0.0;

	if (panning) {
		back_home = false;
		player_pan_pos = pos;
		pan_movement_sum.x = pan_movement_sum.y = 0;
		pan_amount_moved_back.x = pan_amount_moved_back.y = 0;
	}
	
	this->panning = panning;
}

void Player::start_area(void)
{
	input[X] = input[Y] = 0;
	no_direction_change_end_time = al_current_time()+0.25;
	polled_joystick = false;
}

bool Player::is_panning(void)
{
	return panning;
}

void Player::activate(void)
{
	Area_Loop *loop = General::find_in_vector<Area_Loop *, Loop *>(engine->get_loops());
	if (loop) {
		Area_Manager *area = loop->get_area();
		Map_Entity *e = area->activate(this, pos, 16, 40);
		if (e) {
			lua_State *stack = area->get_lua_state();
			lua_getglobal(stack, "activate");
			if (!lua_isnil(stack, -1)) {
				lua_pop(stack, 1);
				Lua::call_lua(stack, "activate", "ii>", id, e->get_id());
			}
			else {
				lua_pop(stack, 1);
			}
		}
	}
}

std::vector<Bones::Bone> &Player::get_current_weapon_bones()
{
	if (weapon_anim_set) {
		std::string sub_name = weapon_anim_set->get_sub_animation_name();
		int frame = weapon_anim_set->get_current_animation()->get_current_frame_num();
		std::pair<std::string, int> p = std::pair<std::string, int>(sub_name, frame);
		return weapon_bones[p];
	}
	else {
		std::string sub_name = anim_set->get_sub_animation_name();
		int frame = anim_set->get_current_animation()->get_current_frame_num();
		std::pair<std::string, int> p = std::pair<std::string, int>(sub_name, frame);
		return bones[p];
	}
}

bool Player::is_attacking()
{
	return attacking;
}

void Player::set_attacking(bool attacking)
{
	this->attacking = attacking;
}

Battle_Attributes &Player::get_battle_attributes()
{
	return battle_attributes;
}

std::vector<std::string> Player::get_abilities()
{
	std::vector<std::string> v;

	v.push_back("ATTACK");
	v.push_back("USE");

	if (name == "egbert") {
		if (battle_attributes.abilities.abilities[2] > 0) {
			v.push_back("ICE");
		}
		if (battle_attributes.abilities.abilities[1] > 0) {
			v.push_back("THROW");
		}
		if (battle_attributes.abilities.abilities[0] > 0) {
			v.push_back("SLASH");
		}
	}
	else if (name == "frogbert") {
		if (battle_attributes.abilities.abilities[0] > 0) {
			v.push_back("KICK");
		}
		if (battle_attributes.abilities.abilities[1] > 0) {
			v.push_back("PLANT");
		}
		if (battle_attributes.abilities.abilities[2] > 0) {
			v.push_back("FIRE");
		}
	}
	else { // bisou
		if (battle_attributes.abilities.abilities[0] > 0) {
			v.push_back("ROLL");
		}
		if (battle_attributes.abilities.abilities[1] > 0) {
			v.push_back("BURROW");
		}
		if (battle_attributes.abilities.abilities[2] > 0) {
			v.push_back("HEAL");
		}
	}

	return v;
}

std::vector<std::string> Player::get_selected_abilities(bool battle, bool runner, bool cart_battle)
{
	if (battle) {
		if (runner) {
			std::vector<std::string> selected_abilities;
			selected_abilities.push_back("");
			selected_abilities.push_back("");
			selected_abilities.push_back("");
			selected_abilities.push_back("JUMP");
			return selected_abilities;
		}
		else if (cart_battle) {
			std::vector<std::string> selected_abilities;
			selected_abilities.push_back("USE");
			selected_abilities.push_back("ATTACK");
			selected_abilities.push_back("");
			selected_abilities.push_back("JUMP");
			return selected_abilities;
		}
		return battle_abilities;
	}
	else {
		std::vector<std::string> selected_abilities;
		selected_abilities.push_back("");
		if (name == "frogbert") {
			selected_abilities.push_back("JUMP");
		}
		else if (name == "egbert") {
			if (engine->milestone_is_complete("learned_chop")) {
				selected_abilities.push_back("CHOP");
			}
			else {
				selected_abilities.push_back("");
			}
		}
		else {
			selected_abilities.push_back("");
		}
		selected_abilities.push_back("");
		selected_abilities.push_back("USE");
		return selected_abilities;
	}
}

void Player::start_attack()
{
	engine->play_sample("sfx/swing_weapon.ogg");
	attacking = true;
	std::string anim_name;
	if (direction == General::DIR_N) {
		anim_name = "attack-up";
	}
	else if (direction == General::DIR_S) {
		anim_name = "attack-down";
	}
	else {
		anim_name = "attack";
	}
	anim_set->set_sub_animation(anim_name);
	anim_set->reset();
	if (weapon_anim_set) {
		weapon_anim_set->set_sub_animation(anim_name);
		weapon_anim_set->reset();
	}
}

void Player::execute_ability(std::string ability, bool onoff)
{
	if (ability == "USE" && onoff) {
		activate();
	}
	else if (ability == "CHOP" && onoff) {
		if (!attacking) {
			start_attack();
			engine->play_sample("sfx/swing_weapon.ogg");
			attacking = true;
			std::string anim_name;
			if (direction == General::DIR_N) {
				anim_name = "attack-up";
			}
			else if (direction == General::DIR_S) {
				anim_name = "attack-down";
			}
			else {
				anim_name = "attack";
			}
			anim_set->set_sub_animation(anim_name);
			anim_set->reset();
			if (weapon_anim_set) {
				weapon_anim_set->set_sub_animation(anim_name);
				weapon_anim_set->reset();
			}
		}
	}
	else if (ability == "JUMP" && onoff) {
		Area_Loop *al = GET_AREA_LOOP;
		if (al && al->get_num_jumping() == 0) {
			Area_Manager *area = al->get_area();
			lua_State *stack = area->get_lua_state();
			Lua::call_lua(stack, "jump", ">");
		}
	}
}

void Player::set_role_paused(bool role_paused)
{
	attacking = false;
	Map_Entity::set_role_paused(role_paused);
}

void Player::set_selected_ability(bool battle, int num, std::string value)
{
	if (battle) {
		battle_abilities[num] = value;
	}
}
	
float Player::get_magic_regen_count()
{
	return magic_regen_count;
}

void Player::set_magic_regen_count(float count)
{
	magic_regen_count = count;
}

