#include "crystalpicnic.h"
#include "character_role.h"
#include "wander_character_role.h"
#include "astar_character_role.h"
#include "character_map_entity.h"
#include "astar.h"

Character_Role *Character_Map_Entity::get_role()
{
	return role;
}

void Character_Map_Entity::set_role(Character_Role *role)
{
	this->role = role;
	if (astar_thread) {
		if (ati) {
			ati->done = true;
			al_signal_cond(ati->cond);
		}
		al_destroy_thread(astar_thread);
		astar_thread = NULL;
	}
	if (ati) {
		al_destroy_mutex(ati->mutex);
		al_destroy_cond(ati->cond);
		delete ati;
		ati = NULL;
	}
}

void Character_Map_Entity::character_set_position(General::Point<float> pos)
{
	if (role == NULL) {
		Map_Entity::set_position(pos);
		return;
	}
	
	Wander_Character_Role *r = dynamic_cast<Wander_Character_Role *>(role);

	if (!home_set && r) {
		home_set = true;
		r->set_home(pos);
	}

	Map_Entity::set_position(pos);
}

void Character_Map_Entity::character_logic()
{
	if (role_paused) {
		Map_Entity::logic();
		return;
	}

	if (input_disabled) {
		input[X] = input[Y] = 0;
		update_direction(false);
		return;
	}

	if (do_kamikaze) {
		do_kamikaze = false;
		Character_Role *old_role = role;
		AStar_Character_Role *r = new AStar_Character_Role(this);
		r->set_kamikaze(true);
		set_role(r);
		delete old_role;
		solid_with_area = solid_with_entities = false;
		bool has_run = anim_set->check_sub_animation_exists("run");
		r->set_destination(kamikaze_point, has_run);
	}

	if (role) {
		role->update(area);
	}

	Map_Entity::logic();

	if (role && !colliding_with_ladder) {
		Map_Entity::auto_set_direction();
		Map_Entity::update_direction(true);
	}
}

void Character_Map_Entity::character_draw()
{
	if (role) {
		role->draw(area);
	}

	Map_Entity::draw();
}

Character_Map_Entity::Character_Map_Entity(std::string name) :
	Map_Entity(name),
	role(NULL),
	home_set(false),
	do_kamikaze(false)
{
	show_shadow = true;
}

Character_Map_Entity::~Character_Map_Entity()
{
	delete role;
}

void Character_Map_Entity::kamikaze(float x, float y)
{
	do_kamikaze = true;
	kamikaze_point = General::Point<float>(x, y);
}
