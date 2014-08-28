#ifndef AREA_LOOP_H
#define AREA_LOOP_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>

#include "loop.h"
#include "animation_set.h"
#include "area_manager.h"
#include "general.h"
#include "graphics.h"
#include "npc.h"
#include "enemy_avatar_wander_character_role.h"

class Player;
class Area_Manager;

class Area_Loop : public Loop {
public:
	static const float SWIPE_TIME_ONE_DIRECTION;
	static const int NUM_ACTION_BUTTONS = 4;

	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();
	void rotated();
	void return_to_loop();
	void destroy_graphics();
	void reload_graphics();

	static const float PAN_FLASH_DELAY; 
	static const float PAN_FLASH_MODULUS;

	void load_area(std::string name);
	void load_area_deferred(std::string name, General::Direction direction, int x, int y);

	void change_areas();

	std::vector<Player *> get_players();
	std::vector<NPC *> get_player_npcs();

	Area_Manager *get_area();
	Area_Manager *get_new_area();

	bool new_area_ready();

	void add_collision_line(int x1, int y1, int x2, int y2);

	void set_input_paused(bool p);

	void set_swiping_in(bool swiping);
	
	Wrap::Bitmap **get_last_battle_screenshot();
	std::string *get_last_used_player_in_battle();

	void set_start(std::string name, int layer, General::Point<float> pos);

	void add_bisou();

	bool battle_event_is_done();
	void set_battle_was_event(Enemy_Avatar_Wander_Character_Role::Battle_Event_Type event_type);
	void set_was_boss_battle(bool was_boss_battle);

	void set_whack_a_skunk_played(bool played);
	void set_whack_a_skunk_score(int score);
	bool whack_a_skunk_was_played();
	int get_whack_a_skunk_score();
	
	void switch_characters(bool sound);

	int get_num_jumping();
	void set_num_jumping(int num_jumping);

	int get_battle_event_count();
	void set_battle_event_count(int battle_event_count);

	Area_Loop();
	virtual ~Area_Loop();

private:
	void destroy_area();

	static const int ACTION_BUTTON_RADIUS = 20;

	Area_Manager *area;
	Area_Manager *new_area;

	// lua function change_areas sets these with load_area_deferred above
	std::string deferred_area_name;
	General::Direction deferred_area_direction;
	int deferred_area_x;
	int deferred_area_y;

	float swipe_in_time;
	bool swiping_in;
	float swipe_out_time;
	bool swiping_out;
	General::Point<float> action_pivot;
	
	bool input_paused;
	
	std::vector<Player *> players;
	std::vector<NPC *> player_npcs;
	
	Wrap::Bitmap *last_battle_screenshot;
	std::string last_used_player_in_battle;
	std::string tmp_keepme;

	std::string start_area_name;
	int start_layer;
	General::Point<float> start_pos;

	std::string game_over_start_area_name;
	int game_over_start_layer;
	General::Point<float> game_over_start_pos;

	Enemy_Avatar_Wander_Character_Role::Battle_Event_Type battle_event_type;
	bool was_boss_battle;

	bool played_whack_a_skunk;
	int whack_a_skunk_score;

	int num_jumping;

	int battle_event_count;
};

#endif // AREA_LOOP_H
