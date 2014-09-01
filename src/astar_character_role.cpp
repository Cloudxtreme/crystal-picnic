#include "crystalpicnic.h"
#include "character_map_entity.h"
#include "character_role.h"
#include "astar_character_role.h"
#include "area_loop.h"
#include "player.h"

void AStar_Character_Role::astar_callback(std::list<A_Star::Node> *list_nodes)
{
	int layer = entity->get_layer();
	
	Area_Loop *area_loop;
	Area_Manager *area;
	std::vector<Triangulate::Triangle> *meshes;
	A_Star::Navigation_Link ****nav_links;

	if (list_nodes == NULL) {
		goto error;
	}
	
	area_loop = General::find_in_vector<Area_Loop *, Loop *>(engine->get_loops());
	if (!area_loop) {
		goto error;
	}
	area = area_loop->get_area();

	meshes = area->get_nav_meshes();
	nav_links = area->get_nav_links();

	astar_intermediate_node = A_Star::next_intermediate_node(*list_nodes, list_nodes->begin());
	astar_intermediate_point = A_Star::dest_point_from_node(meshes[layer], nav_links[layer], *list_nodes, astar_dest, astar_intermediate_node);

	solid_with_area_save = entity->is_solid_with_area();
	entity->set_solid_with_area(false);

	astar_nodes = list_nodes;

end:
	callback_called_time = al_get_time();
	return;
	
error:
	A_Star::stop(astar_info);
	delete astar_nodes;
	astar_nodes = NULL;
	goto end;
}

void AStar_Character_Role::set_destination(General::Point<float> dest, bool run)
{
	A_Star::set_processing(true);

	running = run;
	float *input = entity->get_inputs();
	if (run) {
		input[Map_Entity::BUTTON1] = 1;
	}
	else {
		input[Map_Entity::BUTTON1] = 0;
	}

	int layer = entity->get_layer();
	General::Point<float> pos = entity->get_position();

	Skeleton::Skeleton *skeleton = entity->get_skeleton();
	std::vector<Bones::Bone> bones;
	if (!skeleton) {
		entity->collidable_get_bones(bones);
	}
	// FIXME: no collision with skeletons

	astar_dest = dest;

	if (astar_info) {
		A_Star::destroy(astar_info);
		astar_info = NULL;
	}

	if (astar_nodes) {
		delete astar_nodes;
		astar_nodes = NULL;
	}

	Area_Loop *area_loop = General::find_in_vector<Area_Loop *, Loop *>(engine->get_loops());
	if (!area_loop) {
		return;
	}
	Area_Manager *area = area_loop->get_area();

	astar_info = A_Star::create();

	std::vector<Triangulate::Triangle> *meshes = area->get_nav_meshes();
	A_Star::Navigation_Link ****nav_links = area->get_nav_links();

	A_Star::astar_callback cb = &AStar_Callback_Interface::astar_callback;
	A_Star::set_path(entity, cb, meshes[layer], nav_links[layer], pos, dest, bones, astar_info);
}

void AStar_Character_Role::draw(Area_Manager *area)
{
/*
	al_lock_mutex(A_Star::get_mutex());
	if (astar_nodes == NULL) {
		al_unlock_mutex(A_Star::get_mutex());
		return;
	}
	int layer = entity->get_layer();
	if (astar_info) {
		General::Point<float> top = area->get_top();
		std::list<A_Star::Node>::iterator it;
		std::vector<Triangulate::Triangle> *meshes = area->get_nav_meshes();
		A_Star::Navigation_Link ****nav_links = area->get_nav_links();
		for (it = astar_nodes->begin(); it != astar_nodes->end(); it++) {
			A_Star::Node &n = *it;
			for (int i = 0; i < 3; i++) {
				int i2 = (i+1) % 3;
				General::Point<float> tmp1 = meshes[layer][n.triangle_num].points[i];
				General::Point<float> tmp2 = meshes[layer][n.triangle_num].points[i2];
				al_draw_line((tmp1.x-top.x)*cfg.screens_w, (tmp1.y-top.y)*cfg.screens_h, (tmp2.x-top.x)*cfg.screens_w, (tmp2.y-top.y)*cfg.screens_h, al_map_rgb(255, 255, 255), 1);
			}
		}
	}
	al_unlock_mutex(A_Star::get_mutex());
*/
}

void AStar_Character_Role::destroy_astar_stuff(void)
{
	if (astar_info)
		A_Star::destroy(astar_info);
	astar_info = NULL;
	delete astar_nodes;
	astar_nodes = NULL;
}

void AStar_Character_Role::update(Area_Manager *area)
{
	if (A_Star::is_processing()) {
		return;
	}
	
	ALLEGRO_MUTEX *astar_mutex = A_Star::get_mutex();
	al_lock_mutex(astar_mutex);

	if (wait_after_collision > 0.0) {
		if (al_get_time() < wait_after_collision) {
			al_unlock_mutex(astar_mutex);
			return;
		}
		wait_after_collision = 0.0;
		entity->set_collided_with_entity(false);
		destroy_astar_stuff();
		set_destination(astar_dest, running);
		al_unlock_mutex(astar_mutex);
		return;
	}

	float *input = entity->get_inputs();

	if (entity->get_collided_with_entity() && !doesnt_wait_after_collision) {
		wait_after_collision = al_get_time() + 5.0;
		input[Map_Entity::X] = input[Map_Entity::Y] = 0;
		al_unlock_mutex(astar_mutex);
		return;
	}

	if (astar_nodes) {
		General::Point<float> pos = entity->get_position();
		float curr_speed = entity->get_run_factor() * entity->get_current_speed();
		float dest_dx = astar_dest.x - pos.x;
		float dest_dy = astar_dest.y - pos.y;
		float dist = sqrt(dest_dx*dest_dx + dest_dy*dest_dy);

		if (dist > 20 && !(astar_intermediate_point == astar_dest) && (al_get_time()-callback_called_time >= 1.0)) {
			int layer = entity->get_layer();
			float dx = astar_intermediate_point.x - astar_dest.x;
			float dy = astar_intermediate_point.y - astar_dest.y;
			while (sqrt(dx*dx + dy*dy) > curr_speed*1.1) {
				std::vector<Triangulate::Triangle> *meshes = area->get_nav_meshes();
				A_Star::Navigation_Link ****nav_links = area->get_nav_links();
				astar_intermediate_node++;
				astar_intermediate_node = A_Star::next_intermediate_node(*astar_nodes, astar_intermediate_node);
				astar_intermediate_point = A_Star::dest_point_from_node(meshes[layer], nav_links[layer], *astar_nodes, astar_dest, astar_intermediate_node);
				dx = astar_intermediate_point.x - astar_dest.x;
				dy = astar_intermediate_point.y - astar_dest.y;
			}
			destroy_astar_stuff();
			set_destination(astar_dest, input[Map_Entity::BUTTON1]);
			al_unlock_mutex(astar_mutex);
			return;
		}

		if (dist <= curr_speed*1.1) {
			destroy_astar_stuff();
			input[Map_Entity::X] = input[Map_Entity::Y] = 0;
			entity->set_solid_with_area(solid_with_area_save);
			if (is_kamikaze) {
				is_kamikaze = false;
				Area_Loop *area_loop = GET_AREA_LOOP;
				Area_Manager *area = area_loop->get_area();
				Player *player = dynamic_cast<Player *>(area->get_entity(0));
				Lua::call_lua(area->get_lua_state(), "remove_enemy", "i>", entity->get_id());
				entity->set_delete_me(true);
				player->reset();
				player->set_attacking(false);
				start_battle(
					(Enemy_Avatar *)entity,
					false,
					area_loop->get_last_battle_screenshot(),
					area_loop->get_last_used_player_in_battle()
				);
				area->set_early_break(true);
			}
			entity->set_position(astar_dest);
		}
		else {
			int layer = entity->get_layer();
			float dx = astar_intermediate_point.x - pos.x;
			float dy = astar_intermediate_point.y - pos.y;
			if (sqrt(dx*dx + dy*dy) <= curr_speed*1.1) {
				entity->set_position(astar_intermediate_point);
				std::vector<Triangulate::Triangle> *meshes = area->get_nav_meshes();
				A_Star::Navigation_Link ****nav_links = area->get_nav_links();
				astar_intermediate_node++;
				astar_intermediate_node = A_Star::next_intermediate_node(*astar_nodes, astar_intermediate_node);
				astar_intermediate_point = A_Star::dest_point_from_node(meshes[layer], nav_links[layer], *astar_nodes, astar_dest, astar_intermediate_node);
			}
			else {
				float angle = atan2(dy, dx);
				const float mul = entity->get_run_factor();
				input[Map_Entity::X] = cos(angle) * mul;
				input[Map_Entity::Y] = sin(angle) * mul;
			}
		}
	}

	al_unlock_mutex(astar_mutex);
}

bool AStar_Character_Role::is_following_path(void)
{
	return astar_info != NULL;
}

AStar_Character_Role::AStar_Character_Role(Character_Map_Entity *character) :
	Character_Role(character),
	astar_info(NULL),
	astar_nodes(NULL),
	is_kamikaze(false),
	doesnt_wait_after_collision(false)
{
	callback_called_time = al_get_time();
	running = false;
	wait_after_collision = 0.0;
	character->set_collided_with_entity(false);
}

AStar_Character_Role::~AStar_Character_Role(void)
{
	destroy_astar_stuff();
}

void AStar_Character_Role::set_kamikaze(bool kamikaze)
{
	is_kamikaze = kamikaze;
}

void AStar_Character_Role::set_doesnt_wait_after_collision(bool doesnt_wait)
{
	doesnt_wait_after_collision = doesnt_wait;
}
