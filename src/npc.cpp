#include "crystalpicnic.h"
#include "npc.h"

void NPC::set_position(General::Point<float> pos)
{
	character_set_position(pos);
}

void NPC::logic()
{
	character_logic();
}

void NPC::draw()
{
	character_draw();
}

NPC::NPC(std::string name) :
	Character_Map_Entity(name)
{
	show_shadow = true;
}

