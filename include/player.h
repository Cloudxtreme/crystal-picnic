#ifndef PLAYER_H
#define PLAYER_H

#include "map_entity.h"
#include "sound.h"
#include "speech_loop.h"
#include "weaponized_entity.h"
#include "battle_entity.h"
#include "character_map_entity.h"

class Player : public Character_Map_Entity, public Weaponized_Entity {
public:
	static const float PAN_SCALE;

	// Entity interface
	void set_position(General::Point<float> pos);
	void handle_event(ALLEGRO_EVENT *event);
	void logic();
	void draw();
	
	bool load_weapon(std::string weapon_name);
	bool load();
	void reset();

	void set_panning(bool panning);
	void start_area();
	void activate();

	bool is_panning();
	
	//void stop_walk_sample();
	
	std::vector<Bones::Bone> &get_current_weapon_bones();

	void start_attack();	
	bool is_attacking();
	void set_attacking(bool attacking);
	
	Battle_Attributes &get_battle_attributes();

	std::vector<std::string> get_abilities();
	std::vector<std::string> get_selected_abilities(bool battle, bool runner, bool final_battle);
	void set_selected_ability(bool battle, int num, std::string value);

	void execute_ability(std::string ability, bool onoff);
	
	void set_role_paused(bool role_paused);

	float get_magic_regen_count();
	void set_magic_regen_count(float count);

	Player(std::string name);
	virtual ~Player();

private:
	bool panning;
	General::Point<float> player_pan_pos;
	bool back_home;
	General::Point<float> pan_movement_sum;
	General::Point<float> pan_amount_moved_back;
	double no_direction_change_end_time;
	
	//Sound::Sample *walk_sample;
	//bool walk_sample_playing;
	//bool run_sample_playing;
	
	std::map< std::pair<std::string, int>, std::vector<Bones::Bone> > weapon_bones;

	bool called_attacked;
	
	bool attacking;
	
	Battle_Attributes battle_attributes;

	std::vector<std::string> battle_abilities;

	float magic_regen_count;

	bool polled_joystick;
};

#endif // PLAYER_H
