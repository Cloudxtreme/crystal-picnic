#include "entity.h"

static int curr_id = 1;

void Entity::set_delete_me(bool d)
{
	delete_me = d;
}

bool Entity::get_delete_me(void)
{
	return delete_me;
}

Entity::Entity(void) :
	delete_me(false)
{
	this->id = curr_id++;
}

