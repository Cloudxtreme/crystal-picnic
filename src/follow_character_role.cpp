#include "crystalpicnic.h"
#include "character_map_entity.h"
#include "character_role.h"
#include "follow_character_role.h"
#include "area_loop.h"

void Follow_Character_Role::set_actors(Character_Map_Entity *character, Map_Entity *target)
{
	entity = character;
	this->target = target;
}

void Follow_Character_Role::draw(Area_Manager *area)
{
}

void Follow_Character_Role::reset()
{
	tracking.clear();
}

void Follow_Character_Role::update(Area_Manager *area)
{
	General::Point<float> target_pos = target->get_position();
	std::pair< General::Point<float>, General::Point<float> > pair;
	float *target_input = target->get_inputs();
	General::Point<float> p;
	p.x = target_input[Map_Entity::X];
	p.y = target_input[Map_Entity::Y];

	bool add = false;
	if (tracking.size() > 0) {
		pair = tracking[tracking.size()-1];
		General::Point<float> p2 = pair.second;
		if (!(p2 == target_pos)) {
			add = true;
		}
	}
	else {
		add = true;
	}

	if (add) {
		pair.first = p;
		pair.second = target_pos;
		tracking.push_back(pair);
	}

	float *input = entity->get_inputs();

	if (tracking.size() > 0) {
		pair = tracking[0];
		if (tracking.size() > 20) {
			p = pair.first;
			input[Map_Entity::X] = p.x;
			input[Map_Entity::Y] = p.y;
			tracking.erase(tracking.begin());
			entity->set_position(pair.second);
		}
		else if (input[Map_Entity::X] != 0 || input[Map_Entity::Y] != 0)
		{
			input[Map_Entity::X] = 0;
			input[Map_Entity::Y] = 0;
			entity->set_position(pair.second);
		}
	}
}

Follow_Character_Role::Follow_Character_Role(Character_Map_Entity *character, Map_Entity *target) :
	Character_Role(character),
	target(target)
{
}

Follow_Character_Role::~Follow_Character_Role(void)
{
}

