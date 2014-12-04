#include "game_specific_globals.h"
#include "engine.h"

#include <sstream>

// FIXME:
#include <cstdlib>

static std::vector<Game_Specific_Globals::Item> items;
static std::vector<Equipment::Weapon> weapons;
static std::vector<Equipment::Armor> armor;
static std::vector<Equipment::Accessory> accessories;

namespace Game_Specific_Globals
{

double elapsed_time;
int cash;
int crystals;
bool can_use_crystals = false;

void init()
{
	items.clear();
	weapons.clear();
	armor.clear();
	accessories.clear();
	cash = 100;
	crystals = 0;
	can_use_crystals = false;
}

std::vector<Item> &get_items()
{
	return items;
}

void get_accessory_effects(std::string accessory_name, int *max_hp, int *max_mp, int *attack, int *defense)
{
	if (accessory_name == "HOCKEYMASK") {
		if (defense) {
			*defense += 2;
		}
	}
	else if (accessory_name == "GLOVES") {
		if (attack) {
			*attack += 2;
		}
	}
	else if (accessory_name == "BOOTS") {
		if (max_hp) {
			*max_hp *= 1.1f;
		}
	}
	else if (accessory_name == "STEELTOEBOOTS") {
		if (max_hp) {
			*max_hp *= 1.2f;
		}
	}
	else if (accessory_name == "SOMBRERO") {
		if (max_mp) {
			*max_mp *= 1.2f;
		}
	}
}

std::vector<Equipment::Weapon> &get_weapons()
{
	return weapons;
}

std::vector<Equipment::Armor> &get_armor()
{
	return armor;
}

std::vector<Equipment::Accessory> &get_accessories()
{
	return accessories;
}

std::string get_item_image_name(std::string name)
{
	return "misc_graphics/interface/item_icons/" + General::tolower(name) + ".cpi";
}

std::string get_item_description(std::string name)
{
	return t((name + "_DESC").c_str());
}

int get_item_sell_price(std::string name)
{
	if (name == "ANTIDOTE") {
		return 5;
	}
	if (name == "HEALTHVIAL") {
		return 1;
	}
	if (name == "HEALTHJAR") {
		return 5;
	}
	if (name == "HEALTHFLASK") {
		return 25;
	}
	if (name == "MAGICVIAL") {
		return 5;
	}
	if (name == "MAGICFLASK") {
		return 50;
	}
	if (name == "DIRTYSOCK") {
		return 75;
	}
	if (name == "BAT") {
		return 15;
	}
	if (name == "SHOVEL") {
		return 15;
	}
	if (name == "IRONARROW") {
		return 1;
	}
	if (name == "RAKE") {
		return 30;
	}
	if (name == "SLEDGEHAMMER") {
		return 50;
	}
	if (name == "CLEAVER") {
		return 30;
	}
	if (name == "WOODENSWORD") {
		return 50;
	}
	if (name == "STICK") {
		return 1;
	}
	if (name == "BOX") {
		return 10;
	}
	if (name == "SHIRT") {
		return 10;
	}
	if (name == "BANDSHIRT") {
		return 50;
	}
	if (name == "LEATHERARMOR") {
		return 50;
	}
	if (name == "COVERALLS") {
		return 20;
	}
	if (name == "WOLFHIDE") {
		return 40;
	}
	if (name == "HOCKEYMASK") {
		return 20;
	}
	if (name == "GLOVES") {
		return 50;
	}
	if (name == "BOOTS") {
		return 35;
	}
	if (name == "STEELTOEBOOTS") {
		return 60;
	}
	if (name == "SOMBRERO") {
		return 40;
	}
	if (name == "REDRING") {
		return 40;
	}
	if (name == "GREENRING") {
		return 100;
	}
	return 1;
}

Equipment::Equipment_Type get_equipment_type(std::string name)
{
	if (name == "STICK") {
		return Equipment::WEAPON;
	}
	if (name == "SHOVEL") {
		return Equipment::WEAPON;
	}
	if (name == "BOW") {
		return Equipment::WEAPON;
	}
	if (name == "IRONARROW") {
		return Equipment::WEAPON;
	}
	if (name == "RAKE") {
		return Equipment::WEAPON;
	}
	if (name == "SLEDGEHAMMER") {
		return Equipment::WEAPON;
	}
	if (name == "CLEAVER") {
		return Equipment::WEAPON;
	}
	if (name == "WOODENSWORD") {
		return Equipment::WEAPON;
	}
	if (name == "BAT") {
		return Equipment::WEAPON;
	}
	if (name == "BOX") {
		return Equipment::ARMOR;
	}
	if (name == "SHIRT") {
		return Equipment::ARMOR;
	}
	if (name == "BANDSHIRT") {
		return Equipment::ARMOR;
	}
	if (name == "LEATHERARMOR") {
		return Equipment::ARMOR;
	}
	if (name == "COVERALLS") {
		return Equipment::ARMOR;
	}
	if (name == "WOLFHIDE") {
		return Equipment::ARMOR;
	}
	return (Equipment::Equipment_Type)-1;
}

Equipment::Weapon get_weapon_instance(std::string name, int quantity)
{
	Equipment::Weapon weapon;
	weapon.name = name;
	weapon.quantity = quantity;

	if (name == "STICK") {
		weapon.attack = 1;
		weapon.element = Equipment::ELEMENT_NONE;
		weapon.usable_by = "egbert";
	}
	else if (name == "BAT") {
		weapon.attack = 2;
		weapon.element = Equipment::ELEMENT_NONE;
		weapon.usable_by = "egbert";
	}
	else if (name == "CLEAVER") {
		weapon.attack = 3;
		weapon.element = Equipment::ELEMENT_NONE;
		weapon.usable_by = "egbert";
	}
	else if (name == "WOODENSWORD") {
		weapon.attack = 4;
		weapon.element = Equipment::ELEMENT_NONE;
		weapon.usable_by = "egbert";
	}
	else if (name == "SHOVEL") {
		weapon.attack = 2;
		weapon.element = Equipment::ELEMENT_NONE;
		weapon.usable_by = "frogbert";
	}
	else if (name == "RAKE") {
		weapon.attack = 3;
		weapon.element = Equipment::ELEMENT_NONE;
		weapon.usable_by = "frogbert";
	}
	else if (name == "SLEDGEHAMMER") {
		weapon.attack = 4;
		weapon.element = Equipment::ELEMENT_NONE;
		weapon.usable_by = "frogbert";
	}
	else if (name == "BOW") {
		weapon.attack = 1;
		weapon.element = Equipment::ELEMENT_NONE;
		weapon.usable_by = "bisou";
	}
	else if (name == "IRONARROW") {
		weapon.attack = 2;
		weapon.element = Equipment::ELEMENT_NONE;
		weapon.usable_by = "bisou";
	}

	return weapon;
}

Equipment::Armor get_armor_instance(std::string name)
{
	Equipment::Armor armor;
	armor.name = name;

	if (name == "BOX") {
		armor.defense = 1;
		armor.element = Equipment::ELEMENT_NONE;
	}
	else if (name == "SHIRT") {
		armor.defense = 1;
		armor.element = Equipment::ELEMENT_NONE;
	}
	else if (name == "BANDSHIRT") {
		armor.defense = 3;
		armor.element = Equipment::ELEMENT_NONE;
	}
	else if (name == "LEATHERARMOR") {
		armor.defense = 3;
		armor.element = Equipment::ELEMENT_NONE;
	}
	else if (name == "COVERALLS") {
		armor.defense = 2;
		armor.element = Equipment::ELEMENT_NONE;
	}
	else if (name == "WOLFHIDE") {
		armor.defense = 3;
		armor.element = Equipment::ELEMENT_NONE;
	}

	return armor;
}

Equipment::Accessory get_accessory_instance(std::string name)
{
	Equipment::Accessory accessory;
	accessory.name = name;
	return accessory;
}

Item get_item_instance(std::string name)
{
	Item item;
	item.name = name;
	item.image_filename = get_item_image_name(name);
	item.quantity = 1;
	return item;
}

void give_equipment(Equipment::Equipment_Type type, std::string name, int quantity)
{
	switch (type) {
		case Equipment::WEAPON: {
			if (weapon_is_attachment(name)) {
				Equipment::Weapon weapon = get_weapon_instance(name, quantity);
				weapons.push_back(weapon);
			}
			else {
				Equipment::Weapon weapon = get_weapon_instance(name, 1);
				for (int i = 0; i < quantity; i++) {
					weapons.push_back(weapon);
				}
			}
			break;
		}
		case Equipment::ARMOR: {
			Equipment::Armor a = get_armor_instance(name);
			for (int i = 0; i < quantity; i++) {
				armor.push_back(a);
			}
			break;
		}
		case Equipment::ACCESSORY: {
			Equipment::Accessory accessory = get_accessory_instance(name);
			for (int i = 0; i < quantity; i++) {
				accessories.push_back(accessory);
			}
			break;
		}
	}
}

void give_items(std::string name, int quantity)
{
	if (name == "TINCAN" || name == "BONE") {
		return;
	}

	while (quantity > 0) {
		bool found = false;
		int found_i = -1;
		for (size_t i = 0; i < items.size(); i++) {
			if (items[i].name == name) {
				found = true;
				found_i = i;
				break;
			}
		}
		if (found) {
			Item item = get_item_instance(name);
			int q = MIN(99-items[found_i].quantity, quantity);
			quantity -= q;
			items[found_i].quantity += q;
		}
		else {
			Item item = get_item_instance(name);
			int q = MIN(99, quantity);
			quantity -= q;
			item.quantity = q;
			items.push_back(item);
		}
	}
}

std::string get_weapon_swing_sfx(std::string weapon_name)
{
	if (weapon_name == "SHOVEL" || weapon_name == "RAKE" || weapon_name == "SLEDGEHAMMER") {
		return "sfx/swing_heavy_metal.ogg";
	}
	else if (weapon_name == "BOW") {
		return "sfx/bow_and_arrow.ogg";
	}
	return "sfx/swing_weapon.ogg";
}

std::string get_weapon_hit_sfx(std::string weapon_name)
{
	if (weapon_name == "SHOVEL" || weapon_name == "RAKE" || weapon_name == "SLEDGEHAMMER") {
		return "sfx/hit_heavy_metal.ogg";
	}
	return "sfx/hit.ogg";
}

#define INC_MAX_IF(val, max, num) \
	if (val > 0) val = MIN(max, val + num);
#define INC_MAX(val, max, num) \
	val = MIN(max, val + num);

void use_item(std::string name, Battle_Attributes &attributes, bool sound)
{
	int max_hp = attributes.max_hp;
	int max_mp = attributes.max_mp;
	get_accessory_effects(attributes.equipment.accessory.name, &max_hp, &max_mp, NULL, NULL);

	if (name == "HEALTHVIAL") {
		INC_MAX_IF(attributes.hp, max_hp, 10 * cfg.difficulty_mult())
	}
	else if (name == "HEALTHJAR") {
		INC_MAX_IF(attributes.hp, max_hp, 20 * cfg.difficulty_mult())
	}
	else if (name == "HEALTHFLASK") {
		INC_MAX_IF(attributes.hp, max_hp, 50 * cfg.difficulty_mult())
	}
	else if (name == "MAGICVIAL") {
		INC_MAX(attributes.mp, max_mp, 10)
	}
	else if (name == "MAGICFLASK") {
		INC_MAX(attributes.mp, max_mp, 50)
	}
	else if (name == "DIRTYSOCK") {
		if (attributes.hp <= 0) {
			attributes.hp = max_hp / 2;
		}
		else {
			INC_MAX(attributes.hp, max_hp, max_hp/2)
		}
	}
	else if (name == "ANTIDOTE") {
		attributes.status.name = "";
	}
	else {
		return;
	}

	if (sound) {
		engine->play_sample("sfx/healing_item.ogg", 1.0f, 0.0f, 1.0f);
	}
}

float regenerate_magic(Battle_Attributes &attributes, bool in_battle, float count)
{
	// FIXME: allow for accessories to change these
	int max_mp = attributes.max_mp;
	get_accessory_effects(attributes.equipment.accessory.name, NULL, &max_mp, NULL, NULL);
	float inc;
	if (in_battle) {
		inc = 1.0f/60.0f/60.0f * max_mp;
	}
	else {
		inc = 1.0f/60.0f/120.0f * max_mp; // slower in field view
	}
	count += inc;
	const float cost = 1.0f;
	int points = count / cost;
	INC_MAX(attributes.mp, max_mp, points);
	return inc;
}

int magic_cost(Battle_Attributes &attributes, std::string ability_name)
{
	// FIXME:
	return 1;
}

bool take_magic(Battle_Attributes &attributes, std::string ability_name)
{
	int cost = magic_cost(attributes, ability_name);
	if (cost > attributes.mp) {
		return false;
	}
	attributes.mp -= cost;
	return true;
}

void apply_status(Battle_Attributes &attributes)
{
	int max_hp = attributes.max_hp;
	int max_mp = attributes.max_mp;
	get_accessory_effects(attributes.equipment.accessory.name, &max_hp, &max_mp, NULL, NULL);

	if (attributes.status.name == "POISON") {
		if (attributes.hp > 0) {
			attributes.status.count++;
			if (attributes.status.count % (General::LOGIC_RATE*10) == 0) {
				engine->play_sample("sfx/poison_again.ogg");
				attributes.hp -= (max_hp * 0.1f);
				if (attributes.hp <= 0) {
					attributes.hp = 1;
				}
			}
			if (attributes.status.count >= (General::LOGIC_RATE*60)) {
				attributes.status.name = "";
			}
		}
	}
}

bool weapon_is_attachment(std::string name)
{
	if (name == "IRONARROW") {
		return true;
	}
	return false;
}

bool weapon_attaches_to(std::string attachment, std::string weapon)
{
	if (weapon == "BOW") {
		if (attachment == "IRONARROW") {
			return true;
		}
	}
	return false;
}

} // end namespace Game_Specific_Globals

