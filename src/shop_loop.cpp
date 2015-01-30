#include "shop_loop.h"
#include "speech_types.h"
#include "game_specific_globals.h"
#include "resource_manager.h"

static void list_notifier(void *data, float row_y_pixel)
{
	Shop_Loop::Notification_Data *d = (Shop_Loop::Notification_Data *)data;
	bool buying = d->buying;
	Shop_Loop *loop = d->loop;

	if (buying) {
		loop->set_do_buy(true);
	}
	else {
		loop->set_do_sell(true);
	}
}

bool Shop_Loop::init()
{
	engine->clear_touches();

	if (inited) {
		return true;
	}
	Loop::init();

	engine->load_sample("sfx/kaching.ogg");

	return true;
}

void Shop_Loop::top()
{
}

bool Shop_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (!list_was_activated) {
		if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
			if (
				event->keyboard.keycode == ALLEGRO_KEY_ESCAPE
#if defined ALLEGRO_ANDROID
				|| event->keyboard.keycode == ALLEGRO_KEY_BUTTON_B
				|| event->keyboard.keycode == ALLEGRO_KEY_BACK
#endif
			) {
				esc_pressed = true;
				return true;
			}
		}
		else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
			if (event->joystick.button == cfg.joy_ability[2]) {
				esc_pressed = true;
				return true;
			}
		}
	}
	return false;
}

bool Shop_Loop::logic()
{
	engine->set_touch_input_type(TOUCHINPUT_GUI);

	if (!faded_in) {
		faded_in = true;
		engine->fade_in();
	}

	if (current_list->is_activated()) {
		list_was_activated = true;
	}
	else {
		list_was_activated = false;
	}

	tgui::TGUIWidget *w = tgui::update();

	if (w == buy_button && !buying) {
		buying = true;

		sell_button->set_text_color(al_color_name("lightgrey"));
		buy_button->set_text_color(al_color_name("yellow"));
		set_current_list(buy_list);

		for (int i = 0; i < sell_list->get_num_items(); i++) {
			resource_manager->release_bitmap(Game_Specific_Globals::get_item_image_name(sell_list->get_item_name(i)));
		}

		delete sell_list;
		sell_list = NULL;
	}
	else if (w == sell_button && buying) {
		buying = false;

		create_sell_list();

		buy_button->set_text_color(al_color_name("lightgrey"));
		sell_button->set_text_color(al_color_name("yellow"));
		set_current_list(sell_list);
	}
	else if (w == item_icon_button && current_list->get_num_items() > 0) {
		if (buying) {
			do_buy = true;
		}
		else {
			do_sell = true;
		}
	}
	else if (w == return_button || esc_pressed) {
		engine->fade_out();
		engine->set_loops(std::vector<Loop *>(), true);
		return false;
	}

	if (do_buy || do_sell) {
		int sel = current_list->get_selected();
		std::vector<Game_Specific_Globals::Item> &items = Game_Specific_Globals::get_items();
		std::vector<Equipment::Weapon> &weapons = Game_Specific_Globals::get_weapons();
		std::vector<Equipment::Armor> &armor = Game_Specific_Globals::get_armor();
		std::vector<Equipment::Accessory> &accessories = Game_Specific_Globals::get_accessories();

		if (do_buy) {
			std::string name = current_list->get_item_name(sel);

			int max;
			int existing_index = -1;
			int cost_each = atoi(buy_prices[sel].c_str());

			if (is_item_shop) {
				max = 99;
				for (size_t i = 0; i < items.size(); i++) {
					if (items[i].name == name) {
						max = 99 - items[i].quantity;
						existing_index = i;
						break;
					}
				}
			}
			else {
				max = 3;
			}

			max = MIN(max, Game_Specific_Globals::cash / cost_each);

			std::vector<std::string> v;
			v.push_back(t("BUY_HOW_MANY"));
			int n = engine->get_number(v, max == 0 ? 0 : 1, max, max == 0 ? 0 : 1);

			if (n > 0) {
				int total = cost_each * n;
				if (total > Game_Specific_Globals::cash) {
					engine->play_sample("sfx/error.ogg");
					std::vector<std::string> v;
					v.push_back(t("NOT_ENOUGH_CASH"));
					engine->notify(v);
				}
				else {
					Game_Specific_Globals::cash -= total;
					engine->play_sample("sfx/kaching.ogg", 1.0f, 0.0f, 1.0f);
					if (is_item_shop) {
						if (existing_index != -1) {
							items[existing_index].quantity += n;
						}
						else {
							items.push_back(buy_items[sel]);
							items[items.size()-1].quantity = n;
						}
					}
					else {
						for (int i = 0; i < n; i++) {
							switch (hold_buy_equipment_type[sel]) {
								case Equipment::WEAPON:
									weapons.push_back(buy_weapons[sel]);
									break;
								case Equipment::ARMOR:
									armor.push_back(buy_armor[sel]);
									break;
								case Equipment::ACCESSORY:
									accessories.push_back(buy_accessories[sel]);
									break;
							}
						}
					}
				}
			}
		}
		else if (do_sell) {
			int sel = current_list->get_selected();
			int max = -1;
			int n;
			int gain_each;

			if (is_item_shop) {
				max = items[sel].quantity;
				std::vector<std::string> v;
				v.push_back(t("SELL_HOW_MANY"));
				n = engine->get_number(v, 1, max, 1);
				gain_each = Game_Specific_Globals::get_item_sell_price(
					items[sel].name
				);
			}
			else {
				if (Game_Specific_Globals::weapon_is_attachment(current_list->get_item_name(sel))) {
					max = weapons[sel].quantity;
					std::vector<std::string> v;
					v.push_back(t("SELL_HOW_MANY"));
					n = engine->get_number(v, 1, max, 1);
				}
				else {
					n = 1;
				}
				gain_each = Game_Specific_Globals::get_item_sell_price(current_list->get_item_name(sel));
			}

			if (n > 0) {
				int total = gain_each * n;

				char buf[1000];
				snprintf(buf, 1000, t("SELL_X_FOR_X_CASH"), n, total);

				std::vector<std::string> v;
				v.push_back(buf);

				bool yesno = engine->yes_no_prompt(v);

				if (yesno) {
					resource_manager->release_bitmap(
						Game_Specific_Globals::get_item_image_name(items[sel].name)
					);
					if (is_item_shop) {
						items[sel].quantity -= n;
						if (items[sel].quantity <= 0) {
							items.erase(items.begin()+sel);
						}
					}
					else {
						int i = sel;
						if (i < (int)weapons.size()) {
							weapons[i].quantity -= n;
							if (weapons[i].quantity <= 0) {
								weapons.erase(weapons.begin()+i);
							}
						}
						else if (i < (int)(weapons.size()+armor.size())) {
							i -= weapons.size();
							armor.erase(armor.begin()+i);
						}
						else {
							i -= weapons.size();
							i -= armor.size();
							accessories.erase(accessories.begin()+i);
						}
					}

					Game_Specific_Globals::cash += total;
					engine->play_sample("sfx/kaching.ogg", 1.0f, 0.0f, 1.0f);

					current_list->remove();
					delete sell_list;
					create_sell_list();
					current_list = NULL;
					set_current_list(sell_list);
					tgui::setFocus(sell_list);
				}
			}
		}
	}

	do_buy = do_sell = false;

	return false;
}

void Shop_Loop::draw()
{
	if (!faded_in) {
		return;
	}

	int dx = cfg.screen_w - General::RENDER_W;
	int dy = cfg.screen_h - General::RENDER_H;

	ALLEGRO_COLOR dark = General::UI_GREEN;
	dark = Graphics::change_brightness(dark, 0.65f);

	al_clear_to_color(dark);

	// flea window
	General::draw_speech_window(
		SPEECH_NORMAL,
		2,
		95+dy,
		181+dx,
		60,
		false,
		al_color_name("white"),
		1
	);

	// list window
	General::draw_speech_window(
		SPEECH_NORMAL,
		60,
		2,
		178+dx,
		92+dy,
		false,
		al_color_name("white"),
		1
	);


	al_draw_bitmap(character_bmp->bitmap, 8, 69+dy, 0);

	al_draw_bitmap(flea_bmp->bitmap, 18-al_get_bitmap_width(flea_bmp->bitmap)/2, 108+dy, 0);

	al_draw_filled_rectangle(item_button_x, item_button_y, item_button_x+item_button_w, item_button_y+item_button_h, dark);

	// Draw item icons and descriptions
	if (current_list->get_num_items() > 0) {
		int sel = current_list->get_selected();
		std::string name = current_list->get_item_name(sel);
		std::string image_name = Game_Specific_Globals::get_item_image_name(name);
		Wrap::Bitmap *b = resource_manager->reference_bitmap(image_name);
		al_draw_bitmap(
			b->bitmap,
			item_button_x+item_button_w/2-al_get_bitmap_width(b->bitmap)/2,
			item_button_y+item_button_h/2-al_get_bitmap_height(b->bitmap)/2,
			0
		);
		resource_manager->release_bitmap(image_name);
		General::draw_wrapped_text(
			Game_Specific_Globals::get_item_description(name),
			al_color_name("white"),
			32,
			107+dy,
			135+dx,
			General::FONT_LIGHT
		);
	}

	// Draw cash
	int cx = dx + 184 + return_button->getWidth()/2;
	int cy = dy + 96 + return_button->getHeight() + 3;

	General::draw_text(t("CASH_LABEL"), al_color_name("cyan"), cx, cy, ALLEGRO_ALIGN_CENTER);
	General::draw_text(
		General::itos(Game_Specific_Globals::cash),
		al_color_name("cyan"),
		cx,
		cy+General::get_font_line_height(General::FONT_LIGHT),
		ALLEGRO_ALIGN_CENTER
	);

	tgui::draw();
}

void Shop_Loop::set_current_list(W_Scrolling_List *list)
{
	if (current_list) {
		current_list->remove();
	}

	notification_data.buying = buying;
	notification_data.loop = this;
	list->setNotifier(list_notifier, &notification_data);

	current_list = list;
	current_list->set_value(0.0f);
	current_list->set_selected(0);

	scrollbar->set_tab_size(current_list->get_scrollbar_tab_size());
	scrollbar->setX(current_list->getX()+current_list->getWidth()+1);
	scrollbar->setY(current_list->getY());
	scrollbar->setHeight(current_list->getHeight());
	scrollbar->set_value(0.0f);

	tgui::addWidget(current_list);
}

Shop_Loop::Shop_Loop(bool is_item_shop) :
	is_item_shop(is_item_shop),
	buying(true),
	faded_in(false),
	do_buy(false),
	do_sell(false),
	esc_pressed(false),
	list_was_activated(false)
{
	int dx = cfg.screen_w - General::RENDER_W;
	int dy = cfg.screen_h - General::RENDER_H;

	std::string name = "egbert"; // FIXME
	character_bmp = Wrap::load_bitmap("misc_graphics/interface/" + name + "_normal_icon.png");

	if (is_item_shop) {
		flea_bmp = Wrap::load_bitmap("misc_graphics/interface/item_flea.png");
	}
	else {
		flea_bmp = Wrap::load_bitmap("misc_graphics/interface/equipment_flea.png");
	}

	// create and add widgets
	buy_button = new W_Button("misc_graphics/interface/fat_purple_button.png", t("BUY"));
	buy_button->set_text_color(al_color_name("yellow"));
	buy_button->setX(3);
	buy_button->setY(3);

	sell_button = new W_Button("misc_graphics/interface/fat_purple_button.png", t("SELL"));
	sell_button->setX(3);
	sell_button->setY(5+buy_button->getHeight());

	return_button = new W_Button("misc_graphics/interface/fat_red_button.png", t("RETURN"));
	return_button->setX(184+dx);
	return_button->setY(96+dy);

	listY = 11;
	listH = 75;

	item_button_x = 69;
	item_button_y = listY;
	item_button_w = 55;
	item_button_h = listH;

	listX = item_button_x + item_button_w + 2;
	listW = 98+dx;

	item_icon_button = new W_Button(item_button_x, item_button_y, item_button_w, item_button_h);

	scrollbar = new W_Vertical_Scrollbar(10);

	current_list = NULL;
	sell_list = NULL;
}

Shop_Loop::~Shop_Loop()
{
	engine->destroy_sample("sfx/kaching.ogg");

	if (sell_list) {
		for (int i = 0; i < sell_list->get_num_items(); i++) {
			resource_manager->release_bitmap(Game_Specific_Globals::get_item_image_name(sell_list->get_item_name(i)));
		}
	}
	for (int i = 0; i < buy_list->get_num_items(); i++) {
		resource_manager->release_bitmap(Game_Specific_Globals::get_item_image_name(buy_list->get_item_name(i)));
	}

	Wrap::destroy_bitmap(character_bmp);
	Wrap::destroy_bitmap(flea_bmp);

	if (sell_list) {
		sell_list->remove();
	}
	buy_list->remove();

	buy_button->remove();
	sell_button->remove();
	return_button->remove();
	item_icon_button->remove();
	scrollbar->remove();

	delete buy_button;
	delete sell_button;
	delete return_button;
	delete item_icon_button;
	delete scrollbar;

	delete sell_list;
	delete buy_list;
	
	engine->clear_touches();
}

void Shop_Loop::add_item(std::string name, int price)
{
	resource_manager->reference_bitmap(
		Game_Specific_Globals::get_item_image_name(name)
	);

	buy_prices.push_back(General::itos(price));

	hold_buy_item_names.push_back(name);
	hold_buy_item_icon_filenames.push_back("misc_graphics/interface/items_icon_white.png");

	Game_Specific_Globals::Item item;
	item.name = name;
	item.image_filename = Game_Specific_Globals::get_item_image_name(name);
	item.quantity = 999999;

	buy_items.push_back(item);
}

void Shop_Loop::add_equipment(
	Equipment::Equipment_Type type,
	std::string name,
	int stat,
	Equipment::Element element,
	std::string usable_by,
	int price
	)
{
	resource_manager->reference_bitmap(
		Game_Specific_Globals::get_item_image_name(name)
	);

	buy_prices.push_back(General::itos(price));

	hold_buy_equipment_names.push_back(name);
	hold_buy_equipment_type.push_back(type);

	switch (type) {
		case Equipment::WEAPON: {
			Equipment::Weapon weapon;
			weapon.name = name;
			weapon.attack = stat;
			weapon.element = element;
			weapon.usable_by = usable_by;
			weapon.quantity = 1;
			buy_weapons.push_back(weapon);
			hold_buy_equipment_icon_filenames.push_back("misc_graphics/interface/weapon_icon.png");
			break;
		}
		case Equipment::ARMOR: {
			Equipment::Armor armor;
			armor.name = name;
			armor.defense = stat;
			armor.element = element;
			buy_armor.push_back(armor);
			hold_buy_equipment_icon_filenames.push_back("misc_graphics/interface/armor_icon.png");
			break;
		}
		case Equipment::ACCESSORY: {
			Equipment::Accessory accessory;
			accessory.name = name;
			buy_accessories.push_back(accessory);
			hold_buy_equipment_icon_filenames.push_back("misc_graphics/interface/accessory_icon.png");
			break;
		}
	}
}

void Shop_Loop::end_inventory()
{
	if (is_item_shop) {
		buy_list = new W_Scrolling_List(
			hold_buy_item_names,
			hold_buy_item_icon_filenames,
			buy_prices,
			std::vector<bool>(),
			General::FONT_LIGHT,
			true
		);
	}
	else {
		buy_list = new W_Scrolling_List(
			hold_buy_equipment_names,
			hold_buy_equipment_icon_filenames,
			buy_prices,
			std::vector<bool>(),
			General::FONT_LIGHT,
			true
		);
	}

	buy_list->set_translate_item_names(true);
	buy_list->setX(listX);
	buy_list->setY(listY);
	buy_list->setWidth(listW);
	buy_list->setHeight(listH);

	tgui::setNewWidgetParent(NULL);
	tgui::addWidget(buy_button);
	tgui::addWidget(sell_button);
	tgui::addWidget(return_button);
	tgui::addWidget(item_icon_button);
	tgui::addWidget(scrollbar);
	tgui::setFocus(return_button);

	set_current_list(buy_list);
}

void Shop_Loop::create_sell_list()
{
	if (is_item_shop) {
		std::vector<Game_Specific_Globals::Item> items = Game_Specific_Globals::get_items();

		std::vector<std::string> item_names;
		std::vector<std::string> icon_filenames;
		std::vector<std::string> right_justified_text;

		for (size_t i = 0; i < items.size(); i++) {
			resource_manager->reference_bitmap(
				Game_Specific_Globals::get_item_image_name(items[i].name)
			);
			item_names.push_back(items[i].name);
			icon_filenames.push_back("misc_graphics/interface/items_icon_white.png");
			right_justified_text.push_back(General::itos(items[i].quantity));
		}

		sell_list = new W_Scrolling_List(item_names, icon_filenames, right_justified_text, std::vector<bool>(), General::FONT_LIGHT, true);
		sell_list->set_translate_item_names(true);
		sell_list->setX(listX);
		sell_list->setY(listY);
		sell_list->setWidth(listW);
		sell_list->setHeight(listH);
	}
	else {
		std::vector<Equipment::Weapon> weapons = Game_Specific_Globals::get_weapons();
		std::vector<Equipment::Armor> armor = Game_Specific_Globals::get_armor();
		std::vector<Equipment::Accessory> accessories = Game_Specific_Globals::get_accessories();

		std::vector<std::string> item_names;
		std::vector<std::string> icon_filenames;
		std::vector<Equipment::Equipment_Type> equipment_type;
		std::vector<std::string> right_justified_text;

		for (size_t i = 0; i < weapons.size(); i++) {
			resource_manager->reference_bitmap(
				Game_Specific_Globals::get_item_image_name(weapons[i].name)
			);
			item_names.push_back(weapons[i].name);
			icon_filenames.push_back("misc_graphics/interface/weapon_icon.png");
			equipment_type.push_back(Equipment::WEAPON);
			right_justified_text.push_back(General::itos(weapons[i].quantity));
		}
		for (size_t i = 0; i < armor.size(); i++) {
			resource_manager->reference_bitmap(
				Game_Specific_Globals::get_item_image_name(armor[i].name)
			);
			item_names.push_back(armor[i].name);
			icon_filenames.push_back("misc_graphics/interface/armor_icon.png");
			equipment_type.push_back(Equipment::ARMOR);
			right_justified_text.push_back("1");
		}
		for (size_t i = 0; i < accessories.size(); i++) {
			resource_manager->reference_bitmap(
				Game_Specific_Globals::get_item_image_name(accessories[i].name)
			);
			item_names.push_back(accessories[i].name);
			icon_filenames.push_back("misc_graphics/interface/accessory_icon.png");
			equipment_type.push_back(Equipment::ACCESSORY);
			right_justified_text.push_back("1");
		}

		sell_list = new W_Scrolling_List(item_names, icon_filenames, right_justified_text, std::vector<bool>(), General::FONT_LIGHT, true);
		sell_list->set_translate_item_names(true);
		sell_list->setX(listX);
		sell_list->setY(listY);
		sell_list->setWidth(listW);
		sell_list->setHeight(listH);
	}
}

void Shop_Loop::set_do_buy(bool do_buy)
{
	this->do_buy = do_buy;
}

void Shop_Loop::set_do_sell(bool do_sell)
{
	this->do_sell = do_sell;
}

