#include "enemy_avatar.h"
#include "character_role.h"
#include "enemy_avatar_wander_character_role.h"

std::vector<std::string> Enemy_Avatar::get_enemies()
{
	return enemies;
}
	
std::string Enemy_Avatar::get_level()
{
	return level;
}

std::string Enemy_Avatar::get_script()
{
	return script;
}

bool Enemy_Avatar::is_boss_battle()
{
	return boss_battle;
}

Enemy_Avatar::Enemy_Avatar(std::string level, std::string script, bool boss_battle,
	std::string name, std::vector<std::string> &enemies)
:
	NPC(name),
	enemies(enemies),
	level(level),
	script(script),
	boss_battle(boss_battle)
{
	set_role(new Enemy_Avatar_Wander_Character_Role(this, 400, 0.25, 1.0));
}

Enemy_Avatar::~Enemy_Avatar()
{
}
