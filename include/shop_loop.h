#ifndef SHOP_LOOP_H
#define SHOP_LOOP_H

#include <allegro5/allegro.h>

#include <wrap.h>

#include "loop.h"
#include "player.h"
#include "widgets.h"
#include "game_specific_globals.h"

class Shop_Loop : public Loop {
public:
	struct Notification_Data {
		bool buying;
		Shop_Loop *loop;
	};

	// Loop interface
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();

	void set_do_buy(bool do_buy);
	void set_do_sell(bool do_sell);

	void add_item(std::string name, int price);
	void add_equipment(
		Equipment::Equipment_Type type,
		std::string name,
		int stat,
		Equipment::Element element,
		std::string usable_by,
		int price
	);
	void end_inventory();

	Shop_Loop(bool is_item_shop);
	virtual ~Shop_Loop();

protected:
	void set_current_list(W_Scrolling_List *list);
	void create_sell_list();

	bool is_item_shop;

	Wrap::Bitmap *character_bmp;
	Wrap::Bitmap *flea_bmp;

	int item_button_x;
	int item_button_y;
	int item_button_w;
	int item_button_h;

	W_Button *buy_button;
	W_Button *sell_button;
	W_Button *return_button;
	W_Button *item_icon_button;

	W_Scrolling_List *sell_list;
	W_Scrolling_List *buy_list;

	W_Scrolling_List *current_list;

	W_Vertical_Scrollbar *scrollbar;

	bool buying;

	std::vector<Equipment::Weapon> buy_weapons;
	std::vector<Equipment::Armor> buy_armor;
	std::vector<Equipment::Accessory> buy_accessories;
	std::vector<Game_Specific_Globals::Item> buy_items;
		
	std::vector<std::string> hold_buy_equipment_names;
	std::vector<std::string> hold_buy_equipment_icon_filenames;
	std::vector<Equipment::Equipment_Type> hold_buy_equipment_type;
		
	std::vector<std::string> hold_buy_item_names;
	std::vector<std::string> hold_buy_item_icon_filenames;

	int listX, listY, listW, listH;

	std::vector<std::string> buy_image_names;
	std::vector<std::string> sell_image_names;

	std::vector<std::string> buy_prices;

	bool faded_in;

	Notification_Data notification_data;

	bool do_buy;
	bool do_sell;

	bool esc_pressed;
	bool list_was_activated;
};

#endif
