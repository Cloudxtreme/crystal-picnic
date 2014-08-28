#include "battle_pathfind.h"
#include "engine.h"
#include "runner_loop.h"

bool Battle_AI::init(void)
{
	battle_loop = General::find_in_vector<Battle_Loop *, Loop *>(engine->get_loops());

	if (!battle_loop) {
		battle_loop = General::find_in_vector<Battle_Loop *, Loop *>(engine->get_mini_loops());
	}

	init_lua();

	return true;
}

void Battle_AI::top(void)
{
	if (dynamic_cast<Battle_Player *>(body)) {
		get_A_ENEMY();
	}
	else {
		get_A_PLAYER();
	}
}

bool Battle_AI::handle_event(ALLEGRO_EVENT *event)
{
	if (!(event->type >= ALLEGRO_GET_EVENT_TYPE('B', 'A', 'R', 'Y')))
		return false;
	
	User_Event_Data *ued = (User_Event_Data *)event->user.data1;
	if (ued->target_id != body->get_id())
		return false;

	if (!(body->is_hurt() || body->is_attacking())) {
		switch (event->type) {
			// Not used yet
			/*
			case UE_Find_Path: {
				UE_Find_Path_Data *data = (UE_Find_Path_Data *)event->user.data1;
				if (data->target && data->target->is_jumping()) {
					data->dx = data->target->get_position().x;
					data->dy = data->target->get_position().y;
					engine->push_event(UE_Find_Path, data);
				}
				else {
					Battle_AI_Action *a = new Battle_AI_Action;
					a->type = UE_Find_Path;
					// NOTE: All user events need a copy constructor
					a->data = new UE_Find_Path_Data(*data);
					goals.add_data(a);
				}
				break;
			}
			*/
			case UE_Stop: {
				if (!body->is_jumping()) {
					stop();
				}
				else {
					push_stop();
				}
				break;
			}
		}
	}

	return false;
}

bool Battle_AI::logic(void)
{
	Lua::call_lua(lua_state, "logic", ">");

	if (body->is_hurt()) {
		return false;
	}

	std::map<const int, float> &input = body->get_input();

	if (body->is_attacking()) {
		input[Battle_Entity::ATTACK] = 0;
		return false;
	}

	if (maybe_attack()) {
		return false;
	}
	
	bool first = false;

	if (current_goal == NULL) {
		if (goals.get_list().size() > 0) {
			process_next_action();
			first = true;
		}
		else {
			decide();
		}
		return false;
	}

	input[Battle_Entity::Y] = 0;

	if (!finished_jump) {
		UE_Follow_Edge_Data *d = (UE_Follow_Edge_Data *)current_goal->data;
		float height = d->dy - d->sy;
		float dist = General::distance(d->dx, d->dy, d->sx, d->sy);
		if (jumping_down) {
			//if (body->get_velocity().y <= 0) {
			input[Battle_Entity::JUMP] = 0;
				finished_jump = true;
				next_goal();
			//}
		}
		else {
			if (body->get_velocity().y >= 0) {
				if (fabs(height) > 45 || fabs(dist) > 75) {
					if (!double_jump_stopped) {
						double_jump_stopped = true;
						input[Battle_Entity::JUMP] = 0;
						return false;
					}
					else if (!double_jumped) {
						double_jumped = true;
						input[Battle_Entity::JUMP] = 1;
						return false;
					}
				}
				finished_jump = true;
				next_goal();
			}
		}
		return false;
	}
	
	input[Battle_Entity::JUMP] = 0;

	float start_x_input = input[Battle_Entity::X];

	if (!body->is_jumping()) {
		input[Battle_Entity::X] = 0;
	}

	// proccess current action

	switch (current_goal->type) {
		case UE_Seek_On_Platform: {
			if (body->get_attributes().hp > 0) {
				General::Point<float> pos = body->get_position();
				UE_Seek_On_Platform_Data *d = (UE_Seek_On_Platform_Data *)current_goal->data;
				if (first) {
					if (fabs(pos.x-d->x) < 10) {
						next_goal();
						return false;
					}
				}

				if (pos.x < d->x) {
					input[Battle_Entity::X] = 1;
				}
				else {
					input[Battle_Entity::X] = -1;
				}

				if (General::sign(start_x_input) != General::sign(input[Battle_Entity::X])) {
					body->set_accel(General::Point<float>(0, 0));
					body->set_velocity(General::Point<float>(0, 0));
				}

				if (fabs(pos.x-d->x) < 5) {
					input[Battle_Entity::X] = 0;
					body->set_accel(General::Point<float>(0, 0));
					body->set_velocity(General::Point<float>(0, 0));
					next_goal();
					return false;
				}
			}

			break;
		}
		case UE_Direct_Move: {
			if (body->get_attributes().hp > 0) {
				General::Point<float> pos = body->get_position();
				UE_Direct_Move_Data *d = (UE_Direct_Move_Data *)current_goal->data;
				
				float max_x_vel;

				body->get_physics(
					NULL,
					NULL,
					NULL,
					&max_x_vel,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL
				);

				float speed = body->get_speed_multiplier() * max_x_vel;

				/*
				if ((d->x < pos.x && pos.x < (speed+1)) || (d->x > pos.x && fabsf(pos.x-battle_loop->get_width()) < (speed+1))) {
					next_goal();
					return false;
				}
				*/

				if (d->x >= 0 && d->y < 0) {
					if (d->x <= pos.x) {
						body->set_facing_right(false);
						pos.x -= speed;
						if (pos.x <= d->x) {
							body->set_position(General::Point<float>(d->x, pos.y));
							next_goal();
							return false;
						}
					}
					else if (d->x >= pos.x) {
						body->set_facing_right(true);
						pos.x += speed;
						if (pos.x >= d->x) {
							body->set_position(General::Point<float>(d->x, pos.y));
							next_goal();
							return false;
						}
					}
				}
				else if (d->y >= 0 && d->x < 0) {
					if (d->y <= pos.y) {
						pos.y -= speed;
						if (pos.y <= d->y) {
							body->set_position(General::Point<float>(pos.x, d->y));
							next_goal();
							return false;
						}
					}
					else if (d->y >= pos.y) {
						pos.y += speed;
						if (pos.y >= d->y) {
							body->set_position(General::Point<float>(pos.x, d->y));
							next_goal();
							return false;
						}
					}
				}
				else {
					if (d->x <= pos.x) {
						body->set_facing_right(false);
						pos.x -= speed;
						if (d->x >= pos.x) {
							pos.x = d->x;
							d->x = -1;
						}
					}
					else if (d->x > pos.x) {
						body->set_facing_right(true);
						pos.x += speed;
						if (d->x <= pos.x) {
							pos.x = d->x;
							d->x = -1;
						}
					}
					if (d->y <= pos.y) {
						pos.y -= speed;
						if (d->y >= pos.y) {
							pos.y = d->y;
							d->y = -1;
						}
					}
					else if (d->y > pos.y) {
						pos.y += speed;
						if (d->y <= pos.y) {
							pos.y = d->y;
							d->y = -1;
						}
					}
					if (General::distance(pos.x, pos.y, d->x, d->y) < speed * 1.5f) {
						body->set_position(General::Point<float>(d->x, d->y));
						next_goal();
						return false;
					}
				}
	
				body->set_position(pos);
			}

			break;
		}
		case UE_Follow_Edge: {
			UE_Follow_Edge_Data *d = (UE_Follow_Edge_Data *)current_goal->data;
			/*
			if (d->start_platform != body->get_platform()) {
				next_goal();
				return false;
			}
			*/

			General::Point<float> pos = body->get_position();
			// if the node the entity is on is not the current path node or
			// directly connected to it, replan the path
			float diff = General::distance(pos.x, pos.y, d->dx, d->dy);
			float target_diff;
			UE_Follow_Edge_Data *next = NULL;
			UE_Seek_On_Platform_Data *next_SOPD = NULL;
			if (goals.get_list().size() > 1) {
				std::list<Battle_AI_Action *>::iterator it;
				it = goals.get_list().begin();
				it++;
				if ((*it)->type == UE_Follow_Edge) {
					next = (UE_Follow_Edge_Data *)(*it)->data;
				}
				else if (d->jump_type == Battle_Loop::JUMP_NONE && (*it)->type == UE_Seek_On_Platform) {
					next_SOPD =
						(UE_Seek_On_Platform_Data *)(*it)->data;
				}
			}
			if (next_SOPD) {
				float curr_dist =
					General::distance(d->sx, d->sy, d->dx, d->dy) + fabs(d->dx-next_SOPD->x);
				if (fabs(d->sx-next_SOPD->x) < curr_dist) {
					input[Battle_Entity::X] = start_x_input;
					next_goal();
					return false;
				}
			}
			target_diff = 10;
			if (body->is_jumping()) {
				return false;
			}
			if (d->jump_type != Battle_Loop::JUMP_NONE) {
				if (fabs(pos.x-d->sx) < target_diff) {
					if (d->jump_type != Battle_Loop::JUMP_HORIZONTAL) {
						body->set_velocity(General::Point<float>(0, 0));
						body->set_accel(General::Point<float>(0, 0));
						body->set_position(General::Point<float>(d->sx, d->sy));
						input[Battle_Entity::X] = 0;
					}
					else {
						float x = MIN(1, 0.5f+diff/400.0f);
						input[Battle_Entity::X] = x * ((d->sx < d->dx) ? 1 : -1);
					}
					input[Battle_Entity::JUMP] = 1;
					if (d->jump_type == Battle_Loop::JUMP_DOWN) {
						input[Battle_Entity::Y] = 1;
						jumping_down = true;
					}
					else {
						jumping_down = false;
					}
					finished_jump = false;
					double_jumped = false;
					double_jump_stopped = false;
					return false;
				}
			}
			bool end = false;
			if (next) {
				//if (next->jump_type != Battle_Loop::JUMP_NONE)
				//	end = true;
			}
			else {
				end = true;
			}
			float val = MIN(1.0f, diff / 150) * 0.23f + 0.77f;
			if (end) {
				if (next && next->jump_type == Battle_Loop::JUMP_HORIZONTAL) {
					if (val > 0.8f) {
						val = 0.8f;
					}
				}
				else {
					if (val > 0.77f) {
						val = 0.77f;
					}
				}
			}
			else {
				val = 1;
			}
			if (pos.x < d->dx) {
				input[Battle_Entity::X] = val;
			}
			else if (pos.x > d->dx) {
				input[Battle_Entity::X] = -val;
			}
			if (d->start_platform != body->get_platform()) {
				next_goal();
				return false;
			}
			bool timeout = d->end_time != 0 && d->end_time < al_get_time();
			if (diff < target_diff) {
				if (timeout) {
					stop();
				}
				else {
					next_goal();
				}
				return false;
			}

			break;
		}
		case UE_Rest: {
			UE_Rest_Data *d = (UE_Rest_Data *)current_goal->data;
			d->countdown -= General::LOGIC_MILLIS / 1000.0;
			if (d->countdown <= 0) {
				next_goal();
			}
			break;
		}
		default:
			break;
	}

	if (body->get_attributes().hp > 0) {
		if (input[Battle_Entity::X] < 0) {
			body->set_facing_right(false);
		}
		else if (input[Battle_Entity::X] > 0)
			body->set_facing_right(true);
	}

	return false;
}

void Battle_AI::draw(void)
{
}

bool Battle_AI::get_should_attack()
{
	return should_attack;
}

void Battle_AI::set_should_attack(bool should_attack)
{
	this->should_attack = should_attack;
	attack_type = Battle_Entity::ATTACK;
}

Battle_AI::Battle_AI(Battle_Entity *body) :
	body(body),
	current_goal(NULL),
	missed_attacks(0),
	lua_state(NULL),
	finished_jump(true),
	A_PLAYER(-1),
	A_ENEMY(-1),
	should_attack(false),
	attack_type(Battle_Entity::ATTACK)
{
	if (dynamic_cast<Battle_Player *>(body)) {
		aggressiveness = General::LOGIC_RATE * 20;
	}
	else {
		aggressiveness = General::LOGIC_RATE * 2;
	}
}

Battle_AI::~Battle_AI(void)
{
	if (lua_state) {
		Lua::call_lua(lua_state, "stop", ">");
		lua_close(lua_state);
	}

	while (goals.get_list().size() > 0) {
		next_goal();
	}
}
	
void Battle_AI::process_next_action(void)
{
	if (goals.get_list().size() <= 0) {
		return;
	}

	Battle_AI_Action *general_action = goals.get_data();

	switch (general_action->type) {
		case UE_Find_Path: {
			// remove this general expanding action
			goals.pop_front();
			// split it up into a bunch of follow_edge goals
			UE_Find_Path_Data *d1 = (UE_Find_Path_Data *)general_action->data;
			if (d1->platform != -1) {
				std::vector<Battle_Pathfinder_Node> v = battle_loop->find_path(
					body->get_platform(), body->get_position(),
					d1->platform, General::Point<float>(d1->dx, d1->dy)
				);
				if (v.size() > 0) {
					std::list<Battle_AI_Action *> list;
					for (int i = v.size()-1; i >= 1; i--) {
						Battle_Pathfinder_Node &n = v[i];
						int j = i-1;
						if (j < 0) j = v.size()-1;
						Battle_Pathfinder_Node &n2 = v[j];
						Battle_AI_Action *a = new Battle_AI_Action;
						a->type = UE_Follow_Edge;
						UE_Follow_Edge_Data *d = new UE_Follow_Edge_Data;
						a->data = d;
						d->sx = n.x;
						d->sy = n.y;
						d->start_platform = n.platform;
						// find next node
						Battle_Loop::Jump_Point_Type jpt = Battle_Loop::JUMP_NONE;
						std::list<Battle_Pathfinder_Node::List_Data>::iterator it;
						it = n.links.begin();
						for (; it != n.links.end(); it++) {
							Battle_Pathfinder_Node::List_Data &data = *it;
							if (!data.edge) {
								continue;
							}
							else if (data.edge->start && fabs(data.edge->start->x-n2.x) < 0.1 && fabs(data.edge->start->y-n2.y) < 0.1) {
								jpt = data.jump_point ? data.jump_point->type : Battle_Loop::JUMP_NONE;
								break;
							}
							else if (data.edge->end && fabs(data.edge->end->x-n2.x) < 0.1 && fabs(data.edge->end->y-n2.y) < 0.1) {
								jpt = data.jump_point ? data.jump_point->type : Battle_Loop::JUMP_NONE;
								break;
							}
						}

						d->jump_type = jpt;

						d->end_time = d1->end_time;

						bool done = false;

						if (d1->within >= 0) {
							float destx = d->sx+(n2.x-d->sx)/2;
							float desty = d->sy+(n2.y-d->sy)/2;
							float dist = General::distance(d1->dx, d1->dy, destx, desty);
							if (dist <= d1->within) {
								done = true;
								d->dx = destx;
								d->dy = desty;
							}
						}
						
						if (!done) {
							d->dx = n2.x;
							d->dy = n2.y;
						}
						
						d->end_platform = n2.platform;

						list.push_back(a);
						
						if (done) {
							break;
						}
					}

					if (list.size() == 0) {
						UE_Rest_Data *d = new UE_Rest_Data;
						d->countdown = 0.5;
						d->force = false;
						Battle_AI_Action *a = new Battle_AI_Action;
						a->type = UE_Rest;
						a->data = d;
						goals.add_data(a);
					}
					else {
						goals.add_data(goals.get_list().begin(), list.begin(), list.end());
					}
				}
				else {
					UE_Rest_Data *d = new UE_Rest_Data;
					d->countdown = 0.5;
					d->force = false;
					Battle_AI_Action *a = new Battle_AI_Action;
					a->type = UE_Rest;
					a->data = d;
					goals.add_data(a);
				}
			}
			else {
				UE_Rest_Data *d = new UE_Rest_Data;
				d->countdown = 0.5;
				d->force = false;
				Battle_AI_Action *a = new Battle_AI_Action;
				a->type = UE_Rest;
				a->data = d;
				goals.add_data(a);
			}

			delete general_action->data;
			delete general_action;

			break;
		}
		default:
			break;
	}

	current_goal = goals.get_data();
}

void Battle_AI::init_lua(void)
{
	std::string name = body->get_name();

	lua_state = luaL_newstate();

	Lua::open_lua_libs(lua_state);

	Lua::register_c_functions(lua_state);

	Lua::load_global_scripts(lua_state);

	unsigned char *bytes;

	bytes = General::slurp("battle/global.lua");
	if (luaL_loadstring(lua_state, (char *)bytes)) {
		Lua::dump_lua_stack(lua_state);
		throw Error("Error loading global battle script.\n");
	}
	delete[] bytes;

	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running global battle script.");
	}
	
	bytes = General::slurp("battle/ai/global.lua");
	if (luaL_loadstring(lua_state, (char *)bytes)) {
		Lua::dump_lua_stack(lua_state);
		throw Error("Error loading global AI script.\n");
	}
	delete[] bytes;

	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running global AI script.");
	}
	
	std::string script_name;
	
	if (dynamic_cast<Battle_Player *>(body)) {
		script_name = dynamic_cast<Runner_Loop *>(battle_loop) ? "player_runner" : "player_fighter";
	}
	else {
		script_name = name;
	}

	bytes = General::slurp("battle/ai/" + script_name + ".lua");
	if (luaL_loadstring(lua_state, (char *)bytes)) {
		Lua::dump_lua_stack(lua_state);
		throw Error("Error loading battle AI script.\n");
	}
	delete[] bytes;

	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running AI script.");
	}

	Lua::call_lua(lua_state, "start", "i>", body->get_id());
}

// returns player number not entity id
int Battle_AI::get_A_PLAYER()
{
	bool pick_new_one = false;

	if (A_PLAYER == -1) {
		pick_new_one = true;
	}
	else {
		Battle_Player *p = battle_loop->get_player(A_PLAYER);
		if (p) {
			if (p->get_attributes().hp <= 0) {
				pick_new_one = true;
			}
		}
		else {
			pick_new_one = true;
		}
	}

	if (pick_new_one) {
		A_PLAYER = -1;
		int j = rand() % 3;
		for (int i = 0; i < 3; i++) {
			int k = (i+j) % 3;
			Battle_Player *p = battle_loop->get_player(k);
			if (p) {
				if (p->get_attributes().hp > 0) {
					A_PLAYER = k;
					break;
				}
			}
		}
	}

	return A_PLAYER;
}

// returns an entity id
int Battle_AI::get_A_ENEMY()
{
	bool pick_new_one = false;

	if (A_PLAYER == -1) {
		pick_new_one = true;
	}
	else {
		Battle_Entity *e = battle_loop->get_entity(A_ENEMY);
		if (!e) {
			pick_new_one = true;
		}
	}

	if (pick_new_one) {
		A_ENEMY = -1;
		std::vector<Battle_Entity *> &v = battle_loop->get_entities();
		int sz = v.size();
		int start = rand() % sz;
		for (int i = 0; i < sz; i++) {
			int n = (i+start) % sz;
			Battle_Enemy *e = dynamic_cast<Battle_Enemy *>(v[n]);
			if (e && e->get_attributes().hp > 0 && !General::is_item(e->get_name())) {
				A_ENEMY = e->get_id();
				break;
			}
		}
	}

	return A_ENEMY;
}

void Battle_AI::decide(void)
{
	Lua::call_lua(lua_state, "decide", ">s");
	std::string cmd = lua_tostring(lua_state, -1);
	lua_pop(lua_state, 1);

	std::vector<std::string> c = General::split(cmd);

	int i = 0;
	while (i < (int)c.size()) {
		if (c[i] == "seek") {
			i++;
			float x, y;
			UE_Find_Path_Data *d1 = new UE_Find_Path_Data;
			if (c[i] == "pos") {
				i++;
				x = atof(c[i++].c_str());
				y = atof(c[i++].c_str());
				d1->platform = -1;
				d1->within = -1.0f;
			}
			else {
				float within;
				if (c[i] == "within") {
					i++;
					within = atof(c[i++].c_str());
					int randomness = atoi(c[i++].c_str());
					within += General::rand() % (randomness * 2) - randomness;
				}
				else {
					within = -1.0f;
				}
				d1->within = within;
				float end_time;
				if (c[i] == "timed") {
					i++;
					end_time = al_get_time() +
						atof(c[i++].c_str());
				}
				else {
					end_time = 0;
				}
				d1->end_time = end_time;
				std::string entS = c[i++];
				int to_seek;
				if (entS == "A_PLAYER") {
					to_seek = A_PLAYER == -1 ? -1 : battle_loop->get_player(A_PLAYER)->get_id();
				}
				else if (entS == "A_ENEMY") {
					to_seek = A_ENEMY;
				}
				else {
					to_seek = atoi(entS.c_str());
				}
				Battle_Entity *e = battle_loop->get_entity(to_seek);
				if (e) {
					d1->platform = ((Battle_Entity *)e)->get_platform();
					x = e->get_position().x;
					y = e->get_position().y;
				}
				else {
					// oh no!
					delete d1;
					push_stop();
					return;
				}
			}

			d1->dx = x;
			d1->dy = y;
			Battle_AI_Action *a = new Battle_AI_Action();
			a->type = UE_Find_Path;
			a->data = d1;
			goals.add_data(a);

			if (d1->within < 0) {
				UE_Seek_On_Platform_Data *d2 = new UE_Seek_On_Platform_Data;
				d2->x = x;
				Battle_AI_Action *a2 = new Battle_AI_Action();
				a2->type = UE_Seek_On_Platform;
				a2->data = d2;
				goals.add_data(a2);
			}
		}
		else if (c[i] == "direct_move") {
			i++;
			float x, y;
			x = atof(c[i++].c_str());
			y = atof(c[i++].c_str());
			UE_Direct_Move_Data *d = new UE_Direct_Move_Data;
			d->x = x;
			d->y = y;
			Battle_AI_Action *a = new Battle_AI_Action();
			a->type = UE_Direct_Move;
			a->data = d;
			goals.add_data(a);
		}
		else if (c[i] == "rest") {
			i++;
			if (c[i] == "quick") {
				i++;
				float quick_time = atof(c[i++].c_str());
				float long_time = atof(c[i++].c_str());
				bool quick_rest = General::rand() % 2;
				if (quick_rest) {
					push_rest(quick_time, true);
				}
				else {
					push_rest(long_time, true);
				}
			}
			else {
				float time = atof(c[i++].c_str());
				push_rest(time, true);
			}
		}
		else if (c[i] == "set") {
			i++;
			if (c[i] == "speed") {
				i++;
				int entity_id = atoi(c[i++].c_str());
				float multiplier =
					atof(c[i++].c_str());
				Battle_Entity *e = battle_loop->get_entity(entity_id);
				if (e) {
					e->set_speed_multiplier(
						multiplier
					);
				}
			}
			else if (c[i] == "pos") {
				i++;
				int entity_id = atoi(c[i++].c_str());
				float x = atof(c[i++].c_str());
				float y = atof(c[i++].c_str());
				Battle_Entity *e = battle_loop->get_entity(entity_id);
				if (e) {
					e->set_position(General::Point<float>(x, y));
				}
			}
		}
		else if (c[i] == "attack") {
			i++;
			should_attack = true;
			lua_getglobal(lua_state, "get_attack_type");
			bool exists = !lua_isnil(lua_state, -1);
			lua_pop(lua_state, 1);
			if (exists) {
				Lua::call_lua(lua_state, "get_attack_type", ">i");
				attack_type = lua_tonumber(lua_state, -1);
				lua_pop(lua_state, 1);
			}
			else {
				attack_type = Battle_Entity::ATTACK;
			}
		}
		else if (c[i] == "do_ability") {
			i++;
			std::map<const int, float> &input = body->get_input();
			input[atoi(c[i++].c_str())] = 1;
		}
		else if (c[i] == "heal") {
			i++;
			Battle_Player *p = dynamic_cast<Battle_Player *>(body);
			p->heal(atoi(c[i++].c_str()));
		}
		else if (c[i] == "nil") {
			i++;
			continue;
		}
		if (i < (int)c.size() && c[i] == "nostop") {
			i++;
			continue;
		}
		push_stop();
	}
}

void Battle_AI::push_rest(double time, bool force)
{
	UE_Rest_Data *d = new UE_Rest_Data;
	d->countdown = time;
	d->force = force;
	Battle_AI_Action *a = new Battle_AI_Action;
	a->type = UE_Rest;
	a->data = d;
	goals.add_data(a);
}

void Battle_AI::push_stop(void)
{
	UE_Stop_Data *data = new UE_Stop_Data();
	data->target_id = body->get_id();
	engine->push_event(UE_Stop, data, al_get_time()+1);
}

void Battle_AI::next_goal(void)
{
	if (goals.get_list().size() > 0)
		goals.pop_front();
	if (current_goal) {
		delete current_goal->data;
		delete current_goal;
		current_goal = NULL;
	}
}

void Battle_AI::stop(void)
{
	current_goal = NULL;

	while (goals.get_list().size() > 0) {
		Battle_AI_Action *a = goals.get_data();
		delete a->data;
		delete a;
		goals.pop_front();
	}

	std::map<const int, float> &input = body->get_input();
	input.clear();
}

bool Battle_AI::maybe_attack(void)
{
	std::map<const int, float> &input = body->get_input();

	if (should_attack) {
		should_attack = false;
		input[attack_type] = 1;
		return true;
	}

	Lua::call_lua(lua_state, "get_should_auto_attack", ">b");
	if (lua_gettop(lua_state) == 1) {
		bool auto_attack = lua_toboolean(lua_state, -1);
		lua_pop(lua_state, 1);
		if (!auto_attack) {
			return false;
		}
	}

	// FIXME
	if (body->get_skeleton()) {
		return false;
	}

	if (body->get_animation_set()->get_sub_animation_name() == "hit") {
		return false;
	}

	Battle_Entity *to_attack;
	
	if (dynamic_cast<Battle_Player *>(body)) {
		to_attack = battle_loop->get_entity(A_ENEMY);
	}
	else {
		to_attack = battle_loop->get_player(A_PLAYER);
	}
	
	if (to_attack == NULL || to_attack->get_skeleton()/*FIXME: implement sizes below for skeletons*/) {
		return false;
	}

	int X_MAX_DIFF, Y_MAX_DIFF;

	lua_getglobal(lua_state, "get_melee_distance");
	bool exists = !lua_isnil(lua_state, -1);
	lua_pop(lua_state, 1);

	float melee_dist = 0.0f;
	std::pair<std::string, int> key;
	key.first = "battle-idle";
	key.second = 0;
	int my_bones_h = body->get_bones()[key][0].get_extents().h;
	int to_attack_bones_h = to_attack->get_bones()[key][0].get_extents().h;
	Y_MAX_DIFF = MAX(my_bones_h, to_attack_bones_h);

	if (exists) {
		Lua::call_lua(lua_state, "get_melee_distance", ">d");
		exists = !lua_isnil(lua_state, -1); // function may return nil to abort
		if (exists) {
			melee_dist = lua_tonumber(lua_state, -1);
		}
		lua_pop(lua_state, 1);
	}
	if (exists) {
		X_MAX_DIFF = melee_dist;
	}
	else {
		X_MAX_DIFF = body->get_animation_set()->get_current_animation()->get_current_frame()->get_width()/2 * 1.1f;
	}

	General::Point<float> ppos = to_attack->get_position();
	General::Point<float> epos = body->get_position();

	if (fabs(ppos.y-epos.y) > Y_MAX_DIFF) {
		return false;
	}

	if (fabs(ppos.x-epos.x) > X_MAX_DIFF) {
		return false;
	}

	/*
	if (current_goal && current_goal->type == UE_Rest) {
		UE_Rest_Data *d = (UE_Rest_Data *)current_goal->data;
		if (d->force) {
			return false;
		}
	}
	*/

	if (body->get_name() != "faff") {
		if (ppos.x < epos.x)
			body->set_facing_right(false);
		else
			body->set_facing_right(true);
	}

	if ((int)(General::rand() % aggressiveness) > missed_attacks) {
		missed_attacks++;
		return false;
	}

	missed_attacks = 0;

	int attack_type;
	lua_getglobal(lua_state, "get_attack_type");
	exists = !lua_isnil(lua_state, -1);
	lua_pop(lua_state, 1);
	if (exists) {
		Lua::call_lua(lua_state, "get_attack_type", ">i");
		attack_type = lua_tonumber(lua_state, -1);
		lua_pop(lua_state, 1);
	}
	else {
		attack_type = Battle_Entity::ATTACK;
	}

	input[attack_type] = 1;

	return true;
}

lua_State *Battle_AI::get_lua_state(void)
{
	return lua_state;
}

void Battle_AI::set_aggressiveness(int aggressiveness)
{
	this->aggressiveness = aggressiveness;
}
