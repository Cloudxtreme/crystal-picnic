#ifndef WANDER_CHARACTER_ROLE_H
#define WANDER_CHARACTER_ROLE_H

#include "general.h"
#include "engine.h"
#include "character_role.h"

class Wander_Character_Role : public Character_Role
{
public:
	void set_home(General::Point<float> home);

	virtual void update(Area_Manager *area);
	void draw(Area_Manager *area);

	void set_pause_times(double min, double max);
	void set_minimum_move_distance(int distance);

	void nudge(General::Point<float> player_velocity);

	Wander_Character_Role(
		Character_Map_Entity *character,
		int max_distance_from_home,
		double pause_min,
		double pause_max);
	virtual ~Wander_Character_Role();

protected:
	General::Direction get_next_move_direction();
	float get_next_move_length();
	double get_next_pause_time();

	General::Point<float> home;
	int max_distance_from_home;
	double pause_time_between_moves_min;
	double pause_time_between_moves_max;
	bool moving;
	General::Point<float> move_start_pos;
	General::Point<float> last_update_pos;
	General::Direction moving_direction;
	General::Direction last_move_direction; // don't go back and forth
	float move_length;
	float amount_moved;
	General::Point<float> last_pos;
	double pause_time_remaining;
	bool turn_around;
	// for stuck checking
	int update_count;
	General::Point<float> stuck_check_start_pos;
	int minimum_move_distance;
	double next_nudge_time;
};

#endif // WANDER_CHARACTER_ROLE_H
