#ifndef CRYSTAL_LOOP_H
#define CRYSTAL_LOOP_H

#include <allegro5/allegro.h>

#include <wrap.h>

#include "loop.h"
#include "widgets.h"
#include "abilities.h"
#include "player.h"
#include "resource_manager.h"

class Crystal_Button : public W_Button {
public:
	bool available() {
		int tier;
		Abilities::get_tier(*abilities, &tier, NULL, NULL);
		if (number <= tier) {
			return true;
		}
		return false;
	}

	bool filled() {
		if (id == "hp") {
			if (abilities->hp > number) {
				return true;
			}
		}
		else if (id == "mp") {
			if (abilities->mp > number) {
				return true;
			}
		}
		else {
			if (abilities->abilities[atoi(id.c_str())] > number) {
				return true;
			}
		}
		return false;
	}

	std::string get_description(std::string player_name) {
		if (id == "hp") {
			return t("PLUS10_MAXHP");
		}
		else if (id == "mp") {
			return t("PLUS5_MAXMP");
		}
		else {
			if (player_name == "egbert") {
				if (id == "0") {
					return t("SLASH");
				}
				else if (id == "1") {
					return t("THROW");
				}
				else {
					return t("ICE");
				}
			}
			else if (player_name == "frogbert") {
				if (id == "0") {
					return t("KICK");
				}
				else if (id == "1") {
					return t("PLANT");
				}
				else {
					return t("FIRE");
				}
			}
			else { // bisou
				if (id == "0") {
					return t("ROLL");
				}
				else if (id == "1") {
					return t("BURROW");
				}
				else {
					return t("HEAL");
				}
			}
		}
	}

	bool acceptsFocus() {
		return available();
	}

	virtual void draw(int abs_x, int abs_y) {
		al_draw_bitmap(slot_bmp->bitmap, abs_x, abs_y, 0);
		if (available()) {
			al_draw_bitmap(available_bmp->bitmap, abs_x, abs_y, 0);
		}
		if (filled()) {
			al_draw_bitmap(crystal_bmp->bitmap, abs_x, abs_y, 0);
		}
	}

	Crystal_Button(int x, int y, std::string id, int number, Abilities::Abilities *abilities) :
		W_Button(x, y, 18, 18)
	{
		this->id = id;
		this->number = number;
		this->abilities = abilities;

		crystal_bmp = resource_manager->reference_bitmap("misc_graphics/interface/crystal_menu/crystal.cpi");
		slot_bmp = resource_manager->reference_bitmap("misc_graphics/interface/crystal_menu/slot.cpi");
		available_bmp = resource_manager->reference_bitmap("misc_graphics/interface/crystal_menu/available.cpi");
	}

	virtual ~Crystal_Button() {
		resource_manager->release_bitmap("misc_graphics/interface/crystal_menu/crystal.cpi");
		resource_manager->release_bitmap("misc_graphics/interface/crystal_menu/slot.cpi");
		resource_manager->release_bitmap("misc_graphics/interface/crystal_menu/available.cpi");
	}

private:
	std::string id;
	int number;
	Abilities::Abilities *abilities;

	Wrap::Bitmap *crystal_bmp;
	Wrap::Bitmap *slot_bmp;
	Wrap::Bitmap *available_bmp;
};

class Crystal_Loop : public Loop {
public:
	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();

	Crystal_Loop(std::vector<Player *> players, int start_player);
	virtual ~Crystal_Loop();

private:
	void copy_abilities();
	void next_player();

	Crystal_Button *ability_buttons[3][3];
	Crystal_Button *hp_buttons[8];
	Crystal_Button *mp_buttons[8];

	W_Button *player_button;

	Abilities::Abilities abilities;

	std::vector<Player *> players;
	std::vector<Wrap::Bitmap *> player_bmps;

	W_Button *return_button;

	int current_player;
};

#endif // CRYSTAL_LOOP_H
