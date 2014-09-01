#include "crystalpicnic.h"
#include "character_map_entity.h"
#include "character_role.h"
#include "enemy_avatar_wander_character_role.h"
#include "area_manager.h"
#include "collision_detection.h"
#include "area_loop.h"

void Enemy_Avatar_Wander_Character_Role::update(Area_Manager *area)
{
	if (al_get_time() >= next_check) {
		next_check = al_get_time() + (General::rand()%1000)/1000.0*5.0 + 5.0;
		const float radius = 200.0f;
		Map_Entity *player = area->get_entity(0);
		int layer = player->get_layer();
		General::Point<float> player_pos = player->get_position();
		Area_Loop *loop = GET_AREA_LOOP;
		if (!player->input_is_disabled() && layer == entity->get_layer() && !area->point_is_in_no_enemy_zone(player_pos.x, player_pos.y) && !area->get_in_speech_loop() && (!loop || loop->battle_event_is_done()) && loop->get_num_jumping() == 0) {
			General::Point<float> this_pos = entity->get_position();
			if (General::distance(player_pos.x, player_pos.y, this_pos.x, this_pos.y) <= radius) {
				float dx = player_pos.x - this_pos.x;
				float dy = player_pos.y - this_pos.y;
				float angle1 = atan2(dy, dx) + M_PI / 2.0f;
				float angle2 = angle1 + M_PI;
				General::Point<float> pp1(
					player_pos.x + cos(angle1) * General::TILE_SIZE/2,
					player_pos.y + sin(angle1) * General::TILE_SIZE/2
				);
				General::Point<float> pp2(
					player_pos.x + cos(angle2) * General::TILE_SIZE/2,
					player_pos.y + sin(angle2) * General::TILE_SIZE/2
				);
				General::Point<float> tp1(
					this_pos.x + cos(angle1) * General::TILE_SIZE/2,
					this_pos.y + sin(angle1) * General::TILE_SIZE/2
				);
				General::Point<float> tp2(
					this_pos.x + cos(angle2) * General::TILE_SIZE/2,
					this_pos.y + sin(angle2) * General::TILE_SIZE/2
				);
				std::vector< General::Line<float> > *lines = area->get_collision_lines();
				bool collision = false;
				for (size_t i = 0; i < lines[layer].size(); i++) {
					General::Point<float> p1(lines[layer][i].x1, lines[layer][i].y1);
					General::Point<float> p2(lines[layer][i].x2, lines[layer][i].y2);
					if (checkcoll_line_line(&pp1, &tp1, &p1, &p2, NULL)) {
						collision = true;
						break;
					}
					if (checkcoll_line_line(&pp2, &tp2, &p1, &p2, NULL)) {
						collision = true;
						break;
					}
				}
				if (!collision) {
					Battle_Event_Type type = (Battle_Event_Type)(General::rand() % 3);

					float *inputs = player->get_inputs();

					player->set_panning_to_entity(entity->get_id());
					player->set_input_disabled(true);

					if (type != BATTLE_EVENT_SIGHTED && (inputs[Map_Entity::X] != 0.0f || inputs[Map_Entity::Y] != 0.0f)) {
						General::Direction d = player->get_direction();
						float a;
						if (d == General::DIR_N) {
							player->get_animation_set()->set_sub_animation("trip-up");
							a = M_PI / 2;
						}
						else if (d == General::DIR_S) {
							player->get_animation_set()->set_sub_animation("trip-down");
							a = M_PI * 3 / 2;
						}
						else if (d == General::DIR_E) {
							player->get_animation_set()->set_sub_animation("trip");
							a = M_PI;
						}
						else {
							player->get_animation_set()->set_sub_animation("trip");
							a = 0;
						}
						a += ((General::rand()%1000)/1000.0f)*M_PI/3 - M_PI/6;
						player->get_animation_set()->reset();
						if (type == BATTLE_EVENT_TRIPPED) {
							engine->play_sample("sfx/trip.ogg");
						}
						else if (type == BATTLE_EVENT_SLIPPED) {
							engine->play_sample("sfx/slip.ogg");
							lua_State *stack = area->get_lua_state();
							Lua::call_lua(stack, "toss_banana", "iddd>", layer, player_pos.x, player_pos.y, a);
						}
					}
					else {
						engine->play_sample("sfx/enemy_alerted.ogg");
						player->update_direction(false);
					}
					GET_AREA_LOOP->set_battle_was_event(type);
					entity->kamikaze(this_pos.x + dx * 0.9f, this_pos.y + dy * 0.9f);
					return;
				}
			}
		}
	}

	Wander_Character_Role::update(area);
}
	
Enemy_Avatar_Wander_Character_Role::Enemy_Avatar_Wander_Character_Role(
	Character_Map_Entity *character,
	int max_distance_from_home,
	double pause_min,
	double pause_max) :

	Wander_Character_Role(character, max_distance_from_home, pause_min, pause_max)
{
	next_check = al_get_time() + (General::rand()%1000)/1000.0*5.0 + 5.0;
}

Enemy_Avatar_Wander_Character_Role::~Enemy_Avatar_Wander_Character_Role(void)
{
}

