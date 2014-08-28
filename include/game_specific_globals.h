#ifndef GAME_SPECIFIC_GLOBALS_H
#define GAME_SPECIFIC_GLOBALS_H

#include "battle_entity.h"

#include <vector>
#include <string>

namespace Game_Specific_Globals {

struct Item {
	std::string name;
	std::string image_filename;
	int quantity;
};

extern double elapsed_time;
extern int cash;
extern int crystals;

void init();
std::vector<Item> &get_items();
std::vector<Equipment::Weapon> &get_weapons();
std::vector<Equipment::Armor> &get_armor();
std::vector<Equipment::Accessory> &get_accessories();
void get_accessory_effects(std::string accessory_name, int *max_hp, int *max_mp, int *attack, int *defense);
std::string get_item_image_name(std::string name);
std::string get_item_description(std::string name);
int get_item_sell_price(std::string name);
Equipment::Equipment_Type get_equipment_type(std::string name);
Equipment::Weapon get_weapon_instance(std::string name, int quantity);
Equipment::Armor get_armor_instance(std::string name);
Equipment::Accessory get_accessory_instance(std::string name);
Item get_item_instance(std::string name);
void give_equipment(Equipment::Equipment_Type type, std::string name, int quantity);
void give_items(std::string name, int quantity);
std::string get_weapon_swing_sfx(std::string weapon_name);
std::string get_weapon_hit_sfx(std::string weapon_name);
void use_item(std::string name, Battle_Attributes &attributes, bool sound = false);
float regenerate_magic(Battle_Attributes &attributes, bool battle, float count);
int magic_cost(Battle_Attributes &attributes, std::string ability_name);
bool take_magic(Battle_Attributes &attributes, std::string ability_name); // returns false if not enough magic
void apply_status(Battle_Attributes &attributes);
bool weapon_is_attachment(std::string name);
bool weapon_attaches_to(std::string attachment, std::string weapon);

} // end Game_Specific_Globals namespace

#endif // GAME_SPECIFIC_GLOBALS_H
