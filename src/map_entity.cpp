#include "crystalpicnic.h"
#include "map_entity.h"
#include "area_manager.h"
#include "npc.h"
#include "player.h"
#include "battle_loop.h"
#include "enemy_avatar.h"
#include "character_map_entity.h"
#include "character_role.h"
#include "wander_character_role.h"
#include "collision_detection.h"

void Map_Entity::set_move_cameras_while_input_disabled(bool move_cameras)
{
	move_cameras_while_input_disabled = move_cameras;
}

float Map_Entity::get_run_factor(void)
{
	float run_factor;

	if (input[BUTTON1] == 1)
		run_factor = 1.0;
	else
		run_factor = 0.5;

	return run_factor;
}

bool Map_Entity::is_visible(void)
{
	return visible;
}

void Map_Entity::set_visible(bool visible)
{
	this->visible = visible;
}

ALLEGRO_THREAD *Map_Entity::get_astar_thread(void)
{
	return astar_thread;
}

A_Star::A_Star_Thread_Info *Map_Entity::get_ati(void)
{
	return ati;
}

void Map_Entity::set_ati(A_Star::A_Star_Thread_Info *ati)
{
	this->ati = ati;
}

bool Map_Entity::shadow_is_shown(void)
{
	return show_shadow;
}

void Map_Entity::set_show_shadow(bool show)
{
	show_shadow = show;
}

bool Map_Entity::input_is_disabled(void)
{
	return input_disabled;
}

void Map_Entity::set_input_disabled(bool disabled)
{
	input_disabled = disabled;
}

float *Map_Entity::get_inputs(void)
{
	return input;
}

bool Map_Entity::is_solid_with_area(void)
{
	return solid_with_area;
}

void Map_Entity::set_solid_with_area(bool s)
{
	solid_with_area = s;
}

bool Map_Entity::is_solid_with_entities(void)
{
	return solid_with_entities;
}

void Map_Entity::set_solid_with_entities(bool s)
{
	solid_with_entities = s;
}

void Map_Entity::set_position(General::Point<float> pos)
{
	General::Point<float> old_pos = this->pos;
	this->pos = pos;

	if (!input_disabled || move_cameras_while_input_disabled) {
		for (int i = 0; i < (int)cameras.size(); i++) {
			Camera *w = cameras[i];
			General::Point<float> tmp(pos.x-old_pos.x, pos.y-old_pos.y);
			w->move_camera(tmp, pos, false);
		}
	}
}

void Map_Entity::center_cameras(void)
{
	General::Point<float> ideal;

	ideal.x = pos.x - cfg.screen_w/2;
	ideal.y = pos.y - cfg.screen_h/2;

	for (int i = 0; i < (int)cameras.size(); i++) {
		Camera *w = cameras[i];
		General::Point<float> current = w->get_camera_pos();
		General::Point<float> delta;
		delta.x = ideal.x - current.x;
		delta.y = ideal.y - current.y;
		General::Size<float> extents = get_current_bones()[0].get_extents();
		General::Size<int> sz(extents.w, extents.h);
		w->move_camera(delta, pos, false);
	}
}

float Map_Entity::get_speed(void)
{
	return speed;
}

void Map_Entity::set_speed(float speed)
{
	this->speed = speed;
}

int Map_Entity::get_layer(void)
{
	return layer;
}

void Map_Entity::set_layer(int layer)
{
	this->layer = layer;
}

int Map_Entity::get_sort_offset(void)
{
	return sort_offset;
}

void Map_Entity::set_sort_offset(int sort_offset)
{
	this->sort_offset = sort_offset;
}

General::Direction Map_Entity::get_direction(void)
{
	return direction;
}

void Map_Entity::set_facing_right(General::Direction direction)
{
	if (direction == General::DIR_E || direction == General::DIR_NE || direction == General::DIR_SE) {
		facing_right = true;
	}
	else if (direction == General::DIR_W || direction == General::DIR_NW || direction == General::DIR_SW) {
		facing_right = false;
	}
}

void Map_Entity::set_direction(General::Direction direction)
{
	this->direction = direction;
	set_facing_right(direction);

	std::string anim;

	switch (direction) {
		case General::DIR_N:
			anim = "idle-up";
			break;
		case General::DIR_S:
			anim = "idle-down";
			break;
		default:
			anim = "idle";
			break;
	}

	if (facing_right) {
		if (anim_set->check_sub_animation_exists(anim + "-right")) {
			anim += "-right";
		}
	}
	else {
		if (anim_set->check_sub_animation_exists(anim + "-left")) {
			anim += "-left";
		}
	}

	anim_set->set_sub_animation(anim);

	reset();
}

void Map_Entity::update_direction(bool can_move)
{
	std::string anim;

	std::string curr;
	if (skeleton) {
		curr = skeleton->get_curr_anim_name();
	}
	else {
		curr = anim_set->get_sub_animation_name();
	}

	if ((fabs(input[X]) < 0.2 && fabs(input[Y]) < 0.2) || (!can_move)) {
		if (strncmp(curr.c_str(), "idle", 4)) {
			if (curr == "run-down" || curr == "walk-down") {
				anim = "idle-down";
			}
			else if (curr == "run-up" || curr == "walk-up") {
				anim = "idle-up";
			}
			else {
				anim = "idle";
			}
		}
		else {
			anim = curr;
		}
	}
	else {
		float magnitude = sqrt(input[X]*input[X] + input[Y]*input[Y]);
		if (magnitude >= 0.51f) {
			if (fabs(input[Y]) > fabs(input[X])) {
				if (input[Y] < 0) {
					anim = "run-up";
				}
				else {
					anim = "run-down";
				}
			}
			else {
				anim = "run";
			}
		}
		else {
			if (fabs(input[Y]) > fabs(input[X])) {
				if (input[Y] < 0) {
					anim = "walk-up";
				}
				else {
					anim = "walk-down";
				}
			}
			else {
				anim = "walk";
			}
		}
	}

	if (skeleton) {
		skeleton->set_curr_anim(anim);
	}
	else {
		if (facing_right) {
			if (anim_set->check_sub_animation_exists(anim + "-right")) {
				anim += "-right";
			}
		}
		else {
			if (anim_set->check_sub_animation_exists(anim + "-left")) {
				anim += "-left";
			}
		}

		anim_set->set_sub_animation(anim);
	}
}

void Map_Entity::handle_event(ALLEGRO_EVENT *event)
{
}

void Map_Entity::reset(void)
{
	for (int i = 0; i < NUM_INPUTS; i++)
		input[i] = 0;
	update_direction(false);
	current_collisions.clear();
}

// return true if this entity has lua logic to run
void Map_Entity::run_lua_logic(void)
{
	if (lua_state == NULL) {
		return;
	}

	Area_Loop *area_loop = General::find_in_vector<Area_Loop *, Loop *>(engine->get_loops());
	Area_Manager *area = area_loop->get_area();
	General::Point<float> top = area->get_top();
	top.x += area->_offset.x;
	top.y += area->_offset.y;
	if (pos.x < top.x-General::TILE_SIZE*5 || pos.y < top.y-General::TILE_SIZE*5 || pos.x > top.x+cfg.screen_w+General::TILE_SIZE*5 || pos.y > top.y+cfg.screen_h+General::TILE_SIZE*5) {
		return;
	}

	Lua::call_lua(lua_state, "logic", ">s");
	const char *c_cmd = lua_tostring(lua_state, -1);
	if (c_cmd == NULL) {
		return;
	}
	lua_pop(lua_state, 1);

	std::string cmd = c_cmd;

	if (cmd == "") {
		return;
	}

	std::vector<std::string> c = General::split(cmd);

	int i = 0;
	while (i < (int)c.size()) {
		if (c[i] == "set_position") {
			i++;
			pos.x = atof(c[i].c_str());
			i++;
			pos.y = atof(c[i].c_str());
			i++;
		}
		else if (c[i] == "set_z") {
			i++;
			z = atof(c[i].c_str());
			i++;
		}
		else if (c[i] == "remove") {
			i++;
			set_delete_me(true);
		}
	}
}

static void slide(Area_Manager *area, General::Point<float> pos, float a, int layer, std::vector<Bones::Bone> &bone, bool facing_right, float *pos_inc_x, float *pos_inc_y, bool *should_return)
{

	float dx = cos(a);
	float dy = sin(a);

	General::Point<int> new_pos_both(
		pos.x + dx,
		pos.y + dy
	);
	General::Point<int> new_pos_x(
		pos.x + dx,
		pos.y
	);
	General::Point<int> new_pos_y(
		pos.x,
		pos.y + dy
	);

	if (dx != 0 || dy != 0) {
		if (area->area_is_colliding(layer, new_pos_both, bone, facing_right, NULL, NULL)) {
			if (dx == 0 || area->area_is_colliding(layer, new_pos_x, bone, facing_right, NULL, NULL)) {
				if (dy == 0 || area->area_is_colliding(layer, new_pos_y, bone, facing_right, NULL, NULL)) {
					*should_return = true;
				}
				else {
					*pos_inc_y = dy;
				}
			}
			else {
				*pos_inc_x = dx;
			}
		}
		else {
			*pos_inc_x = dx;
			*pos_inc_y = dy;
		}
	}
	else {
		*pos_inc_x = dx;
		*pos_inc_y = dy;
	}
}

void Map_Entity::logic(void)
{
	run_lua_logic();

	if (skeleton) {
		skeleton->update(General::LOGIC_MILLIS);
		General::Point<float> top = area->get_top();
		skeleton->transform(General::Point<float>(pos.x-top.x, pos.y-top.y), facing_right);
	}
	else if (anim_set) {
		if (!colliding_with_ladder) {
			anim_set->update();
		}
	}

	colliding_with_ladder = false;

	float curr_speed = get_current_speed();

	float dx = 0;
	float dy = 0;
	if (!dynamic_cast<Player *>(this) || !((Player *)this)->is_attacking()) {
		if (!input_disabled) {
			if (!cfg.use_joy && area->is_isometric()) {
				std::vector<Loop *> loops = engine->get_loops();
				Area_Loop *al = General::find_in_vector<Area_Loop *, Loop *>(loops);
				if (al) {
					Area_Manager *area = al->get_area();
					if (area) {
						switch (direction) {
							case General::DIR_NE:
							case General::DIR_SE:
							case General::DIR_SW:
							case General::DIR_NW:
								dx = dy = 0;
								// These values are based on the
								// ~26 degree slopes of the level
								dx = input[X] * 0.89442f;
								dy = input[Y] * 0.44721f;
								break;
							default:
								dx = input[X];
								dy = input[Y];
								break;
						}
						dx *= curr_speed;
						dy *= curr_speed;
					}
				}
			}
			else {
				dx = input[X] * curr_speed;
				dy = input[Y] * curr_speed;
			}
		}
	}

	if (pan_to_entity >= 0) {
		Area_Loop *area_loop = General::find_in_vector<Area_Loop *, Loop *>(engine->get_loops());
		if (area_loop) {
			Area_Manager *area = area_loop->get_area();
			if (area) {
				Map_Entity *e = area->get_entity(pan_to_entity);
				if (e) {
					Camera *camera = cameras[0]; // area window
					if (camera) {
						if (panning_back_to_player) {
							General::Point<float> epos = e->get_position();
							General::Point<float> cpos = camera->get_camera_pos();
							float dx = epos.x - (cpos.x+cfg.screen_w/2);
							float dy = epos.y - (cpos.y+cfg.screen_h/2);
							camera->move_camera(General::Point<float>(dx, dy), pos, true);
						}
						else {
							General::Point<float> epos = e->get_position();
							General::Point<float> cpos = camera->get_camera_pos();
							float dx = epos.x - (cpos.x+cfg.screen_w/2);
							float dy = epos.y - (cpos.y+cfg.screen_h/2);
							float a = atan2(dy, dx);
							float dist = sqrt(dx*dx + dy*dy);
							bool done = dist <= 2.0f + e->get_current_speed() + 0.1f;
							float pan_speed;
							if (done) {
								pan_speed = dist;
							}
							else {
								pan_speed = 2.0f;
							}
							General::Point<float> pan(cos(a) * pan_speed, sin(a) * pan_speed);
							camera->move_camera(pan, pos, true);
							if (done) {
								panning_back_to_player = true;
							}
						}
					}
				}
			}
		}
	}

	General::Point<int> new_pos_both(
		pos.x + dx,
		pos.y + dy
	);
	General::Point<int> new_pos_x(
		pos.x + dx,
		pos.y
	);
	General::Point<int> new_pos_y(
		pos.x,
		pos.y + dy
	);

	float pos_inc_x = 0;
	float pos_inc_y = 0;

	if (skeleton) {
		pos_inc_x = dx;
		pos_inc_y = dy;
	}
	else {
		bool should_return = false;

		std::vector<Bones::Bone> &bone = get_current_bones();

		// Slide
		if (solid_with_area && (dx != 0 || dy != 0)) {
			if (area->area_is_colliding(layer, new_pos_both, bone, facing_right, NULL, NULL)) {
				if (dx == 0 || area->area_is_colliding(layer, new_pos_x, bone, facing_right, NULL, NULL)) {
					if (dy == 0 || area->area_is_colliding(layer, new_pos_y, bone, facing_right, NULL, NULL)) {
						// Try auto-slide
						// Try to find near perpendicular line
						float five_deg = 5.0f * (M_PI * 2.0f / 360.0f);
						std::vector< General::Point<float> > pts = facing_right ? bone[0].get_outline() : bone[0].get_outline_mirrored();
						bool found_favorable = false;
						bool found_unfavorable = false;
						bool unusable = false;
						General::Point<float> p1(0, 0), p2(0, 0), p3(0, 0), p4(0, 0);
						General::Point<float> unp1(0, 0), unp2(0, 0), unp3(0, 0), unp4(0, 0);
						float closest = FLT_MAX;
						float un_closest = FLT_MAX;
						for (size_t j = 0; j < pts.size(); j++) {
							int k = (j+1) % pts.size();
							General::Point<float> pt_pos1(new_pos_both.x + pts[j].x, new_pos_both.y + pts[j].y);
							General::Point<float> pt_pos2(new_pos_both.x + pts[k].x, new_pos_both.y + pts[k].y);
							std::vector< General::Line<float> > v1 = area->get_quadrant(layer, pt_pos1);
							std::vector< General::Line<float> > v2 = area->get_quadrant(layer, pt_pos2);
							std::vector< General::Line<float> > v;
							v.insert(v.end(), v1.begin(), v1.end());
							v.insert(v.end(), v2.begin(), v2.end());
							for (size_t i = 0; i < v.size(); i++) {
								General::Point<float> tmp1(v[i].x1, v[i].y1);
								General::Point<float> tmp2(v[i].x2, v[i].y2);
								General::Point<float> full_pos(new_pos_both.x, new_pos_both.y + General::BOTTOM_SPRITE_PADDING);
								General::Point<float> pp1(full_pos.x+pts[j].x, full_pos.y+pts[j].y);
								General::Point<float> pp2(full_pos.x+pts[k].x, full_pos.y+pts[k].y);
								if (checkcoll_line_line(&tmp1, &tmp2, &pp1, &pp2, NULL)) {
									for (size_t j = 0; j < pts.size(); j++) {
										int k = (j+1) % pts.size();
										General::Point<float> tmp3(pts[j].x+full_pos.x, pts[j].y+full_pos.y);
										General::Point<float> tmp4(pts[k].x+full_pos.x, pts[k].y+full_pos.y);
										float a1 = atan2(tmp4.y - tmp3.y, tmp4.x - tmp3.x);
										float a_line = atan2(tmp2.y - tmp1.y, tmp2.x - tmp1.x);
										float a_walk = atan2(dy, dx);
										// Line and bone not parallel, don't favor
										float ad1 = General::angle_difference_clockwise(a1, a_line);
										float ad2 = General::angle_difference_counter_clockwise(a1, a_line);
										bool favorable = true;
										if (fabs(ad1) > five_deg && fabs(ad2) > five_deg && fabs(M_PI-ad1) > five_deg && fabs(M_PI-ad2) > five_deg) {
											favorable = false;
										}
										// If line parallel to walking direction, don't favor
										ad1 = General::angle_difference_clockwise(a_walk, a_line);
										ad2 = General::angle_difference_counter_clockwise(a_walk, a_line);
										if (fabs(ad1) < five_deg || fabs(ad2) < five_deg || fabs(M_PI-ad1) < five_deg || fabs(M_PI-ad2) < five_deg) {
											favorable = false;
										}
										// If both line ends points far away don't favor (see below for unusable)
										float dist1 = General::distance(pos.x, pos.y, tmp1.x, tmp1.y);
										float dist2 = General::distance(pos.x, pos.y, tmp2.x, tmp2.y);
										bool this_unusable = false;
										if (dist1 > General::TILE_SIZE && dist2 > General::TILE_SIZE) {
											favorable = false;
											this_unusable = true;
										}
										float d1 =
											General::distance(tmp3.x, tmp3.y, tmp1.x, tmp1.y) +
											General::distance(tmp3.x, tmp3.y, tmp2.x, tmp2.y) +
											General::distance(tmp4.x, tmp4.y, tmp1.x, tmp1.y) +
											General::distance(tmp4.x, tmp4.y, tmp2.x, tmp2.y);
										if (favorable && d1 < closest) {
											found_favorable = true;
											closest = d1;
											p1 = tmp1;
											p2 = tmp2;
											p3 = tmp3;
											p4 = tmp4;
										}
										else if (!favorable && d1 < un_closest) {
											found_unfavorable = true;
											un_closest = d1;
											unp1 = tmp1;
											unp2 = tmp2;
											unp3 = tmp3;
											unp4 = tmp4;
											unusable = this_unusable;
										}
									}
								}
							}
						}
						if (found_favorable) {
							// Slide to opening eg ladder
							float d1 = General::distance(p1.x, p1.y, p3.x, p3.y) + General::distance(p1.x, p1.y, p4.x, p4.y);
							float d2 = General::distance(p2.x, p2.y, p3.x, p3.y) + General::distance(p2.x, p2.y, p4.x, p4.y);
							float a;

							if (d1 < d2) {
								a = atan2(p1.y - p2.y, p1.x - p2.x);
							}
							else {
								a = atan2(p2.y - p1.y, p2.x - p1.x);
							}

							slide(area, pos, a, layer, bone, facing_right, &pos_inc_x, &pos_inc_y, &should_return);
						}
						else if (found_unfavorable) {
							// Slide along slants and off of corners (still not 100% on corners)
							General::Point<float> p1(0, 0), p2(0, 0);
							bool found = false;
							// Order points based on direction player is moving
							if (dx < 0 && dy == 0) {
								found = true;
								if (unp1.x < unp2.x) {
									p1 = unp1;
									p2 = unp2;
								}
								else {
									p1 = unp2;
									p2 = unp1;
								}
							}
							else if (dx > 0 && dy == 0) {
								found = true;
								if (unp1.x > unp2.x) {
									p1 = unp1;
									p2 = unp2;
								}
								else {
									p1 = unp2;
									p2 = unp1;
								}
							}
							else if (dx == 0 && dy < 0) {
								found = true;
								if (unp1.y < unp2.y) {
									p1 = unp1;
									p2 = unp2;
								}
								else {
									p1 = unp2;
									p2 = unp1;
								}
							}
							else if (dx == 0 && dy > 0) {
								found = true;
								if (unp1.y > unp2.y) {
									p1 = unp1;
									p2 = unp2;
								}
								else {
									p1 = unp2;
									p2 = unp1;
								}
							}
							if (found) {
								float a = atan2(p1.y - p2.y, p1.x - p2.x);
								if (a < 0) a += M_PI*2;
								// If not away from edge and not a straight wall
								if (!(unusable && (fabs(a) < five_deg || fabs(M_PI/2-a) < five_deg || fabs(M_PI-a) < five_deg || fabs(M_PI*3/2-a) < five_deg))) {
									slide(area, pos, a, layer, bone, facing_right, &pos_inc_x, &pos_inc_y, &should_return);
									std::vector< General::Line<float> > v = area->get_quadrant(layer, unp1);
									bool found = false;
									General::Point<float> tmp1(0, 0), tmp2(0, 0);
									// Find connecting line
									for (size_t i = 0; i < v.size(); i++) {
										if (v[i].x1 == unp1.x && v[i].y1 == unp1.y) {
											// If other point is not unp2
											if (v[i].x2 != unp2.x || v[i].y2 != unp2.y) {
												found = true;
												tmp1.x = v[i].x1;
												tmp1.y = v[i].y1;
												tmp2.x = v[i].x2;
												tmp2.y = v[i].y2;
												break;
											}
										}
										else if (v[i].x2 == unp1.x && v[i].y2 == unp1.y) {
											if (v[i].x2 != unp2.x || v[i].y2 != unp2.y) {
												found = true;
												tmp1.x = v[i].x2;
												tmp1.y = v[i].y2;
												tmp2.x = v[i].x1;
												tmp2.y = v[i].y1;
												break;
											}
										}
									}
									// If it's not a right angle, maybe don't process
									float a1 = atan2(unp1.y - unp2.y, unp1.x - unp2.x);
									float a2 = atan2(tmp1.y - tmp2.y, tmp1.x - tmp2.x);
									if (a1 < 0) a1 += M_PI*2;
									if (a2 < 0) a2 += M_PI*2;
									if (!((fabs(a1) < five_deg || fabs(M_PI-a1) < five_deg || fabs(M_PI/2-a1) < five_deg || fabs(M_PI*3/2-a1) < five_deg) && (General::angle_difference(a1, a2)-M_PI/2) < five_deg)) {
										if (!should_return) {
											found = false;
											// Check if two lines "mirror" (eg if walking up, one goes left one goes right)
											int diff1x = General::sign(p1.x - p2.x);
											int diff1y = General::sign(p1.y - p2.y);
											int diff2x = General::sign(tmp1.x - tmp2.x);
											int diff2y = General::sign(tmp1.y - tmp2.y);
											if (dx == 0) {
												if (diff1x == diff2x || diff1x == 0 || diff2x == 0) {
													pos_inc_x = General::sign(diff1x + diff2x);
													pos_inc_y = 0;
												}
												else {
													//found = true;
												}
											}
											else if (dy == 0) {
												if (diff1y == diff2y || diff1y == 0 || diff2y == 0) {
													pos_inc_x = 0;
													pos_inc_y = General::sign(diff1y + diff2y);
												}
												else {
													//found = true;
												}
											}
											if (pos_inc_x == 0 && pos_inc_y == 0) {
												should_return = true;
											}
										}
									}
									if (found) {
										// Try to slide out of collision
										float a1 = atan2(p1.y - p2.y, p1.x - p2.x);
										float a2 = atan2(tmp1.y - tmp2.y, tmp1.x - tmp2.x);
										float avg = (a1 + a2) / 2;
										float dxavg = General::sign(cos(avg));
										float dyavg = General::sign(sin(avg));
										float mul = get_current_speed();
										General::Point<float> pp1(
											pos.x + dxavg * mul,
											pos.y
										);
										General::Point<float> pp2(
											pos.x,
											pos.y + dyavg * mul
										);
										should_return = false;
										if (dx == 0) {
											if (!area->area_is_colliding(layer, pp1, bone, facing_right, NULL, NULL)) {
												pos_inc_x = dxavg * mul;
												pos_inc_y = 0.0f;
											}
											else {
												should_return = true;
											}
										}
										else if (dy == 0) {
											if (!area->area_is_colliding(layer, pp2, bone, facing_right, NULL, NULL)) {
												pos_inc_x = 0.0f;
												pos_inc_y = dyavg * mul;
											}
											else {
												should_return = true;
											}
										}
										else {
											should_return = true;
										}
									}
								}
							}
						}
						else {
							should_return = true;
						}
					}
					else {
						pos_inc_y = dy;
					}
				}
				else {
					pos_inc_x = dx;
				}
			}
			else {
				pos_inc_x = dx;
				pos_inc_y = dy;
			}
		}
		else {
			pos_inc_x = dx;
			pos_inc_y = dy;
		}

		std::list<Map_Entity *> collided;

		bool skip_entity_collision_test = false;

		Character_Map_Entity *cme = dynamic_cast<Character_Map_Entity *>(this);
		if (cme) {
			Character_Role *role = cme->get_role();
			if (role) {
				Wander_Character_Role *wcr = dynamic_cast<Wander_Character_Role *>(role);
				if (wcr) {
					General::Point<float> top = area->get_top();

					if (!(
						pos.x+(General::TILE_SIZE*4) >= top.x &&
						pos.y+(General::TILE_SIZE*2) >= top.y &&
						pos.x-(General::TILE_SIZE*4) < top.x+cfg.screen_w &&
						pos.y-(General::TILE_SIZE*6) < top.y+cfg.screen_h))
					{
						skip_entity_collision_test = true;
					}
				}
			}
		}

		if (!skip_entity_collision_test) {
			area->shake_bushes(this, General::Point<float>(pos.x+pos_inc_x, pos.y+pos_inc_y));

			if (solid_with_entities && (input_disabled || (pos_inc_x != 0.0f || pos_inc_y != 0.0f))) {
				collided = area->entity_is_colliding(this, General::Point<float>(pos.x+pos_inc_x, pos.y+pos_inc_y), false);
				if (!moved_out_of_collision_start && collided.size() == 0) {
					moved_out_of_collision_start = true;
				}
				else if (!moved_out_of_collision_start) {
					collided.clear();
				}
			}
		}

		bool starting_battle = false;

		if (collided.size() > 0) {
			collided_with_entity = true;

			Map_Entity *e = *(collided.begin());
			should_return = true;

			// check if it's an enemy avatar
			if (
				(dynamic_cast<Enemy_Avatar *>(this) &&
				dynamic_cast<Player *>(e)) ||
				(dynamic_cast<Player *>(this) &&
				dynamic_cast<Enemy_Avatar *>(e))
			) {
				e->reset();
				reset();
				Area_Loop *area_loop = General::find_in_vector<Area_Loop *, Loop *>(engine->get_loops());
				if (dynamic_cast<Enemy_Avatar *>(e)) {
					if (!e->get_delete_me())
					{
						if (area_loop->get_num_jumping() == 0 && !input_is_disabled() && !area->point_is_in_no_enemy_zone(pos.x, pos.y) && !area->get_in_speech_loop() && (!area_loop || area_loop->battle_event_is_done())) {
							Lua::call_lua(area->get_lua_state(), "remove_enemy", "i>", e->get_id());
							e->set_delete_me(true);
							dynamic_cast<Player *>(this)->reset();
							dynamic_cast<Player *>(this)->set_attacking(false);
							start_battle((Enemy_Avatar *)e, false, area_loop->get_last_battle_screenshot(), area_loop->get_last_used_player_in_battle());
							area->set_early_break(true);
							starting_battle = true;
						}
					}
				}
				else {
					if (!get_delete_me()) {
						General::Point<float> epos = e->get_position();
						if (area_loop->get_num_jumping() == 0 && !e->input_is_disabled() && !area->point_is_in_no_enemy_zone(epos.x, epos.y) && !area->get_in_speech_loop() && (!area_loop || area_loop->battle_event_is_done())) {
							Lua::call_lua(area->get_lua_state(), "remove_enemy", "i>", get_id());
							set_delete_me(true);
							dynamic_cast<Player *>(e)->reset();
							dynamic_cast<Player *>(e)->set_attacking(false);
							start_battle((Enemy_Avatar *)this, false, area_loop->get_last_battle_screenshot(), area_loop->get_last_used_player_in_battle());
							area->set_early_break(true);
							starting_battle = true;
						}
					}
				}
			}
			
			float incx = 0, incy = 0;

			if (e->get_entities_slide_on_self()) {
				if (fabs(pos_inc_x) > fabs(pos_inc_y)) {
					std::list<Map_Entity *> collided2;
					collided2 = area->entity_is_colliding(this, General::Point<float>(pos.x, pos.y+1), false);
					if (collided2.size() == 0) {
						incy = 1;
					}
					else {
						collided2 = area->entity_is_colliding(this, General::Point<float>(pos.x, pos.y-1), false);
						if (collided2.size() == 0) {
							incy = -1;
						}
					}
				}
				else {
					std::list<Map_Entity *> collided2;
					collided2 = area->entity_is_colliding(this, General::Point<float>(pos.x+1, pos.y), false);
					if (collided2.size() == 0) {
						incx = 1;
					}
					else {
						collided2 = area->entity_is_colliding(this, General::Point<float>(pos.x-1, pos.y), false);
						if (collided2.size() == 0) {
							incx = -1;
						}
					}
				}
			}
			else {
				incx = pos_inc_x * 0.2f;
				incy = pos_inc_y * 0.2f;
			}

			General::Point<float> p = General::Point<float>(
				pos.x+incx,
				pos.y+incy
			);
			bool colliding = area->area_is_colliding(layer, p, bone, facing_right, NULL, NULL);

			if (!colliding) {
				pos_inc_x = incx;
				pos_inc_y = incy;
				should_return = false;
			}
		}

		collided.sort();
		current_collisions.sort();

		std::list<Map_Entity *> shared;
		General::find_shared_in_lists<Map_Entity *>(
			collided.begin(),
			collided.end(),
			current_collisions.begin(),
			current_collisions.end(),
			shared
		);

		// remove shared elements from each list
		General::erase_from_list<Map_Entity *>(collided, shared);

		std::list<Map_Entity *> uncolliding = current_collisions;
		General::erase_from_list<Map_Entity *>(uncolliding, shared);
		General::erase_from_list<Map_Entity *>(current_collisions, uncolliding);

		std::list<Map_Entity *>::iterator it;
		for (it = uncolliding.begin(); it != uncolliding.end(); it++) {
			Map_Entity *collided_entity = *it;
			int collided_id = collided_entity->get_id();
			lua_State *lua_state = area->get_lua_state();
			Lua::call_lua(lua_state, "uncollide", "ii>", id, collided_id);
		}

		for (it = collided.begin(); it != collided.end(); it++) {
			Map_Entity *collided_entity = *it;
			current_collisions.push_back(collided_entity);
			int collided_id = collided_entity->get_id();
			lua_State *lua_state = area->get_lua_state();
			Lua::call_lua(lua_state, "collide", "ii>", id, collided_id);

			Character_Map_Entity *cme = dynamic_cast<Character_Map_Entity *>(collided_entity);

			if (cme && id == 0 && cme->get_role()) {
				Wander_Character_Role *r = dynamic_cast<Wander_Character_Role *>(cme->get_role());
				if (r) {
					r->nudge(General::Point<float>(dx, dy));
				}
			}

			Area_Loop *area_loop = General::find_in_vector<Area_Loop *, Loop *>(engine->get_loops());
			if (area_loop) {
				if (area_loop->new_area_ready())
					return;
			}
		}

		if (!starting_battle && area->ladder_is_colliding(layer, this)) {
			colliding_with_ladder = true;
			anim_set->set_sub_animation("ladder");
			if (pos_inc_x != 0 || pos_inc_y != 0) {
				anim_set->update();
				pos_inc_x *= 0.75f;
				pos_inc_y *= 0.75f;
			}
		}
		
		if (should_return) {
			return;
		}
	}

	for (int i = 0; i < (int)cameras.size(); i++) {
		Camera *w = cameras[i];
		General::Point<float> tmp(pos_inc_x, pos_inc_y);
		General::Size<float> extents = get_current_bones()[0].get_extents();
		General::Size<int> sz(extents.w, extents.h);
		w->move_camera(tmp, pos, true);
	}

	pos.x += pos_inc_x;
	pos.y += pos_inc_y;
}

void Map_Entity::add_camera(Camera *camera)
{
	cameras.push_back(camera);
}

void Map_Entity::remove_camera(Camera *camera)
{
	std::vector<Camera *>::iterator it = std::find(cameras.begin(), cameras.end(), camera);
	if (it != cameras.end()) {
		cameras.erase(it);
	}
}

bool Map_Entity::load(void)
{
	skeleton = new Skeleton::Skeleton(name + ".xml");
	bool success = skeleton->load();

	if (!success) {
		delete skeleton;
		skeleton = NULL;

		std::string path = "map_entities/" + name;
		anim_set = new Animation_Set();
		anim_set->load(path);
		Bones::load(anim_set, path + "/info.xml", path, bones);
	}

	return true;
}

void Map_Entity::set_area_loop(Area_Loop *al)
{
	area_loop = al;
	area = al->get_area();
}

bool Map_Entity::collides_with_circle(float cx, float cy, float radius, float &out_distance)
{
	/*
	for (int i = 0; i < (int)collision_polygon.size(); i++) {
		Triangulate::Triangle &t = collision_polygon[i];
		for (int j = 0; j < 3; j++) {
			int j2 = (j+1) % 3;
			float distance = General::circle_line_distance(cx, cy,
				t.points[j].x+pos.x, t.points[j].y+pos.y, t.points[j2].x+pos.x, t.points[j2].y)+pos.y;
			if (distance < radius) {
				return true;
			}
		}
	}
	*/

	return false;
}

void Map_Entity::get_astar_thread_info(ALLEGRO_THREAD **thread, A_Star::A_Star_Thread_Info **ati)
{
	*thread = astar_thread;
	*ati = this->ati;
}

void Map_Entity::set_astar_thread_info(ALLEGRO_THREAD *thread, A_Star::A_Star_Thread_Info *ati)
{
	astar_thread = thread;
	this->ati = ati;
}

void Map_Entity::init(void)
{
	init_lua();
}

void Map_Entity::init_lua(void)
{
	if (dynamic_cast<Player *>(this)) {
		lua_state = NULL;
		return;
	}

	if (dynamic_cast<NPC *>(this)) {
		if (General::is_hero(name)) {
			lua_state = NULL;
			return;
		}
	}

	// Allow map entities to have an initialization routine in Lua
	lua_state = luaL_newstate();
	Lua::open_lua_libs(lua_state);
	Lua::register_c_functions(lua_state);

	Lua::load_global_scripts(lua_state);

	unsigned char *bytes;
	bytes = General::slurp("areas/global.lua");
	if (luaL_loadstring(lua_state, (char *)bytes)) {
		Lua::dump_lua_stack(lua_state);
		throw Error("Error loading global area script.\n");
	}
	delete[] bytes;
	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running global area script.");
	}

	std::string path = "map_entities/" + name;

	if (General::exists(path + "/map_entity_script.lua")) {
		unsigned char *bytes = General::slurp(path + "/map_entity_script.lua");
		if (luaL_loadstring(lua_state, (char *)bytes)) {
			Lua::dump_lua_stack(lua_state);
			throw Error("Error loading map entity script.\n");
		}
		delete[] bytes;

		if (lua_pcall(lua_state, 0, 0, 0)) {
			Lua::dump_lua_stack(lua_state);
			lua_close(lua_state);
			throw Error("Error running map entity script.");
		}

		Lua::call_lua(lua_state, "start", "i>", id);
	}
}

void Map_Entity::construct(std::string name)
{
	this->pos = General::Point<float>(0, 0);
	this->layer = 0;
	this->speed = 3;
	this->anim_set = NULL;
	this->name = name;
	this->direction = General::DIR_S;
	this->solid_with_area = true;
	this->solid_with_entities = true;
	this->input_disabled = false;
	this->move_cameras_while_input_disabled = true;
	this->show_shadow = false;
	this->astar_thread = NULL;
	this->ati = NULL;
	this->visible = true;
	this->sort_offset = 0;
	this->facing_right = true;
	this->lua_state = NULL;
	this->stationary = false;
	this->role_paused = false;
	this->collided_with_entity = false;
	this->moved_out_of_collision_start = false;
	this->colliding_with_ladder = false;
	this->pan_to_entity = -1;
	this->panning_back_to_player = false;
	this->skeleton = NULL;
	this->entities_slide = true;

	for (int i = 0; i < NUM_INPUTS; i++) {
		input[i] = 0;
	}
}

Map_Entity::Map_Entity(std::string name)
{
	construct(name);
}

Map_Entity::~Map_Entity(void)
{
	if (anim_set) {
		delete anim_set;
	}
	if (skeleton) {
		delete skeleton;
	}
	if (astar_thread) {
		al_destroy_thread(astar_thread);
		astar_thread = NULL;
	}

	delete ati;
	ati = NULL;

	if (lua_state) {
		lua_close(lua_state);
	}
}


float Map_Entity::get_current_speed(void)
{
	float curr_speed = speed;
	if (input[X] != 0 && input[Y] != 0) {
		curr_speed = sqrt(curr_speed*curr_speed + curr_speed*curr_speed) / 2.0;
	}
	return curr_speed;
}

std::string Map_Entity::get_name(void)
{
	return name;
}

void Map_Entity::auto_set_direction(void)
{
	int digital[2];

	General::analog_to_digital_joystick(input, digital);
    
	const General::Direction directions[3][3] = {
		{ General::DIR_NW, General::DIR_N,         General::DIR_NE },
		{ General::DIR_W,  (General::Direction)-1, General::DIR_E  },
		{ General::DIR_SW, General::DIR_S,         General::DIR_SE }
	};

	General::Direction tmp = directions[digital[1]+1][digital[0]+1];
	if (tmp >= 0) {
		direction = tmp;
		set_facing_right(tmp);
	}
}

void Map_Entity::draw(void)
{
	// KEEPME: draw bones
	/*
	std::vector<Bones::Bone> &bones = get_current_bones();

	for (size_t i = 0; i < bones.size(); i++) {
		std::vector<Triangulate::Triangle> &tris = bones[i].get();
		for (size_t j = 0; j < tris.size(); j++) {
			Triangulate::Triangle &t = tris[j];
			General::Point<float> top = area->get_top();
			al_draw_triangle(
				(t.points[0].x+pos.x-top.x)*cfg.screens_w,
				(t.points[0].y+General::BOTTOM_SPRITE_PADDING+pos.y-top.y)*cfg.screens_h,
				(t.points[1].x+pos.x-top.x)*cfg.screens_w,
				(t.points[1].y+General::BOTTOM_SPRITE_PADDING+pos.y-top.y)*cfg.screens_h,
				(t.points[2].x+pos.x-top.x)*cfg.screens_w,
				(t.points[2].y+General::BOTTOM_SPRITE_PADDING+pos.y-top.y)*cfg.screens_h,
				al_color_name("white"),
				1
			);
		}
	}
	*/
}

Collidable_Type Map_Entity::collidable_get_type(void)
{
	return COLLIDABLE_BONES;
}

void Map_Entity::collidable_get_position(
		General::Point<float> &pos
	)
{
	pos = this->pos;
}

void Map_Entity::collidable_get_bones(
		std::vector<Bones::Bone> &bones
	)
{
	bones = get_current_bones();
}

void Map_Entity::face(General::Point<float> point)
{
	float dx = point.x - pos.x;
	float dy = point.y - pos.y;
	float a = atan2(dy, dx);
	General::Direction dir = General::angle_to_direction(a);
	set_direction(dir);
}

std::vector<Bones::Bone> &Map_Entity::get_current_bones()
{
	std::string sub_name = anim_set->get_sub_animation_name();
	int frame = anim_set->get_current_animation()->get_current_frame_num();
	std::pair<std::string, int> p = std::pair<std::string, int>(sub_name, frame);
	return bones[p];
}

bool Map_Entity::is_facing_right()
{
	return facing_right;
}

bool Map_Entity::is_stationary()
{
	return stationary;
}

void Map_Entity::set_stationary(bool stationary)
{
	this->stationary = stationary;
}

void Map_Entity::set_role_paused(bool role_paused)
{
	this->role_paused = role_paused;
}

bool Map_Entity::get_collided_with_entity()
{
	return collided_with_entity;
}

void Map_Entity::set_collided_with_entity(bool collided)
{
	collided_with_entity = collided;
}

void Map_Entity::set_panning_to_entity(int ent)
{
	pan_to_entity = ent;
	if (ent == -1) {
		panning_back_to_player = false;
	}
}

