#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "map_loop.h"
#include "luainc.h"
#include "error.h"
#include "music.h"
#include "area_loop.h"
#include "main_menu_loop.h"

bool Map_Loop::init()
{
	engine->clear_touches();

	if (inited) {
		return true;
	}
	Loop::init();

	lua_state = luaL_newstate();

	Lua::open_lua_libs(lua_state);

	Lua::register_c_functions(lua_state);

	Lua::load_global_scripts(lua_state);

	unsigned char *bytes;
	
	bytes = General::slurp("scripts/map.lua");
	if (luaL_loadstring(lua_state, (char *)bytes)) {
		Lua::dump_lua_stack(lua_state);
		throw Error("Error loading map script.\n");
	}
	delete[] bytes;

	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running map script.");
	}

	Lua::call_lua(lua_state, "start", "u>", this);
	
	for (size_t i = 0; i < locations.size(); i++) {
		if (locations[i].name == start_place) {
			curr_loc = i;
			break;
		}
	}
	
	offset = get_desired_position(curr_loc);
	desired_offset = offset;

	return true;
}

void Map_Loop::top()
{
}

bool Map_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (!faded_in) {
		return false;
	}

	if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
		int new_loc = -1;
		
#ifndef OUYA
		if (event->keyboard.keycode == cfg.key_left) {
			new_loc = locations[curr_loc].left;
		}
		else if (event->keyboard.keycode == cfg.key_right) {
			new_loc = locations[curr_loc].right;
		}
		else if (event->keyboard.keycode == cfg.key_up) {
			new_loc = locations[curr_loc].up;
		}
		else if (event->keyboard.keycode == cfg.key_down) {
			new_loc = locations[curr_loc].down;
		}
		else if (event->keyboard.keycode == ALLEGRO_KEY_ENTER || event->keyboard.keycode == cfg.key_ability[3]) {
			engine->play_sample("sfx/use_item.ogg", 1.0f, 0.0f, 1.0f);
			engine->fade_out();
			Lua::call_lua(lua_state, "get_dest", "ss>siii",
				locations[curr_loc].name.c_str(), engine->get_last_area().c_str());
			std::string dest = lua_tostring(lua_state, -4);
			int layer = lua_tonumber(lua_state, -3);
			int x = lua_tonumber(lua_state, -2);
			int y = lua_tonumber(lua_state, -1);
			lua_pop(lua_state, 4);

			if (dest != "") {
				Area_Loop *al = new Area_Loop();
				al->set_start(dest, layer, General::Point<float>(x, y));
				std::vector<Loop *> loops;
				loops.push_back(al);
				engine->set_loops_delay_init(loops, true, true);
			}

			return true;
		}
		else
#endif
		if (
			event->keyboard.keycode == cfg.key_menu ||
			event->keyboard.keycode == ALLEGRO_KEY_MENU
#if defined FIRETV || (defined ALLEGRO_ANDROID && !defined OUYA)
#ifdef FIRETV
			|| event->keyboard.keycode == ALLEGRO_KEY_BUTTON_B
#endif
			|| event->keyboard.keycode == ALLEGRO_KEY_BACK
#endif
			) {
			std::vector<Loop *> loops;
			loops.push_back(new Main_Menu_Loop(players, engine->get_loops()));
			engine->set_loops(loops, false);
			return true;
		}
		
		if (new_loc >= 0) {
			curr_loc = new_loc;
			desired_offset = get_desired_position(curr_loc);
			return true;
		}
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_AXIS && event->joystick.stick == 0) {
		int new_loc = -1;

		if (need_joy_axis_release) {
			if (fabs(event->joystick.pos) <= 0.25f) {
				need_joy_axis_release = false;
			}
		}
		else {
			if (event->joystick.axis == 0) {
				if (event->joystick.pos < -0.5f) {
					need_joy_axis_release = true;
					new_loc = locations[curr_loc].left;
				}
				else if (event->joystick.pos > 0.5f) {
					need_joy_axis_release = true;
					new_loc = locations[curr_loc].right;
				}
			}
			else if (event->joystick.axis == 1) {
				if (event->joystick.pos < -0.5f) {
					need_joy_axis_release = true;
					new_loc = locations[curr_loc].up;
				}
				else if (event->joystick.pos > 0.5f) {
					need_joy_axis_release = true;
					new_loc = locations[curr_loc].down;
				}
			}
		}
		
		if (new_loc >= 0) {
			curr_loc = new_loc;
			desired_offset = get_desired_position(curr_loc);
			return true;
		}
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
#ifndef OUYA
		if (event->joystick.button == cfg.joy_menu) {
			std::vector<Loop *> loops;
			loops.push_back(new Main_Menu_Loop(players, engine->get_loops()));
			engine->set_loops(loops, false);
			return true;
		}
		else
#endif
		if (event->joystick.button == cfg.joy_ability[3]) {
			engine->play_sample("sfx/use_item.ogg", 1.0f, 0.0f, 1.0f);
			engine->fade_out();
			Lua::call_lua(lua_state, "get_dest", "ss>siii",
				locations[curr_loc].name.c_str(), engine->get_last_area().c_str());
			std::string dest = lua_tostring(lua_state, -4);
			int layer = lua_tonumber(lua_state, -3);
			int x = lua_tonumber(lua_state, -2);
			int y = lua_tonumber(lua_state, -1);
			lua_pop(lua_state, 4);

			if (dest != "") {
				Area_Loop *al = new Area_Loop();
				al->set_start(dest, layer, General::Point<float>(x, y));
				std::vector<Loop *> loops;
				loops.push_back(al);
				engine->set_loops_delay_init(loops, true, true);
			}

			return true;
		}
	}

	return false;
}

bool Map_Loop::logic()
{
	engine->set_touch_input_type(TOUCHINPUT_MAP);

	if (!faded_in) {
		faded_in = true;
		engine->fade_in();
		engine->flush_event_queue();
	}

	if (!(desired_offset == offset)) {
		float dx = desired_offset.x - offset.x;
		float dy = desired_offset.y - offset.y;
		float angle = atan2(dy, dx);
		float dist = sqrt(dx*dx + dy*dy);
		const float speed = 3;
		if (dist <= speed) {
			offset = desired_offset;
		}
		else {
			offset.x += cos(angle) * speed;
			offset.y += sin(angle) * speed;
		}
	}

	return false;
}

void Map_Loop::draw()
{
	if (!faded_in) {
		return;
	}

	al_draw_bitmap(map_bmp->bitmap, -offset.x, -offset.y, 0);

	if (!engine->is_render_buffer(al_get_target_bitmap())) {
		// FIXME remove all this return;
	}

	for (size_t i = 0; i < locations.size(); i++) {
		Location &l = locations[i];
		if (curr_loc == i) {
			#define draw(dir) \
				Graphics::draw_stippled_line( \
					l.pos.x, \
					l.pos.y, \
					locations[dir].pos.x, \
					locations[dir].pos.y, \
					offset.x, \
					offset.y, \
					al_color_name("red"), \
					5, \
					5, \
					5 \
				)
			if (l.left != -1) {
				draw(l.left);
			}
			if (l.right != -1) {
				draw(l.right);
			}
			if (l.up != -1) {
				draw(l.up);
			}
			if (l.down != -1) {
				draw(l.down);
			}
			#undef draw
		}
	}
	for (size_t i = 0; i < locations.size(); i++) {
		Location &l = locations[i];
		Wrap::Bitmap *b;
		if (curr_loc == i) {
			b = x_bmp;
		}
		else {
			b = node_bmp;
		}
		al_draw_bitmap(b->bitmap, l.pos.x-offset.x-al_get_bitmap_width(b->bitmap)/2, l.pos.y-offset.y-al_get_bitmap_height(b->bitmap)/2, 0);
	}

	// draw name

	const int PADDING = 5;
	const int TRAIL = 15;
	int tw = General::get_text_width(General::FONT_HEAVY, locations[curr_loc].name);
	int th = General::get_font_line_height(General::FONT_HEAVY);
	int x = 15 + PADDING;
	int y = cfg.screen_h - 15 - th - PADDING*2;
	al_draw_filled_rectangle(x, y, x+tw+PADDING*2, y+th+PADDING*2, al_color_name("lightgrey"));
	Graphics::draw_gradient_rectangle(x+tw+PADDING*2, y, x+tw+PADDING*2+TRAIL, y+th+PADDING*2, al_color_name("lightgrey"), al_map_rgba_f(0, 0, 0, 0), al_map_rgba_f(0, 0, 0, 0), al_color_name("lightgrey"));
	General::draw_text(t(locations[curr_loc].name.c_str()), al_color_name("black"), x+PADDING, y+PADDING, 0, General::FONT_HEAVY);
}

void Map_Loop::add_location(std::string name, General::Point<int> pos)
{
	Location l;
	l.name = name;
	l.pos = pos;
	l.left = l.right = l.up = l.down = -1;
	locations.push_back(l);
}

void Map_Loop::set_location_neighbors(std::string name, std::string left, std::string right, std::string up, std::string down)
{
	int i = name_to_int(name);
	if (i < 0) return;
	Location &l = locations[i];
	l.left = name_to_int(left);
	l.right = name_to_int(right);
	l.up = name_to_int(up);
	l.down = name_to_int(down);
}

Map_Loop::Map_Loop(std::string start_place) :
	start_place(start_place),
	curr_loc(-1),
	need_joy_axis_release(false),
	faded_in(false)
{
	Music::play("music/map.mid");

	lua_state = NULL;
	map_bmp = Wrap::load_bitmap("misc_graphics/map/map.cpi");
	x_bmp = Wrap::load_bitmap("misc_graphics/map/x.cpi");
	node_bmp = Wrap::load_bitmap("misc_graphics/map/node.cpi");
	offset = General::Point<float>(0, 0);

	engine->set_game_just_loaded(false);
	create_temporary_players();
}

Map_Loop::~Map_Loop()
{
	if (lua_state) {
		lua_close(lua_state);
	}

	Wrap::destroy_bitmap(map_bmp);
	Wrap::destroy_bitmap(x_bmp);
	Wrap::destroy_bitmap(node_bmp);

	Lua::store_battle_attributes(players);

	for (size_t i = 0; i < players.size(); i++) {
		delete players[i];
	}
	
	engine->clear_touches();
}

General::Point<int> Map_Loop::get_desired_position(int loc)
{
	Location &l = locations[loc];

	General::Point<int> p(
		l.pos.x - cfg.screen_w/2,
		l.pos.y - cfg.screen_h/2
	);

	if (p.x < 0) p.x = 0;
	if (p.x+cfg.screen_w/2 > al_get_bitmap_width(map_bmp->bitmap)) {
		p.x = al_get_bitmap_width(map_bmp->bitmap) - cfg.screen_w/2;
	}

	if (p.y < 0) p.y = 0;
	if (p.y+cfg.screen_h/2 > al_get_bitmap_height(map_bmp->bitmap)) {
		p.y = al_get_bitmap_height(map_bmp->bitmap) - cfg.screen_h/2;
	}
	
	return p;
}

int Map_Loop::name_to_int(std::string name)
{
	int i = -1;
	for (size_t j = 0; j < locations.size(); j++) {
		if (locations[j].name == name) {
			i = j;
			break;
		}
	}
	return i;
}

std::string Map_Loop::get_current_location_name()
{
	return locations[curr_loc].name;
}

void Map_Loop::create_temporary_players()
{
	std::vector<std::string> player_names = Lua::get_saved_players();
	for (size_t i = 0; i < player_names.size(); i++) {
		Player *p = new Player(player_names[i]);
		p->load();
		players.push_back(p);
	}
}

std::vector<Player *> Map_Loop::get_players()
{
	return players;
}
