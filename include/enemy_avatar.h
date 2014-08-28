#ifndef ENEMY_AVATAR_H
#define ENEMY_AVATAR_H

#include "npc.h"

class Enemy_Avatar : public NPC {
public:
	std::vector<std::string> get_enemies();
	std::string get_level();
	std::string get_script();
	bool is_boss_battle();

	Enemy_Avatar(
		std::string level,
		std::string script,
		bool boss_battle,
		std::string name,
		std::vector<std::string> &enemies
	);
	virtual ~Enemy_Avatar();

private:
	std::vector<std::string> enemies;
	std::string level;
	std::string script;
	bool boss_battle;
};

#endif // ENEMY_AVATAR_H
