#include "crystalpicnic.h"
#include "character_map_entity.h"
#include "character_role.h"
#include "wander_character_role.h"
#include "area_manager.h"

void Wander_Character_Role::set_home(General::Point<float> home)
{
	this->home = home;
}

void Wander_Character_Role::draw(Area_Manager *area)
{
}

void Wander_Character_Role::update(Area_Manager *area)
{
	float *input = entity->get_inputs();
	General::Point<float> pos = entity->get_position();

	General::Point<float> top = area->get_top();

	if (!(
		pos.x+(General::TILE_SIZE*4) >= top.x &&
		pos.y+(General::TILE_SIZE*2) >= top.y &&
		pos.x-(General::TILE_SIZE*4) < top.x+cfg.screen_w &&
		pos.y-(General::TILE_SIZE*6) < top.y+cfg.screen_h))
	{
		input[Map_Entity::X] = input[Map_Entity::Y] = 0.0f;
		input[Map_Entity::BUTTON1] = 0.0f;
		return;
	}

	if (last_update_pos.x == -1 && last_update_pos.y == -1) {
		last_update_pos = pos;
		stuck_check_start_pos = pos;
	}

	update_count++;
	if (moving && update_count > 15) {
		update_count = 0;
		float dist = General::distance(pos.x, pos.y,
			stuck_check_start_pos.x, stuck_check_start_pos.y);
		stuck_check_start_pos = pos;
		if (dist < 5) {
			moving = false;
			pause_time_remaining = get_next_pause_time();
			input[Map_Entity::X] = input[Map_Entity::Y] = 0.0f;
			input[Map_Entity::BUTTON1] = 0.0f;
		}
	}

	if (moving) {
		amount_moved += MAX(1, General::distance(
			last_update_pos.x, last_update_pos.y,
			pos.x, pos.y
		));
		// check for reached length of walk segment
		bool should_pause = false;
		if (fabs(amount_moved-move_length) < 5) {
			should_pause = true;
			amount_moved = 0.0f;
		}
		// check wandered too far
		if (!should_pause) {
			float travelled = General::distance(
				home.x, home.y,
				pos.x, pos.y
			);
			if (travelled >= max_distance_from_home) {
				should_pause = true;
				turn_around = true;
			}
		}
		if (should_pause) {
			moving = false;
			pause_time_remaining = get_next_pause_time();
			input[Map_Entity::X] = input[Map_Entity::Y] = 0.0f;
			input[Map_Entity::BUTTON1] = 0.0f;
		}
	}
	else {
		pause_time_remaining -= General::LOGIC_MILLIS/1000.0;
		if (pause_time_remaining <= 0.0) {
			moving = true;
			update_count = 0;
			move_start_pos = pos;
			stuck_check_start_pos = pos;
			moving_direction = get_next_move_direction();
			move_length = get_next_move_length();
			float move_x = 0.0f, move_y = 0.0f;
			switch (moving_direction) {
			case General::DIR_N:
				move_y = -1.0f;
				break;
			case General::DIR_NE:
				move_x = 1.0f;
				move_y = -1.0f;
				break;
			case General::DIR_E:
				move_x = 1.0f;
				break;
			case General::DIR_SE:
				move_x = 1.0f;
				move_y = 1.0f;
				break;
			case General::DIR_S:
				move_y = 1.0f;
				break;
			case General::DIR_SW:
				move_x = -1.0f;
				move_y = 1.0f;
				break;
			case General::DIR_W:
				move_x = -1.0f;
				break;
			case General::DIR_NW:
				move_x = -1.0f;
				move_y = -1.0f;
				break;
			default:
				break;
			}
			bool has_walk = entity->get_animation_set()->check_sub_animation_exists("walk");
			bool has_run = entity->get_animation_set()->check_sub_animation_exists("run");
			float mul;
			float BUTTON1;
			if (has_walk && has_run) {
				if (General::rand() % 2 == 0) {
					mul = 0.75f;
					BUTTON1 = 1.0f;
				}
				else {
					mul = 0.5f;
					BUTTON1 = 0.0f;
				}
			}
			else if (has_run) {
				mul = 0.75f;
				BUTTON1 = 1.0f;
			}
			else {
				mul = 0.5f;
				BUTTON1 = 0.0f;
			}
			input[Map_Entity::X] = move_x * mul;
			input[Map_Entity::Y] = move_y * mul;
			input[Map_Entity::BUTTON1] = BUTTON1;
		}
	}

	if (area->point_is_in_no_enemy_zone(pos.x+input[Map_Entity::X], pos.y+input[Map_Entity::Y])) {
		moving = false;
		pause_time_remaining = get_next_pause_time();
		input[Map_Entity::X] = 0.0f;
		input[Map_Entity::Y] = 0.0f;
		input[Map_Entity::BUTTON1] = 0.0f;
	}

	last_update_pos = pos;
}
	
General::Direction Wander_Character_Role::get_next_move_direction(void)
{
	if (turn_around && General::rand() % 2) {
		turn_around = false;
		General::Point<float> pos = entity->get_position();
		float xdiff = fabs(pos.x-home.x);
		float ydiff = fabs(pos.y-home.y);
		if (xdiff > ydiff) {
			if (pos.x < home.x) {
				return General::DIR_E;
			}
			else {
				return General::DIR_W;
			}
		}
		else {
			if (pos.y < home.y) {
				return General::DIR_S;
			}
			else {
				return General::DIR_N;
			}
		}
	}

	int dir = (General::rand() % 4) * 2;
	int curr = (int)moving_direction;
	if (dir == curr) {
		dir += 2;
		dir %= 8;
	}
	return (General::Direction)dir;
}

float Wander_Character_Role::get_next_move_length(void)
{
	return ((float)(General::rand()%1000)/1000) * (((int)max_distance_from_home)/2) + minimum_move_distance;
}

double Wander_Character_Role::get_next_pause_time(void)
{
	double t = pause_time_between_moves_min;
	double add = ((double)(General::rand()%1000)/1000) *
		(pause_time_between_moves_max - pause_time_between_moves_min);
	t += add;
	return t;
}

Wander_Character_Role::Wander_Character_Role(
	Character_Map_Entity *character,
	int max_distance_from_home,
	double pause_min,
	double pause_max) :

	Character_Role(character),
	next_nudge_time(0.0)
{
	this->max_distance_from_home = max_distance_from_home;
	pause_time_between_moves_min = pause_min;
	pause_time_between_moves_max = pause_max;
	moving = false;
	moving_direction = (General::Direction)(General::rand() % 8);
	last_move_direction = (General::Direction)(General::rand() % 8);
	pause_time_remaining = 0.0;
	turn_around = false;
	amount_moved = 0.0f;
	last_update_pos = General::Point<float>(-1, -1);
	update_count = 0;
	minimum_move_distance = General::TILE_SIZE;
}

Wander_Character_Role::~Wander_Character_Role(void)
{
}

void Wander_Character_Role::set_pause_times(double min, double max)
{
	pause_time_between_moves_min = min;
	pause_time_between_moves_max = max;
}

void Wander_Character_Role::set_minimum_move_distance(int distance)
{
	minimum_move_distance = distance;
}

void Wander_Character_Role::nudge(General::Point<float> player_velocity)
{
	if (al_get_time() < next_nudge_time) {
		return;
	}

	next_nudge_time = al_get_time() + 0.5;

	float *input = entity->get_inputs();
	General::Point<float> pos = entity->get_position();
	pause_time_remaining = get_next_pause_time();

	moving = true;
	update_count = 0;
	move_start_pos = pos;
	stuck_check_start_pos = pos;

	float a = atan2(player_velocity.y, player_velocity.x);
	a = a + ((General::rand() % 2) == 0 ? -1 : 1) * M_PI/2.0f;

	while (a >= M_PI*2) a -= M_PI*2;
	while (a < 0) a += M_PI*2;

	float fortyfive = M_PI / 4.0f;

	if (a >= (M_PI*2-fortyfive) || a <= fortyfive) {
		moving_direction = General::DIR_E;
	}
	else if (a >= fortyfive && a <= M_PI/2+fortyfive) {
		moving_direction = General::DIR_S;
	}
	else if (a >= M_PI/2+fortyfive && a <= M_PI+fortyfive) {
		moving_direction = General::DIR_W;
	}
	else {
		moving_direction = General::DIR_N;
	}

	move_length = General::TILE_SIZE*3;
	float move_x = 0.0f, move_y = 0.0f;
	switch (moving_direction) {
	case General::DIR_N:
		move_y = -1.0f;
		break;
	case General::DIR_NE:
		move_x = 1.0f;
		move_y = -1.0f;
		break;
	case General::DIR_E:
		move_x = 1.0f;
		break;
	case General::DIR_SE:
		move_x = 1.0f;
		move_y = 1.0f;
		break;
	case General::DIR_S:
		move_y = 1.0f;
		break;
	case General::DIR_SW:
		move_x = -1.0f;
		move_y = 1.0f;
		break;
	case General::DIR_W:
		move_x = -1.0f;
		break;
	case General::DIR_NW:
		move_x = -1.0f;
		move_y = -1.0f;
		break;
	default:
		break;
	}
	bool has_walk = entity->get_animation_set()->check_sub_animation_exists("walk");
	bool has_run = entity->get_animation_set()->check_sub_animation_exists("run");
	float mul;
	float BUTTON1;
	if (has_walk && has_run) {
		if (General::rand() % 2 == 0) {
			mul = 0.75f;
			BUTTON1 = 1.0f;
		}
		else {
			mul = 0.5f;
			BUTTON1 = 0.0f;
		}
	}
	else if (has_run) {
		mul = 0.75f;
		BUTTON1 = 1.0f;
	}
	else {
		mul = 0.5f;
		BUTTON1 = 0.0f;
	}
	input[Map_Entity::X] = move_x * mul;
	input[Map_Entity::Y] = move_y * mul;
	input[Map_Entity::BUTTON1] = BUTTON1;
}
