#ifndef EQUIPMENT_H
#define EQUIPMENT_H

namespace Equipment
{

enum Element
{
	ELEMENT_NONE = 0,
	ELEMENT_FIRE,
	ELEMENT_ICE,
	ELEMENT_LIGHTNING
};

// Only used to determine icon in lists
enum Equipment_Type
{
	WEAPON = 1,
	ARMOR,
	ACCESSORY
};

struct Weapon
{
	std::string name;
	int attack;
	Element element;
	std::string usable_by;
	int quantity;
	std::vector<struct Weapon> attachments;
};

struct Armor
{
	std::string name;
	int defense;
	Element element;
};

struct Accessory
{
	std::string name;
};

struct Equipment {
	Weapon weapon;
	Armor armor;
	Accessory accessory;
};

} // end namespace

#endif
