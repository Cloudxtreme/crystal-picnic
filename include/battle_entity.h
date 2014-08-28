#ifndef BATTLE_ENTITY_H
#define BATTLE_ENTITY_H

#include <map>
#include <cfloat>

#include "animation.h"
#include "general.h"
#include "animation_set.h"
#include "user_events.h"
#include "battle_ai.h"
#include "bones.h"
#include "battle_loop.h"
#include "skeleton.h"
#include "visible_entity.h"
#include "weaponized_entity.h"
#include "collidable.h"
#include "equipment.h"
#include "abilities.h"

// FIXME: set these with the engine
#define BATTLE_LOGIC_RATE 60
#define BATTLE_LOGIC_MILLIS (1000.0/BATTLE_LOGIC_RATE)

#define BURROW_TICKS 150
#define BURROW_START 0
#define BURROW_WAIT 1
#define BURROW_WAIT2 2

struct Battle_Attributes {
	int hp;
	int max_hp;
	int mp;
	int max_mp;
	int attack;
	int defense;
	Equipment::Equipment equipment;
	Abilities::Abilities abilities;
	struct {
		std::string name;
		int count;
	} status;
};

class Player;

class Battle_Entity : public Positioned_Entity, public Visible_Entity, public Weaponized_Entity, public Collidable {
public:
	static const int X;
	static const int Y;
	static const int JUMP;
	static const int ATTACK;
	static const int USE;
	static const int ICE;
	static const int SLASH;
	static const int THROW;
	static const int KICK;
	static const int PLANT;
	static const int FIRE;
	static const int ROLL;
	static const int BURROW;
	static const int HEAL;
	
	static const float DEATH_ANIM_LENGTH;
	static const float ITEM_CIRCLE_RADIUS;
	static const float ITEM_CIRCLE_ICON_RADIUS;
	static const double ITEM_CIRCLE_REPEAT_DELAY;
	static const double ITEM_CIRCLE_FADE_TIME;
	static const float ITEM_CIRCLE_ROTATION_TIME;

	enum Hit_Direction {
		HIT_SIDE = 0,
		HIT_UP,
		HIT_DOWN,
		HIT_STOP
	};

	enum Item_Circle_State {
		ITEM_CIRCLE_NONE,
		ITEM_CIRCLE_ITEMS_FADING_IN,
		ITEM_CIRCLE_ITEMS_FADING_OUT,
		ITEM_CIRCLE_ITEMS,
		ITEM_CIRCLE_PLAYERS_FADING_IN,
		ITEM_CIRCLE_PLAYERS_FADING_OUT,
		ITEM_CIRCLE_PLAYERS
	};

	enum Item_Circle_Reason {
		ITEM_CIRCLE_REASON_ITEM,
		ITEM_CIRCLE_REASON_HEAL
	};

	Battle_AI *get_ai();

	std::string get_name();

	void set_speed_x(float percent, int dir);

	void set_speed_multiplier(float mult);
	float get_speed_multiplier() { return speed_multiplier; }

	void gravity();

	virtual void handle_event(ALLEGRO_EVENT *event) {}
	virtual void logic();
	virtual void draw();
	virtual void take_damage(Battle_Entity *hitter, Hit_Direction direction, bool facing_right, General::Point<float> force, bool hard, bool long_hurt, int particle_damage, float multiplier);
	virtual void stop() {};
	
	void pre_draw();
	void post_draw();
	void script_pre_draw();
	void script_post_draw();
	void ui_draw();

	void apply_force(Hit_Direction direction, bool facing_right, General::Point<float> force);

	float ground_diff(float dx, std::vector< std::vector< General::Point<int> > > &geometry);

	void move_up(std::vector< std::vector< General::Point<int> > > &geometry) {
		int i1, i2, plat;
		float line_y = get_closest_line_vertically(pos, true, geometry, &i1, &i2, &plat);
		if (i1 < 0) {
			return;
		}
		pos.y = line_y-0.5f;
		old_y = pos.y;
		platform = plat;
	}

	General::Point<float> &get_velocity() {
		return velocity;
	}
	void set_velocity(General::Point<float> v) {
		velocity = v;
	}
	General::Point<float> &get_accel() {
		return accel;
	}
	void set_accel(General::Point<float> a) {
		accel = a;
	}

	int get_platform() { return platform; }
	void set_platform(int p) { platform = p; }

	bool is_on_ground() { return on_ground; }
	void set_on_ground(bool onoff) { on_ground = onoff; }

	int get_jumps() { return jumps; }
	void set_jumps(int j) { jumps = j; }

	bool get_check_platform() { return check_platform; }
	void set_check_platform(bool onoff) { check_platform = onoff; }

	bool is_jumping() { return jumping; }
	void set_jumping(bool onoff) { jumping = onoff; }

	bool is_jump_released() { return jump_released; }
	void set_jump_released(bool onoff) { jump_released = onoff; }

	bool is_facing_right() { return facing_right; }
	void set_facing_right(bool r) { facing_right = r; }

	bool is_attacking() { return attacking; }
	void set_attacking(bool a) { attacking = a; if (!attacking) { hit_something_this_attack = false; } }
	bool is_defending() { return defending; }
	void set_defending(bool d) { defending = d; }

	bool is_hurt() { return hurt; }
	void set_hurt(bool h) { hurt = h; }
	void set_hurt_end(double hurt_end) { this->hurt_end = hurt_end; }

	bool is_unhittable() { return unhittable; }
	void set_unhittable(bool onoff) { unhittable = onoff; }

	std::map<const int, float> &get_input() { return input; }

	std::map< std::pair<std::string, int>, std::vector<Bones::Bone> >
		&get_bones() { return bones; }
	std::map< std::pair<std::string, int>, std::vector<Bones::Bone> >
		&get_weapon_bones() { return weapon_bones; }
	
	Battle_Attributes &get_attributes();

	double get_death_animation_start();
	void set_death_animation_start(double start) { death_animation_start = start; }

	ALLEGRO_COLOR get_tint();
	void set_tint(ALLEGRO_COLOR tint);

	// Collidable interface
	Collidable_Type collidable_get_type();
	void collidable_get_position(
		General::Point<float> &pos
	);
	void collidable_get_bones(
		std::vector<Bones::Bone> &bones
	);

	void construct();

	Battle_Entity(Battle_Loop *battle_loop, std::string name);
	virtual ~Battle_Entity();

	void set_physics(
		float MAX_X_ACCEL = -1,
		float MAX_X_ACCEL_ROLL = -1,
		float MAX_X_ACCEL_HIT = -1,
		float MAX_X_VEL = -1,
		float MAX_X_VEL_ROLL = -1,
		float MAX_X_VEL_HIT = -1,
		float MAX_Y_VEL = -1,
		float GROUND_FRICTION = -1,
		float AIR_FRICTION = -1,
		float MAX_STEEPNESS = -1,
		float JUMP_ACCEL = -1,
		float GRAVITY_ACCEL = -1,
		float MAX_Y_ACCEL = -1,
		float WALK_SPEED = -1,
		float AIR_WALK_SPEED = -1,
		float ROLL_SPEED = -1,
		float HIT_FRICTION = -1,
		float MAX_X_ACCEL_SPECIAL = -1,
		float MAX_X_VEL_SPECIAL = -1
	);

	void get_physics(
		float *MAX_X_ACCEL,
		float *MAX_X_ACCEL_ROLL,
		float *MAX_X_ACCEL_HIT,
		float *MAX_X_VEL,
		float *MAX_X_VEL_ROLL,
		float *MAX_X_VEL_HIT,
		float *MAX_Y_VEL,
		float *GROUND_FRICTION,
		float *AIR_FRICTION,
		float *MAX_STEEPNESS,
		float *JUMP_ACCEL,
		float *GRAVITY_ACCEL,
		float *MAX_Y_ACCEL,
		float *WALK_SPEED,
		float *AIR_WALK_SPEED,
		float *ROLL_SPEED,
		float *HIT_FRICTION,
		float *MAX_X_ACCEL_SPECIAL,
		float *MAX_X_VEL_SPECIAL
	) {
		if (MAX_X_ACCEL) *MAX_X_ACCEL = this->MAX_X_ACCEL;
		if (MAX_X_ACCEL_ROLL) *MAX_X_ACCEL_ROLL = this->MAX_X_ACCEL_ROLL;
		if (MAX_X_ACCEL_HIT) *MAX_X_ACCEL_HIT = this->MAX_X_ACCEL_HIT;
		if (MAX_X_VEL) *MAX_X_VEL = this->MAX_X_VEL;
		if (MAX_X_VEL_ROLL) *MAX_X_VEL_ROLL = this->MAX_X_VEL_ROLL;
		if (MAX_X_VEL_HIT) *MAX_X_VEL_HIT = this->MAX_X_VEL_HIT;
		if (MAX_Y_VEL) *MAX_Y_VEL = this->MAX_Y_VEL;
		if (GROUND_FRICTION) *GROUND_FRICTION = this->GROUND_FRICTION;
		if (AIR_FRICTION) *AIR_FRICTION = this->AIR_FRICTION;
		if (MAX_STEEPNESS) *MAX_STEEPNESS = this->MAX_STEEPNESS;
		if (JUMP_ACCEL) *JUMP_ACCEL = this->JUMP_ACCEL;
		if (GRAVITY_ACCEL) *GRAVITY_ACCEL = this->GRAVITY_ACCEL;
		if (MAX_Y_ACCEL) *MAX_Y_ACCEL = this->MAX_Y_ACCEL;
		if (WALK_SPEED) *WALK_SPEED = this->WALK_SPEED;
		if (AIR_WALK_SPEED) *AIR_WALK_SPEED = this->AIR_WALK_SPEED;
		if (ROLL_SPEED) *ROLL_SPEED = this->ROLL_SPEED;
		if (HIT_FRICTION) *HIT_FRICTION = this->HIT_FRICTION;
		if (MAX_X_ACCEL_SPECIAL) *MAX_X_ACCEL_SPECIAL = this->MAX_X_ACCEL_SPECIAL;
		if (MAX_X_VEL_SPECIAL) *MAX_X_VEL_SPECIAL = this->MAX_X_VEL_SPECIAL;
	}

	void set_flying_entity(bool flying_entity);
	bool is_flying_entity();

	bool get_hit_something_this_attack();

	Player *get_player();

	bool is_immovable();
	void set_immovable(bool immovable);

	float get_height_from_ground();

	void init_ai();

	void close_circles();

	void set_visible(bool visible);
	bool is_visible();

	lua_State *get_lua_state();

	bool is_burrowing();
	double get_burrow_time();
	General::Point<float> get_burrow_start_position();

	bool is_item();
	bool can_accelerate_quickly();
	void set_can_accelerate_quickly(bool accelerates_quickly);

	int get_layer();
	void set_layer(int layer);

	bool get_stops_battle_end();
	void set_stops_battle_end(bool stops_battle_end);

	bool fully_initialized();

	void set_show_shadow(bool draw_shadow) { this->draw_shadow = draw_shadow; }

	void set_input_disabled(bool disabled) { input_disabled = disabled; }

protected:
	struct Circle_Item {
		int num_in_inventory;
		int smallest_stack_index;
	} circle_items[7];

	void open_items_circle();
	void open_items_player_circle();

	// FIXME: up ignored
	float get_closest_line_vertically(General::Point<float> pos, bool up, std::vector< std::vector< General::Point<int> > > &geometry,
			int *index1, int *index2, int *platform_ret) {
		int closest_index1 = -1;
		int closest_index2 = -1;
		float closest_dist = General::BIG_FLOAT;
		float line_y = -1;
		for (unsigned int plat = 0; plat < geometry.size(); plat++) {
			for (unsigned int point = 1; point < geometry[plat].size(); point++) {
				General::Point<int> &p1 = geometry[plat][point-1];
				General::Point<int> &p2 = geometry[plat][point];
				float y_at_x;
				float length = p2.x - p1.x;
				float rel_x = pos.x - p1.x;
				if (rel_x < 0 || rel_x > length) {
					continue;
				}
				y_at_x = p1.y + (p2.y - p1.y) * (rel_x/length);
				float diff = fabs(y_at_x - pos.y);
				if (diff < closest_dist) {
					closest_index1 = point-1;
					closest_index2 = point;
					closest_dist = diff;
					line_y = y_at_x;
					*platform_ret = plat;
				}
			}
		}
		*index1 = closest_index1;
		*index2 = closest_index2;
		return line_y;
	}

	void handle_weapon_bone(Battle_Entity *ent, Bones::Bone &weap_bone);
	void handle_bone(Battle_Entity *ent, Bones::Bone &bone);
	
	bool ground_entity_logic();
	bool flying_entity_logic();

	std::pair<std::string, int> get_entity_bone_id(Battle_Entity *ent);
	void skeleton_entity_handle_one_part(Skeleton::Link *l, Battle_Entity *ent);
	bool handle_single_weapon_bone_piece(Bones::Bone &weap_bone, Battle_Entity *ent, Bones::Bone &ent_bone, General::Point<float> pos_inc, bool clang_played);
	bool handle_single_bone_piece(Bones::Bone &bone, Battle_Entity *ent, Bones::Bone &ent_bone, General::Point<float> pos_inc);
	bool do_attack(Battle_Entity *ent, Bones::Bone &ent_bone, bool clang_played);
	bool handle_skeleton_target(Bones::Bone &weap_bone, Battle_Entity *ent, Skeleton::Link *link, bool clang_played);

	void inc_y(float inc);

	Battle_Loop *battle_loop;

	General::Point<float> velocity;
	General::Point<float> accel;
	General::Point<float> hit_velocity;
	General::Point<float> hit_accel;
	int platform;
	bool on_ground;
	int jumps;
	bool check_platform;
	bool jumping;
	bool jump_released;
	bool facing_right;
	bool crouching;
	bool attacking;
	bool defending;
	bool hurt;
	bool unhittable;
	bool hit_something_this_attack;
	bool casting;

	double hurt_end;

	std::map<const int, float> input;

	// string is animset sub anim name, int is frame
	std::map< std::pair<std::string, int>, std::vector<Bones::Bone> > bones;
	std::map< std::pair<std::string, int>, std::vector<Bones::Bone> > weapon_bones;

	std::string name;

	std::string sfx_name_attack;
	std::string sfx_name_hit;

	Battle_Attributes attributes;
	double death_animation_start;

	float speed_multiplier;
	float old_x, old_y;

	ALLEGRO_COLOR tint;

	// physical attributes
	float MAX_X_ACCEL;
	float MAX_X_ACCEL_ROLL;
	float MAX_X_ACCEL_HIT;
	float MAX_X_VEL;
	float MAX_X_VEL_ROLL;
	float MAX_X_VEL_HIT;
	float MAX_Y_VEL;
	float GROUND_FRICTION;
	float AIR_FRICTION;
	float MAX_STEEPNESS;
	float JUMP_ACCEL;
	float GRAVITY_ACCEL;
	float MAX_Y_ACCEL;
	float WALK_SPEED;
	float AIR_WALK_SPEED;
	float ROLL_SPEED;
	float HIT_FRICTION;
	float MAX_X_ACCEL_SPECIAL;
	float MAX_X_VEL_SPECIAL;

	double next_attack_time;

	int current_attack_number;

	bool flying_entity;

	Battle_AI *ai;
	bool first_run;

	double first_ice;
	int num_ice;

	Player *player;

	std::string next_attack;
	double next_attack_time_end;

	bool immovable;

	Item_Circle_Reason item_circle_reason;
	Item_Circle_State item_circle_state;
	double item_circle_fade_start;
	int item_circle_index;
	float item_circle_angle;
	double item_circle_next_repeat;
	int item_circle_axis;
	int item_circle_last_axis;
	int selected_item;
	int num_items_found;
	std::map<std::string, Wrap::Bitmap *> item_icons;
	std::map<std::string, Wrap::Bitmap *> player_icons;

	bool visible;

	std::string attack_name;

	double roll_start;

	bool burrowing;
	int burrow_stage;
	double burrow_time;
	General::Point<float> burrow_start_pos;

	bool _is_item;
	bool accelerates_quickly;

	bool draw_shadow;

	int layer;

	bool stops_battle_end;

	bool input_disabled;
};

class Battle_Player : public Battle_Entity {
public:
	void heal(int player_to_heal);

	void take_damage(Battle_Entity *hitter, Hit_Direction direction, bool facing_right, General::Point<float> force, bool hard, bool long_hurt, int particle_damage, float multiplier);

	void handle_event(ALLEGRO_EVENT *event);
	void logic();
	void draw();

	void construct2();
	void stop();

	int get_player_id() { return player_id; }
	Wrap::Bitmap *get_profile_bitmap();

	void execute_ability(std::string ability, bool onoff);
	
	float get_magic_regen_count();
	void set_magic_regen_count(float count);

	bool get_lost_weapon() { return lost_weapon; }
	void set_lost_weapon(bool lost_weapon) { this->lost_weapon = lost_weapon; }

	Battle_Player(Battle_Loop *battle_loop, Player *player, std::string name, int player_id);
	virtual ~Battle_Player();

protected:
	int player_id;

	Wrap::Bitmap *profile_bmp;

	float magic_regen_count;

	bool lost_weapon;
};

class Battle_Enemy : public Battle_Entity {
public:
	void take_damage(Battle_Entity *hitter, Hit_Direction direction, bool facing_right, General::Point<float> force, bool hard, bool long_hurt, int particle_damage, float multiplier);

	void handle_event(ALLEGRO_EVENT *event);
	void logic();

	void stop();
	
	Battle_Enemy(Battle_Loop *battle_loop, std::string name);
	virtual ~Battle_Enemy();
};

#endif // BATTLE_ENTITY_H
