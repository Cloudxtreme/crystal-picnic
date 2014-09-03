#include "crystalpicnic.h"
#include "area_loop.h"

#include "player.h"
#include "main_menu_loop.h"
#include "npc.h"
#include "character_role.h"
#include "follow_character_role.h"
#include "enemy_avatar_wander_character_role.h"
#include "game_specific_globals.h"


// FIXME:
#include "runner_loop.h"

ALLEGRO_DEBUG_CHANNEL("CrystalPicnic")


const float Area_Loop::SWIPE_TIME_ONE_DIRECTION = 0.777f;
const float Area_Loop::PAN_FLASH_DELAY = 0.2; 
const float Area_Loop::PAN_FLASH_MODULUS = 0.4;

const float TURN_TIME = 0.5f;
const float HALF_TURN_TIME = TURN_TIME / 2.0f;

void Area_Loop::set_swiping_in(bool swiping)
{
	swiping_in = swiping;
}

bool Area_Loop::init(void)
{
	engine->clear_touches();

	if (inited) {
		return true;
	}
	Loop::init();

	Player *player = NULL;

	std::vector<std::string> saved_players = Lua::get_saved_players();
	for (size_t i = 1; i < saved_players.size(); i++) {
		NPC *npc = new NPC(saved_players[i]);
		npc->set_id(i);
		npc->set_show_shadow(true);
		npc->load();
		npc->set_solid_with_area(false);
		npc->set_solid_with_entities(false);
		player_npcs.push_back(npc);
	}
	for (size_t i = 0; i < saved_players.size(); i++) {
		Player *p = new Player(saved_players[i]);
		p->set_id(i);
		p->set_show_shadow(true);
		p->load();
		players.push_back(p);
		if (i == 0) {
			player = p;
		}
	}
	for (size_t i = 1; i < players.size(); i++) {
		if (i == 1) {
			Character_Role *role = new Follow_Character_Role(player_npcs[i-1], players[0]);
			player_npcs[i-1]->set_role(role);
		}
		else {
			Character_Role *role = new Follow_Character_Role(player_npcs[i-1], player_npcs[i-2]);
			player_npcs[i-1]->set_role(role);
		}
	}

	bool warped = false;
	int index;
#ifdef ALLEGRO_ANDROID
	if (false) {
#else
	if ((index = General::find_arg("-warp")) > 0) {
#endif
		const char *area_name = General::argv[index+1];
		int layer = atoi(General::argv[index+2]);
		int start_x = atoi(General::argv[index+3]);
		int start_y = atoi(General::argv[index+4]);
		player->set_position(General::Point<float>(start_x, start_y));
		player->set_layer(layer);
		for (size_t i = 0; i < player_npcs.size(); i++) {
			player_npcs[i]->set_position(General::Point<float>(start_x, start_y));
			player_npcs[i]->set_layer(layer);
		}
		load_area(area_name);
		warped = true;
	}

	if (!warped) {
		ALLEGRO_DEBUG("Going to load_area");

		std::string area_name = Lua::get_saved_area_name();
		if (engine->game_is_over()) { // died and have a save state, return to river town
			engine->set_game_over(false);
			player->set_position(game_over_start_pos);
			player->set_layer(game_over_start_layer);
			for (size_t i = 0; i < player_npcs.size(); i++) {
				player_npcs[i]->set_position(game_over_start_pos);
				player_npcs[i]->set_layer(game_over_start_layer);
			}
			load_area(game_over_start_area_name);
		}
		else if (engine->get_game_just_loaded() && area_name != "") {
			load_area(area_name);
		}
		else {
			player->set_position(start_pos);
			player->set_layer(start_layer);
			for (size_t i = 0; i < player_npcs.size(); i++) {
				player_npcs[i]->set_position(start_pos);
				player_npcs[i]->set_layer(start_layer);
			}
			load_area(start_area_name);
		}
	}
	
	swiping_in = true;
	swipe_in_time = 0.0;

	ALLEGRO_DEBUG("Loaded area");

	change_areas();

	ALLEGRO_DEBUG("change_areas'd");
	
	player->add_camera(area);

	engine->reset_logic_count();

	ALLEGRO_DEBUG("Area_Loop::init done");

	return true;
}

void Area_Loop::destroy_area()
{
	if (area) {
		players[0]->reset();
		for (size_t i = 0; i < player_npcs.size(); i++) {
			player_npcs[i]->reset();
		}
		area->shutdown();
		Lua::call_lua(area->get_lua_state(), "stop", ">");
		lua_getglobal(area->get_lua_state(), "next_player_layer");
		bool exists = !lua_isnil(area->get_lua_state(), -1);
		if (exists) {
			players[0]->set_layer(lua_tonumber(area->get_lua_state(), -1));
		}
		lua_pop(area->get_lua_state(), 1);
		int layer = players[0]->get_layer();
		for (size_t i = 0; i < player_npcs.size(); i++) {
			player_npcs[i]->set_layer(layer);
		}
		delete area;
	}
}

void Area_Loop::top(void)
{
	if (deferred_area_name == "" || swiping_out)
		return;
	
	players[0]->remove_camera(area);
	players[0]->set_position(General::Point<float>(deferred_area_x, deferred_area_y));

	for (size_t i = 0; i < player_npcs.size(); i++) {
		Character_Role *role = player_npcs[i]->get_role();
		if (role) {
			Follow_Character_Role *fcrole = dynamic_cast<Follow_Character_Role *>(role);
			if (fcrole) {
				fcrole->reset();
			}
		}
		player_npcs[i]->set_position(players[0]->get_position());
	}

	destroy_area();

	load_area(deferred_area_name);
	
	players[0]->add_camera(new_area);

	change_areas();

	deferred_area_name = "";

	players[0]->set_direction(deferred_area_direction);

	players[0]->start_area();

	engine->play_sample("sfx/enter_area.ogg");

	swiping_in = true;
	swipe_in_time = 0.0;

	engine->reset_logic_count();
}

bool Area_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (deferred_area_name != "" || swiping_in || input_paused)
		return false;

	bool pass_to_player = true;

	if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
		if (event->keyboard.keycode == cfg.key_menu || event->keyboard.keycode == ALLEGRO_KEY_MENU) {
			Map_Entity *player = area->get_entity(0);
			if (!player->input_is_disabled()) {
				std::vector<Loop *> loops;
				loops.push_back(new Main_Menu_Loop(players, engine->get_loops()));
				engine->set_loops(loops, false);
				pass_to_player = false;
			}
		}
		else if (
#ifdef ALLEGRO_ANDROID
			event->keyboard.keycode == ALLEGRO_KEY_BUTTON_L1 ||
#endif
			event->keyboard.keycode == cfg.key_switch
		) {
			if (num_jumping == 0) {
				Map_Entity *player = area->get_entity(0);
				if (!player->input_is_disabled()) {
					if (engine->milestone_is_complete(Engine::JUMP_SWITCH_INFO_MILESTONE)) {
						switch_characters(true);
					}
				}
			}
		}
	}
	else
	if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
		if (event->joystick.button == cfg.joy_menu) {
			Map_Entity *player = area->get_entity(0);
			if (!player->input_is_disabled()) {
				std::vector<Loop *> loops;
				loops.push_back(new Main_Menu_Loop(players, engine->get_loops()));
				engine->set_loops(loops, false);
				pass_to_player = false;
			}
		}
		else if (event->joystick.button == cfg.joy_switch) {
			if (num_jumping == 0) {
				Map_Entity *player = area->get_entity(0);
				if (!player->input_is_disabled()) {
					if (engine->milestone_is_complete(Engine::JUMP_SWITCH_INFO_MILESTONE)) {
						switch_characters(true);
					}
				}
			}
		}
	}

	if (pass_to_player)
		players[0]->handle_event(event);

	return true;
}

bool Area_Loop::logic(void)
{
	engine->set_touch_input_type(TOUCHINPUT_AREA);

	if (swiping_in) {
		swipe_in_time += General::LOGIC_MILLIS / 1000.0;
	}
	else if (swiping_out) {
		swipe_out_time += General::LOGIC_MILLIS / 1000.0;
	}

	if (last_battle_screenshot) {
		return false;
	}

	if (deferred_area_name != "")
		return false;
	
	if (area->logic()) {
		return true;
	}
	if (deferred_area_name != "") {
		return true;
	}

	for (size_t i = 0; i < players.size(); i++) {
		// Regenerate magic
		float count = players[i]->get_magic_regen_count();
		count += Game_Specific_Globals::regenerate_magic(players[i]->get_battle_attributes(), false, count);
		players[i]->set_magic_regen_count(count-floor(count));
		// Apply status effects
		Game_Specific_Globals::apply_status(players[i]->get_battle_attributes());
	}

	return false;
}

void Area_Loop::draw(void)
{
	if (deferred_area_name != "" && !swiping_out) {
		return;
	}

	ALLEGRO_BITMAP *start_target = NULL;
	Wrap::Bitmap *transition_back_bmp = NULL;
	bool enable_input = true;
	if (last_battle_screenshot) {
		if (battle_event_count > 0) {
			battle_event_count--;
			if (battle_event_count < 0) {
				battle_event_count = 0;
			}
			enable_input = battle_event_count == 0;
			players[0]->set_panning_to_entity(-1);
			players[0]->center_cameras();
			if (battle_event_type != Enemy_Avatar_Wander_Character_Role::BATTLE_EVENT_SIGHTED) {
				players[0]->get_animation_set()->set_sub_animation("idle-down");
			}
		}
		if (engine->milestone_is_complete(Engine::JUMP_SWITCH_INFO_MILESTONE) && !was_boss_battle) {
			while (players[0]->get_name() != last_used_player_in_battle) {
				players[0]->set_input_disabled(false);
				switch_characters(false);
			}
			players[0]->set_input_disabled(true);
		}
		was_boss_battle = false;
		if (area->get_name() != "caverns7") {
			start_target = al_get_target_bitmap();
			transition_back_bmp = Wrap::create_bitmap_no_preserve(cfg.screen_w, cfg.screen_h);
			al_set_target_bitmap(transition_back_bmp->bitmap);
		}
	}
	
	al_clear_to_color(area->get_clear_color());

	area->draw();

	for (size_t i = 0; i < players.size(); i++) {
		Map_Entity *ent;
		if (i == 0) {
			ent = players[0];
		}
		else {
			ent = player_npcs[i-1];
		}
		if (players[i]->get_battle_attributes().status.name == "POISON") {
			General::Point<float> top = area->get_top();
			General::Point<float> pos = ent->get_position();
			General::draw_poison_bubbles(General::Point<float>(pos.x-top.x, pos.y-top.y-24));
		}
	}

	std::vector<Loop *> loops = engine->get_loops();
	bool found = false;
	for (size_t i = 0; i < loops.size(); i++) {
		if (loops[i] == this) {
			found = true;
			break;
		}
	}
	if (!found) {
		if (transition_back_bmp) {
			Wrap::destroy_bitmap(transition_back_bmp);
		}
		return;
	}
	
	if (swiping_in) {
		double ratio;
		if (swipe_in_time >= SWIPE_TIME_ONE_DIRECTION) {
			swiping_in = false;
			ratio = 1.0;
		}
		else {
			ratio = swipe_in_time / SWIPE_TIME_ONE_DIRECTION;
		}
		Graphics::side_swipe_in(al_color_name("black"), ratio);
	}
	else if (swiping_out) {
		double ratio;
		if (swipe_out_time >= SWIPE_TIME_ONE_DIRECTION) {
			swiping_out = false;
			ratio = 1;
		}
		else {
			ratio = swipe_out_time / SWIPE_TIME_ONE_DIRECTION;
		}
		Graphics::side_swipe_out(al_color_name("black"), ratio);
	}
	
	if (last_battle_screenshot && area->get_name() != "caverns7") {
		Wrap::Bitmap *turn_bitmap = last_battle_screenshot;

		double start = al_get_time();
		double transition_time = 0.0;
		bool switched_transition_bitmaps = false;
		
		al_set_target_backbuffer(engine->get_display());

		engine->play_sample("sfx/exit_battle.ogg");
		
		while (true) {
			double now = al_get_time();
			double elapsed = now - start;
			start = now;
			transition_time += elapsed;
			if (transition_time > TURN_TIME) {
				break;
			}
			else if (transition_time >= HALF_TURN_TIME) {
				if (!switched_transition_bitmaps) {
					switched_transition_bitmaps = true;
					turn_bitmap = transition_back_bmp;
				}
			}
			al_clear_to_color(al_map_rgb(0, 0, 0));
			if (transition_time > TURN_TIME) {
				transition_time = TURN_TIME;
			}
			float angle;
			if (transition_time < HALF_TURN_TIME) {
				angle = (transition_time / HALF_TURN_TIME) * (M_PI/2);
			}
			else {
				angle = ((transition_time - HALF_TURN_TIME) / HALF_TURN_TIME) * (M_PI/2) + (M_PI/2);
			}
			Graphics::turn_bitmap(turn_bitmap, angle);
			al_flip_display();
			al_rest(1.0/60.0);

			players[0]->set_input_disabled(!enable_input);
		}

		Wrap::destroy_bitmap(last_battle_screenshot);
		last_battle_screenshot = NULL;
		Wrap::destroy_bitmap(transition_back_bmp);

		al_set_target_bitmap(start_target);

		draw();
	}
	else if (last_battle_screenshot) {
		Wrap::destroy_bitmap(last_battle_screenshot);
		last_battle_screenshot = NULL;
	}
}

Area_Loop::Area_Loop(void) :
	area(NULL),
	deferred_area_name(""),
	swiping_in(false),
	swipe_out_time(0.0),
	swiping_out(false),
	input_paused(false),
	last_battle_screenshot(NULL),
	was_boss_battle(false),
	played_whack_a_skunk(false),
	num_jumping(0),
	battle_event_count(0)
{
	start_area_name = "castle";
	start_pos = General::Point<float>(652, 1250);
	start_layer = 2;

	game_over_start_area_name = "river_town";
	game_over_start_pos = General::Point<float>(1226, 279);
	game_over_start_layer = 2;
}

Area_Loop::~Area_Loop(void)
{
	destroy_area();

	for (size_t i = 0; i < players.size(); i++) {
		delete players[i];
	}
	for (size_t i = 0; i < player_npcs.size(); i++) {
		delete player_npcs[i];
	}
	
	engine->clear_touches();
}

void Area_Loop::load_area(std::string name)
{
	new_area = new Area_Manager();

	for (size_t i = 0; i < player_npcs.size(); i++) {
		new_area->add_entity(player_npcs[i]);
	}
			
	new_area->add_entity(players[0]);

	new_area->load(name);

	engine->set_last_area(name);
}

void Area_Loop::load_area_deferred(std::string name, General::Direction direction, int x, int y)
{
	this->deferred_area_name = name;
	this->deferred_area_direction = direction;
	this->deferred_area_x = x;
	this->deferred_area_y = y;

	swipe_out_time = 0.0;
	swiping_out = true;
}

void Area_Loop::change_areas(void)
{
	area = new_area;
	new_area = NULL;

	ALLEGRO_DEBUG("Calling area->start");

	area->start();

	ALLEGRO_DEBUG("area->start called");
}

std::vector<Player *> Area_Loop::get_players(void)
{
	return players;
}

std::vector<NPC *> Area_Loop::get_player_npcs(void)
{
	return player_npcs;
}
	
Area_Manager *Area_Loop::get_area(void)
{
	return new_area ? new_area : area;
}

Area_Manager *Area_Loop::get_new_area(void)
{
	return new_area;
}

bool Area_Loop::new_area_ready(void)
{
	return deferred_area_name != "";
}

void Area_Loop::rotated(void)
{
}

void Area_Loop::return_to_loop(void)
{
	area->get_entity(0)->reset();
}

void Area_Loop::set_input_paused(bool p)
{
	input_paused = p;
}

Wrap::Bitmap **Area_Loop::get_last_battle_screenshot()
{
	return &last_battle_screenshot;
}

std::string *Area_Loop::get_last_used_player_in_battle()
{
	return &last_used_player_in_battle;
}

void Area_Loop::set_start(std::string name, int layer, General::Point<float> pos)
{
	start_area_name = name;
	start_layer = layer;
	start_pos = pos;
}

void Area_Loop::switch_characters(bool sound)
{
	area->remove_entity_from_vector(players[0]);
	Character_Role **roles = new Character_Role *[player_npcs.size()];
	General::Point<float> *positions = new General::Point<float>[player_npcs.size()];
	int *ids = new int[player_npcs.size()];
	for (size_t i = 0; i < player_npcs.size(); i++) {
		roles[i] = player_npcs[i]->get_role();
		player_npcs[i]->set_role(NULL);
		positions[i] = player_npcs[i]->get_position();
		ids[i] = player_npcs[i]->get_id();
	}
	area->remove_entity_from_vector(player_npcs[0]);
	delete player_npcs[0];
	player_npcs.erase(player_npcs.begin());

	NPC *npc = new NPC(players[0]->get_name());
	npc->set_layer(players[0]->get_layer());
	npc->set_show_shadow(true);
	npc->load();
	npc->set_solid_with_area(false);
	npc->set_solid_with_entities(false);
	player_npcs.push_back(npc);

	for (size_t i = 0; i < player_npcs.size(); i++) {
		player_npcs[i]->set_id(ids[i]);
	}

	area->add_entity(npc);

	Player *player = players[0];
	players.erase(players.begin());
	players.push_back(player);
	area->add_entity(players[0]);
	players[0]->set_position(player->get_position());
	players[0]->set_layer(player->get_layer());
	player->remove_camera(area);
	players[0]->add_camera(area);

	for (size_t i = 0; i < players.size(); i++) {
		players[i]->set_id(i);
	}

	for (size_t i = 0; i < player_npcs.size(); i++) {
		player_npcs[i]->set_id(ids[i]);
		if (i == 0) {
			dynamic_cast<Follow_Character_Role *>(roles[i])->
				set_actors(player_npcs[i], players[0]);
		}
		else {
			dynamic_cast<Follow_Character_Role *>(roles[i])->
				set_actors(player_npcs[i], player_npcs[i-1]);
		}
		player_npcs[i]->set_role(roles[i]);
		player_npcs[i]->set_position(positions[i]);
	}

	if (sound) {
		engine->play_sample("sfx/switch_to_" + players[0]->get_name() + ".ogg", 1.0f, 0.0f, 1.0f);
	}

	delete[] roles;
	delete[] positions;
	delete[] ids;

	if (sound) { // not sure needed, but this is to stop players running after tab
		float *input = players[0]->get_inputs();
		for (int i = 0; i < Map_Entity::NUM_INPUTS; i++) {
			input[i] = 0;
		}
	}
}

void Area_Loop::add_bisou()
{
	bool frogbert_leading = players[0]->get_name() == "frogbert";

	NPC *npc = new NPC("bisou");
	npc->set_position(players[0]->get_position());
	npc->set_layer(players[0]->get_layer());
	npc->set_show_shadow(true);
	npc->load();
	npc->set_solid_with_area(false);
	npc->set_solid_with_entities(false);
	if (frogbert_leading) {
		player_npcs.insert(player_npcs.begin(), npc);
	}
	else {
		player_npcs.push_back(npc);
	}
	area->add_entity(npc);
	Character_Role *role = new Follow_Character_Role(npc, frogbert_leading ? (Map_Entity *)players[0] : (Map_Entity *)player_npcs[0]);
	npc->set_role(role);

	Player *bisou_player = new Player("bisou");
	bisou_player->load();

	if (frogbert_leading) {
		players.insert(players.begin()+1, bisou_player);
	}
	else {
		players.push_back(bisou_player);
	}
}

bool Area_Loop::battle_event_is_done()
{
	return battle_event_count <= 0;
}

void Area_Loop::set_battle_was_event(Enemy_Avatar_Wander_Character_Role::Battle_Event_Type event_type)
{
	battle_event_count++;
	battle_event_type = event_type;
}

void Area_Loop::set_was_boss_battle(bool was_boss_battle)
{
	this->was_boss_battle = was_boss_battle;
}

void Area_Loop::set_whack_a_skunk_played(bool played)
{
	played_whack_a_skunk = played;
}

void Area_Loop::set_whack_a_skunk_score(int score)
{
	whack_a_skunk_score = score;
}

bool Area_Loop::whack_a_skunk_was_played()
{
	return played_whack_a_skunk;
}

int Area_Loop::get_whack_a_skunk_score()
{
	return whack_a_skunk_score;
}

void Area_Loop::destroy_graphics()
{
	area->destroy_sheets();
}

void Area_Loop::reload_graphics()
{
	area->load_sheets();
}

int Area_Loop::get_num_jumping()
{
	return num_jumping;
}

void Area_Loop::set_num_jumping(int num_jumping)
{
	if (num_jumping < 0) num_jumping = 0;
	else if (num_jumping > (int)players.size()) num_jumping = players.size();
	this->num_jumping = num_jumping;
}

int Area_Loop::get_battle_event_count()
{
	return battle_event_count;
}
