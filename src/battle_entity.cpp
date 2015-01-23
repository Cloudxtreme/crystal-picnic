#include "battle_entity.h"
#include "collision_detection.h"
#include "particle.h"
#include "runner_loop.h"
#include "engine.h"
#include "game_specific_globals.h"
#include "player.h"
#include "resource_manager.h"

ALLEGRO_DEBUG_CHANNEL("CrystalPicnic")

#define NEXT_ATTACK_TIME (name == "bisou" ? 0.5 : 0.3)

const float MAX_COLLISION_DIST = 100.0f;

const int Battle_Entity::X = 0;
const int Battle_Entity::Y = 1;
const int Battle_Entity::JUMP = 2;
const int Battle_Entity::ATTACK = 3;
const int Battle_Entity::USE = 4;
const int Battle_Entity::ICE = 5;
const int Battle_Entity::SLASH = 6;
const int Battle_Entity::THROW = 7;
const int Battle_Entity::KICK = 8;
const int Battle_Entity::PLANT = 9;
const int Battle_Entity::FIRE = 10;
const int Battle_Entity::ROLL = 11;
const int Battle_Entity::BURROW = 12;
const int Battle_Entity::HEAL = 13;
 		
const float Battle_Entity::DEATH_ANIM_LENGTH = 1.5;
const float Battle_Entity::ITEM_CIRCLE_RADIUS = 40;
const float Battle_Entity::ITEM_CIRCLE_ICON_RADIUS = 16;
const double Battle_Entity::ITEM_CIRCLE_REPEAT_DELAY = 0.25;
const double Battle_Entity::ITEM_CIRCLE_FADE_TIME = 0.25;
const float Battle_Entity::ITEM_CIRCLE_ROTATION_TIME = 0.1;

static std::string item_names[7] = {
	"HEALTHVIAL",
	"HEALTHJAR",
	"HEALTHFLASK",
	"MAGICVIAL",
	"MAGICFLASK",
	"ANTIDOTE",
	"DIRTYSOCK"
};

int get_num_players(Battle_Loop *battle_loop)
{
	std::vector<Battle_Entity *> entities = battle_loop->get_entities();
	int num_players = 0;
	int sz = entities.size();
	for (int i = 0; i < sz; i++) {
		Battle_Player *p = dynamic_cast<Battle_Player *>(entities[i]);
		if (p) {
			std::string name = p->get_name();
			if (name == "egbert" || name == "frogbert" || name == "bisou") {
				num_players++;
			}
		}
	}
	return num_players;
}

static void draw_item_circle_text(int x, int y, std::string text)
{
	int w = General::get_text_width(General::FONT_LIGHT, text);
	int h = General::get_font_line_height(General::FONT_LIGHT);
	al_hold_bitmap_drawing(true);
	General::draw_text(text, al_color_name("black"), x-w/2+1, y-h+1);
	General::draw_text(text, al_color_name("black"), x-w/2+1, y-h);
	General::draw_text(text, al_color_name("black"), x-w/2, y-h+1);
	General::draw_text(text, al_color_name("white"), x-w/2, y-h);
	al_hold_bitmap_drawing(false);
}

Battle_AI *Battle_Entity::get_ai()
{
	return ai;
}

void Battle_Entity::inc_y(float inc)
{
	if (inc == 0 || immovable) {
		return;
	}
	pos.y += inc;
}

void Battle_Entity::set_physics(
	float MAX_X_ACCEL,
	float MAX_X_ACCEL_ROLL,
	float MAX_X_ACCEL_HIT,
	float MAX_X_VEL,
	float MAX_X_VEL_ROLL,
	float MAX_X_VEL_HIT,
	float MAX_Y_VEL,
	float GROUND_FRICTION,
	float AIR_FRICTION,
	float MAX_STEEPNESS,
	float JUMP_ACCEL,
	float GRAVITY_ACCEL,
	float MAX_Y_ACCEL,
	float WALK_SPEED,
	float AIR_WALK_SPEED,
	float ROLL_SPEED,
	float HIT_FRICTION,
	float MAX_X_ACCEL_SPECIAL,
	float MAX_X_VEL_SPECIAL
)
{
	if (MAX_X_ACCEL >= 0.0f) this->MAX_X_ACCEL = MAX_X_ACCEL;
	if (MAX_X_ACCEL_ROLL >= 0.0f) this->MAX_X_ACCEL_ROLL = MAX_X_ACCEL_ROLL;
	if (MAX_X_ACCEL_HIT >= 0.0f) this->MAX_X_ACCEL_HIT = MAX_X_ACCEL_HIT;
	if (MAX_X_VEL >= 0.0f) this->MAX_X_VEL = MAX_X_VEL;
	if (MAX_X_VEL_ROLL >= 0.0f) this->MAX_X_VEL_ROLL = MAX_X_VEL_ROLL;
	if (MAX_X_VEL_HIT >= 0.0f) this->MAX_X_VEL_HIT = MAX_X_VEL_HIT;
	if (MAX_Y_VEL >= 0.0f) this->MAX_Y_VEL = MAX_Y_VEL;
	if (GROUND_FRICTION >= 0.0f) this->GROUND_FRICTION = GROUND_FRICTION;
	if (AIR_FRICTION >= 0.0f) this->AIR_FRICTION = AIR_FRICTION;
	if (MAX_STEEPNESS >= 0.0f) this->MAX_STEEPNESS = MAX_STEEPNESS;
	if (JUMP_ACCEL >= 0.0f) this->JUMP_ACCEL = JUMP_ACCEL;
	if (GRAVITY_ACCEL >= 0.0f) this->GRAVITY_ACCEL = GRAVITY_ACCEL;
	if (MAX_Y_ACCEL >= 0.0f) this->MAX_Y_ACCEL = MAX_Y_ACCEL;
	if (WALK_SPEED >= 0.0f) this->WALK_SPEED = WALK_SPEED;
	if (AIR_WALK_SPEED >= 0.0f) this->AIR_WALK_SPEED = AIR_WALK_SPEED;
	if (ROLL_SPEED >= 0.0f) this->ROLL_SPEED = ROLL_SPEED;
	if (HIT_FRICTION >= 0.0f) this->HIT_FRICTION = HIT_FRICTION;
	if (MAX_X_ACCEL_SPECIAL >= 0.0f) this->MAX_X_ACCEL_SPECIAL = MAX_X_ACCEL_SPECIAL;
	if (MAX_X_VEL_SPECIAL >= 0.0f) this->MAX_X_VEL_SPECIAL = MAX_X_VEL_SPECIAL;
}

ALLEGRO_COLOR Battle_Entity::get_tint()
{
	return tint;
}

void Battle_Entity::set_tint(ALLEGRO_COLOR color)
{
	tint = color;
}

void Battle_Entity::set_speed_x(float percent, int dir)
{
	accel.x = MAX_X_ACCEL * dir * percent;
	velocity.x = MAX_X_VEL * dir * percent * speed_multiplier;
}

void Battle_Entity::set_speed_multiplier(float mult)
{
	speed_multiplier = mult;
}

void Battle_Entity::gravity()
{
	if (flying_entity) {
		return;
	}

	// gravity
	if (!on_ground && !immovable) {
		float old_player_ay = accel.y;

		accel.y += GRAVITY_ACCEL;
		if (accel.y < 0 && jump_released) {
			accel.y += GRAVITY_ACCEL;
		}
		if (accel.y > MAX_Y_ACCEL) {
			accel.y = MAX_Y_ACCEL;
		}
		else if (accel.y < -MAX_Y_ACCEL) {
			accel.y = -MAX_Y_ACCEL;
		}

		if (old_player_ay <= 0 && accel.y >= 0) {
			// hit apex now falling
			check_platform = true;
			jumping = true;
		}

		velocity.y += accel.y;
		if (velocity.y > MAX_Y_VEL) {
			velocity.y = MAX_Y_VEL;
		}
		else if (velocity.y < -MAX_Y_VEL) {
			velocity.y = -MAX_Y_VEL;
		}
		old_y = pos.y;
		inc_y(velocity.y);

		if (velocity.y > 0 && check_platform) {
			General::Point<float> p1(old_x, old_y);
			General::Point<float> p2(pos.x, pos.y);

			std::vector< std::pair< General::Point<float>, General::Point<float> > > &sector1 = battle_loop->get_sector(old_x / Battle_Loop::SECTOR_SIZE);
			std::vector< std::pair< General::Point<float>, General::Point<float> > > &sector2 = battle_loop->get_sector(pos.x / Battle_Loop::SECTOR_SIZE);
			std::vector< std::pair< General::Point<float>, General::Point<float> > > sector;
			sector.insert(sector.end(), sector1.begin(), sector1.end());
			sector.insert(sector.end(), sector2.begin(), sector2.end());

			int sz = sector.size();
			for (int i = 0; i < sz; i++) {
				std::pair< General::Point<float>, General::Point<float> > &p = sector[i];
				General::Point<float> result;
				if (checkcoll_line_line(&p1, &p2, &p.first, &p.second, &result)) {
					int tmp_plat;
					if (i < (int)sector1.size()) {
						tmp_plat = battle_loop->get_sector_platform(old_x / Battle_Loop::SECTOR_SIZE, i);
					}
					else {
						tmp_plat = battle_loop->get_sector_platform(pos.x / Battle_Loop::SECTOR_SIZE, i-sector1.size());
					}
					if (battle_loop->get_platform_solid()[tmp_plat]) {
						platform = tmp_plat;
						pos.x = result.x;
						pos.y = result.y-0.5f;
						on_ground = true;
						jumps = 0;
						if (!dynamic_cast<Runner_Loop *>(battle_loop)) {
							accel.x *= 0.5f;
							velocity.x *= 0.5f;
						}
						accel.y = 0;
						velocity.y = 0;
						if (anim_set->get_sub_animation_name() == "die" || anim_set->get_sub_animation_name() == "recoil") {
							// do nothing
						}
						else if (anim_set->get_sub_animation_name() == "hit" && attributes.hp > 0) {
							anim_set->set_sub_animation("battle-idle");
							anim_set->reset();
						}
						else if (jumping && attributes.hp > 0) {
							if (anim_set->get_sub_animation_name() == (attack_name + "-down")) {
								attacking = false;
								anim_set->reset();
							}
							anim_set->set_sub_animation("jump-landed");
							anim_set->reset();
						}
						engine->play_sample("sfx/land.ogg");
						jumping = false;
						break;
					}
				}
			}
		}
	}
}

bool Battle_Entity::do_attack(Battle_Entity *ent, Bones::Bone &ent_bone, bool clang_played)
{
	if (ent_bone.type == Bones::BONE_RESISTANT) {
		if (!clang_played) {
			clang_played = true;
		}
	}
	else if (!ent->is_hurt() && !ent->is_unhittable() && (ent->get_name() != "antboss" || !ent->is_attacking())) {
		General::Point<float> acc = ent->get_accel();
		acc.x = 0.0f;
		ent->set_accel(acc);

		std::string dirS;
		if (flying_entity || name == "antboss") {
			dirS = "";
		}
		else {
			dirS = anim_set->get_current_animation()->get_tag("attack");
		}
		Hit_Direction dir;
		float xforce = 4.0f;
		float yforce;
		if (dirS == "" || dirS.substr(7) == "side") {
			dir = HIT_SIDE;
			yforce = 0.0f;
		}
		else if (dirS.substr(7) == "up") {
			dir = HIT_UP;
			yforce = 0.75f;
		}
		else { // down
			dir = HIT_DOWN;
			yforce = 1.0f;
		}

		float multiplier = 1.0f;
		if (!skeleton) {
			std::string multS = anim_set->get_current_animation()->get_tag("multiplier");
			if (multS != "") {
				multiplier = atof(multS.substr(11).c_str());
			}
		}

		if (!skeleton && attributes.equipment.accessory.name != "GREENRING") {
			std::string poisonS = anim_set->get_current_animation()->get_tag("poison");
			float poison_p;
			if (poisonS != "") {
				poison_p = atof(poisonS.substr(7).c_str());
			}
			else {
				poison_p = 0;
			}
			bool poisoned = ((General::rand() % 1000) / 1000.0f) < poison_p;
			if (poisoned) {
				engine->play_sample("sfx/poison_initial.ogg");
				ent->get_attributes().status.name = "POISON";
				ent->get_attributes().status.count = 0;
			}
		}

		Battle_Enemy *enemy = dynamic_cast<Battle_Enemy *>(this);
		if (enemy) {
			lua_State *lua_state = enemy->get_lua_state();
			bool exists;

			lua_getglobal(lua_state, "get_hit_sound");
			exists = !lua_isnil(lua_state, -1);
			lua_pop(lua_state, 1);
			std::string sfx_name;
			if (exists) {
				Lua::call_lua(lua_state, "get_hit_sound", ">s");
				sfx_name = lua_tostring(lua_state, -1);
				lua_pop(lua_state, 1);
			}
			else {
				sfx_name = sfx_name_hit;
			}
			engine->play_sample(sfx_name);
		}
		else {
			Battle_Player *p = dynamic_cast<Battle_Player *>(this);
			if (p && anim_set->get_sub_animation_name() == "kick") {
				engine->play_sample("sfx/kick_hit.ogg");
			}
			else {
				engine->play_sample(sfx_name_hit);
			}
		}

		std::string hardS;
		if (flying_entity || name == "antboss") {
			hardS = "";
		}
		else {
			hardS = anim_set->get_current_animation()->get_tag("hard-hitting");
		}
		bool hard;
		if (hardS != "") {
			int chance = atoi(hardS.substr(13).c_str());
			hard = (int)(General::rand() % 100) < chance;
		}
		else {
			hard = false;
		}
		if (hard) {
			std::vector<std::string> v;
			v.push_back("bash/1");
			v.push_back("bash/2");
			v.push_back("bash/3");
			v.push_back("bash/4");
			v.push_back("bash/5");
			v.push_back("bash/6");
			v.push_back("bash/7");
			int id = engine->add_particle_group("bash", battle_loop->get_entity_layer(), PARTICLE_HURT_NONE, v);
			Particle::Particle *p = Particle::add_particle(
				id,
				1, 1,
				al_map_rgba_f(1.0f, 1.0f, 1.0f, 1.0f),
				General::rand() % 7,
				0,
				true,
				false
			);
			General::Point<float> pos2 = ent->get_position();
			float x = (pos.x + pos2.x) / 2;
			float y = (pos.y + pos2.y) / 2;
			float h = 0;
			std::pair<std::string, int> pair;
			pair.first = "battle-idle";
			pair.second = 0;
			{
				std::vector<Bones::Bone> &bones = ent->get_bones()[pair];
				if (bones.size() > 0) {
					General::Size<float> extents = bones[0].get_extents();
					h += extents.h;
				}
				else {
					h += 16;
				}
			}
			{
				std::vector<Bones::Bone> &bones = get_bones()[pair];
				if (bones.size() > 0) {
					General::Size<float> extents = bones[0].get_extents();
					h += extents.h;
				}
				else {
					h += 16;
				}
			}
			y -= h/2;
			p->set_position(General::Point<float>(x, y));
			p->data[0] = x;
			p->data[1] = y;
		}
		ent->take_damage(this, dir, facing_right, General::Point<float>(xforce, yforce), hard, false, 0, multiplier);
		hit_something_this_attack = true;
	}

	return clang_played;
}

bool Battle_Entity::handle_single_weapon_bone_piece(Bones::Bone &weap_bone, Battle_Entity *ent, Bones::Bone &ent_bone, General::Point<float> pos_inc, bool clang_played)
{
	if (weap_bone.type != Bones::BONE_ATTACK) {
		return clang_played;
	}

	General::Point<float> top = battle_loop->get_top();

	int tlx1, tly1;
	int tlx2, tly2;

	tlx1 = pos.x-top.x;
	tly1 = pos.y-top.y;
	std::vector< General::Point<float> > weap_outline;
	Skeleton::Skeleton *skel1 = skeleton;
	weap_outline = (skel1 || is_facing_right()) ? weap_bone.get_outline() : weap_bone.get_outline_mirrored();

	General::Point<float> pos2 = ent->get_position();
	tlx2 = pos2.x-top.x;
	tly2 = pos2.y-top.y;
	std::vector< General::Point<float> > ent_outline;
	Skeleton::Skeleton *skel2 = ent->get_skeleton();
	ent_outline = (skel2 || ent->is_facing_right()) ? ent_bone.get_outline() : ent_bone.get_outline_mirrored();

	General::Point<float> topleft1 = General::Point<float>(
		tlx1, tly1
	);
	General::Point<float> topleft2 = General::Point<float>(
		tlx2, tly2
	);

	if (checkcoll_polygon_polygon(weap_outline, topleft1, ent_outline, topleft2)) {
		clang_played = do_attack(ent, ent_bone, clang_played);
	}

	return clang_played;
}

bool Battle_Entity::handle_single_bone_piece(Bones::Bone &bone, Battle_Entity *ent, Bones::Bone &ent_bone, General::Point<float> pos_inc)
{
	General::Point<float> top = battle_loop->get_top();

	int tlx1, tly1;
	int tlx2, tly2;

	tlx1 = pos.x-top.x;
	tly1 = pos.y-top.y;

	std::vector< General::Point<float> > outline;
	Skeleton::Skeleton *skel1 = skeleton;
	outline = (skel1 || is_facing_right()) ? bone.get_outline() : bone.get_outline_mirrored();

	General::Point<float> pos2 = ent->get_position();
	tlx2 = pos2.x-top.x;
	tly2 = pos2.y-top.y;
	std::vector< General::Point<float> > ent_outline;
	Skeleton::Skeleton *skel2 = ent->get_skeleton();
	ent_outline = (skel2 || ent->is_facing_right()) ? ent_bone.get_outline() : ent_bone.get_outline_mirrored();

	General::Point<float> topleft1 = General::Point<float>(
		tlx1, tly1
	);
	General::Point<float> topleft2 = General::Point<float>(
		tlx2, tly2
	);

	if (checkcoll_polygon_polygon(outline, topleft1, ent_outline, topleft2)) {
		lua_State *stack = ent->get_lua_state();
		if (stack) {
			lua_getglobal(stack, "collide");
			bool exists = !lua_isnil(stack, -1);
			lua_pop(stack, 1);

			if (exists) {
				Lua::call_lua(stack, "collide", "ii>", id, ent->get_id());
				return true;
			}
		}
	}

	return false;
}

bool Battle_Entity::handle_skeleton_target(Bones::Bone &weap_bone, Battle_Entity *ent, Skeleton::Link *link, bool clang_played)
{
	std::vector<Bones::Bone> &v = link->part->get_transformed_bones()[link->part->get_curr_bitmap()];
	for (size_t i = 0; i < v.size(); i++) {
		Bones::Bone &b = v[i];
		clang_played = handle_single_weapon_bone_piece(weap_bone, ent, b, General::Point<float>(0, 0), clang_played);
	}

	for (int i = 0; i < link->num_children; i++) {
		clang_played = handle_skeleton_target(weap_bone, ent, link->children[i], clang_played);
	}

	return clang_played;
}

void Battle_Entity::handle_weapon_bone(Battle_Entity *ent, Bones::Bone &weap_bone)
{
	Skeleton::Skeleton *ent_skel = ent->get_skeleton();

	if (ent_skel) {
		Skeleton::Animation *anim = ent_skel->get_animations()[ent_skel->get_curr_anim()];
		handle_skeleton_target(weap_bone, ent, anim->work, false);
	}
	else {
		std::pair<std::string, int> p = get_entity_bone_id(ent);
		for (size_t z = 0; z < ent->get_bones()[p].size(); z++) {
			Bones::Bone &ent_bone = ent->get_bones()[p][z];
			handle_single_weapon_bone_piece(weap_bone, ent, ent_bone, General::Point<float>(0, 0), false);
		}
	}
}

void Battle_Entity::handle_bone(Battle_Entity *ent, Bones::Bone &bone)
{
	Skeleton::Skeleton *ent_skel = ent->get_skeleton();

	if (ent_skel) {
		// FIXME: handle skeletal entities
		/*
		Skeleton::Animation *anim = ent_skel->get_animations()[ent_skel->get_curr_anim()];
		handle_skeleton_target(bone, ent, anim->work, false);
		*/
	}
	else {
		std::pair<std::string, int> p = get_entity_bone_id(ent);
		for (size_t z = 0; z < ent->get_bones()[p].size(); z++) {
			Bones::Bone &ent_bone = ent->get_bones()[p][z];
			if (handle_single_bone_piece(bone, ent, ent_bone, General::Point<float>(0, 0))) {
				return;
			}
		}
	}
}

bool Battle_Entity::ground_entity_logic()
{
	if (attributes.hp <= 0) {
		gravity();
		old_x = pos.x;
		old_y = pos.y;
		return false;
	}

	bool was_crouching = crouching;
	crouching = false;

	if (skeleton) {
		skeleton->update(General::LOGIC_MILLIS);
	}
	else {
		anim_set->update(BATTLE_LOGIC_MILLIS);
		if (weapon_anim_set)
			weapon_anim_set->sync(anim_set);
	}

	if (item_circle_state == ITEM_CIRCLE_ITEMS) {
		if ((item_circle_axis < 0 && al_get_time()-item_circle_next_repeat >= 0) || (item_circle_axis == 0 && input[X] < -0.5f)) {
			if (input[X] < -0.5f) {
				engine->play_sample("sfx/circle_scroll_left.ogg", 1.0f, 0.0f, 1.0f);
			}
			item_circle_next_repeat = al_get_time()+ITEM_CIRCLE_REPEAT_DELAY;
			item_circle_last_axis = item_circle_axis = -1;
			do {
				item_circle_index--;
				if (item_circle_index < 0) {
					item_circle_index = 6;
				}
			} while (circle_items[item_circle_index].num_in_inventory <= 0);
		}
		else if ((item_circle_axis > 0 && al_get_time()-item_circle_next_repeat >= 0) || (item_circle_axis == 0 && input[X] > 0.5f)) {
			if (input[X] > 0.5f) {
				engine->play_sample("sfx/circle_scroll_right.ogg", 1.0f, 0.0f, 1.0f);
			}
			item_circle_next_repeat = al_get_time()+ITEM_CIRCLE_REPEAT_DELAY;
			item_circle_last_axis = item_circle_axis = 1;
			do {
				item_circle_index++;
				if (item_circle_index >= 7) {
					item_circle_index = 0;
				}
			} while (circle_items[item_circle_index].num_in_inventory <= 0);
		}
	}
	else if (item_circle_state == ITEM_CIRCLE_PLAYERS) {
		int num_players = get_num_players(battle_loop);
		if ((item_circle_axis < 0 && al_get_time()-item_circle_next_repeat >= 0) || (item_circle_axis == 0 && input[X] < -0.5f)) {
			if (input[X] < -0.5f) {
				engine->play_sample("sfx/circle_scroll_left.ogg", 1.0f, 0.0f, 1.0f);
			}
			item_circle_next_repeat = al_get_time()+ITEM_CIRCLE_REPEAT_DELAY;
			item_circle_last_axis = item_circle_axis = -1;
			item_circle_index--;

			if (item_circle_index < 0) {
				item_circle_index = num_players-1;
			}
		}
		else if ((item_circle_axis > 0 && al_get_time()-item_circle_next_repeat >= 0) || (item_circle_axis == 0 && input[X] > 0.5f)) {
			if (input[X] > 0.5f) {
				engine->play_sample("sfx/circle_scroll_right.ogg", 1.0f, 0.0f, 1.0f);
			}
			item_circle_next_repeat = al_get_time()+ITEM_CIRCLE_REPEAT_DELAY;
			item_circle_last_axis = item_circle_axis = 1;
			item_circle_index++;
			if (item_circle_index >= num_players) {
				item_circle_index = 0;
			}
		}
	}
	else if (!hurt) {
		if (!casting && input[ICE]) {
			std::vector<int>::iterator it;
			if (first_ice+2 < al_get_time()) {
				first_ice = al_get_time();
				num_ice = 0;
			}
			if (num_ice < 2 && Game_Specific_Globals::take_magic(attributes, "ICE")) {
				casting = true;
				anim_set->set_sub_animation("magic");
				anim_set->reset();
				input[ICE] = 0;
				num_ice++;
			}
		}
		else if (!casting && input[FIRE] && !battle_loop->is_cart_battle()) {
			std::vector<int>::iterator it;
			if (first_ice+2 < al_get_time()) {
				first_ice = al_get_time();
				num_ice = 0;
			}
			if (num_ice < 2 && Game_Specific_Globals::take_magic(attributes, "FIRE")) {
				casting = true;
				anim_set->set_sub_animation("magic");
				anim_set->reset();
				input[FIRE] = 0;
				num_ice++;
			}
		}
		else if (attacking) {
			std::string sub_name;
			bool stick;

			if (skeleton) {
				sub_name = skeleton->get_curr_anim_name();
				stick = false;
			}
			else {
				sub_name = anim_set->get_sub_animation_name();
				if ((name != "bisou" || !weapon_anim_set) && sub_name == (attack_name + "-down")) {
					stick = true;
				}
				else {
					stick = false;
				}
			}
			if ((!strstr(sub_name.c_str(), attack_name.c_str()) && sub_name != "slash" && sub_name != "throw" && sub_name != "kick" && sub_name != "plant" && sub_name != "roll") || (!stick && ((skeleton && skeleton->get_loops() > 0) || (!skeleton && anim_set->get_current_animation()->is_finished()))) || (sub_name == "roll" && (al_get_time()-roll_start) > NEXT_ATTACK_TIME)) {
				attacking = false;
				hit_something_this_attack = false;
				if (!skeleton && anim_set->check_sub_animation_exists("recoil")) {
					anim_set->set_sub_animation("recoil");
					anim_set->reset();
					velocity = General::Point<float>(0.0f, 0.0f);
					accel = General::Point<float>(0.0f, 0.0f);
					General::Point<float> force(3.25f, 0.6f);
					take_damage(this, HIT_UP, !facing_right, force, true, true, 0, 1.0f);
				}
				else {
					if (name == "frogbert" && sub_name == "plant") {
						// add plant
						Battle_Enemy *enemy = new Battle_Enemy(battle_loop, "plant");
						enemy->construct();
						enemy->set_id(battle_loop->get_next_id());
						battle_loop->get_entities().push_back(enemy);
						enemy->set_position(pos);
					}
					if (skeleton) {
						skeleton->set_curr_anim("battle-idle");
					}
					else {
						anim_set->set_sub_animation("battle-idle");
						anim_set->reset();
					}
				}
				/* Add arrows for Bisou */
				if (name == "bisou" && weapon_anim_set && sub_name != "roll") {
					std::string arrow_type;
					int damage;
					if (attributes.equipment.weapon.attachments.size() > 0) {
						arrow_type = "-" + attributes.equipment.weapon.attachments[0].name;
						damage = attributes.equipment.weapon.attachments[0].attack;
					}
					else {
						arrow_type = "";
						damage = 1;
					}
					int accessory_attack = 0;
					Game_Specific_Globals::get_accessory_effects(attributes.equipment.accessory.name, NULL, NULL, &accessory_attack, NULL);
					damage += accessory_attack;
					if (sub_name == (attack_name + "-up") || sub_name == (attack_name + "-down")) {
						std::vector<std::string> v;
						v.push_back("vert_arrow" + arrow_type);
						int id = engine->add_particle_group("vert_arrow", battle_loop->get_entity_layer(), PARTICLE_HURT_ENEMY, v);
						Particle::Particle *p = Particle::add_particle(
							id,
							3, 3,
							al_map_rgba_f(1.0f, 1.0f, 1.0f, 1.0f),
							0,
							sub_name == (attack_name + "-up") ? HIT_UP : HIT_DOWN,
							false,
							false // FIXME: Make special arrows hard hitting
						);
						p->bullet_time_len = cfg.screen_w/4.0f;
						if (sub_name == (attack_name + "-up")) {
							p->set_position(General::Point<float>(pos.x, pos.y-24));
							p->draw_offset = General::Point<int>(0, 0);
							p->data[0] = 1;
						}
						else {
							p->set_position(General::Point<float>(pos.x, pos.y));
							p->draw_offset = General::Point<int>(0,  -5);
							p->extra_draw_flags = ALLEGRO_FLIP_VERTICAL;
							p->data[0] = 0;
						}
						p->damage = damage;
					}
					else {
						std::vector<std::string> v;
						v.push_back("arrow" + arrow_type);
						int id = engine->add_particle_group("arrow", battle_loop->get_entity_layer(), PARTICLE_HURT_ENEMY, v);
						Particle::Particle *p = Particle::add_particle(
							id,
							3, 3,
							al_map_rgba_f(1.0f, 1.0f, 1.0f, 1.0f),
							0,
							HIT_SIDE,
							facing_right,
							false // FIXME: Make special arrows hard hitting
						);
						p->bullet_time_len = cfg.screen_w/4.0f;
						if (facing_right) {
							p->set_position(General::Point<float>(pos.x+8, pos.y-12));
							p->draw_offset = General::Point<int>(-15/2, 0);
							p->data[0] = 1;
						}
						else {
							p->set_position(General::Point<float>(pos.x-8, pos.y-12));
							p->draw_offset = General::Point<int>(15/2,  0);
							p->data[0] = 0;
						}
						p->damage = damage;
					}
					if (arrow_type != "") {
						attributes.equipment.weapon.attachments[0].quantity--;
						if (attributes.equipment.weapon.attachments[0].quantity <= 0) {
							attributes.equipment.weapon.attachments.erase(attributes.equipment.weapon.attachments.begin());
						}
					}
				}
			}
		}
		else if (!skeleton) {
			if (anim_set->get_sub_animation_name() == "recoil") {
				anim_set->set_sub_animation("battle-idle");
				anim_set->reset();
			}
			else if (jumping && anim_set->get_sub_animation_name() == "jump-start") {
				if (anim_set->get_current_animation()->is_finished()) {
					anim_set->set_sub_animation("jump-in-air");
					anim_set->reset();
				}
			}
			else if (anim_set->get_sub_animation_name() == "jump-landed") {
				if (anim_set->get_current_animation()->is_finished()) {
					anim_set->set_sub_animation("run");
					anim_set->reset();
				}
			}
			else if (jumping && anim_set->get_sub_animation_name() == "jump-flipping") {
				if (anim_set->get_current_animation()->is_finished()) {
					anim_set->set_sub_animation("jump-in-air");
					anim_set->reset();
				}
			}
		}

		if (casting) {
			if (!strstr(anim_set->get_sub_animation_name().c_str(), "magic") || anim_set->get_current_animation()->is_finished()) {
				casting = false;
				if (name == "egbert") {
					engine->play_sample("sfx/ice_blast.ogg", 1, 0, 1);
					anim_set->set_sub_animation("battle-idle");
					anim_set->reset();
					std::vector<std::string> bitmap_names;
					bitmap_names.push_back("ice_pellet");
					int id = engine->add_particle_group("ice_pellet", battle_loop->get_entity_layer(), PARTICLE_HURT_ENEMY, bitmap_names);
					Particle::Particle *p = Particle::add_particle(
						id,
						3, 3,
						al_map_rgb_f(1, 1, 1),
						0,
						HIT_SIDE,
						facing_right,
						false
					);
					p->high = true;
					p->bullet_time_len = cfg.screen_w/4.0f;
					std::string aname = anim_set->get_sub_animation_name();
					int curr_frame = anim_set->get_current_animation()->get_current_frame_num();
					std::pair<std::string, int> pair;
					pair.first = aname;
					pair.second = curr_frame;
					General::Point<float> ppos;
					if (facing_right) {
						ppos.x = pos.x + 4;
					}
					else {
						ppos.x = pos.x - 4 - al_get_bitmap_width(engine->get_particle_group(id)->bitmaps[0]->bitmap);
					}
					ppos.y = pos.y - 12;
					p->set_position(ppos);
					p->data[0] = facing_right ? 1.0f : 0.0f;
				}
				else if (name == "frogbert") {
					engine->play_sample("sfx/fire.ogg", 1, 0, 1);
					anim_set->set_sub_animation("battle-idle");
					anim_set->reset();
					std::vector<std::string> bitmap_names;
					bitmap_names.push_back("fire");
					int id = engine->add_particle_group("fire", battle_loop->get_entity_layer(), PARTICLE_HURT_ENEMY, bitmap_names);
					Particle::Particle *p = Particle::add_particle(
						id,
						3, 3,
						al_map_rgb_f(1, 1, 1),
						0,
						HIT_SIDE,
						facing_right,
						false
					);
					p->high = true;
					p->bullet_time_len = cfg.screen_w/4.0f;
					std::string aname = anim_set->get_sub_animation_name();
					int curr_frame = anim_set->get_current_animation()->get_current_frame_num();
					std::pair<std::string, int> pair;
					pair.first = aname;
					pair.second = curr_frame;
					General::Point<float> ppos;
					if (facing_right) {
						ppos.x = pos.x + 4;
					}
					else {
						ppos.x = pos.x - 4 - al_get_bitmap_width(engine->get_particle_group(id)->bitmaps[0]->bitmap);
					}
					ppos.y = pos.y - 12;
					p->set_position(ppos);
					p->data[0] = facing_right ? 1.0f : 0.0f;
				}
				else { // bisou
					anim_set->set_sub_animation("battle-idle");
				}
			}
		}
		else if (on_ground && input[JUMP] && input[Y] >= 0.9f) {
			std::vector< std::pair< General::Point<float>, General::Point<float> > > sector = battle_loop->get_sector(pos.x / Battle_Loop::SECTOR_SIZE);

			bool can_drop = false;
			int sz = sector.size();
			for (int i = 0; i < sz; i++) {
				General::Point<float> top_pt =
					General::Point<float>(pos.x, pos.y+10);
				General::Point<float> bottom_pt =
					General::Point<float>(pos.x, General::BIG_FLOAT);
				std::pair< General::Point<float>, General::Point<float> > &p = sector[i];
				General::Point<float> result;
				if (checkcoll_line_line(&top_pt, &bottom_pt, &p.first, &p.second, &result)) {
					can_drop = true;
					break;
				}
			}
			if (can_drop) {
				anim_set->set_sub_animation("jump-in-air");
				anim_set->reset();
				input[ATTACK] = 0;
				inc_y(10);
				on_ground = false;
				check_platform = true;
				jumps = 2;
				platform = -1;
			}
		}
		else if (on_ground && input[Y] > 0.5) {
			if (!attacking) {
				if (dynamic_cast<Runner_Loop *>(battle_loop)) {
					anim_set->set_sub_animation("slide");
				}
				else {
					anim_set->set_sub_animation("crouch");
					input[X] = 0;
				}
				if (!was_crouching) {
					anim_set->reset();
				}
				crouching = true;
			}
		}
		else if (!attacking && on_ground && input[ATTACK] && al_get_time() >= next_attack_time) {
			engine->play_sample(sfx_name_attack);
			attacking = true;
			next_attack_time = al_get_time() + NEXT_ATTACK_TIME;
			std::string anim;
			if (name == "bisou" && input[Y] < -0.5f) {
				next_attack = attack_name + "-up";
			}
			else if (al_get_time() > next_attack_time_end || name == "bisou") {
				next_attack = attack_name;
			}

			anim = next_attack;

			if (skeleton) {
				skeleton->set_curr_anim(anim);
				skeleton->reset_current_animation();
			}
			else {
				anim_set->set_sub_animation(anim);

				std::string tag = anim_set->get_current_animation()->get_tag("next-attack");

				// parse
				size_t o = tag.find(' ', 12);

				if (o != std::string::npos) {
					next_attack = tag.substr(12, o-12);
					std::string t = tag.substr(o+1);
					double ti = atof(t.c_str());
					if (ti == 0.0) {
						next_attack_time_end = DBL_MAX;
					}
					else {
						next_attack_time_end = al_get_time() + ti;
					}
				}

				anim_set->reset();
			}

			input[ATTACK] = 0;
		}
		else if (!attacking && !casting && jumping && input[ATTACK] && al_get_time() >= next_attack_time) {
			engine->play_sample(sfx_name_attack);
			attacking = true;
			next_attack_time = al_get_time() + NEXT_ATTACK_TIME;
			if (input[Y] < -0.5f) {
				anim_set->set_sub_animation((attack_name + "-up"));
			}
			else if (input[Y] > 0.5f) {
				anim_set->set_sub_animation((attack_name + "-down"));
			}
			else {
				anim_set->set_sub_animation("air-" + attack_name);
			}
			anim_set->reset();
			input[ATTACK] = 0;
		}
		else if (!attacking && !casting && input[ROLL] && al_get_time() >= next_attack_time) {
			if (Game_Specific_Globals::take_magic(attributes, "ROLL")) {
				engine->play_sample("sfx/roll.ogg");
				attacking = true;
				next_attack_time = al_get_time() + NEXT_ATTACK_TIME;
				roll_start = al_get_time();
				anim_set->set_sub_animation("roll");
				anim_set->reset();
				input[X] = 0.0f;
				apply_force(HIT_SIDE, facing_right, General::Point<float>(10.0f, 0.0f));
				input[ROLL] = 0;
			}
		}
		else if (!attacking && !casting && on_ground && input[BURROW] && battle_loop->get_distance_from_nearest_edge(id) > 16) {
			bool found = false;
			std::vector<Battle_Entity *> v = battle_loop->get_entities();
			int sz = v.size();
			for (int i = 0; i < sz; i++) {
				Battle_Entity *e = v[i];
				if (e) {
					if (e->is_item()) {
						found = true;
						break;
					}
				}
			}
			if (!found && Game_Specific_Globals::take_magic(attributes, "BURROW")) {
				burrowing = true;
				draw_shadow = false;
				burrow_stage = BURROW_START;
				burrow_time = al_get_time();
				unhittable = true;
				anim_set->set_sub_animation("burrow");
				weapon_anim_set->set_sub_animation("burrow");
				battle_loop->add_burrow_hole(pos, al_get_time() + ((BURROW_TICKS+1)*BATTLE_LOGIC_MILLIS/1000.0));
				burrow_start_pos = pos;
				input[BURROW] = 0;
			}
		}
		else if (!attacking && !casting && input[SLASH] && al_get_time() >= next_attack_time) {
			if (Game_Specific_Globals::take_magic(attributes, "SLASH")) {
				engine->play_sample("sfx/slash.ogg");
				attacking = true;
				next_attack_time = al_get_time() + NEXT_ATTACK_TIME;
				anim_set->set_sub_animation("slash");
				anim_set->reset();
				input[X] = 0.0f;
				apply_force(HIT_SIDE, facing_right, General::Point<float>(10.0f, 0.0f));
				input[SLASH] = 0;
			}
		}
		else if (on_ground && !attacking && !casting && input[PLANT] && al_get_time() >= next_attack_time && !battle_loop->is_cart_battle()) {
			if (Game_Specific_Globals::take_magic(attributes, "PLANT")) {
				engine->play_sample("sfx/plant_shovel.ogg");
				Lua::call_lua(battle_loop->get_lua_state(), "play_sample_later", "sd>", "sfx/plant_seed.ogg", 0.25);
				attacking = true;
				next_attack_time = al_get_time() + NEXT_ATTACK_TIME;
				anim_set->set_sub_animation("plant");
				anim_set->reset();
				input[PLANT] = 0;
			}
		}
		else if (!attacking && !casting && input[KICK] && al_get_time() >= next_attack_time && !battle_loop->is_cart_battle()) {
			if (Game_Specific_Globals::take_magic(attributes, "KICK")) {
				engine->play_sample("sfx/kick.ogg");
				attacking = true;
				next_attack_time = al_get_time() + NEXT_ATTACK_TIME;
				anim_set->set_sub_animation("kick");
				anim_set->reset();
				input[X] = 0.0f;
				apply_force(HIT_SIDE, facing_right, General::Point<float>(7.0f, 0.0f));
				input[KICK] = 0;
			}
		}
		else if (!attacking && !casting && input[THROW] && al_get_time() >= next_attack_time) {
			if (Game_Specific_Globals::take_magic(attributes, "THROW")) {
				engine->play_sample("sfx/throw_ability.ogg");
				attacking = true;
				next_attack_time = al_get_time() + NEXT_ATTACK_TIME;
				anim_set->set_sub_animation("throw");
				anim_set->reset();
				input[THROW] = 0;
				std::string weapon = attributes.equipment.weapon.name;
				Battle_Player *player = dynamic_cast<Battle_Player *>(this);
				if (!player->get_lost_weapon() && weapon != "") {
					std::vector<std::string> v;
					v.push_back("throw/" + weapon + "/1");
					v.push_back("throw/" + weapon + "/2");
					v.push_back("throw/" + weapon + "/3");
					v.push_back("throw/" + weapon + "/4");
					v.push_back("throw/" + weapon + "/5");
					v.push_back("throw/" + weapon + "/6");
					v.push_back("throw/" + weapon + "/7");
					v.push_back("throw/" + weapon + "/8");
					int id = engine->add_particle_group("throw", battle_loop->get_entity_layer(), PARTICLE_HURT_ENEMY, v);
					Particle::Particle *p = Particle::add_particle(
						id,
						20, 20,
						al_map_rgba_f(1.0f, 1.0f, 1.0f, 1.0f),
						0,
						HIT_SIDE,
						facing_right,
						true
					);
					p->set_position(General::Point<float>(pos.x, pos.y-15));
					p->damage = attributes.equipment.weapon.attack;
					int accessory_attack = 0;
					Game_Specific_Globals::get_accessory_effects(attributes.equipment.accessory.name, NULL, NULL, &accessory_attack, NULL);
					p->damage += accessory_attack;
					p->data[0] = facing_right ? 1 : 0;
					p->data[1] = 0.0f; // count
					p->data[2] = pos.x; // start x
					p->data[3] = 0; // falling
					p->data[4] = 0.0f; // vel_y
					player->set_lost_weapon(true);
				}
			}
		}
		else if (!skeleton) {
			if (input[X] && !attacking && !casting && !jumping && (item_circle_state == ITEM_CIRCLE_NONE)) {
				if (fabs(velocity.x) < 1.5f && anim_set->check_sub_animation_exists("walk")) {
					anim_set->set_sub_animation("walk");
				}
				else {
					anim_set->set_sub_animation("run");
				}
			}
			else if (input[X] == 0 && !jumping && !attacking && !casting) {
				anim_set->set_sub_animation("battle-idle");
			}
		}
	}
	
	float max_x_accel;
	float max_x_vel;

	float MAX_X_ACCEL_JUMP = MAX_X_ACCEL * 1.0;
	float MAX_X_VEL_JUMP = MAX_X_VEL * 1.0;

	std::string sub_name;
	if (skeleton) {
		sub_name = skeleton->get_curr_anim_name();
	}
	else {
		sub_name = anim_set->get_sub_animation_name();
	}

	if (hurt) {
		max_x_accel = MAX_X_ACCEL_HIT;
		max_x_vel = MAX_X_VEL_HIT;
	}
	else if (jumping) {
		if (accelerates_quickly || _is_item || sub_name == "slash" || sub_name == "kick" || sub_name == "roll") {
			max_x_accel = MAX_X_ACCEL_SPECIAL / 3.0f;
			max_x_vel = MAX_X_VEL_SPECIAL / 3.0f;
		}
		else {
			max_x_accel = MAX_X_ACCEL_JUMP;
			max_x_vel = MAX_X_VEL_JUMP;
		}
	}
	else if (accelerates_quickly || _is_item || sub_name == "slash" || sub_name == "kick" || sub_name == "roll") {
		max_x_accel = MAX_X_ACCEL_SPECIAL;
		max_x_vel = MAX_X_VEL_SPECIAL;
	}
	else {
		max_x_accel = MAX_X_ACCEL;
		max_x_vel = MAX_X_VEL;
	}

	float max_x_accel_hit = max_x_accel;
	float max_x_vel_hit = max_x_vel;

	if (input[X] == 0 && on_ground) {
		max_x_vel /= 8;
	}
	else {
		max_x_vel *= speed_multiplier;
	}

	float a_add = 0.0f;

	if (velocity.x < 0) {
		if (on_ground) {
			a_add = GROUND_FRICTION;
		}
		else {
			a_add = AIR_FRICTION;
		}
	}
	else if (velocity.x > 0) {
		if (on_ground) {
			a_add = -GROUND_FRICTION;
		}
		else {
			a_add = -AIR_FRICTION;
		}
	}

	// brake hit velocity
	float hit_a_add = HIT_FRICTION;
	if (hit_accel.x > 0.0f) {
		hit_accel.x -= hit_a_add;
		if (hit_accel.x < 0.0f) hit_accel.x = 0.0f;
	}
	else if (hit_accel.x < 0.0f) {
		hit_accel.x += hit_a_add;
		if (hit_accel.x > 0.0f) hit_accel.x = 0.0f;
	}
	else {
		if (hit_velocity.x > 0.0f) {
			hit_velocity.x -= hit_a_add;
			if (hit_velocity.x < 0.0f) hit_velocity.x = 0.0f;
		}
		else if (hit_velocity.x < 0.0f) {
			hit_velocity.x += hit_a_add;
			if (hit_velocity.x > 0.0f) hit_velocity.x = 0.0f;
		}
	}
	hit_velocity.x += hit_accel.x;
	if (hit_accel.x < -max_x_accel_hit) {
		hit_accel.x = -max_x_accel_hit;
	}
	if (hit_accel.x > max_x_accel_hit) {
		hit_accel.x = max_x_accel_hit;
	}
	if (hit_velocity.x < -max_x_vel_hit) {
		hit_velocity.x = -max_x_vel_hit;
	}
	if (hit_velocity.x > max_x_vel_hit) {
		hit_velocity.x = max_x_vel_hit;
	}

	if (!hurt && (item_circle_state == ITEM_CIRCLE_NONE)) {
		if (on_ground) {
			a_add += WALK_SPEED * input[X];
		}
		else {
			a_add += AIR_WALK_SPEED * input[X];
		}
	}

	bool positive = accel.x > 0;
	bool negative = accel.x < 0;
	accel.x += a_add;
	if (positive && accel.x < 0) accel.x = 0;
	if (negative && accel.x > 0) accel.x = 0;

	if (accel.x < -max_x_accel) {
		accel.x = -max_x_accel;
	}
	if (accel.x > max_x_accel) {
		accel.x = max_x_accel;
	}

	positive = velocity.x > 0;
	negative = velocity.x < 0;
	velocity.x += accel.x;
	if (positive && velocity.x < 0) velocity.x = 0;
	if (negative && velocity.x > 0) velocity.x = 0;

	if (velocity.x < -max_x_vel) {
		velocity.x = -max_x_vel;
	}
	if (velocity.x > max_x_vel) {
		velocity.x = max_x_vel;
	}
	if (fabs(velocity.x) < MIN(AIR_WALK_SPEED, MIN(WALK_SPEED, ROLL_SPEED))) {
		velocity.x = accel.x = 0.0f;
	}

	old_x = pos.x;

	// stay within screen
	if (pos.x+velocity.x+hit_velocity.x < 0) {
		if (!immovable) {
			pos.x = 0.0f;
		}
		velocity.x = 0.0f;
		hit_velocity.x = 0.0f;
	}
	else if (pos.x+velocity.x+hit_velocity.x > battle_loop->get_width()-1) {
		if (!immovable) {
			pos.x = battle_loop->get_width()-1;
		}
		velocity.x = 0.0f;
		hit_velocity.x = 0.0f;
	}

	if (!input[JUMP] && !jump_released) {
		jump_released = true;
	}
	else if (input[JUMP] && jumps < 2 && jump_released) {
		if (dynamic_cast<Battle_Player *>(this)) {
			if (jumps == 0) {
				engine->play_sample("sfx/single_jump.ogg");
			}
			else {
				engine->play_sample("sfx/double_jump.ogg");
			}
		}
		else {
			engine->play_sample("sfx/enemy_jump.ogg");
		}
		if (check_platform && jumps != 0) {
			accel.y = JUMP_ACCEL * 0.75;
			velocity.y = 0;
		}
		else {
			accel.y += JUMP_ACCEL;
		}
		on_ground = false;
		jumps++;
		jumping = true;
		platform = -1;
		check_platform = false;
		jump_released = false;
		if (jumps > 1) {
			attacking = false;
			casting = false;
			anim_set->set_sub_animation("jump-flipping");
			anim_set->reset();
		}
		else {
			anim_set->set_sub_animation("jump-start");
			anim_set->reset();
		}
	}
	else if (on_ground && !immovable) {
		float diff = ground_diff(velocity.x+hit_velocity.x, battle_loop->get_geometry());
		if (diff >= General::BIG_FLOAT) {
			if (!skeleton) {
				anim_set->set_sub_animation("jump-in-air");
				anim_set->reset();
			}
			if (!immovable) {
				pos.x += velocity.x;
				pos.x += hit_velocity.x;
			}
			old_x = pos.x;
			inc_y(5);
			on_ground = false;
			check_platform = true;
			platform = -1;
			jumping = true;
			jumps = 2;
			attacking = false;
			casting = false;
		}
		else if (diff >= -MAX_STEEPNESS) {
			if (!immovable) {
				pos.x += velocity.x;
				pos.x += hit_velocity.x;
			}
			inc_y(diff);
		}
	}
	else if (!immovable) {
		pos.x += velocity.x;
		pos.x += hit_velocity.x;
	}

	gravity();

	return true;
}

bool Battle_Entity::flying_entity_logic()
{
	if (attributes.hp <= 0) {
		return true;
	}

	if (anim_set) {
		anim_set->update(BATTLE_LOGIC_MILLIS);
	}

	if (skeleton) {
		skeleton->update(General::LOGIC_MILLIS);
	}

	if (!attacking) {
		hit_something_this_attack = false;
	}

	if (name == "faff") { // or other "ground"/"regular" attackers
		if (!attacking && input[ATTACK] && al_get_time() >= next_attack_time) {
			engine->play_sample(sfx_name_attack);
			attacking = true;
			next_attack_time = al_get_time() + NEXT_ATTACK_TIME;
			std::string anim;
			if (al_get_time() > next_attack_time_end) {
				next_attack = attack_name;
			}

			anim = next_attack;
			anim_set->set_sub_animation(anim);

			std::string tag = anim_set->get_current_animation()->get_tag("next-attack");

			// parse
			size_t o = tag.find(' ', 12);

			if (o != std::string::npos) {
				next_attack = tag.substr(12, o-12);
				std::string t = tag.substr(o+1);
				double ti = atof(t.c_str());
				if (ti == 0.0) {
					next_attack_time_end = DBL_MAX;
				}
				else {
					next_attack_time_end = al_get_time() + ti;
				}
			}
			anim_set->reset();
			input[ATTACK] = 0;
		}
		else if (attacking) {
			if (anim_set->get_current_animation()->is_finished()) {
				attacking = false;
				hit_something_this_attack = false;
				anim_set->set_sub_animation("battle-idle");
				anim_set->reset();
			}
		}
	}

	return true;
}

std::pair<std::string, int> Battle_Entity::get_entity_bone_id(Battle_Entity *ent)
{
	std::string aname = ent->get_skeleton() ? "" : ent->get_animation_set()->get_sub_animation_name();
	int curr_frame = ent->get_skeleton() ? 0 : ent->get_animation_set()->get_current_animation()->get_current_frame_num();
	std::pair<std::string, int> p;
	p.first = aname;
	p.second = curr_frame;
	return p;
}

void Battle_Entity::skeleton_entity_handle_one_part(Skeleton::Link *l, Battle_Entity *ent)
{
	std::vector<Bones::Bone> &v = l->part->get_transformed_bones()[l->part->get_curr_bitmap()];
	for (size_t y = 0; y < v.size(); y++) {
		handle_weapon_bone(ent, v[y]);
	}

	for (int i = 0; i < l->num_children; i++) {
		skeleton_entity_handle_one_part(l->children[i], ent);
	}
}

void Battle_Entity::logic()
{
	General::Point<float> top = battle_loop->get_top();
	if (skeleton) {
		skeleton->transform(General::Point<float>(pos.x-top.x, pos.y-top.y), !facing_right);
	}

	if (burrowing) {
		anim_set->update(BATTLE_LOGIC_MILLIS);
		if (burrow_stage == BURROW_START) {
			float p = (al_get_time() - burrow_time) / (BURROW_TICKS*BATTLE_LOGIC_MILLIS/1000.0);
			bool done = p >= 1;
			if (done) p = 1;
			pos = General::Point<float>(burrow_start_pos.x, burrow_start_pos.y+p*64);
			if (done) {
				burrow_time = al_get_time();
				burrow_stage = BURROW_WAIT;
				battle_loop->add_static_burrow_hole(burrow_start_pos, al_get_time()+6);
			}
		}
		else if (burrow_stage == BURROW_WAIT) {
			if (al_get_time() > burrow_time+4) {
				// Pop up an item
				std::string types[] = {
					"antidote",
					"healthvial",
					"magicvial",
					"bone",
					"tincan",
					""
				};

				int count;
				for (count = 0; types[count] != ""; count++)
					;
				int type = General::rand() % count;

				Battle_Enemy *enemy = new Battle_Enemy(battle_loop, types[type]);
				enemy->construct();
				enemy->set_id(battle_loop->get_next_id());
				battle_loop->get_entities().push_back(enemy);
				enemy->set_position(burrow_start_pos);

				engine->play_sample("sfx/throw.ogg");

				bool right;
				if (burrow_start_pos.x < battle_loop->get_width()/2) {
					right = true;
				}
				else {
					right = false;
				}
				enemy->apply_force(HIT_UP, right, General::Point<float>(0.0f, 0.75f));
				enemy->apply_force(HIT_SIDE, right, General::Point<float>(0.25f, 0.0f));
				
				burrow_time = al_get_time();
				burrow_stage = BURROW_WAIT2;
			}
		}
		else if (burrow_stage == BURROW_WAIT2) {
			if (al_get_time() > burrow_time+1) {
				unhittable = false;
				burrowing = false;
				draw_shadow = true;
				pos = burrow_start_pos;
				engine->play_sample("sfx/single_jump.ogg");
				anim_set->set_sub_animation("jump-start");
				on_ground = false;
				check_platform = true;
				platform = -1;
				jumping = true;
				jumps = 2;
				apply_force(HIT_UP, facing_right, General::Point<float>(0.0f, 0.75f));
			}
		}
		return;
	}

	if (flying_entity) {
		if (!flying_entity_logic()) {
			return;
		}
	}
	else {
		if (!ground_entity_logic()) {
			return;
		}
	}

	if (item_circle_state == ITEM_CIRCLE_ITEMS_FADING_IN) {
		if (al_get_time()-item_circle_fade_start >= ITEM_CIRCLE_FADE_TIME) {
			item_circle_state = ITEM_CIRCLE_ITEMS;
		}
	}
	else if (item_circle_state == ITEM_CIRCLE_ITEMS_FADING_OUT) {
		if (al_get_time()-item_circle_fade_start >= ITEM_CIRCLE_FADE_TIME) {
			item_circle_state = ITEM_CIRCLE_NONE;
		}
	}
	else if (item_circle_state == ITEM_CIRCLE_PLAYERS_FADING_IN) {
		if (al_get_time()-item_circle_fade_start >= ITEM_CIRCLE_FADE_TIME) {
			item_circle_state = ITEM_CIRCLE_PLAYERS;
		}
	}
	else if (item_circle_state == ITEM_CIRCLE_PLAYERS_FADING_OUT) {
		if (al_get_time()-item_circle_fade_start >= ITEM_CIRCLE_FADE_TIME) {
			item_circle_state = ITEM_CIRCLE_NONE;
		}
	}
	else if (item_circle_state == ITEM_CIRCLE_ITEMS || item_circle_state == ITEM_CIRCLE_PLAYERS) {
		float num_choices = (item_circle_state == ITEM_CIRCLE_ITEMS) ? 7.0f : get_num_players(battle_loop);
		float num_active = (item_circle_state == ITEM_CIRCLE_ITEMS) ? num_items_found : 2.0f;
		float desired_angle;
		if (item_circle_index == 0) {
			desired_angle = 0.0f;
		}
		else {
			desired_angle = (M_PI * 2) - (item_circle_index / num_choices * M_PI * 2);
		}
		float SPEED = 0.1f * 7.0f / num_active;
		if (desired_angle != item_circle_angle) {
			if (item_circle_last_axis < 0) {
				float dist = General::angle_difference_clockwise(
					item_circle_angle, desired_angle
				);
				if (dist <= SPEED) {
					item_circle_angle = desired_angle;
				}
				else {
					item_circle_angle += SPEED;
				}
			}
			else {
				float dist = General::angle_difference_counter_clockwise(
					item_circle_angle, desired_angle
				);
				if (dist <= SPEED) {
					item_circle_angle = desired_angle;
				}
				else {
					item_circle_angle -= SPEED;
				}
			}
		}
	}

	// Check for overlaps in entities
	{
		std::vector<Battle_Entity *> all_ents = battle_loop->get_entities();

		int sz = all_ents.size();
		for (int i = 0; i < sz; i++) {
			Battle_Entity *ent = all_ents[i];
			// might not be a Battle_Entity
			if (ent == NULL) continue;
			// don't collide with self
			if (ent == this) continue;
			if (!ent->fully_initialized()) continue;

			General::Point<float> ent_pos = ent->get_position();
			if (General::distance(ent_pos.x, ent_pos.y, pos.x, pos.y) > MAX_COLLISION_DIST) {
				continue;
			}

			/* ->transform updates collision detection info which is needed for skeleton entities */
			/*
			General::Point<float> top = battle_loop->get_top();
			if (ent->get_skeleton()) {
				ent->get_skeleton()->transform(General::Point<float>(ent_pos.x-top.x, ent_pos.y-top.y), !ent->is_facing_right());
			}
			*/

			if (skeleton) {
				// FIXME: collisions with skeletal entities
				/*
				skeleton->transform(General::Point<float>(pos.x-top.x, pos.y-top.y), !facing_right);
				Skeleton::Animation *anim = skeleton->get_animations()[skeleton->get_curr_anim()];
				skeleton_entity_handle_one_part(anim->work, ent);
				*/
			}
			else {
				std::string aname = anim_set->get_sub_animation_name();
				int curr_frame = anim_set->get_current_animation()->get_current_frame_num();
				std::pair<std::string, int> p;
				p.first = aname;
				p.second = curr_frame;

				for (size_t j = 0; j < bones[p].size(); j++) {
					Bones::Bone &b = bones[p][j];
					handle_bone(ent, b);
				}
			}
		}
	}

	// attack logic
	if (attacking && !hit_something_this_attack) {
		std::vector<Battle_Entity *> all_ents = battle_loop->get_entities();

		int sz = all_ents.size();
		for (int i = 0; i < sz; i++) {
			Battle_Entity *ent = all_ents[i];
			// might not be a Battle_Entity
			if (ent == NULL) continue;
			// don't attack self
			if (ent == this) continue;
			if (!ent->fully_initialized()) continue;

			bool this_is_player = dynamic_cast<Battle_Player *>(this);
			bool ent_is_player = dynamic_cast<Battle_Player *>(ent);

			// enemies can't attack other enemies
			if (!this_is_player && !ent_is_player) continue;
			// players can't attack other players
			if (this_is_player && ent_is_player) continue;

			General::Point<float> ent_pos = ent->get_position();
			if (General::distance(ent_pos.x, ent_pos.y, pos.x, pos.y) > MAX_COLLISION_DIST) {
				continue;
			}

			/* ->transform updates collision detection info which is needed for skeleton entities */
			//General::Point<float> top = battle_loop->get_top();
			//if (ent->get_skeleton()) {
				//ent->get_skeleton()->transform(General::Point<float>(ent_pos.x-top.x, ent_pos.y-top.y), !ent->is_facing_right());
			//}

			if (skeleton) {
				//skeleton->transform(General::Point<float>(pos.x-top.x, pos.y-top.y), !facing_right);
				Skeleton::Animation *anim = skeleton->get_animations()[skeleton->get_curr_anim()];
				skeleton_entity_handle_one_part(anim->work, ent);
			}
			else {
				std::string aname = anim_set->get_sub_animation_name();
				int curr_frame = anim_set->get_current_animation()->get_current_frame_num();
				std::pair<std::string, int> p;
				p.first = aname;
				p.second = curr_frame;

				for (size_t y = 0; y < weapon_bones[p].size(); y++) {
					handle_weapon_bone(ent, weapon_bones[p][y]);
				}
				for (size_t j = 0; j < bones[p].size(); j++) {
					Bones::Bone &b = bones[p][j];
					if (b.type == Bones::BONE_ATTACK) {
						handle_weapon_bone(ent, b);
					}
				}
			}
		}
	}
	else if (hurt) {
		if (al_get_time() > hurt_end) {
			hurt = false;
			if (attributes.hp > 0) {
				if (skeleton) {
					skeleton->set_curr_anim("battle-idle");
				}
				else {
					anim_set->set_sub_animation("battle-idle");
					anim_set->reset();
				}
			}
		}
	}

	std::vector<Particle::Particle *> &particles = battle_loop->get_particles();
	int sz = particles.size();
	for (int i = 0; i < sz; i++) {
		Particle::Particle *p = particles[i];
		if (!p->hidden) {
			int alignment = p->particle_group->alignment;

			if (alignment == PARTICLE_HURT_NONE) {
				continue;
			}
			bool is_player = dynamic_cast<Battle_Player *>(this);
			if ((alignment == PARTICLE_HURT_ENEMY && is_player) || (alignment == PARTICLE_HURT_PLAYER && !is_player)) {
				continue;
			}
			if (alignment == PARTICLE_HURT_EGBERT && name != "egbert") {
				continue;
			}
			if (unhittable) {
				continue;
			}
			General::Point<float> ppos = p->get_position();
			if (General::distance(ppos.x, ppos.y, pos.x, pos.y) > MAX_COLLISION_DIST) {
				continue;
			}
			if (collides_with(*p)) {
				lua_State *lua_state = p->particle_group->lua_state;
				bool exists;

				lua_getglobal(lua_state, "collide");
				exists = !lua_isnil(lua_state, -1);
				lua_pop(lua_state, 1);
				if (exists) {
					Lua::call_lua(lua_state, "collide", "ii>", p->get_id(), id);
				}
				if (alignment != PARTICLE_HURT_EGBERT && !is_hurt()) {
					Hit_Direction dir = (Hit_Direction)p->get_hit_direction();
					float xforce = 1.0f;
					float yforce;
					if (dir == HIT_SIDE) {
						yforce = 0.0;
					}
					else if (dir == HIT_UP) {
						yforce = 0.75;
					}
					else if (dir == HIT_STOP) {
						xforce = -xforce;
						yforce = 0;
						accel.x = 0;
						velocity.x = 0;
						hit_accel.x = 0;
						hit_velocity.x = 0;
					}
					else { // down
						yforce = 1.25;
					}
					take_damage(NULL, dir, p->is_facing_right(), General::Point<float>(xforce, yforce), p->is_hard_hitting(), p->is_hard_hitting(), p->damage, 1.0f);
				}
			}
		}
	}
}

void Battle_Entity::script_pre_draw()
{
	if (first_run) {
		return;
	}

	Battle_Enemy *enemy;
	if ((enemy = dynamic_cast<Battle_Enemy *>(this))) {
		bool exists;
		lua_State *lua_state = enemy->get_lua_state();

		lua_getglobal(lua_state, "pre_draw");
		exists = !lua_isnil(lua_state, -1);
		lua_pop(lua_state, 1);
		if (exists) {
			Lua::call_lua(lua_state, "pre_draw", ">");
		}
	}
}

void Battle_Entity::pre_draw()
{
	if (first_run) {
		return;
	}

	script_pre_draw();

	/* draw shadow */
	if (draw_shadow) {
		std::vector< std::pair< General::Point<float>, General::Point<float> > > sector = battle_loop->get_sector(pos.x / Battle_Loop::SECTOR_SIZE);

		float closest = General::BIG_FLOAT;
		General::Point<float> closest_pt;
		bool found = false;
		
		Wrap::Bitmap *hero_shadow = engine->get_hero_shadow();
		int hero_shadow_w = al_get_bitmap_width(hero_shadow->bitmap);
		int hero_shadow_h = al_get_bitmap_height(hero_shadow->bitmap);
		int found_i = 0;
		int res_x = 0;

		int sz = sector.size();
		for (int i = 0; i < sz; i++) {
			General::Point<float> top_pt =
				General::Point<float>(pos.x, pos.y-1);
			General::Point<float> bottom_pt =
				General::Point<float>(pos.x, General::BIG_FLOAT);
			std::pair< General::Point<float>, General::Point<float> > &p = sector[i];
			General::Point<float> result;
			if (checkcoll_line_line(&top_pt, &bottom_pt, &p.first, &p.second, &result))
			{
				float y_dist = result.y - top_pt.y;
				if (y_dist >= 0 && y_dist < closest) {
					found = true;
					closest_pt = result;
					closest = y_dist;
					res_x = result.x;
					found_i = i;
				}
			}
		}

		if (found) {
			// portion of shadow to draw (to avoid overhang of shadow)
			int start_x = 0;
			int end_x = hero_shadow_w;
			std::vector< std::vector< General::Point<int> > > &geo = battle_loop->get_geometry();
			int platform = battle_loop->get_sector_platform(pos.x / Battle_Loop::SECTOR_SIZE, found_i);
			float first_x = geo[platform][0].x;
			float last_x = geo[platform][geo[platform].size()-1].x;
			if ((res_x-first_x) < hero_shadow_w/2) {
				start_x = (hero_shadow_w/2) - (res_x-first_x);
			}
			else {
				start_x = 0;
			}
			if ((last_x-res_x) < hero_shadow_w/2) {
				end_x = (hero_shadow_w/2) + (last_x-res_x) + 1;
			}
			else {
				end_x = hero_shadow_w;
			}
			General::Point<float> top = battle_loop->get_top();
			float scale;
			if (closest > 100.0f) {
				scale = 0.5f;
			}
			else if (closest <= 0.0f) {
				scale = 1.0f;
			}
			else {
				scale = (0.5f - (0.5f * (closest / 100.0f))) + 0.5f;
			}
			int width_to_draw = end_x - start_x;
			al_draw_scaled_bitmap(
				hero_shadow->bitmap,
				start_x, 0,
				width_to_draw, hero_shadow_h,
				closest_pt.x - top.x - (hero_shadow_w/2-start_x)*scale,
				closest_pt.y - top.y - (hero_shadow_h/2)*scale-2,
				width_to_draw*scale,
				hero_shadow_h*scale,
				0
			);
		}
	}
}

void Battle_Entity::draw()
{
	if (first_run) {
		return;
	}

	/* KEEPME: draw weapon polygons */
	/*
	if (attacking) {
		General::Point<float> top = battle_loop->get_top();

		std::string aname = anim_set->get_sub_animation_name();
		int curr_frame = anim_set->get_current_animation()->get_current_frame_num();
		std::pair<std::string, int> p;
		p.first = aname;
		p.second = curr_frame;
		for (size_t i = 0; i < weapon_bones[p].size(); i++) {
			Bones::Bone &weap_bone = weapon_bones[p][i];
			std::vector<Triangulate::Triangle> &weap_tris = 
				(skeleton || facing_right) ? weap_bone.get() : weap_bone.get_mirrored();
			std::vector<Triangulate::Triangle>::iterator it = weap_tris.begin();
			for (; it != weap_tris.end(); it++) {
				Triangulate::Triangle &t = *it;
				al_draw_triangle(
					pos.x+t.points[0].x-top.x,
					pos.y+t.points[0].y-top.y + General::TILE_SIZE,
					pos.x+t.points[1].x-top.x,
					pos.y+t.points[1].y-top.y + General::TILE_SIZE,
					pos.x+t.points[2].x-top.x,
					pos.y+t.points[2].y-top.y + General::TILE_SIZE,
					al_color_name("red"), 1);
			}
		}
	}
	*/
}

void Battle_Entity::script_post_draw()
{
	if (first_run) {
		return;
	}

	Battle_Enemy *enemy;
	if ((enemy = dynamic_cast<Battle_Enemy *>(this))) {
		bool exists;
		lua_State *lua_state = enemy->get_lua_state();

		lua_getglobal(lua_state, "post_draw");
		exists = !lua_isnil(lua_state, -1);
		lua_pop(lua_state, 1);
		if (exists) {
			Lua::call_lua(lua_state, "post_draw", ">");
		}
	}
}

void Battle_Entity::post_draw()
{
	if (first_run) {
		return;
	}

	script_post_draw();

	if (attributes.status.name == "POISON") {
		General::Point<float> top = battle_loop->get_top();
		std::pair<std::string, int> key;
		key.first = "battle-idle";
		key.second = 0;
		int my_bones_h = get_bones()[key][0].get_extents().h;
		General::draw_poison_bubbles(General::Point<float>(pos.x-top.x, pos.y-top.y-my_bones_h));
	}
}

void Battle_Entity::ui_draw()
{
	if (attributes.hp <= 0) {
		return;
	}

	float alpha;
	if (item_circle_state == ITEM_CIRCLE_ITEMS_FADING_IN || item_circle_state == ITEM_CIRCLE_PLAYERS_FADING_IN) {
		double elapsed = al_get_time() - item_circle_fade_start;
		if (elapsed > ITEM_CIRCLE_FADE_TIME) {
			elapsed = ITEM_CIRCLE_FADE_TIME;
		}
		alpha = elapsed / ITEM_CIRCLE_FADE_TIME;
	}
	else if (item_circle_state == ITEM_CIRCLE_ITEMS_FADING_OUT || item_circle_state == ITEM_CIRCLE_PLAYERS_FADING_OUT) {
		double elapsed = al_get_time() - item_circle_fade_start;
		if (elapsed > ITEM_CIRCLE_FADE_TIME) {
			elapsed = ITEM_CIRCLE_FADE_TIME;
		}
		alpha = 1.0f - (elapsed / ITEM_CIRCLE_FADE_TIME);
	}
	else {
		alpha = 1.0f;
	}
	General::Point<float> center = pos;
	center.y -= 16;
	float buffer = ITEM_CIRCLE_RADIUS + ITEM_CIRCLE_ICON_RADIUS;
	if (center.x < buffer) {
		center.x = buffer;
	}
	else if (center.x > battle_loop->get_width()-buffer) {
		center.x = battle_loop->get_width()-buffer;
	}
	if (center.y < buffer) {
		center.y = buffer;
	}
	else if (center.y > battle_loop->get_height()-buffer) {
		center.y = battle_loop->get_height()-buffer;
	}
	center.x -= battle_loop->get_top().x;
	center.y -= battle_loop->get_top().y;
	float angle = item_circle_angle + M_PI*3/2;
	if (item_circle_state == ITEM_CIRCLE_ITEMS || item_circle_state == ITEM_CIRCLE_ITEMS_FADING_IN || item_circle_state == ITEM_CIRCLE_ITEMS_FADING_OUT) {
		for (int i = 0; i < 7; i++) {
			Wrap::Bitmap *bmp = item_icons[item_names[i]];
			int w = al_get_bitmap_width(bmp->bitmap);
			int h = al_get_bitmap_height(bmp->bitmap);
			float dx = center.x + cos(angle) * ITEM_CIRCLE_RADIUS;
			float dy = center.y + sin(angle) * ITEM_CIRCLE_RADIUS;
			float r, g, b;
			if (circle_items[i].num_in_inventory > 0) {
				r = g = b = 1.0f;
			}
			else {
				r = g = b = 0.5f;
			}
			al_draw_tinted_rotated_bitmap(bmp->bitmap, al_map_rgba_f(r*alpha, g*alpha, b*alpha, alpha), w/2, h/2, dx, dy, 0.0f, 0);
			angle = angle + (M_PI*2) / 7;
		}
		draw_item_circle_text(center.x, center.y-buffer, std::string(t(item_names[item_circle_index].c_str())) + " x " + General::itos(MIN(99, circle_items[item_circle_index].num_in_inventory)));
	}
	else if (item_circle_state == ITEM_CIRCLE_PLAYERS || item_circle_state == ITEM_CIRCLE_PLAYERS_FADING_IN || item_circle_state == ITEM_CIRCLE_PLAYERS_FADING_OUT) {

		std::vector<std::string> names;
		std::vector<Battle_Entity *> entities = battle_loop->get_entities();

		int sz = entities.size();
		for (int i = 0; i < sz; i++) {
			Battle_Player *p = dynamic_cast<Battle_Player *>(entities[i]);
			if (p) {
				std::string name = p->get_name();
				if (name == "egbert" || name == "frogbert" || name == "bisou") {
					names.push_back(name);
				}
			}
		}

		for (size_t i = 0; i < names.size(); i++) {
			Wrap::Bitmap *bmp = player_icons[names[i]];
			int w = al_get_bitmap_width(bmp->bitmap);
			int h = al_get_bitmap_height(bmp->bitmap);
			float dx = center.x + cos(angle) * ITEM_CIRCLE_RADIUS;
			float dy = center.y + sin(angle) * ITEM_CIRCLE_RADIUS;
			al_draw_tinted_rotated_bitmap(bmp->bitmap, al_map_rgba_f(alpha, alpha, alpha, alpha), w/2, h/2, dx, dy, 0.0f, 0);
			angle = angle + (M_PI*2) / names.size();
		}
		draw_item_circle_text(center.x, center.y-buffer, General::get_hero_printable_name(names[item_circle_index]));
	}
}

void Battle_Entity::apply_force(Hit_Direction direction, bool facing_right, General::Point<float> force)
{
	if (!flying_entity) {
		accel.x += (facing_right ? 1.0 : -1.0) * force.x;
		if (direction != HIT_SIDE) {
			if (direction == HIT_UP) {
				accel.y += -force.y;
			}
			else if (direction == HIT_DOWN) {
				accel.y += force.y;
				inc_y(-1);
				on_ground = false;
				check_platform = true;
				jumps = 2;
			}
			on_ground = false;
			check_platform = true;
			platform = -1;
			jumping = true;
			jumps = 2;
		}
		// no y for HIT_SIDE
	}
}

void Battle_Entity::take_damage(Battle_Entity *hitter, Hit_Direction direction, bool facing_right, General::Point<float> force, bool hard, bool long_hurt, int particle_damage, float multiplier)
{
	hurt = true;

	if (name == "antboss") {
		hurt_end = al_get_time() + 1.0;
		return;
	}

	hurt_end = al_get_time() + 0.5 + (long_hurt ? 1.5 : 0.0);
	attacking = false;
	casting = false;
	bool set = false;

	if (skeleton || anim_set->get_sub_animation_name() != "recoil") {
		if (skeleton) {
			skeleton->set_curr_anim("hit");
			skeleton->reset_current_animation();
		}
		else {
			if (hard) {
				set = anim_set->set_sub_animation("die");
			}
			if (!set) {
				anim_set->set_sub_animation("hit");
			}
			anim_set->reset();
		}
	}

	if (hard) {
		force.x *= 2.0f;
	}

	apply_force(direction, facing_right, force);

	hit_accel.x = force.x * (facing_right ? 1 : -1);
}

Battle_Attributes &Battle_Entity::get_attributes()
{
	return attributes;
}

double Battle_Entity::get_death_animation_start()
{
	return death_animation_start;
}

void Battle_Entity::construct()
{
	next_attack_time = al_get_time();
	this->facing_right = (bool)(General::rand() % 2);
	int half = battle_loop->get_width() / 2;
	float start_x;
	if (dynamic_cast<Runner_Loop *>(battle_loop)) {
		start_x = 40;
		this->facing_right = true;
	}
	else if (battle_loop->is_cart_battle() && name == "frogbert") {
		start_x = half;
		this->facing_right = false;
	}
	else if (facing_right) {
		start_x = General::rand() % (half - 100);
	}
	else {
		start_x = General::rand() % (half - 100) + half + 100;
	}
	this->pos = General::Point<float>(start_x, General::BIG_FLOAT);
	this->velocity = General::Point<float>(0, 0);
	this->accel = General::Point<float>(0, 0);
	this->hit_accel = General::Point<float>(0, 0);
	this->hit_velocity = General::Point<float>(0, 0);
	this->platform = -1;
	this->on_ground = true;
	this->jumps = 0;
	this->check_platform = true;
	this->jumping = false;
	this->jump_released = true;
	this->crouching = false;
	this->attacking = false;
	this->defending = false;
	this->hurt = false;
	this->hit_something_this_attack = false;
	this->casting = false;
	this->anim_set = NULL;
	// sfx default names (some are sets)
	this->speed_multiplier = 1.0f;
	//doing_jump_delay = false;
	this->flying_entity = false;
	this->immovable = false;
	this->unhittable = false;
	this->item_circle_state = ITEM_CIRCLE_NONE;
	this->item_circle_index = 0;
	this->item_circle_angle = 0.0f;
	this->visible = true;
	this->attack_name = "attack";
	this->burrowing = false;
	this->draw_shadow = true;
	this->layer = -1; // entity_layer
	this->stops_battle_end = true;
	this->accelerates_quickly = false;
	this->input_disabled = false;

	if (name == "tincan" || name == "bone" || name == "antidote" || name == "dirtysock" || name == "healthvial" || name == "healthjar" || name == "healthflask" || name == "magicvial" || name == "magicjar" || name == "magicflask") {
		_is_item = true;
	}
	else {
		_is_item = false;
	}

	MAX_X_ACCEL = 0.075f;
	MAX_X_ACCEL_ROLL = 0.35f;
	MAX_X_ACCEL_HIT = 0.5f;
	MAX_X_VEL = 3.0f;
	MAX_X_VEL_ROLL = 5.0f;
	MAX_X_VEL_HIT = 4.0f;
	MAX_Y_VEL = 3.5f;
	GROUND_FRICTION = 0.05f;
	AIR_FRICTION = 0.02f;
	MAX_STEEPNESS = 25.0f;
	JUMP_ACCEL = -0.675f;
	GRAVITY_ACCEL = 0.05f;
	MAX_Y_ACCEL = 2.5f;
	WALK_SPEED = 0.065f;
	AIR_WALK_SPEED = 0.0225f;
	ROLL_SPEED = 0.0625f;
	HIT_FRICTION = 0.5f;
	MAX_X_ACCEL_SPECIAL = 4.0f;
	MAX_X_VEL_SPECIAL = 30.0f;

	tint = al_color_name("white");

	old_x = pos.x;

	skeleton = new Skeleton::Skeleton(name + ".xml");

	bool success = skeleton->load();

	if (!success) {
		delete skeleton;
		skeleton = NULL;

		std::string path;
		std::string weapon_path = "";

		path = "battle/entities/" + name;

		if (General::is_hero(name) && attributes.equipment.weapon.name != "") {
			weapon_path = "battle/weapons/" + attributes.equipment.weapon.name + "/main";
		}

		if (player) {
			anim_set = player->get_animation_set();
			set_weapon_animation_set(player->get_weapon_animation_set());
		}
		else {
			anim_set = new Animation_Set();
			anim_set->load(path);
		}

		if (General::is_hero(name) && weapon_anim_set) {
			Bones::load(weapon_anim_set, path + "/info.xml", weapon_path, weapon_bones);
		}

		Bones::load(anim_set, path + "/info.xml", path, bones);
	
		if (weapon_anim_set == NULL && anim_set->check_sub_animation_exists("attack-unequipped")) {
			attack_name = "attack-unequipped";
		}
	}
	else {
		attack_name = "attack";
	}

	if (!General::is_hero(name)) {
		attributes.max_hp = 3;
		attributes.hp = 3;
	}

	next_attack = attack_name;
	next_attack_time_end = DBL_MAX;

	first_ice = al_get_time();
	num_ice = 0;
	
	move_up(battle_loop->get_geometry());
}

Battle_Entity::Battle_Entity(Battle_Loop *battle_loop, std::string name) :
	battle_loop(battle_loop),
	name(name),
	ai(NULL),
	player(NULL)
{
}

Battle_Entity::~Battle_Entity()
{
	delete ai;

	if (!player) {
		delete anim_set;
		delete weapon_anim_set;
	}
}

std::string Battle_Entity::get_name()
{
	return name;
}

void Battle_Entity::set_flying_entity(bool flying_entity)
{
	this->flying_entity = flying_entity;
}

bool Battle_Entity::is_flying_entity()
{
	return flying_entity;
}

Player *Battle_Entity::get_player()
{
	return player;
}

void Battle_Entity::init_ai()
{
	ai = new Battle_AI(this);
	ai->init();
}

void Battle_Entity::close_circles()
{
	if (item_circle_state == ITEM_CIRCLE_ITEMS) {
		item_circle_state = ITEM_CIRCLE_ITEMS_FADING_OUT;
		item_circle_fade_start = al_get_time();
	}
	else if (item_circle_state == ITEM_CIRCLE_ITEMS_FADING_IN) {
		item_circle_state = ITEM_CIRCLE_ITEMS_FADING_OUT; // keep fade time
	}
	else if (item_circle_state == ITEM_CIRCLE_PLAYERS) {
		item_circle_state = ITEM_CIRCLE_PLAYERS_FADING_OUT;
		item_circle_fade_start = al_get_time();
	}
	else if (item_circle_state == ITEM_CIRCLE_PLAYERS_FADING_IN) {
		item_circle_state = ITEM_CIRCLE_PLAYERS_FADING_OUT;
	}
}

void Battle_Entity::open_items_circle()
{
	engine->play_sample("sfx/open_circle.ogg", 1.0f, 0.0f, 1.0f);
	item_circle_state = ITEM_CIRCLE_ITEMS_FADING_IN;
	item_circle_fade_start = al_get_time();
	item_circle_angle = 0;
	item_circle_next_repeat = al_get_time();
	item_circle_last_axis = item_circle_axis = 0;
	// Pick first non-empty item
	for (item_circle_index = 0; item_circle_index < 7; item_circle_index++) {
		if (circle_items[item_circle_index].num_in_inventory > 0) {
			break;
		}
	}
}

void Battle_Entity::open_items_player_circle()
{
	engine->play_sample("sfx/open_circle.ogg", 1.0f, 0.0f, 1.0f);
	selected_item = item_circle_index;
	item_circle_state = ITEM_CIRCLE_PLAYERS_FADING_IN;
	item_circle_fade_start = al_get_time();
	item_circle_index = 0;
	item_circle_angle = 0;
	item_circle_next_repeat = al_get_time();
	item_circle_last_axis = item_circle_axis = 0;
}

void Battle_Entity::set_visible(bool visible)
{
	this->visible = visible;
}

bool Battle_Entity::is_visible()
{
	return visible;
}

// ----------------------------------------------

Wrap::Bitmap *Battle_Player::get_profile_bitmap()
{
	return profile_bmp;
}

void Battle_Player::take_damage(Battle_Entity *hitter, Hit_Direction direction, bool facing_right, General::Point<float> force, bool hard, bool long_hurt, int particle_damage, float multiplier)
{
	Battle_Entity::take_damage(hitter, direction, facing_right, force, hard, long_hurt, particle_damage, multiplier);

	int damage;
	if  (hitter) {
		int defense = (attributes.defense + (attributes.equipment.armor.name == "" ? 0 : attributes.equipment.armor.defense));
		Game_Specific_Globals::get_accessory_effects(attributes.equipment.accessory.name, NULL, NULL, NULL, &defense);
		damage = MAX(1, hitter->get_attributes().attack - defense);
	}
	else {
		damage = particle_damage;
	}
	damage *= multiplier;
	attributes.hp -= damage;

	if (battle_loop->get_active_player() == this && attributes.hp <= 0) {
		battle_loop->set_switch_players(true);
	}

	if (attributes.hp <= 0) {
		attributes.status.name = "";

		anim_set->set_sub_animation("die");
		anim_set->reset();

		bool all_dead = true;
		std::vector<Battle_Entity *> all_ents = battle_loop->get_entities();
		int sz = all_ents.size();
		for (int i = 0; i < sz; i++) {
			Battle_Player *p = dynamic_cast<Battle_Player *>(all_ents[i]);
			if (p) {
				Battle_Attributes &attr = p->get_attributes();
				if (attr.hp > 0) {
					all_dead = false;
					break;
				}
			}
		}

		if (all_dead) {
			Music::play("music/game_over.mid", 1, false);
			battle_loop->set_game_over_time(al_get_time());
		}
	}
	
	if (ai) {
		lua_State *lua_state = ai->get_lua_state();
		lua_getglobal(lua_state, "got_hit");
		bool exists = !lua_isnil(lua_state, -1);
		lua_pop(lua_state, 1);
		if (exists) {
			Lua::call_lua(lua_state, "got_hit", "i>", hitter ? hitter->get_id() : -1);
		}
	}
}

void Battle_Player::draw()
{
	Battle_Entity::draw();
}

Battle_Player::Battle_Player(Battle_Loop *battle_loop, Player *player, std::string name, int player_id) :
	Battle_Entity(battle_loop, name),
	player_id(player_id),
	magic_regen_count(0.0f),
	lost_weapon(false)
{
	this->player = player;

	construct2();
}

Battle_Player::~Battle_Player()
{
	resource_manager->release_bitmap("battle/misc_graphics/item_icons/HEALTHVIAL.cpi");
	resource_manager->release_bitmap("battle/misc_graphics/item_icons/HEALTHJAR.cpi");
	resource_manager->release_bitmap("battle/misc_graphics/item_icons/HEALTHFLASK.cpi");
	resource_manager->release_bitmap("battle/misc_graphics/item_icons/MAGICVIAL.cpi");
	resource_manager->release_bitmap("battle/misc_graphics/item_icons/MAGICFLASK.cpi");
	resource_manager->release_bitmap("battle/misc_graphics/item_icons/ANTIDOTE.cpi");
	resource_manager->release_bitmap("battle/misc_graphics/item_icons/DIRTYSOCK.cpi");

	resource_manager->release_bitmap("battle/misc_graphics/player_icons/egbert.cpi");
	resource_manager->release_bitmap("battle/misc_graphics/player_icons/frogbert.cpi");
	resource_manager->release_bitmap("battle/misc_graphics/player_icons/bisou.cpi");

	Wrap::destroy_bitmap(profile_bmp);
}

void Battle_Player::handle_event(ALLEGRO_EVENT *event)
{
	if (input_disabled) {
		return;
	}

	if (attributes.hp <= 0 || burrowing) {
		return;
	}

	if (this != battle_loop->get_active_player()) {
		return;
	}

	if (event->type == ALLEGRO_EVENT_JOYSTICK_AXIS && event->joystick.stick == 0) {
		float pos = event->joystick.pos;
		if (event->joystick.axis == 0) {
			if (fabs(pos) < 0.2f) {
				item_circle_axis = 0;
				input[X] = 0;
			}
			else {
				input[X] = pos;
			}
		}
		else {
			if (fabs(pos) < 0.2f) {
				input[Y] = 0;
			}
			else {
				input[Y] = pos;
			}
		}
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
		if (event->joystick.button == cfg.joy_ability[0]) {
			execute_ability(player->get_selected_abilities(true, dynamic_cast<Runner_Loop *>(battle_loop), battle_loop->is_cart_battle())[0], true);
		}
		else if (event->joystick.button == cfg.joy_ability[1]) {
			execute_ability(player->get_selected_abilities(true, dynamic_cast<Runner_Loop *>(battle_loop), battle_loop->is_cart_battle())[1], true);
		}
		else if (event->joystick.button == cfg.joy_ability[2]) {
			execute_ability(player->get_selected_abilities(true, dynamic_cast<Runner_Loop *>(battle_loop), battle_loop->is_cart_battle())[2], true);
		}
		else if (event->joystick.button == cfg.joy_ability[3]) {
			execute_ability(player->get_selected_abilities(true, dynamic_cast<Runner_Loop *>(battle_loop), battle_loop->is_cart_battle())[3], true);
		}
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP) {
		if (event->joystick.button == cfg.joy_ability[0]) {
			execute_ability(player->get_selected_abilities(true, dynamic_cast<Runner_Loop *>(battle_loop), battle_loop->is_cart_battle())[0], false);
		}
		else if (event->joystick.button == cfg.joy_ability[1]) {
			execute_ability(player->get_selected_abilities(true, dynamic_cast<Runner_Loop *>(battle_loop), battle_loop->is_cart_battle())[1], false);
		}
		else if (event->joystick.button == cfg.joy_ability[2]) {
			execute_ability(player->get_selected_abilities(true, dynamic_cast<Runner_Loop *>(battle_loop), battle_loop->is_cart_battle())[2], false);
		}
		else if (event->joystick.button == cfg.joy_ability[3]) {
			execute_ability(player->get_selected_abilities(true, dynamic_cast<Runner_Loop *>(battle_loop), battle_loop->is_cart_battle())[3], false);
		}
	}
	else {
		switch (event->type) {
			case ALLEGRO_EVENT_KEY_DOWN: {
				if (anim_set->get_sub_animation_name() != "die") {
					int keycode = event->keyboard.keycode;
					if (keycode == cfg.key_left) {
						input[X] = -1;
						accel.x = 0;
						velocity.x = 0;
					}
					else if (keycode == cfg.key_right) {
						input[X] = 1;
						accel.x = 0;
						velocity.x = 0;
					}
					else if (keycode == cfg.key_up) {
						input[Y] = -1;
					}
					else if (keycode == cfg.key_down) {
						input[Y] = 1;
					}
					for (int j = 0; j < 4; j++) {
						if (keycode == cfg.key_ability[j]) {
							execute_ability(player->get_selected_abilities(true, dynamic_cast<Runner_Loop *>(battle_loop), battle_loop->is_cart_battle())[j], true);
						}
					}
					break;
				}
			}
			case ALLEGRO_EVENT_KEY_UP: {
				int keycode = event->keyboard.keycode;
				if (keycode == cfg.key_left) {
					item_circle_axis = 0;
					if (input[X] < 0) {
						input[X] = 0;
					}
				}
				else if (keycode == cfg.key_right) {
					item_circle_axis = 0;
					if (input[X] > 0) {
						input[X] = 0;
					}
				}
				else if (keycode == cfg.key_up ||
					keycode == cfg.key_down) {
					input[Y] = 0;
				}
				for (int j = 0; j < 4; j++) {
					if (keycode == cfg.key_ability[j]) {
						execute_ability(player->get_selected_abilities(true, dynamic_cast<Runner_Loop *>(battle_loop), battle_loop->is_cart_battle())[j], false);
					}
				}
				break;
			}
		}
	}

	if (!casting && !immovable) {
		if (input[X] < 0)
			facing_right = false;
		else if (input[X] > 0)
			facing_right = true;
	}
}

void Battle_Player::logic()
{
	if (first_run) {
		first_run = false;
		init_ai();
	}

	if (!battle_loop->battle_transition_is_done()) {
		return;
	}

	if (attributes.hp <= 0) {
		close_circles();
		anim_set->update(BATTLE_LOGIC_MILLIS);
		if (flying_entity) {
			flying_entity_logic();
		}
		else {
			ground_entity_logic();
		}
		return;
	}

	if (this != battle_loop->get_active_player()) {
		ai->top();
		ai->logic();
	}

	Battle_Entity::logic();
}

void Battle_Player::stop()
{
	if (this != battle_loop->get_active_player()) {
		ai->stop();
	}
}

void Battle_Player::construct2()
{
	first_run = true;

	profile_bmp = Wrap::load_bitmap("battle/misc_graphics/profiles/" + name + "_profile.cpi");
	
	attributes = player->get_battle_attributes();

	sfx_name_attack = Game_Specific_Globals::get_weapon_swing_sfx(attributes.equipment.weapon.name);
	sfx_name_hit = Game_Specific_Globals::get_weapon_hit_sfx(attributes.equipment.weapon.name);
	
	item_icons["HEALTHVIAL"] = resource_manager->reference_bitmap("battle/misc_graphics/item_icons/HEALTHVIAL.cpi");
	item_icons["HEALTHJAR"] = resource_manager->reference_bitmap("battle/misc_graphics/item_icons/HEALTHJAR.cpi");
	item_icons["HEALTHFLASK"] = resource_manager->reference_bitmap("battle/misc_graphics/item_icons/HEALTHFLASK.cpi");
	item_icons["MAGICVIAL"] = resource_manager->reference_bitmap("battle/misc_graphics/item_icons/MAGICVIAL.cpi");
	item_icons["MAGICFLASK"] = resource_manager->reference_bitmap("battle/misc_graphics/item_icons/MAGICFLASK.cpi");
	item_icons["ANTIDOTE"] = resource_manager->reference_bitmap("battle/misc_graphics/item_icons/ANTIDOTE.cpi");
	item_icons["DIRTYSOCK"] = resource_manager->reference_bitmap("battle/misc_graphics/item_icons/DIRTYSOCK.cpi");

	player_icons["egbert"] = resource_manager->reference_bitmap("battle/misc_graphics/player_icons/egbert.cpi");
	player_icons["frogbert"] = resource_manager->reference_bitmap("battle/misc_graphics/player_icons/frogbert.cpi");
	player_icons["bisou"] = resource_manager->reference_bitmap("battle/misc_graphics/player_icons/bisou.cpi");
}

void Battle_Player::execute_ability(std::string ability, bool onoff)
{
	if (item_circle_state != ITEM_CIRCLE_NONE) {
		if (onoff) {
			if (ability == "USE" && item_circle_reason == ITEM_CIRCLE_REASON_ITEM) {
				if (item_circle_state == ITEM_CIRCLE_ITEMS) {
					open_items_player_circle();
				}
				else if (item_circle_state == ITEM_CIRCLE_PLAYERS) {
					std::vector<Battle_Entity *> &entities = battle_loop->get_entities();
					int count = 0;
					Battle_Player *player = NULL;
					int sz = entities.size();
					for (int i = 0; i < sz; i++) {
						if ((player = dynamic_cast<Battle_Player *>(entities[i]))) {
							if (count == item_circle_index) {
								break;
							}
							count++;
						}
					}
					if (player) {
						Game_Specific_Globals::use_item(item_names[selected_item], player->get_attributes(), true);
					}
					std::vector<Game_Specific_Globals::Item> &items =
						Game_Specific_Globals::get_items();
					int index = circle_items[selected_item].smallest_stack_index;
					items[index].quantity--;
					if (items[index].quantity <= 0) {
						items.erase(items.begin()+index);
					}
					close_circles();
				}
			}
			else if (ability == "HEAL" && item_circle_reason == ITEM_CIRCLE_REASON_HEAL) {
				if (item_circle_state == ITEM_CIRCLE_PLAYERS) {
					std::vector<Battle_Entity *> &entities = battle_loop->get_entities();
					int count = 0;
					Battle_Player *player = NULL;
					int sz = entities.size();
					for (int i = 0; i < sz; i++) {
						if ((player = dynamic_cast<Battle_Player *>(entities[i]))) {
							if (count == item_circle_index) {
								break;
							}
							count++;
						}
					}
					if (player) {
						if (Game_Specific_Globals::take_magic(attributes, "HEAL")) {
							heal(player->get_id());
							input[HEAL] = 0;
						}
					}
					close_circles();
				}
			}
			else {
				close_circles();
			}
		}
		return;
	}

	if (onoff) {
		if (ability == "JUMP") {
			input[JUMP] = 1;
		}
		else if (ability == "ATTACK") {
			input[ATTACK] = 1;
		}
		else if (ability == "USE") {
			// Fill in some details about inventory
			std::vector<Game_Specific_Globals::Item> &items =
				Game_Specific_Globals::get_items();
			num_items_found = 0;
			for (int i = 0; i < 7; i++) {
				// Find smallest stack and total quantity
				int total = 0;
				int smallest = INT_MAX;
				int smallest_index = -1;
				bool found = false;
				for (size_t j = 0; j < items.size(); j++) {
					if (items[j].name == item_names[i]) {
						found = true;
						total += items[j].quantity;
						// less than equal so it picks the one near the end
						if (items[j].quantity <= smallest) {
							smallest = items[j].quantity;
							smallest_index = j;
						}
					}
				}
				if (found) {
					num_items_found++;
				}
				circle_items[i].num_in_inventory = total;
				circle_items[i].smallest_stack_index = smallest_index;
			}
			if (num_items_found > 0) {
				item_circle_reason = ITEM_CIRCLE_REASON_ITEM;
				open_items_circle();
			}
			else {
				engine->play_sample("sfx/error.ogg");
			}
		}
		else if (ability == "HEAL") {
			item_circle_reason = ITEM_CIRCLE_REASON_HEAL;
			open_items_player_circle();
		}
		else if (ability == "ICE") {
			input[ICE] = 1;
		}
		else if (ability == "SLASH") {
			input[SLASH] = 1;
		}
		else if (ability == "THROW") {
			input[THROW] = 1;
		}
		else if (ability == "KICK") {
			input[KICK] = 1;
		}
		else if (ability == "PLANT") {
			input[PLANT] = 1;
		}
		else if (ability == "FIRE") {
			input[FIRE] = 1;
		}
		else if (ability == "ROLL") {
			input[ROLL] = 1;
		}
		else if (ability == "BURROW") {
			input[BURROW] = 1;
		}
		else if (ability == "HEAL") {
			input[HEAL] = 1;
		}
	}
	else {
		if (ability == "JUMP") {
			input[JUMP] = 0;
		}
		else if (ability == "ATTACK") {
			input[ATTACK] = 0;
		}
		else if (ability == "ICE") {
			input[ICE] = 0;
		}
		else if (ability == "SLASH") {
			input[SLASH] = 0;
		}
		else if (ability == "THROW") {
			input[THROW] = 0;
		}
		else if (ability == "KICK") {
			input[KICK] = 0;
		}
		else if (ability == "PLANT") {
			input[PLANT] = 0;
		}
		else if (ability == "FIRE") {
			input[FIRE] = 0;
		}
		else if (ability == "ROLL") {
			input[ROLL] = 0;
		}
		else if (ability == "BURROW") {
			input[BURROW] = 0;
		}
		else if (ability == "HEAL") {
			input[HEAL] = 0;
		}
	}
}

float Battle_Player::get_magic_regen_count()
{
	return magic_regen_count;
}

void Battle_Player::set_magic_regen_count(float count)
{
	magic_regen_count = count;
}

void Battle_Player::heal(int who)
{
	engine->play_sample("sfx/heal_cast.ogg");
	casting = true;
	anim_set->set_sub_animation("magic");
	anim_set->reset();
	Lua::call_lua(battle_loop->get_lua_state(), "heal", "ii>", who, 10);
}

void Battle_Enemy::handle_event(ALLEGRO_EVENT *event)
{
	if (ai) {
		ai->handle_event(event);
	}
}

void Battle_Enemy::logic()
{
	if (first_run) {
		first_run = false;
		init_ai();
		lua_State *stack = ai->get_lua_state();
		Lua::call_lua(stack, "get_attack_sound", ">s");
		sfx_name_attack = lua_tostring(stack, 1);
		lua_pop(stack, 1);
		engine->load_sample(sfx_name_attack);
	}

	if (!battle_loop->battle_transition_is_done()) {
		return;
	}

	if (attributes.hp <= 0 &&
		death_animation_start < al_get_time()-DEATH_ANIM_LENGTH)
	{
		delete_me = true;
	}

	ai->top();
	ai->logic();

	Battle_Entity::logic();
}

void Battle_Enemy::take_damage(Battle_Entity *hitter, Hit_Direction direction, bool facing_right, General::Point<float> force, bool hard, bool long_hurt, int particle_damage, float multiplier)
{
	if (ai) {
		ai->stop();
	}

	Battle_Entity::take_damage(hitter, direction, facing_right, force, hard, long_hurt, particle_damage, multiplier);

	if (attributes.hp > 0 && hitter != this) {
		Battle_Player *player = dynamic_cast<Battle_Player *>(hitter);
		if (player) {
			Battle_Attributes &attr = player->get_attributes();
			int accessory_attack = 0;
			Game_Specific_Globals::get_accessory_effects(attr.equipment.accessory.name, NULL, NULL, &accessory_attack, NULL);
			int damage;
			if (player->get_weapon_animation_set()) {
				damage = attr.equipment.weapon.attack + accessory_attack;
			}
			else {
				damage = attr.attack + accessory_attack;
			}
			attributes.hp -= damage * multiplier * cfg.difficulty_mult();
		}
		else {
			attributes.hp -= particle_damage * multiplier * cfg.difficulty_mult();
		}
		if (attributes.hp <= 0) {
			engine->play_sample("sfx/enemy_die.ogg");
			Lua::call_lua(get_lua_state(), "die", ">");
			death_animation_start = al_get_time();
			if (skeleton) {
				skeleton->set_curr_anim("death");
			}
			else {
				anim_set->set_sub_animation("death");
				anim_set->reset();
			}
			if (weapon_anim_set) {
				weapon_anim_set->set_sub_animation("death");
				weapon_anim_set->reset();
			}
		}
	}
	
	if (ai) {
		lua_State *lua_state = ai->get_lua_state();
		lua_getglobal(lua_state, "got_hit");
		bool exists = !lua_isnil(lua_state, -1);
		lua_pop(lua_state, 1);
		if (exists) {
			Lua::call_lua(lua_state, "got_hit", "i>", hitter ? hitter->get_id() : -1);
		}
	}
}

Battle_Enemy::Battle_Enemy(Battle_Loop *battle_loop, std::string name) :
	Battle_Entity(battle_loop, name)
{
	first_run = true;

	attributes.attack = 1;

	sfx_name_attack = "sfx/swing_weapon.ogg";
	sfx_name_hit = "sfx/hit.ogg";
}

void Battle_Enemy::stop()
{
	ai->stop();
}

Battle_Enemy::~Battle_Enemy()
{
}

Collidable_Type Battle_Entity::collidable_get_type()
{
	return COLLIDABLE_BONES;
}

void Battle_Entity::collidable_get_position(
		General::Point<float> &pos
	)
{
	pos = General::Point<float>(this->pos.x, this->pos.y + General::BOTTOM_SPRITE_PADDING);
}

void Battle_Entity::collidable_get_bones(
		std::vector<Bones::Bone> &bones
	)
{
	if (skeleton) {
	}
	else {
		std::pair<std::string, int> p;
		p.first = anim_set->get_current_animation()->get_name();
		p.second = anim_set->get_current_animation()->get_current_frame_num();
		bones = this->bones[p];
	}
}
       	
float Battle_Entity::ground_diff(float dx, std::vector< std::vector< General::Point<int> > > &geometry)
{
	if (dx == 0)
		return 0;

	std::vector< General::Point<int> > &plat = geometry[platform];
	General::Point<float> pos = General::Point<float>(this->pos.x+dx, this->pos.y);

	if (pos.x < 1) {
		pos.x = 1;
	}
	else if (pos.x > battle_loop->get_width()-1) { // FIXME: -2???
		pos.x = battle_loop->get_width()-1;
	}

	int point;

	if (plat.size() == 2) {
		point = 1;
	}
	else {
		int sz = plat.size();
		for (point = 1; point < sz; point++) {
			if (plat[point].x >= pos.x && plat[point-1].x <= pos.x) {
				break;
			}
		}
		if (point == (int)plat.size()) {
			return General::BIG_FLOAT;
		}
	}

	int length = plat[point].x - plat[point-1].x;
	int height = plat[point].y - plat[point-1].y;
	int rel_x = pos.x - plat[point-1].x;

	if (plat.size() == 2) {
		if (rel_x < 0 || rel_x >= length)
			return General::BIG_FLOAT;
	}

	return (plat[point-1].y + ((float)rel_x/length) * height) - this->pos.y;
}

bool Battle_Entity::get_hit_something_this_attack()
{
	return hit_something_this_attack;
}

bool Battle_Entity::is_immovable()
{
	return immovable;
}

void Battle_Entity::set_immovable(bool immovable)
{
	this->immovable = immovable;
}

float Battle_Entity::get_height_from_ground()
{
	std::vector< std::pair< General::Point<float>, General::Point<float> > > sector = battle_loop->get_sector(pos.x / Battle_Loop::SECTOR_SIZE);

	float closest = General::BIG_FLOAT;
	bool found = false;

	int sz = sector.size();
	for (int i = 0; i < sz; i++) {
		General::Point<float> top_pt =
			General::Point<float>(pos.x, pos.y-1);
		General::Point<float> bottom_pt =
			General::Point<float>(pos.x, General::BIG_FLOAT);
		std::pair< General::Point<float>, General::Point<float> > &p = sector[i];
		General::Point<float> result;
		if (checkcoll_line_line(&top_pt, &bottom_pt, &p.first, &p.second, &result)) {
			float y_dist = result.y - top_pt.y;
			if (y_dist >= 0 && y_dist < closest) {
				closest = y_dist;
				found = true;
			}
		}
	}

	if (found) {
		return closest;
	}

	return 0.0f;
}

lua_State *Battle_Entity::get_lua_state()
{
	if (ai) {
		return ai->get_lua_state();
	}
	return NULL;
}

bool Battle_Entity::is_burrowing()
{
	return burrowing;
}

double Battle_Entity::get_burrow_time()
{
	return burrow_time;
}

General::Point<float> Battle_Entity::get_burrow_start_position()
{
	return burrow_start_pos;
}

bool Battle_Entity::is_item()
{
	return _is_item;
}

int Battle_Entity::get_layer()
{
	return layer;
}

void Battle_Entity::set_layer(int layer)
{
	this->layer = layer;
}

bool Battle_Entity::get_stops_battle_end()
{
	return stops_battle_end;
}

void Battle_Entity::set_stops_battle_end(bool stops_battle_end)
{
	this->stops_battle_end = stops_battle_end;
}

bool Battle_Entity::can_accelerate_quickly()
{
	return accelerates_quickly;
}

void Battle_Entity::set_can_accelerate_quickly(bool accelerates_quickly)
{
	this->accelerates_quickly = accelerates_quickly;
}

bool Battle_Entity::fully_initialized()
{
	return first_run == false;
}
