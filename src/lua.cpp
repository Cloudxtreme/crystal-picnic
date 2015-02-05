#include <cstring>

#include "luainc.h"
#include "map_entity.h"
#include "general.h"
#include "engine.h"
#include "crystalpicnic.h"
#include "npc.h"
#include "character_map_entity.h"
#include "character_role.h"
#include "astar_character_role.h"
#include "follow_character_role.h"
#include "wander_character_role.h"
#include "area_loop.h"
#include "battle_loop.h"
#include "whack_a_skunk_loop.h"
#include "map_loop.h"
#include "shop_loop.h"
#include "particle.h"
#include "collision_detection.h"
#include "video_player.h"
#include "shaders.h"
#include "enemy_avatar.h"
#include "noop_character_role.h"
#include "resource_manager.h"
#include "runner_loop.h"
#include "credits_loop.h"

enum Saveable_Type {
	SAVE_PLAYER = 1,
	SAVE_CHOPPABLE = 2,
	SAVE_ENEMY = 3
};

struct Saveable {
	Saveable_Type type;
	struct {
		int layer;
		float x;
		float y;
	} player;
	struct {
		std::string name;
		int layer;
		float x;
		float y;
	} choppable;
	struct {
		std::string level;
		std::string script;
		int layer;
		float x;
		float y;
		std::string avatar;
		std::vector<std::string> enemies;
	} enemy;
};

static std::vector<Saveable> saved_items;
static std::vector<std::string> saved_lua_lines;
static std::string saved_area_name = "";
static std::pair<std::string, Battle_Attributes> battle_attributes[3];
static std::vector<std::string> saved_players;
static std::string selected_abilities[3][4];
static int save_state_version = 1;

namespace Lua {

int get_save_state_version()
{
	return save_state_version;
}

void set_save_state_version(int version)
{
	save_state_version = version;
}

void reset_game()
{
	saved_players.clear();
	saved_players.push_back("egbert");
	saved_players.push_back("frogbert");
}

std::vector<std::string> get_saved_players()
{
	return saved_players;
}

void init_battle_attributes()
{
	for (int i = 0; i < 3; i++) {
		battle_attributes[i].second.max_hp = 10 * cfg.difficulty_mult();
		battle_attributes[i].second.hp = 10 * cfg.difficulty_mult();
		battle_attributes[i].second.max_mp = 5;
		battle_attributes[i].second.mp = 5;
		battle_attributes[i].second.attack = 1;
		battle_attributes[i].second.defense = 0;

		battle_attributes[i].second.abilities.abilities[0] = 0;
		battle_attributes[i].second.abilities.abilities[1] = 0;
		battle_attributes[i].second.abilities.abilities[2] = 0;
		battle_attributes[i].second.abilities.hp = 0;
		battle_attributes[i].second.abilities.mp = 0;

		battle_attributes[i].second.status.name = "";

		if (i == 0) {
			battle_attributes[i].first = "egbert";
			battle_attributes[i].second.equipment.weapon.name = "STICK";
			battle_attributes[i].second.equipment.weapon.attack = 1;
			battle_attributes[i].second.equipment.weapon.element = Equipment::ELEMENT_NONE;
			battle_attributes[i].second.equipment.weapon.usable_by = "egbert";
			battle_attributes[i].second.equipment.armor.name = "";
			battle_attributes[i].second.equipment.armor.defense = 0;
			battle_attributes[i].second.equipment.accessory.name = "";
		}
		else if (i == 1) {
			battle_attributes[i].first = "frogbert";
			battle_attributes[i].second.equipment.weapon.name = "SHOVEL";
			battle_attributes[i].second.equipment.weapon.attack = 2;
			battle_attributes[i].second.equipment.weapon.element = Equipment::ELEMENT_NONE;
			battle_attributes[i].second.equipment.weapon.usable_by = "frogbert";
			battle_attributes[i].second.equipment.armor.name = "";
			battle_attributes[i].second.equipment.armor.defense = 0;
			battle_attributes[i].second.equipment.accessory.name = "";
		}
		else {
			battle_attributes[i].first = "bisou";
			battle_attributes[i].second.equipment.weapon.name = "BOW";
			battle_attributes[i].second.equipment.weapon.attack = 2;
			battle_attributes[i].second.equipment.weapon.element = Equipment::ELEMENT_NONE;
			battle_attributes[i].second.equipment.weapon.usable_by = "bisou";
			battle_attributes[i].second.equipment.armor.name = "";
			battle_attributes[i].second.equipment.armor.defense = 0;
			battle_attributes[i].second.equipment.accessory.name = "";
		}

		selected_abilities[i][0] = "USE";
		selected_abilities[i][1] = "ATTACK";
		selected_abilities[i][2] = "";
		selected_abilities[i][3] = "JUMP";
	}
}

void store_battle_attributes(std::vector<Player *> players)
{
	saved_players.clear();
	for (size_t i = 0; i < players.size(); i++) {
		saved_players.push_back(players[i]->get_name());
		battle_attributes[i].first = players[i]->get_name();
		battle_attributes[i].second = players[i]->get_battle_attributes();
		std::vector<std::string> abilities = players[i]->get_selected_abilities(true, false, false);
		for (int j = 0; j < 4; j++) {
			selected_abilities[i][j] = abilities[j];
		}
	}
}

void restore_battle_attributes(Player *p)
{
	for (int i = 0; i < 3; i++) {
		if (battle_attributes[i].first == p->get_name()) {
			p->get_battle_attributes() = battle_attributes[i].second;
			for (int j = 0; j < 4; j++) {
				p->set_selected_ability(true, j, selected_abilities[i][j]);
			}
			return;
		}
	}
}

void add_battle_attributes_lines()
{
	for (int i = 0; i < 3; i++) {
		char line[2000];
		if (battle_attributes[i].first == "") {
			snprintf(line, 2000, "set_name(%d, \"\")\n", i);
			saved_lua_lines.push_back(line);
			snprintf(line, 2000, "set_attributes(%d, 0, 0, 0, 0, 0, 0, 0)\n", i);
			saved_lua_lines.push_back(line);
			snprintf(line, 2000, "set_status(%d, \"\")\n", i);
			saved_lua_lines.push_back(line);
			snprintf(line, 2000, "set_weapon(%d, \"\")\n", i);
			saved_lua_lines.push_back(line);
			snprintf(line, 2000, "set_armor(%d, \"\")\n", i);
			saved_lua_lines.push_back(line);
			snprintf(line, 2000, "set_accessory(%d, \"\")\n", i);
			saved_lua_lines.push_back(line);
			snprintf(line, 2000, "set_crystals(%d, 0, 0, 0, 0, 0)\n", i);
			saved_lua_lines.push_back(line);
			snprintf(line, 2000, "set_selected_abilities(%d, \"%s\", \"%s\", \"%s\", \"%s\")\n",
				i,
				selected_abilities[i][0].c_str(),
				selected_abilities[i][1].c_str(),
				selected_abilities[i][2].c_str(),
				selected_abilities[i][3].c_str()
			);
			saved_lua_lines.push_back(line);
			continue;
		}
		snprintf(
			line,
			2000,
			"set_name(%d, \"%s\")\n",
			i,
			battle_attributes[i].first.c_str()
		);
		saved_lua_lines.push_back(line);
		snprintf(
			line,
			2000,
			"set_attributes(%d, %d, %d, %d, %d, %d, %d, %d)\n",
			i,
			battle_attributes[i].second.hp / cfg.difficulty_mult(),
			battle_attributes[i].second.max_hp / cfg.difficulty_mult(),
			battle_attributes[i].second.mp,
			battle_attributes[i].second.max_mp,
			battle_attributes[i].second.attack,
			battle_attributes[i].second.defense,
			42 // was experience
		);
		saved_lua_lines.push_back(line);
		snprintf(
			line,
			2000,
			"set_status(%d, \"%s\")\n",
			i,
			battle_attributes[i].second.status.name.c_str()
		);
		saved_lua_lines.push_back(line);
		snprintf(line, 2000, "set_weapon(%d, \"%s\")\n",
			i,
			battle_attributes[i].second.equipment.weapon.name.c_str());
		saved_lua_lines.push_back(line);
		for (size_t j = 0; j < battle_attributes[i].second.equipment.weapon.attachments.size(); j++) {
			snprintf(line, 2000, "set_weapon_attachment(%d, \"%s\", %d)\n",
				i,
				battle_attributes[i].second.equipment.weapon.attachments[j].name.c_str(),
				battle_attributes[i].second.equipment.weapon.attachments[j].quantity
			);
			saved_lua_lines.push_back(line);
		}
		snprintf(line, 2000, "set_armor(%d, \"%s\")\n",
			i,
			battle_attributes[i].second.equipment.armor.name.c_str());
		saved_lua_lines.push_back(line);
		snprintf(line, 2000, "set_accessory(%d, \"%s\")\n",
			i,
			battle_attributes[i].second.equipment.accessory.name.c_str());
		saved_lua_lines.push_back(line);
		snprintf(line, 2000, "set_crystals(%d, %d, %d, %d, %d, %d)\n",
			i,
			battle_attributes[i].second.abilities.abilities[0],
			battle_attributes[i].second.abilities.abilities[1],
			battle_attributes[i].second.abilities.abilities[2],
			battle_attributes[i].second.abilities.hp,
			battle_attributes[i].second.abilities.mp);
		saved_lua_lines.push_back(line);
		snprintf(line, 2000, "set_selected_abilities(%d, \"%s\", \"%s\", \"%s\", \"%s\")\n",
			i,
			selected_abilities[i][0].c_str(),
			selected_abilities[i][1].c_str(),
			selected_abilities[i][2].c_str(),
			selected_abilities[i][3].c_str()
		);
		saved_lua_lines.push_back(line);
	}
}

void clear_saved_lua_lines()
{
	saved_lua_lines.clear();
}

void clear_before_load()
{
	saved_items.clear();
	init_battle_attributes();
}

void add_saved_lua_line(std::string line)
{
	saved_lua_lines.push_back(line);
}

void write_saved_lua_lines(ALLEGRO_FILE *f)
{
	for (size_t i = 0; i < saved_lua_lines.size(); i++) {
		al_fputs(f, saved_lua_lines[i].c_str());
	}
}

std::string get_saved_area_name()
{
	return saved_area_name;
}

void open_lua_libs(lua_State *lua_state)
{
	luaL_openlibs(lua_state);
}

/*
 * Call a Lua function, leaving the results on the stack.
 */
void call_lua(lua_State* lua_state, const char *func, const char *sig, ...)
{
	va_list vl;
	int narg, nres;  /* number of arguments and results */

	va_start(vl, sig);
	lua_getglobal(lua_state, func);  /* get function */

	if (!lua_isfunction(lua_state, -1)) {
		lua_pop(lua_state, 1);
		return;
	}

	/* push arguments */
	narg = 0;
	while (*sig) {  /* push arguments */
		switch (*sig++) {
			case 'd':  /* double argument */
				lua_pushnumber(lua_state, va_arg(vl, double));
				break;
			case 'b':  /* boolean (int) argument */
				lua_pushboolean(lua_state, va_arg(vl, int));
				break;
			case 'i':  /* int argument */
				lua_pushnumber(lua_state, va_arg(vl, int));
				break;

			case 's':  /* string argument */
				lua_pushstring(lua_state, va_arg(vl, char *));
				break;
			case 'u':  /* userdata argument */
				lua_pushlightuserdata(lua_state, va_arg(vl, void *));
				break;
			case '>':
				goto endwhile;
			default:
				break;
		}
		narg++;
		luaL_checkstack(lua_state, 1, "too many arguments");
	}
endwhile:

	/* do the call */
	nres = strlen(sig);  /* number of expected results */
	if (lua_pcall(lua_state, narg, nres, 0) != 0) {
		General::log_message("*** error running function: " + (func == NULL ? "(NULL)" : std::string(func)));
		dump_lua_stack(lua_state);
	}

	va_end(vl);
}

void dump_lua_stack(lua_State *l)
{
        int i;
        int top = lua_gettop(l);
	char buf[1000];

        snprintf(buf, 1000, "--- stack ---\n");
	General::log_message(buf);
        snprintf(buf, 1000, "top=%u   ...   ", top);
	General::log_message(buf);

        for (i = 1; i <= top; i++) {  /* repeat for each level */
                int t = lua_type(l, i);
                switch (t) {

                case LUA_TSTRING:  /* strings */
                        snprintf(buf, 1000, "`%s'", lua_tostring(l, i));
			General::log_message(buf);
                        break;

                case LUA_TBOOLEAN:  /* booleans */
                        snprintf(buf, 1000, lua_toboolean(l, i) ? "true" : "false");
			General::log_message(buf);
                        break;

                case LUA_TNUMBER:  /* numbers */
                        snprintf(buf, 1000, "%g", lua_tonumber(l, i));
			General::log_message(buf);
                        break;

                case LUA_TTABLE:   /* table */
                        snprintf(buf, 1000, "table");
			General::log_message(buf);
                        break;

                default:  /* other values */
                        snprintf(buf, 1000, "%s", lua_typename(l, t));
			General::log_message(buf);
                        break;

                }
                snprintf(buf, 1000, "  ");  /* put a separator */
        }
        snprintf(buf, 1000, "\n");  /* end the listing */
	General::log_message(buf);

        snprintf(buf, 1000, "-------------\n");
	General::log_message(buf);
}

static void stop_entity(int id)
{
	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		Battle_Entity *e =
			dynamic_cast<Battle_Entity *>(bl->get_entity(id));
		if (e) {
			e->stop();
			e->get_input().clear();
			e->set_velocity(General::Point<float>(0.0f, 0.0f));
			e->set_accel(General::Point<float>(0.0f, 0.0f));
		}
	}
	else {
		Area_Loop *l = GET_AREA_LOOP;
		if (l) {
			Area_Manager *area = l->get_area();
			Map_Entity *e = area->get_entity(id);
			float *inputs = e->get_inputs();
			for (int i = 0; i < Map_Entity::NUM_INPUTS; i++) {
				inputs[i] = 0.0f;
			}
			if (e->get_animation_set()->get_current_animation()->is_looping()) {
				e->update_direction(false);
			}
		}
	}
}

static int c_add_entity(lua_State *stack)
{
	const char *name = lua_tostring(stack, 1);
	int layer = lua_tonumber(stack, 2);
	int pos_x = lua_tonumber(stack, 3);
	int pos_y = lua_tonumber(stack, 4);

	Map_Entity *entity = new Map_Entity(std::string(name));
	entity->load();
	entity->set_layer(layer);
	entity->set_position(General::Point<float>(pos_x, pos_y));

	Area_Loop *area_loop = GET_AREA_LOOP;
	if (area_loop) {
		int id = area_loop->get_area()->add_entity(entity);
		lua_pushnumber(stack, id);
		return 1;
	}

	return 0;
}

static int c_add_polygon_entity(lua_State *stack)
{
	int layer = lua_tonumber(stack, 1);
	int i = 2;
	std::vector< General::Point<float> > vertices;
	std::vector<int> splits;
	std::vector<Triangulate::Triangle> triangles;

	while (i < lua_gettop(stack)) {
		float x = lua_tonumber(stack, i++);
		float y = lua_tonumber(stack, i++);
		vertices.push_back(General::Point<float>(x, y));
	}

	splits.push_back(vertices.size());

	int minx = INT_MAX;
	int miny = INT_MAX;
	int maxx = INT_MIN;
	int maxy = INT_MIN;

	for (size_t i = 0; i < vertices.size(); i++) {
		if (vertices[i].x > maxx) {
			maxx = vertices[i].x;
		}
		if (vertices[i].x < minx) {
			minx = vertices[i].x;
		}
		if (vertices[i].y > maxy) {
			maxy = vertices[i].y;
		}
		if (vertices[i].y < miny) {
			miny = vertices[i].y;
		}
	}

	float midx = minx + (maxx-minx)/2;

	for (size_t i = 0; i < vertices.size(); i++) {
		vertices[i].x -= midx;
		vertices[i].y -= maxy;
		vertices[i].y -= General::BOTTOM_SPRITE_PADDING;
	}

	Triangulate::get_triangles(vertices, splits, triangles);

	Map_Entity *entity = new Map_Entity("polygon");
	entity->load();
	entity->set_layer(layer);
	entity->set_position(General::Point<float>(midx, maxy));

	Bones::Bone bone = Bones::Bone(
		Bones::BONE_NORMAL,
		vertices,
		triangles,
		General::Size<int>(maxx-minx, maxy-miny)
	);
	entity->get_current_bones().clear();
	entity->get_current_bones().push_back(bone);

	Area_Loop *area_loop = GET_AREA_LOOP;
	if (area_loop) {
		int id = area_loop->get_area()->add_entity(entity);
		lua_pushnumber(stack, id);
		return 1;
	}

	return 0;
}

static int c_add_npc(lua_State *stack)
{
	const char *name = lua_tostring(stack, 1);
	int layer = lua_tonumber(stack, 2);
	int pos_x = lua_tonumber(stack, 3);
	int pos_y = lua_tonumber(stack, 4);

	NPC *entity = new NPC(std::string(name));
	entity->load();
	entity->set_layer(layer);
	entity->set_position(General::Point<float>(pos_x, pos_y));

	Area_Loop *area_loop = GET_AREA_LOOP;
	if (area_loop) {
		int id = area_loop->get_area()->add_entity(entity);
		lua_pushnumber(stack, id);
		return 1;
	}

	return 0;
}

static int c_set_character_role(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	const char *type = lua_tostring(stack, 2);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area) {
			Map_Entity *entity = area->get_entity(id);
			if (entity) {
				Character_Map_Entity *character = dynamic_cast<Character_Map_Entity *>(entity);
				if (character) {
					Character_Role *old = character->get_role();
					if (old) {
						delete old;
					}
					Character_Role *role = NULL;
					if (!strcmp(type, "astar")) {
						role = new AStar_Character_Role(character);
						if (lua_gettop(stack) == 3) {
							bool doesnt_wait_after_collision = lua_toboolean(stack, 3);
							((AStar_Character_Role *)role)->set_doesnt_wait_after_collision(doesnt_wait_after_collision);
						}
					}
					else if (!strcmp(type, "wander")) {
						int max_dist = lua_tonumber(stack, 3);
						double pause_min = lua_tonumber(stack, 4);
						double pause_max = lua_tonumber(stack, 5);
						role = new Wander_Character_Role(
							character,
							max_dist,
							pause_min,
							pause_max);
						((Wander_Character_Role *)role)->set_home(character->get_position());
					}
					else if (!strcmp(type, "follow")) {
						int id2 = lua_tonumber(stack, 3);
						Map_Entity *target = area->get_entity(id2);
						role = new Follow_Character_Role(character, target);
					}
					else if (!strcmp(type, "noop")) {
						role = new NoOp_Character_Role(character);
					}
					character->set_role(role);
				}
			}
		}
	}

	return 0;
}

static int c_change_areas(lua_State *stack)
{
	const char *name = lua_tostring(stack, 1);
	General::Direction direction = (General::Direction)(int)lua_tonumber(stack, 2);
	int x = lua_tonumber(stack, 3);
	int y = lua_tonumber(stack, 4);

	Area_Loop *area_loop = GET_AREA_LOOP;
	if (area_loop) {
		area_loop->load_area_deferred(std::string(name), direction, x, y);
	}

	return 0;
}

static int c_get_screen_size(lua_State *stack)
{
	lua_pushnumber(stack, cfg.screen_w);
	lua_pushnumber(stack, cfg.screen_h);
	return 2;
}

static int c_get_screen_scale(lua_State *stack)
{
	lua_pushnumber(stack, cfg.screens_w);
	lua_pushnumber(stack, cfg.screens_h);
	return 2;
}

static int c_get_entity_position(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Positioned_Entity *entity = NULL;

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area) {
			entity = area->get_entity(id);
		}
		else {
			return 0;
		}
	}
	else {
		Battle_Loop *bl = GET_BATTLE_LOOP;
		if (bl) {
			entity = bl->get_entity(id);
		}
		else {
			return 0;
		}
	}

	if (entity) {
		General::Point<float> pos = entity->get_position();
		lua_pushnumber(stack, pos.x);
		lua_pushnumber(stack, pos.y);
		return 2;
	}

	return 0;
}

static int c_get_entity_z(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Positioned_Entity *entity = NULL;

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area) {
			entity = area->get_entity(id);
		}
		else {
			return 0;
		}
	}
	else {
		Battle_Loop *bl = GET_BATTLE_LOOP;
		if (bl) {
			entity = bl->get_entity(id);
		}
		else {
			return 0;
		}
	}

	if (entity) {
		float z = entity->get_z();
		lua_pushnumber(stack, z);
		return 1;
	}

	return 0;
}

static int c_set_entity_position(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	float x = lua_tonumber(stack, 2);
	float y = lua_tonumber(stack, 3);

	Positioned_Entity *entity = NULL;
	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area)
			entity = area->get_entity(id);
	}
	else {
		Battle_Loop *bl = GET_BATTLE_LOOP;
		if (bl) {
			entity = bl->get_entity(id);
		}
	}
	if (entity) {
		entity->set_position(General::Point<float>(x, y));
	}

	return 0;
}

static int c_set_entity_z(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	float z = lua_tonumber(stack, 2);

	Positioned_Entity *entity = NULL;
	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area)
			entity = area->get_entity(id);
		if (entity) {
			entity->set_z(z);
		}
	}

	return 0;
}

static int c_get_entity_animation_size(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Area_Loop *al = GET_AREA_LOOP;
	if (al) {
		Map_Entity *entity =
			dynamic_cast<Map_Entity *>(al->get_area()->get_entity(id));
		if (entity) {
			Bitmap *bmp =
				entity->get_animation_set()->get_current_animation()->get_current_frame()->get_bitmap();
			lua_pushnumber(stack, bmp->get_width());
			lua_pushnumber(stack, bmp->get_height());
			return 2;
		}
	}
	else {
		Battle_Loop *bl = GET_BATTLE_LOOP;
		if (bl) {
			Battle_Entity *entity =
				dynamic_cast<Battle_Entity *>(bl->get_entity(id));
			if (entity) {
				Bitmap *bmp =
					entity->get_animation_set()->get_current_animation()->get_current_frame()->get_bitmap();
				lua_pushnumber(stack, bmp->get_width());
				lua_pushnumber(stack, bmp->get_height());
				return 2;
			}
		}
	}

	return 0;
}

static int c_get_entity_bone_size(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		Battle_Entity *entity =
			dynamic_cast<Battle_Entity *>(bl->get_entity(id));
		if (entity) {
			std::pair<std::string, int> p;
			p.first = "battle-idle";
			p.second = 0;
			std::vector<Bones::Bone> &bones = entity->get_bones()[p];
			if (bones.size() > 0) {
				General::Size<float> extents = bones[0].get_extents();
				lua_pushnumber(stack, extents.w);
				lua_pushnumber(stack, extents.h);
				return 2;
			}
		}
	}

	return 0;
}

static int c_get_entity_layer(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Map_Entity *entity = NULL;
	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		entity = area->get_entity(id);
		if (entity) {
			lua_pushnumber(stack, entity->get_layer());
			return 1;
		}
	}

	return 0;
}

static int c_set_entity_layer(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int layer = lua_tonumber(stack, 2);

	Map_Entity *entity = NULL;
	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		entity = area->get_entity(id);
		if (entity) {
			if (entity) {
				entity->set_layer(layer);
			}
		}
	}

	return 0;
}

static int c_get_area_pixel_size(lua_State *stack)
{
	Area_Loop *loop = GET_AREA_LOOP;
	Area_Manager *area = loop->get_area();

	General::Size<int> pixel_size = area->get_pixel_size();
	lua_pushnumber(stack, pixel_size.w);
	lua_pushnumber(stack, pixel_size.h);

	return 2;
}

static int c_get_area_tile_size(lua_State *stack)
{
	Area_Loop *loop = GET_AREA_LOOP;
	Area_Manager *area = loop->get_area();

	lua_pushnumber(stack, area->get_width());
	lua_pushnumber(stack, area->get_height());

	return 2;
}

static int c_add_floating_image(lua_State *stack)
{
	const char *filename = lua_tostring(stack, 1);
	int layer = lua_tonumber(stack, 2);
	int offset_x = lua_tonumber(stack, 3);
	int offset_y = lua_tonumber(stack, 4);
	bool pre = (bool)lua_toboolean(stack, 5);
	bool subtractive = (bool)lua_toboolean(stack, 6);

	Wrap::Bitmap *bitmap = Wrap::load_bitmap(std::string("misc_graphics/")+filename);

	if (!bitmap) {
		General::log_message("Error loading floating bitmap '" +
			std::string(filename) + "'.");
		return 0;
	}

	Area_Loop *loop = GET_AREA_LOOP;
	Area_Manager *area = loop->get_area();

	area->add_floating_image(bitmap, layer, offset_x, offset_y, pre, subtractive);

	General::log_message("Loaded floating image: '" + std::string(filename) + "'");

	return 0;
}

static int c_add_floating_rectangle(lua_State *stack)
{
	float r1 = (float)lua_tonumber(stack, 1);
	float g1 = (float)lua_tonumber(stack, 2);
	float b1 = (float)lua_tonumber(stack, 3);
	float r2 = (float)lua_tonumber(stack, 4);
	float g2 = (float)lua_tonumber(stack, 5);
	float b2 = (float)lua_tonumber(stack, 6);
	float r3 = (float)lua_tonumber(stack, 7);
	float g3 = (float)lua_tonumber(stack, 8);
	float b3 = (float)lua_tonumber(stack, 9);
	float r4 = (float)lua_tonumber(stack, 10);
	float g4 = (float)lua_tonumber(stack, 11);
	float b4 = (float)lua_tonumber(stack, 12);
	int w = (int)lua_tonumber(stack, 13);
	int h = (int)lua_tonumber(stack, 14);
	int layer = lua_tonumber(stack, 15);
	int offset_x = lua_tonumber(stack, 16);
	int offset_y = lua_tonumber(stack, 17);
	bool pre = (bool)lua_toboolean(stack, 18);

	Area_Loop *loop = GET_AREA_LOOP;
	Area_Manager *area = loop->get_area();

	area->add_floating_rectangle(
		al_map_rgb_f(r1, g1, b1),
		al_map_rgb_f(r2, g2, b2),
		al_map_rgb_f(r3, g3, b3),
		al_map_rgb_f(r4, g4, b4),
		w, h, layer, offset_x, offset_y, pre);

	return 0;
}

static int c_add_outline_point(lua_State *stack)
{
	int layer = lua_tonumber(stack, 1);
	int x = lua_tonumber(stack, 2);
	int y = lua_tonumber(stack, 3);

	Area_Loop *loop = GET_AREA_LOOP;
	Area_Manager *area = loop->get_area();
	area->add_outline_point(layer, x, y);

	return 0;
}

static int c_add_outline_split(lua_State *stack)
{
	int layer = lua_tonumber(stack, 1);
	int index = lua_tonumber(stack, 2);

	Area_Loop *loop = GET_AREA_LOOP;
	Area_Manager *area = loop->get_area();
	area->add_outline_split(layer, index);

	return 0;
}

static int c_set_entity_solid_with_area(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool solid = lua_toboolean(stack, 2);

	Area_Loop *loop = GET_AREA_LOOP;
	Area_Manager *area = loop->get_area();
	Map_Entity *entity = area->get_entity(id);
	if (entity) {
		entity->set_solid_with_area(solid);
	}

	return 0;
}

static int c_set_entity_solid_with_entities(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool solid = lua_toboolean(stack, 2);

	Area_Loop *loop = GET_AREA_LOOP;
	Area_Manager *area = loop->get_area();
	Map_Entity *entity = area->get_entity(id);
	if (entity) {
		entity->set_solid_with_entities(solid);
	}

	return 0;
}

static int c_add_tween(lua_State *stack)
{
	int tween_id = lua_tonumber(stack, 1);

	engine->add_tween(tween_id);

	return 0;
}

static int c_set_entity_input_disabled(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);
	bool disabled = lua_toboolean(stack, 2);

	Battle_Loop *loop = GET_BATTLE_LOOP;
	if (loop) {
		Battle_Entity *entity = loop->get_entity(entity_id);
		if (entity) {
			entity->set_input_disabled(disabled);
		}
	}
	else {
		Area_Loop *loop = GET_AREA_LOOP;
		if (loop) {
			Area_Manager *area = loop->get_area();
			Map_Entity *entity = area->get_entity(entity_id);
			if (entity) {
				entity->set_input_disabled(disabled);
			}
		}
	}

	return 0;
}

static int c_get_entity_input_disabled(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		Map_Entity *entity = area->get_entity(entity_id);
		if (entity) {
			lua_pushboolean(stack, entity->input_is_disabled());
			return 1;
		}
	}

	return 0;
}

static int c_set_show_entity_shadow(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);
	bool show = lua_toboolean(stack, 2);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *entity =
			dynamic_cast<Battle_Entity *>(l->get_entity(entity_id));
		if (entity) {
			entity->set_show_shadow(show);
		}
	}
	else {
		Area_Loop *loop = GET_AREA_LOOP;
		if (loop) {
			Area_Manager *area = loop->get_area();
			Map_Entity *entity = area->get_entity(entity_id);
			if (entity) {
				entity->set_show_shadow(show);
			}
		}
	}

	return 0;
}

static int c_get_entity_shadow_is_shown(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);

	bool shown = false;

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		Map_Entity *entity = area->get_entity(entity_id);
		if (entity) {
			shown = entity->shadow_is_shown();
		}
	}

	lua_pushboolean(stack, shown);

	return 1;
}

static int c_play_music(lua_State *stack)
{
	const char *name = lua_tostring(stack, 1);

	Music::play(name);

	/* In some cases (starting a water cooler then leaving the area) the music volume
	 * can be off or low so make sure it's back to normal. In normal cases this does
	 * nothing.
	 */
	Music::ramp_up(0.5);

	return 0;
}

static int c_set_entity_animation(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);
	const char *anim_name = lua_tostring(stack, 2);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		Map_Entity *entity = area->get_entity(entity_id);
		if (entity) {
			Skeleton::Skeleton *skeleton = entity->get_skeleton();
			if (skeleton) {
				skeleton->set_curr_anim(anim_name);
				skeleton->reset_current_animation();
			}
			else {
				entity->get_animation_set()->set_sub_animation(anim_name);
				entity->get_animation_set()->reset();
			}
		}
	}
	else {
		Battle_Loop *bl = GET_BATTLE_LOOP;
		if (bl) {
			Battle_Entity *entity =
				dynamic_cast<Battle_Entity *>(bl->get_entity(entity_id));
			if (entity) {
				Skeleton::Skeleton *skeleton = entity->get_skeleton();
				if (skeleton) {
					skeleton->set_curr_anim(anim_name);
					skeleton->reset_current_animation();
				}
				else {
					entity->get_animation_set()->set_sub_animation(anim_name);
					entity->get_animation_set()->reset();
				}
			}
		}
	}

	return 0;
}

static int c_get_entity_animation(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		Map_Entity *entity = area->get_entity(entity_id);
		if (entity) {
			Skeleton::Skeleton *skeleton = entity->get_skeleton();
			if (skeleton) {
				Skeleton::Animation *anim = skeleton->get_animations()[skeleton->get_curr_anim()];
				lua_pushstring(stack, anim->name.c_str());
			}
			else {
				lua_pushstring(stack, entity->get_animation_set()->get_current_animation()->get_name().c_str());
			}
			return 1;
		}
	}

	return 0;
}

static int c_get_entity_animation_frame(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		Map_Entity *entity = area->get_entity(entity_id);
		if (entity) {
			Skeleton::Skeleton *skeleton = entity->get_skeleton();
			if (skeleton) {
				lua_pushnumber(stack, skeleton->get_animations()[skeleton->get_curr_anim()]->curr_frame);
			}
			else {
				lua_pushnumber(stack, entity->get_animation_set()->get_current_animation()->get_current_frame_num());
			}
			return 1;
		}
	}

	return 0;
}

static int c_set_entity_animation_no_reset(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);
	const char *anim_name = lua_tostring(stack, 2);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		Map_Entity *entity = area->get_entity(entity_id);
		if (entity) {
			entity->get_animation_set()->set_sub_animation(std::string(anim_name));
		}
	}
	else {
		Battle_Loop *bl = GET_BATTLE_LOOP;
		if (bl) {
			Battle_Entity *entity =
				dynamic_cast<Battle_Entity *>(bl->get_entity(entity_id));
			if (entity) {
				entity->get_animation_set()->set_sub_animation(anim_name);
			}
		}
	}

	return 0;
}

static int c_get_entity_animation_num_frames(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		Map_Entity *entity = area->get_entity(entity_id);
		if (entity) {
			int nf =
				entity->get_animation_set()->
				get_current_animation()->get_num_frames();
			lua_pushnumber(stack, nf);
		}
	}

	return 1;
}

static int c_set_entity_animation_frame(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);
	int frame = lua_tonumber(stack, 2);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		Map_Entity *entity = area->get_entity(entity_id);
		if (entity) {
			entity->get_animation_set()->get_current_animation()
				->set_frame(frame);
		}
	}

	return 0;
}

static int c_reset_entity_animation(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		Map_Entity *entity = area->get_entity(entity_id);
		if (entity) {
			entity->get_animation_set()->
				get_current_animation()->reset();
		}
	}

	return 0;
}

static int c_set_entity_direction(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);
	General::Direction dir = (General::Direction)(int)lua_tonumber(stack, 2);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		Map_Entity *entity = area->get_entity(entity_id);
		if (entity) {
			entity->set_direction(dir);
		}
	}

	return 0;
}

static int c_get_entity_direction(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		Map_Entity *entity = area->get_entity(entity_id);
		if (entity) {
			lua_pushnumber(stack, (int)entity->get_direction());
			return 1;
		}
	}

	return 0;
}

static int c_get_num_entities_in_area(lua_State *stack)
{
	int num = -1;

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		num = area->get_num_entities();
	}

	lua_pushnumber(stack, num);

	return 1;
}

static int c_get_num_entities_in_battle(lua_State *stack)
{
	Battle_Loop *loop = GET_BATTLE_LOOP;
	if (loop) {
		lua_pushnumber(stack, loop->get_entities().size());
		return 1;
	}

	return 0;
}

static int c_get_area_entity_id_by_number(lua_State *stack)
{
	int num = lua_tonumber(stack, 1);

	int id = -1;

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		id = area->get_entity_id_by_number(num);
	}

	lua_pushnumber(stack, id);

	return 1;
}

static int c_get_battle_entity_id_by_number(lua_State *stack)
{
	int num = lua_tonumber(stack, 1);

	Battle_Loop *loop = GET_BATTLE_LOOP;
	if (loop) {
		std::vector<Battle_Entity *> &v = loop->get_entities();
		lua_pushnumber(stack, v[num]->get_id());
		return 1;
	}

	return 0;
}

static int c_set_area_player_underlay_bitmap_add(lua_State *stack)
{
	const char *name = lua_tostring(stack, 1);
	int layer = lua_tonumber(stack, 2);
	bool top_also = lua_toboolean(stack, 3);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		Wrap::Bitmap *bmp;
		bmp = Wrap::load_bitmap(std::string("misc_graphics/")+name+".png");
		area->set_player_underlay_bitmap_add(bmp, layer, top_also);
	}

	return 0;
}

static int c_set_character_destination(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int dx = lua_tonumber(stack, 2);
	int dy = lua_tonumber(stack, 3);
	bool run = lua_toboolean(stack, 4);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Character_Map_Entity *character = dynamic_cast<Character_Map_Entity *>(loop->get_area()->get_entity(id));
		if (character) {
			AStar_Character_Role *role = dynamic_cast<AStar_Character_Role *>(character->get_role());
			if (role) {
				role->set_destination(General::Point<float>(dx, dy), run);
			}
		}
	}

	return 0;
}

static int c_character_is_following_path(lua_State *stack)
{
	bool ret = false;
	int id = lua_tonumber(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Character_Map_Entity *character = dynamic_cast<Character_Map_Entity *>(loop->get_area()->get_entity(id));
		if (character) {
			AStar_Character_Role *role = dynamic_cast<AStar_Character_Role *>(character->get_role());
			if (role) {
				ret = role->is_following_path();
			}
		}
	}

	lua_pushboolean(stack, ret);
	return 1;
}

static int c_reset_outline(lua_State *stack)
{
	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		area->reset_outline();
	}

	return 0;
}

static int c_process_outline(lua_State *stack)
{
	int x, y;
	Area_Manager *area;

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		area = loop->get_area();
	}
	else {
		return 0;
	}

	for (int i = 0; i < 256; i++) {
		int splits_found = 0;
		char table_name[100];
		snprintf(table_name, 100, "outline%d", i);
		lua_getglobal(stack, table_name);
		if (!lua_istable(stack, -1)) {
			lua_pop(stack, 1);
			area->process_outline();
			return 0;
		}
		int j = 1;
		lua_pushnumber(stack, j++);
		lua_gettable(stack, -2);
		int layer = lua_tonumber(stack, -1);
		lua_pop(stack, 1);
		General::Point<int> last(-1, -1);
		while (true) {
			lua_pushnumber(stack, j++);
			lua_gettable(stack, -2);
			if (lua_isnil(stack, -1)) {
				lua_pop(stack, 1);
				break;
			}
			x = lua_tonumber(stack, -1);
			lua_pop(stack, 1);
			lua_pushnumber(stack, j++);
			lua_gettable(stack, -2);
			y = lua_tonumber(stack, -1);
			if (x == -1 && y == -1) {
				splits_found++;
				int points = (j-2-(splits_found*2))/2;
				area->add_outline_split(layer, points);
			}
			else {
				// This is to straighten out lines that are nearly straight
				if (last.x != -1 || last.y != -1) {
					int dx = x - last.x;
					int dy = y - last.y;
					if (abs(dx) < 4 && abs(dy) > 50) {
						x = last.x;
					}
					else if (abs(dy) < 4 && abs(dx) > 50) {
						y = last.y;
					}
				}
				area->add_outline_point(layer, x, y);
			}
			last.x = x;
			last.y = y;
			lua_pop(stack, 1);
		}
		lua_pop(stack, 1);
	}

	return 0;
}

static int c_set_entity_animation_set_prefix(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	const char *prefix = lua_tostring(stack, 2);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Map_Entity *e = dynamic_cast<Map_Entity *>(loop->get_area()->get_entity(id));
		if (e) {
			e->get_animation_set()->set_prefix(prefix);
		}
	}
	else {
		Battle_Loop *l = GET_BATTLE_LOOP;
		if (l) {
			Battle_Entity *e = dynamic_cast<Battle_Entity *>(l->get_entity(id));
			if (e) {
				e->get_animation_set()->set_prefix(prefix);
			}
		}
	}

	return 0;
}

static int c_get_entity_animation_set_prefix(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Map_Entity *e = dynamic_cast<Map_Entity *>(loop->get_area()->get_entity(id));
		if (e) {
			lua_pushstring(stack, e->get_animation_set()->get_prefix().c_str());
			return 1;
		}
	}
	else {
		Battle_Loop *l = GET_BATTLE_LOOP;
		if (l) {
			Battle_Entity *e = dynamic_cast<Battle_Entity *>(l->get_entity(id));
			if (e) {
				lua_pushstring(stack, e->get_animation_set()->get_prefix().c_str());
				return 1;
			}
		}
	}

	return 0;
}

static int c_center_entity_cameras(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Map_Entity *e = dynamic_cast<Map_Entity *>(loop->get_area()->get_entity(id));
		if (e) {
			e->center_cameras();
			return 0;
		}
	}

	return 0;
}

static int c_set_move_entity_cameras_while_input_disabled(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool move_cameras = lua_toboolean(stack, 2);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Map_Entity *e = dynamic_cast<Map_Entity *>(loop->get_area()->get_entity(id));
		if (e) {
			e->set_move_cameras_while_input_disabled(move_cameras);
			return 0;
		}
	}

	return 0;
}

static int c_set_entity_speed(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	float speed = lua_tonumber(stack, 2);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Map_Entity *e = dynamic_cast<Map_Entity *>(loop->get_area()->get_entity(id));
		if (e) {
			e->set_speed(speed);
		}
	}

	return 0;
}

static int c_set_shadow_color(lua_State *stack)
{
	float r = lua_tonumber(stack, 1);
	float g = lua_tonumber(stack, 2);
	float b = lua_tonumber(stack, 3);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		loop->get_area()->set_shadow_color(r, g, b);
	}

	return 0;
}

static int c_set_clear_color(lua_State *stack)
{
	float r = lua_tonumber(stack, 1);
	float g = lua_tonumber(stack, 2);
	float b = lua_tonumber(stack, 3);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		loop->get_area()->set_clear_color(r, g, b);
	}

	return 0;
}

static int c_set_parallax_parameters(lua_State *stack)
{
	bool parallax_x = (bool)lua_toboolean(stack, 1);
	bool parallax_y = (bool)lua_toboolean(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		loop->get_area()->set_parallax_params(parallax_x, parallax_y);
	}

	return 0;
}

static int c_load_bitmap(lua_State *stack)
{
	std::string filename = lua_tostring(stack, 1);

	Wrap::Bitmap *bmp = resource_manager->reference_bitmap(filename);

	lua_pushlightuserdata(stack, bmp);

	return 1;
}

static int c_destroy_bitmap(lua_State *stack)
{
	std::string filename = lua_tostring(stack, 1);
	resource_manager->release_bitmap(filename);
	return 0;
}

static int c_get_bitmap_size(lua_State *stack)
{
	Wrap::Bitmap *bmp = (Wrap::Bitmap *)lua_touserdata(stack, 1);
	lua_pushnumber(stack, al_get_bitmap_width(bmp->bitmap));
	lua_pushnumber(stack, al_get_bitmap_height(bmp->bitmap));
	return 2;
}

static int c_get_bitmap_texture_size(lua_State *stack)
{
	Wrap::Bitmap *bmp = (Wrap::Bitmap *)lua_touserdata(stack, 1);
	int true_w, true_h;
	if (al_get_display_flags(engine->get_display()) & ALLEGRO_OPENGL) {
		al_get_opengl_texture_size(bmp->bitmap, &true_w, &true_h);
	}
#ifdef ALLEGRO_WINDOWS
	else {
		al_get_d3d_texture_size(bmp->bitmap, &true_w, &true_h);
	}
#endif
	lua_pushnumber(stack, true_w);
	lua_pushnumber(stack, true_h);
	return 2;
}

static int c_draw_bitmap(lua_State *stack)
{
	Wrap::Bitmap *bmp = (Wrap::Bitmap *)lua_touserdata(stack, 1);
	float x = lua_tonumber(stack, 2);
	float y = lua_tonumber(stack, 3);
	int flags = (int)lua_tonumber(stack, 4);

	if (bmp) {
		al_draw_bitmap(bmp->bitmap, x, y, flags);
	}

	return 0;
}

static int c_draw_bitmap_yellow_glow(lua_State *stack)
{
	Wrap::Bitmap *b = (Wrap::Bitmap *)lua_touserdata(stack, 1);
	float x = lua_tonumber(stack, 2);
	float y = lua_tonumber(stack, 3);
	int flags = (int)lua_tonumber(stack, 4);
	int r1 = (int)lua_tonumber(stack, 5);
	int g1 = (int)lua_tonumber(stack, 6);
	int b1 = (int)lua_tonumber(stack, 7);
	int r2 = (int)lua_tonumber(stack, 8);
	int g2 = (int)lua_tonumber(stack, 9);
	int b2 = (int)lua_tonumber(stack, 10);

	if (b) {
		Graphics::draw_tinted_bitmap_region_depth_yellow_glow(
			b,
			al_map_rgb_f(1, 1, 1),
			0, 0,
			al_get_bitmap_width(b->bitmap),
			al_get_bitmap_height(b->bitmap),
			x, y,
			flags,
			1.0f,
			r1, g1, b1,
			r2, g2, b2
		);
	}

	return 0;
}

static int c_draw_bitmap_additive(lua_State *stack)
{
	Wrap::Bitmap *bmp = (Wrap::Bitmap *)lua_touserdata(stack, 1);
	float x = lua_tonumber(stack, 2);
	float y = lua_tonumber(stack, 3);
	float alpha = lua_tonumber(stack, 4);
	int flags = (int)lua_tonumber(stack, 5);

	if (bmp) {
		ALLEGRO_STATE state;
		al_store_state(&state, ALLEGRO_STATE_BLENDER);
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
		al_draw_tinted_bitmap(
			bmp->bitmap,
			al_map_rgba_f(alpha, alpha, alpha, alpha),
			x,
			y,
			flags
		);
		al_restore_state(&state);
	}

	return 0;
}

static int c_draw_tinted_rotated_bitmap(lua_State *stack)
{
	Wrap::Bitmap *bmp = (Wrap::Bitmap *)lua_touserdata(stack, 1);
	float r = lua_tonumber(stack, 2);
	float g = lua_tonumber(stack, 3);
	float b = lua_tonumber(stack, 4);
	float a = lua_tonumber(stack, 5);
	float cx = lua_tonumber(stack, 6);
	float cy = lua_tonumber(stack, 7);
	float dx = lua_tonumber(stack, 8);
	float dy = lua_tonumber(stack, 9);
	float angle = lua_tonumber(stack, 10);
	int flags = lua_tonumber(stack, 11);

	if (bmp) {
		al_draw_tinted_rotated_bitmap(bmp->bitmap, al_map_rgba_f(r, g, b, a), cx, cy, dx, dy, angle, flags);
	}

	return 0;
}

static int c_draw_tinted_rotated_bitmap_additive(lua_State *stack)
{
	Wrap::Bitmap *bmp = (Wrap::Bitmap *)lua_touserdata(stack, 1);
	float r = lua_tonumber(stack, 2);
	float g = lua_tonumber(stack, 3);
	float b = lua_tonumber(stack, 4);
	float a = lua_tonumber(stack, 5);
	float cx = lua_tonumber(stack, 6);
	float cy = lua_tonumber(stack, 7);
	float dx = lua_tonumber(stack, 8);
	float dy = lua_tonumber(stack, 9);
	float angle = lua_tonumber(stack, 10);
	int flags = lua_tonumber(stack, 11);

	if (bmp) {
		ALLEGRO_STATE state;
		al_store_state(&state, ALLEGRO_STATE_BLENDER);
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
		al_draw_tinted_rotated_bitmap(bmp->bitmap, al_map_rgba_f(r, g, b, a), cx, cy, dx, dy, angle, flags);
		al_restore_state(&state);
	}

	return 0;
}

static int c_draw_bitmap_region(lua_State *stack)
{
	Wrap::Bitmap *bmp = (Wrap::Bitmap *)lua_touserdata(stack, 1);
	float x = lua_tonumber(stack, 2);
	float y = lua_tonumber(stack, 3);
	int w = (int)lua_tonumber(stack, 4);
	int h = (int)lua_tonumber(stack, 5);
	float dx = (float)lua_tonumber(stack, 6);
	float dy = (float)lua_tonumber(stack, 7);
	int flags = (int)lua_tonumber(stack, 8);

	if (bmp) {
		al_draw_bitmap_region(bmp->bitmap, x, y, w, h, dx, dy, flags);
	}

	return 0;
}

static int c_get_area_top(lua_State *stack)
{
	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area) {
			General::Point<float> top = area->get_top();
			lua_pushnumber(stack, top.x);
			lua_pushnumber(stack, top.y);
			return 2;
		}
	}

	return 0;
}

static int c_get_battle_top(lua_State *stack)
{
	Battle_Loop *loop = GET_BATTLE_LOOP;
	if (loop) {
		General::Point<float> top = loop->get_top();
		lua_pushnumber(stack, top.x);
		lua_pushnumber(stack, top.y);
		return 2;
	}

	return 0;
}

static int c_add_tile_group(lua_State *stack)
{
	Area_Manager::Tile_Group tg;

	tg.layer = lua_tonumber(stack, 1);
	tg.top_left = General::Point<int>(
		lua_tonumber(stack, 2),
		lua_tonumber(stack, 3)
	);
	tg.size = General::Size<int>(
		lua_tonumber(stack, 4),
		lua_tonumber(stack, 5)
	);

	tg.flags = lua_tonumber(stack, 6);

	tg.duration = 0.0;

	int i = 7;

	while (i < lua_gettop(stack)) {
		int x = lua_tonumber(stack, i++);
		int y = lua_tonumber(stack, i++);
		tg.tiles.push_back(General::Point<int>(x, y));
	}

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area) {
			area->add_tile_group(tg);
		}
	}

	return 0;
}

static int c_get_entity_iso_position(lua_State *stack)
{
	int id = (int)lua_tonumber(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area) {
			General::Point<int> p = area->get_entity_iso_pos(id);
			lua_pushnumber(stack, p.x);
			lua_pushnumber(stack, p.y);
			return 2;
		}
	}

	return 0;
}

static int c_area_is_isometric(lua_State *stack)
{
	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area) {
			lua_pushboolean(stack, area->is_isometric());
			return 1;
		}
	}

	return 0;
}

static int speak_loc(lua_State *stack, Speech_Location loc)
{
	bool thoughts = lua_toboolean(stack, 1);
	bool autoscroll = lua_toboolean(stack, 2);
	bool wait = autoscroll ? false : lua_toboolean(stack, 3);
        int num = (lua_gettop(stack) - 3) / 3;
	int face = (lua_gettop(stack) - 3) % 3 == 1 ? lua_tonumber(stack, 4+num*3) : -1;
	General::Direction old_dir = General::DIR_S;

	if (face != -1) {
		Area_Loop *l = GET_AREA_LOOP;
		if (l) {
			Area_Manager *area = l->get_area();
			Map_Entity *e = area->get_entity(face);
			if (e) {
				old_dir = e->get_direction();
				e->face(((Map_Entity *)area->get_entity(0))->get_position());
			}
		}
	}

	std::vector<ALLEGRO_USTR *> texts;
	std::vector<std::string> gestures;
	std::vector<int> ids;

	int arg = 4;
	for (int i = 0; i < num; i++) {
		const char *text = lua_tostring(stack, arg++);
		const char *gesture = lua_tostring(stack, arg++);
		int id = lua_tonumber(stack, arg++);
		texts.push_back(al_ustr_new(text));
		gestures.push_back(std::string(gesture));
		ids.push_back(id);
	}

	Speech_Loop *l = new Speech_Loop(wait, ids, gestures, texts, loc, thoughts ? SPEECH_THOUGHTS : SPEECH_NORMAL, autoscroll, /*wait ?*/ engine->get_loops() /*: std::vector<Loop *>()*/, face, old_dir);
	std::vector<Loop *> v;
	v.push_back(l);

	if (wait) {
		l->init();
		engine->do_blocking_mini_loop(v, "");
	}
	else if (autoscroll) {
		l->init();
		engine->add_loop(l);
	}
	else {
		engine->set_loops(v, false);
	}

	return 0;
}

static int c_speak(lua_State *stack)
{
	return speak_loc(stack, SPEECH_LOC_BOTTOM);
}

static int c_speak_top(lua_State *stack)
{
	return speak_loc(stack, SPEECH_LOC_TOP);
}

static int c_speak_force_t(lua_State *stack)
{
	return speak_loc(stack, SPEECH_LOC_FORCE_TOP);
}

static int c_speak_force_b(lua_State *stack)
{
	return speak_loc(stack, SPEECH_LOC_FORCE_BOTTOM);
}

static int c_block_on_condition(lua_State *stack)
{
	const char *callback = lua_tostring(stack, 1);
	char cb[100];
	strcpy(cb, callback);

	engine->do_blocking_mini_loop(engine->get_loops(), cb);

	return 0;
}

static int c_bit_and(lua_State *stack)
{
	int n = (int)lua_tonumber(stack, 1);
	int a = (int)lua_tonumber(stack, 2);

	lua_pushnumber(stack, n & a);

	return 1;
}

static int c_bit_rshift(lua_State *stack)
{
	int n = (int)lua_tonumber(stack, 1);
	int s = (int)lua_tonumber(stack, 2);

	lua_pushnumber(stack, n >> s);

	return 1;
}

static int c_load_sample(lua_State *stack)
{
	const char *filename = lua_tostring(stack, 1);
	bool loop = lua_toboolean(stack, 2);

	engine->load_sample(filename, loop);

	return 0;
}

static int c_play_sample(lua_State *stack)
{
	const char *name = lua_tostring(stack, 1);
	float volume = lua_tonumber(stack, 2);
	float pan = lua_tonumber(stack, 3);
	float speed = lua_tonumber(stack, 4);

	engine->play_sample(name, volume, pan, speed);

	return 0;
}

static int c_stop_sample(lua_State *stack)
{
	const char *name = lua_tostring(stack, 1);

	engine->stop_sample(name);

	return 0;
}

static int c_adjust_sample(lua_State *stack)
{
	const char *filename = lua_tostring(stack, 1);
	float volume = lua_tonumber(stack, 2);
	float pan = lua_tonumber(stack, 3);
	float speed = lua_tonumber(stack, 4);

	engine->adjust_sample(filename, volume, pan, speed);

	return 0;
}

static int c_destroy_sample(lua_State *stack)
{
	const char *filename = lua_tostring(stack, 1);

	engine->destroy_sample(filename);

	return 0;
}

static int c_add_parallax_bitmap(lua_State *stack)
{
	const char *filename = lua_tostring(stack, 1);
	bool foreground = lua_toboolean(stack, 2);

	std::string fn = std::string("battle/parallax/") + filename;

	Battle_Loop *loop = GET_BATTLE_LOOP;
	if (loop) {
		loop->add_parallax_bitmap(Wrap::load_bitmap(fn), foreground);
	}

	return 0;
}

static int c_change_tile_layer(lua_State *stack)
{
	int x = lua_tonumber(stack, 1);
	int y = lua_tonumber(stack, 2);
	int from = lua_tonumber(stack, 3);
	int to = lua_tonumber(stack, 4);

	Area_Loop *al = GET_AREA_LOOP;
	if (al) {
		Area_Manager *area = al->get_area();
		std::vector<Tiled_Layer *> &t = area->get_tiled_layers();
		if (t[from]->tiles[y][x].number < 0 || t[from]->tiles[y][x].sheet < 0)
			return 0;
		t[to]->tiles[y][x].number = t[from]->tiles[y][x].number;
		t[to]->tiles[y][x].sheet = t[from]->tiles[y][x].sheet;
		t[to]->tiles[y][x].solid = t[from]->tiles[y][x].solid;
		t[from]->tiles[y][x].number = -1;
		t[from]->tiles[y][x].sheet = -1;
		t[from]->tiles[y][x].solid= 0;
	}

	return 0;
}

bool get_random_enemy_spawn_point(Enemy_Avatar *ea, General::Point<float> &pos)
{
	bool found = false;

	Area_Loop *al = GET_AREA_LOOP;

	if (al) {
		Area_Manager *area = al->get_area();

		// pick unblocked mesh triangle center
		std::vector<Triangulate::Triangle> &mesh = area->get_nav_meshes()[ea->get_layer()];
		int r = General::rand() % mesh.size();
		int start = r;
		std::vector<Bones::Bone> bones;
		ea->collidable_get_bones(bones);
		while (true) {
			pos = A_Star::barycenter(&mesh[r]);

			if (!area->point_is_in_no_enemy_zone(pos.x, pos.y) && area->point_collides(ea->get_layer(), pos)) {
				bool colliding = false;
				bool stop;
				if (area->area_is_colliding(ea->get_layer(), pos, bones, ea->is_facing_right(), NULL, &stop)) {
					colliding = true;
				}
				if ((!colliding) && area->entity_is_colliding(ea, pos, false).size() > 0) {
					colliding = true;
				}
				if (!colliding) {
					found = true;
					break;
				}
			}

			r++;
			r %= mesh.size();
			if (r == start)
				break;
		}
	}

	return found;
}

static int c_get_random_enemy_spawn_point(lua_State *stack)
{
	std::vector<std::string> v;

	Enemy_Avatar *ea = new Enemy_Avatar(
		"",
		"",
		false,
		lua_tostring(stack, 1),
		v
	);
	ea->load();
	ea->set_layer(lua_tonumber(stack, 2));

	General::Point<float> pos;
	bool found = get_random_enemy_spawn_point(ea, pos);

	delete ea;

	if (found) {
		lua_pushnumber(stack, pos.x);
		lua_pushnumber(stack, pos.y);
		return 2;
	}
	else {
		return 0;
	}
}

static int c_add_enemy_avatar(lua_State *stack)
{
	const char *level = lua_tostring(stack, 1);
	const char *script = lua_tostring(stack, 2);
	int layer = lua_tonumber(stack, 3);
	bool set_position = lua_toboolean(stack, 4);
	int x = lua_tonumber(stack, 5);
	int y = lua_tonumber(stack, 6);
	const char *avatar = lua_tostring(stack, 7);
	int num = lua_tonumber(stack, 8);
	int ret = 0;

	std::vector<std::string> enemies;

	for (int i = 9; i < 9+num; i++) {
		enemies.push_back(std::string(lua_tostring(stack, i)));
	}

	Area_Loop *al = GET_AREA_LOOP;
	if (al) {
		Area_Manager *area = al->get_area();

		Enemy_Avatar *ea = new Enemy_Avatar(
			std::string(level),
			std::string(script),
			false,
			std::string(avatar),
			enemies
		);
		ea->load();
		ea->set_layer(layer);

		if (set_position) {
			ea->set_position(General::Point<float>(x, y));
			area->add_entity(ea);
			lua_pushnumber(stack, ea->get_id());
			ret = 1;
		}
		else {
			General::Point<float> pos;
			bool found = get_random_enemy_spawn_point(ea, pos);
			if (found) {
				ea->set_position(pos);
				area->add_entity(ea);
				lua_pushnumber(stack, ea->get_id());
				ret = 1;
			}
			else {
				delete ea;
			}
		}
	}

	return ret;
}

static int c_rand(lua_State *stack)
{
	int n = lua_tonumber(stack, 1);

	lua_pushnumber(stack, (int)(((float)General::rand()/0xffffffff)*n));

	return 1;
}

static int c_set_entity_visible(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool visible = lua_toboolean(stack, 2);

	Map_Entity *entity = NULL;
	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area)
			entity = area->get_entity(id);
		if (entity) {
			entity->set_visible(visible);
		}
	}
	else {
		Battle_Loop *bl = GET_BATTLE_LOOP;
		Battle_Entity *e = dynamic_cast<Battle_Entity *>(bl->get_entity(id));
		if (e) {
			e->set_visible(visible);
		}
	}

	return 0;
}

static int c_set_tile(lua_State *stack)
{
	int layer = lua_tonumber(stack, 1);
	int x = lua_tonumber(stack, 2);
	int y = lua_tonumber(stack, 3);
	int sheet = lua_tonumber(stack, 4);
	int num = lua_tonumber(stack, 5);
	bool solid = lua_tonumber(stack, 6);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area) {
			std::vector<Tiled_Layer *>
				&layers = area->get_tiled_layers();
			layers[layer]->tiles[y][x].sheet = sheet;
			layers[layer]->tiles[y][x].number = num;
			layers[layer]->tiles[y][x].solid = solid;
		}
	}

	return 0;
}

static int c_get_tile(lua_State *stack)
{
	int layer = lua_tonumber(stack, 1);
	int x = lua_tonumber(stack, 2);
	int y = lua_tonumber(stack, 3);

	int sheet = -1;
	int num = -1;
	bool solid = false;

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area) {
			std::vector<Tiled_Layer *>
				&layers = area->get_tiled_layers();
			sheet = layers[layer]->tiles[y][x].sheet;
			num = layers[layer]->tiles[y][x].number;
			solid = layers[layer]->tiles[y][x].solid;
		}
	}

	lua_pushnumber(stack, sheet);
	lua_pushnumber(stack, num);
	lua_pushboolean(stack, solid);

	return 3;
}

static int c_add_flash(lua_State *stack)
{
	double start = lua_tonumber(stack, 1);
	double up = lua_tonumber(stack, 2);
	double stay = lua_tonumber(stack, 3);
	double down = lua_tonumber(stack, 4);
	int r = lua_tonumber(stack, 5);
	int g = lua_tonumber(stack, 6);
	int b = lua_tonumber(stack, 7);
	int a = lua_tonumber(stack, 8);

	engine->add_flash(start, up, stay, down, al_map_rgba(r, g, b, a));

	return 0;
}

static int c_shake(lua_State *stack)
{
	double start = lua_tonumber(stack, 1);
	double duration = lua_tonumber(stack, 2);
	int amount = lua_tonumber(stack, 3);

	engine->shake(start, duration, amount);

	return 0;
}

static int c_end_shake(lua_State *stack)
{
	engine->end_shake();

	return 0;
}

static int c_fade(lua_State *stack)
{
	double start = lua_tonumber(stack, 1);
	double duration = lua_tonumber(stack, 2);
	int r = lua_tonumber(stack, 3);
	int g = lua_tonumber(stack, 4);
	int b = lua_tonumber(stack, 5);
	int a = lua_tonumber(stack, 6);

	engine->fade(start, duration, al_map_rgba(r, g, b, a));

	return 0;
}

static int c_get_camera_position(lua_State *stack)
{
	Area_Loop *al;
	Battle_Loop *bl;
	double x = 0;
	double y = 0;

	if ((al = GET_AREA_LOOP)) {
		General::Point<float> t = al->get_area()->get_top();
		x = t.x;
		y = t.y;
	}
	else if ((bl = GET_BATTLE_LOOP)) {
		int _x, _y;
		bl->get_area_offset(&_x, &_y);
		x = _x;
		y = _y;
	}

	lua_pushnumber(stack, x);
	lua_pushnumber(stack, y);

	return 2;
}

static int c_set_camera_offset(lua_State *stack)
{
	double x = lua_tonumber(stack, 1);
	double y = lua_tonumber(stack, 2);

	Area_Loop *al;
	Battle_Loop *bl;

	if ((al = GET_AREA_LOOP)) {
		al->get_area()->_offset = General::Point<float>(x, y);
	}
	else if ((bl = GET_BATTLE_LOOP)) {
		bl->_offset = General::Point<float>(x, y);
	}

	return 0;
}

static int c_get_camera_offset(lua_State *stack)
{
	double x = 0;
	double y = 0;

	Area_Loop *al;
	Battle_Loop *bl;

	if ((al = GET_AREA_LOOP)) {
		x = al->get_area()->_offset.x;
		y = al->get_area()->_offset.y;
	}
	else if ((bl = GET_BATTLE_LOOP)) {
		x = bl->_offset.x;
		y = bl->_offset.y;
	}

	lua_pushnumber(stack, x);
	lua_pushnumber(stack, y);

	return 2;
}

static int c_milestone_is_complete(lua_State *stack)
{
	const char *name = lua_tostring(stack, 1);

	lua_pushboolean(stack, engine->milestone_is_complete(name));

	return 1;
}

static int c_set_milestone_complete(lua_State *stack)
{
	const char *name = lua_tostring(stack, 1);
	bool complete = lua_toboolean(stack, 2);

	engine->set_milestone_complete(name, complete);

	return 0;
}

static int c_remove_entity(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Entity *e = NULL;

	Area_Loop *al = GET_AREA_LOOP;
	if (al) {
		e = al->get_area()->get_entity(id);
	}
	else {
		Battle_Loop *bl = GET_BATTLE_LOOP;
		if (bl) {
			e = bl->get_entity(id);
		}
	}

	if (e) {
		e->set_delete_me(true);
	}

	return 0;
}

static int c_remove_particle(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Particle::remove_particle(id);

	return 0;
}

static int c_ai_get(lua_State *stack)
{
	int entity_id = lua_tonumber(stack, 1);
	std::string cmd = lua_tostring(stack, 2);

	Battle_Loop *loop = GET_BATTLE_LOOP;
	if (loop) {
		Battle_Entity *e = dynamic_cast<Battle_Entity *>(loop->get_entity(entity_id));
		if (e) {
			lua_State *lua_state;
			lua_state = e->get_lua_state();
			return loop->ai_get(lua_state, entity_id, cmd);
		}
	}

	return 0;
}

static int c_add_particle_group(lua_State *stack)
{
	std::string type = lua_tostring(stack, 1);
	int layer = lua_tonumber(stack, 2);
	int alignment = lua_tonumber(stack, 3);
	
	std::vector<std::string> bitmap_names;
	
	for (int i = 4; i <= lua_gettop(stack); i++) {
		bitmap_names.push_back(lua_tostring(stack, i));
	}

	int id = engine->add_particle_group(type, layer, alignment, bitmap_names);
	lua_pushnumber(stack, id);
	
	return 1;
}

static int c_delete_particle_group(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	engine->delete_particle_group(id);

	return 0;
}

static int c_get_particle_group_blackboard(lua_State *stack)
{
	int group_id = lua_tonumber(stack, 1);
	int slot = lua_tonumber(stack, 2);

	Particle::Particle_Group *pg = engine->get_particle_group(group_id);
	if (pg) {
		lua_pushnumber(stack, pg->data[slot]);
		return 1;
	}

	return 0;
}

static int c_set_particle_group_blackboard(lua_State *stack)
{
	int group_id = lua_tonumber(stack, 1);
	int slot = lua_tonumber(stack, 2);
	float value = lua_tonumber(stack, 3);

	Particle::Particle_Group *pg = engine->get_particle_group(group_id);
	if (pg) {
		pg->data[slot] = value;
	}

	return 0;
}

static int c_get_particle_group_alignment(lua_State *stack)
{
	int group_id = lua_tonumber(stack, 1);

	Particle::Particle_Group *pg = engine->get_particle_group(group_id);
	if (pg) {
		lua_pushnumber(stack, pg->alignment);
		return 1;
	}

	return 0;
}

static int c_set_particle_group_alignment(lua_State *stack)
{
	int group_id = lua_tonumber(stack, 1);
	int alignment = lua_tonumber(stack, 2);

	Particle::Particle_Group *pg = engine->get_particle_group(group_id);
	if (pg) {
		pg->alignment = alignment;
	}

	return 0;
}

static int c_add_particle(lua_State *stack)
{
	int group = lua_tonumber(stack, 1);
	int width = lua_tonumber(stack, 2);
	int height = lua_tonumber(stack, 3);
	float r = lua_tonumber(stack, 4);
	float g = lua_tonumber(stack, 5);
	float b = lua_tonumber(stack, 6);
	float a = lua_tonumber(stack, 7);
	int bitmap_index = lua_tonumber(stack, 8);
	int hit_dir = lua_tonumber(stack, 9);
	bool right = lua_toboolean(stack, 10);
	bool hard_hitting = lua_toboolean(stack, 11);

	Particle::Particle *p = Particle::add_particle(
		group,
		width, height,
		al_map_rgba_f(r, g, b, a),
		bitmap_index,
		hit_dir,
		right,
		hard_hitting
	);

	lua_pushnumber(stack, p->get_id());

	return 1;
}

static int c_get_particle_position(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		General::Point<float> pos = p->get_position();
		lua_pushnumber(stack, pos.x);
		lua_pushnumber(stack, pos.y);
		return 2;
	}
	return 0;
}

static int c_set_particle_position(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	float x = lua_tonumber(stack, 2);
	float y = lua_tonumber(stack, 3);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		p->set_position(General::Point<float>(x, y));
	}
	return 0;
}

static int c_get_particle_size(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		lua_pushnumber(stack, p->width);
		lua_pushnumber(stack, p->height);
		return 2;
	}
	return 0;
}

static int c_set_particle_size(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	float width = lua_tonumber(stack, 2);
	float height = lua_tonumber(stack, 3);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		p->width = width;
		p->height = height;
	}
	return 0;
}

static int c_get_particle_tint(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		lua_pushnumber(stack, p->tint.r);
		lua_pushnumber(stack, p->tint.g);
		lua_pushnumber(stack, p->tint.b);
		lua_pushnumber(stack, p->tint.a);
		return 4;
	}
	return 0;
}

static int c_set_particle_tint(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	float r = lua_tonumber(stack, 2);
	float g = lua_tonumber(stack, 3);
	float b = lua_tonumber(stack, 4);
	float a = lua_tonumber(stack, 5);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		p->tint = al_map_rgba_f(r, g, b, a);
	}
	return 0;
}

static int c_set_particle_draw_offset(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int x = lua_tonumber(stack, 2);
	int y = lua_tonumber(stack, 3);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		p->draw_offset = General::Point<int>(x, y);
	}
	return 0;
}

static int c_get_particle_bitmap_index(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		lua_pushnumber(stack, p->bitmap_index);
		return 1;
	}
	return 0;
}

static int c_set_particle_bitmap_index(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int index = lua_tonumber(stack, 2);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		p->bitmap_index = index;
	}
	return 0;
}

static int c_set_particle_hidden(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool hidden = lua_toboolean(stack, 2);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		p->hidden = hidden;
	}
	return 0;
}

static int c_get_particle_hidden(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		lua_pushboolean(stack, p->hidden);
		return 1;
	}
	return 0;
}

static int c_set_particle_damage(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int damage = lua_tonumber(stack, 2);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		p->damage = damage;
	}
	return 0;
}

static int c_get_particle_damage(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		lua_pushnumber(stack, p->damage);
		return 1;
	}
	return 0;
}

static int c_get_particle_bitmap_size(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int bitmap_index = lua_tonumber(stack, 2);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		ALLEGRO_BITMAP *bmp = p->particle_group->bitmaps[bitmap_index]->bitmap;
		lua_pushnumber(stack, al_get_bitmap_width(bmp));
		lua_pushnumber(stack, al_get_bitmap_height(bmp));
		return 2;
	}
	return 0;
}

static int c_get_particle_blackboard(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int slot = lua_tonumber(stack, 2);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		lua_pushnumber(stack, p->data[slot]);
		return 1;
	}
	return 0;
}

static int c_set_particle_blackboard(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int slot = lua_tonumber(stack, 2);
	float value = lua_tonumber(stack, 3);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		p->data[slot] = value;
	}
	return 0;
}

static int c_get_particle_group_id(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		lua_pushnumber(stack, p->group_id);
		return 1;
	}
	return 0;
}

static int c_set_battle_entity_attacking(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool attacking = lua_toboolean(stack, 2);

	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(
			bl->get_entity(id)
		);
		if (be) {
			be->set_attacking(attacking);
		}
	}

	return 0;
}

static int c_set_battle_entity_sub_animation(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	std::string sub_name = lua_tostring(stack, 2);

	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(
			bl->get_entity(id)
		);
		if (be) {
			Skeleton::Skeleton *skel = be->get_skeleton();
			if (skel) {
				skel->set_curr_anim(sub_name);
			}
			else {
				be->get_animation_set()->set_sub_animation(sub_name);
			}
		}
	}

	return 0;
}

static int c_reset_battle_entity_animation(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(
			bl->get_entity(id)
		);
		if (be) {
			Skeleton::Skeleton *skel = be->get_skeleton();
			if (skel) {
				skel->reset_current_animation();
			}
			else {
				be->get_animation_set()->reset();
			}
		}
	}

	return 0;
}

static int c_set_battle_entity_flying(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool flying = lua_toboolean(stack, 2);

	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(
			bl->get_entity(id)
		);
		if (be) {
			be->set_flying_entity(flying);
		}
	}

	return 0;
}

static int c_t(lua_State *stack)
{
	const char *tag = lua_tostring(stack, 1);

	lua_pushstring(stack, t(tag));

	return 1;
}

static int c_set_area_loop_input_paused(lua_State *stack)
{
	bool paused = lua_toboolean(stack, 1);

	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		l->set_input_paused(paused);
	}

	return 0;
}

static int c_stop_entity(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	stop_entity(id);

	return 0;
}

static int c_hold_milestones(lua_State *stack)
{
	bool hold = lua_toboolean(stack, 1);

	engine->hold_milestones(hold);

	return 0;
}

static int c_checkcoll_line_player(lua_State *stack)
{
	float x1 = lua_tonumber(stack, 1);
	float y1 = lua_tonumber(stack, 2);
	float x2 = lua_tonumber(stack, 3);
	float y2 = lua_tonumber(stack, 4);

	bool result = false;
	int id = -1;
	
	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		std::vector<Battle_Entity *> &entities = bl->get_entities();
		for (size_t j = 0; j < entities.size(); j++) {
			Battle_Player *bp = dynamic_cast<Battle_Player *>(entities[j]);
			if (bp) {
				std::vector<Bones::Bone> bones;
				bp->collidable_get_bones(bones);
				for (size_t i = 0; i < bones.size(); i++) {
					std::vector< General::Point<float> > this_outline = bones[i].get_outline();
					std::vector<Triangulate::Triangle> this_triangles = bones[i].get();
					bool b = checkcoll_line_polygon(
						General::Point<float>(x1, y1),
						General::Point<float>(x2, y2),
						this_outline,
						bp->get_position()
					);
					if (b) {
						result = true;
						id = bp->get_id();
						break;
					}
				}
			}
			if (result) {
				break;
			}
		}
	}

	lua_pushboolean(stack, result);

	if (result) {
		lua_pushnumber(stack, id);
		return 2;
	}

	return 1;
}

static int c_entity_animation_is_finished(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	
	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		Area_Manager *area = l->get_area();
		Map_Entity *e = area->get_entity(id);
		Animation_Set *anim = e->get_animation_set();
		lua_pushboolean(stack, anim->get_current_animation()->is_finished());
		return 1;
	}

	return 0;
}

static int c_get_time(lua_State *stack)
{
	lua_pushnumber(stack, al_get_time());
	return 1;
}

static int c_set_entity_right(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool right = lua_toboolean(stack, 2);
	
	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			be->set_facing_right(right);
		}
	}
	else {
		Area_Loop *l = GET_AREA_LOOP;
		if (l) {
			Map_Entity *e = dynamic_cast<Map_Entity *>(l->get_area()->get_entity(id));
			if (e) {
				// This function is weird
				if (right) {
					e->set_facing_right(General::DIR_E);
				}
				else {
					e->set_facing_right(General::DIR_W);
				}
			}
		}
	}
	
	return 0;
}

static int c_get_entity_right(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	
	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = l->get_entity(id);
		if (be) {
			lua_pushboolean(stack, be->is_facing_right());
			return 1;
		}
	}
	else {
		Area_Loop *l = GET_AREA_LOOP;
		if (l) {
			Map_Entity *e = l->get_area()->get_entity(id);
			if (e) {
				lua_pushboolean(stack, e->is_facing_right());
				return 1;
			}
		}
	}
	
	return 0;
}

static int c_get_entity_animation_length(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	const char *anim_name = lua_tostring(stack, 2);
	
	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		Battle_Entity *e = dynamic_cast<Battle_Entity *>(bl->get_entity(id));
		if (e) {
			lua_pushnumber(stack, e->get_animation_set()->get_length(anim_name));
			return 1;
		}
	}
	
	return 0;
}

static int c_start_map(lua_State *stack)
{
	store_battle_attributes(GET_AREA_LOOP->get_players());

	std::string start_place = lua_tostring(stack, 1);

	Map_Loop *ml = new Map_Loop(start_place);

	std::vector<Loop *> loops;
	loops.push_back(ml);

	engine->set_loops(loops, true);

	return 0;
}

static int c_add_map_location(lua_State *stack)
{
	Map_Loop *map = (Map_Loop *)lua_touserdata(stack, 1);
	std::string name = lua_tostring(stack, 2);
	int x = lua_tonumber(stack, 3);
	int y = lua_tonumber(stack, 4);

	map->add_location(name, General::Point<int>(x, y));

	return 0;
}

static int c_set_map_neighbors(lua_State *stack)
{
	Map_Loop *map = (Map_Loop *)lua_touserdata(stack, 1);
	std::string name = lua_tostring(stack, 2);
	std::string left = lua_tostring(stack, 3);
	std::string right = lua_tostring(stack, 4);
	std::string up = lua_tostring(stack, 5);
	std::string down = lua_tostring(stack, 6);

	map->set_location_neighbors(name, left, right, up, down);

	return 0;
}

static int c_get_particle_angle(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		lua_pushnumber(stack, p->angle);
		return 1;
	}
	return 0;
}

static int c_set_particle_angle(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	float angle = lua_tonumber(stack, 2);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		p->angle = angle;
	}
	return 0;
}

static int c_get_particle_scale(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		lua_pushnumber(stack, p->xscale);
		lua_pushnumber(stack, p->yscale);
		return 2;
	}
	return 0;
}

static int c_set_particle_scale(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	float xscale = lua_tonumber(stack, 2);
	float yscale = lua_tonumber(stack, 3);
	Particle::Particle *p = Particle::get_particle(id);
	if (p) {
		p->xscale = xscale;
		p->yscale = yscale;
	}
	return 0;
}

static int c_set_particle_bullet_time(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool value = lua_toboolean(stack, 2);
	Particle::Particle *p = Particle::get_particle(id);
	if (value) {
		p->bullet_time_len = cfg.screen_w/4.0f;
	}
	else {
		p->bullet_time_len = 0.0f;
	}

	return 0;
}

static int c_get_battle_width(lua_State *stack)
{
	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		lua_pushnumber(stack, bl->get_width());
		return 1;
	}
	
	return 0;
}

static int c_get_battle_height(lua_State *stack)
{
	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		lua_pushnumber(stack, bl->get_height());
		return 1;
	}
	
	return 0;
}

static int c_set_wander_pause_times(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	double min = lua_tonumber(stack, 2);
	double max = lua_tonumber(stack, 3);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area) {
			Map_Entity *entity = area->get_entity(id);
			if (entity) {
				Character_Map_Entity *character = dynamic_cast<Character_Map_Entity *>(entity);
				if (character) {
					Character_Role *role = character->get_role();
					Wander_Character_Role *wander = dynamic_cast<Wander_Character_Role *>(role);
					if (wander) {
						wander->set_pause_times(min, max);
					}
				}
			}
		}
	}

	return 0;
}
	
static int c_set_wander_minimum_move_distance(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int distance = lua_tonumber(stack, 2);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area) {
			Map_Entity *entity = area->get_entity(id);
			if (entity) {
				Character_Map_Entity *character = dynamic_cast<Character_Map_Entity *>(entity);
				if (character) {
					Character_Role *role = character->get_role();
					Wander_Character_Role *wander = dynamic_cast<Wander_Character_Role *>(role);
					if (wander) {
						wander->set_minimum_move_distance(distance);
					}
				}
			}
		}
	}

	return 0;
}
	
static int c_get_work_bitmap(lua_State *stack)
{
	lua_pushlightuserdata(stack, engine->get_work_bitmap());
	return 1;
}

static int c_set_target_bitmap(lua_State *stack)
{
	Wrap::Bitmap *bmp = (Wrap::Bitmap *)lua_touserdata(stack, 1);
	al_set_target_bitmap(bmp->bitmap);
	return 0;
}

static int c_set_target_allegro_bitmap(lua_State *stack)
{
	ALLEGRO_BITMAP *bmp = (ALLEGRO_BITMAP *)lua_touserdata(stack, 1);
	al_set_target_bitmap(bmp);
	return 0;
}

static int c_get_target_bitmap(lua_State *stack)
{
	lua_pushlightuserdata(stack, al_get_target_bitmap());
	return 1;
}

static int c_load_video(lua_State *stack)
{
	std::string dirname = lua_tostring(stack, 1);

	Video_Player *vp = new Video_Player(dirname, 5);

	lua_pushlightuserdata(stack, vp);

	return 1;
}

static int c_start_video(lua_State *stack)
{
	Video_Player *vp = (Video_Player *)lua_touserdata(stack, 1);

	vp->start();

	return 0;
}

static int c_update_video(lua_State *stack)
{
	Video_Player *vp = (Video_Player *)lua_touserdata(stack, 1);

	vp->update();

	return 0;
}

static int c_set_video_offset(lua_State *stack)
{
	Video_Player *vp = (Video_Player *)lua_touserdata(stack, 1);
	float x = lua_tonumber(stack, 2);
	float y = lua_tonumber(stack, 3);

	vp->set_offset(General::Point<float>(x, y));

	return 0;
}

static int c_draw_video(lua_State *stack)
{
	Video_Player *vp = (Video_Player *)lua_touserdata(stack, 1);

	lua_pushboolean(stack, vp->draw());

	return 1;
}

static int c_get_video_frame_num(lua_State *stack)
{
	Video_Player *vp = (Video_Player *)lua_touserdata(stack, 1);

	lua_pushnumber(stack, vp->get_current_frame_num());

	return 1;
}

static int c_destroy_video(lua_State *stack)
{
	Video_Player *vp = (Video_Player *)lua_touserdata(stack, 1);
	delete vp;
	return 0;
}

static int c_create_shader(lua_State *stack)
{
	std::string name = lua_tostring(stack, 1);

	lua_pushlightuserdata(stack, Shader::get(name));

	return 1;
}

static int c_set_shader_float(lua_State *stack)
{
	const char *name = lua_tostring(stack, 1);
	float f = lua_tonumber(stack, 2);

	al_set_shader_float(name, f);

	return 0;
}

static int c_use_shader(lua_State *stack)
{
	Wrap::Shader *shader = (Wrap::Shader *)lua_touserdata(stack, 1);

	Shader::use(shader);

	return 0;
}

static int c_destroy_shader(lua_State *stack)
{
	Wrap::Shader *shader = (Wrap::Shader *)lua_touserdata(stack, 1);

	Shader::destroy(shader);

	return 0;
}

static int c_set_area_swiping_in(lua_State *stack)
{
	bool swiping = lua_toboolean(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		loop->set_swiping_in(swiping);
	}

	return 0;
}

static int c_get_entity_name(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		Area_Manager *area = l->get_area();
		Map_Entity *e = area->get_entity(id);
		lua_pushstring(stack, e->get_name().c_str());
		return 1;
	}
	else {
		Battle_Loop *bl = GET_BATTLE_LOOP;
		Battle_Entity *e = dynamic_cast<Battle_Entity *>(bl->get_entity(id));
		if (e) {
			lua_pushstring(stack, e->get_name().c_str());
			return 1;
		}
	}

	return 0;
}

static int c_push_entity_to_front(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Area_Loop *al = GET_AREA_LOOP;
	if (al) {
		Area_Manager *a = al->get_area();
		a->reposition_entity_in_vector(id, 0);
	}
	else {
		Battle_Loop *l = GET_BATTLE_LOOP;
		if (l) {
			l->reposition_entity_in_vector(id, 0);
		}
	}

	return 0;
}

static int c_push_entity_to_back(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Area_Loop *al = GET_AREA_LOOP;
	if (al) {
		Area_Manager *a = al->get_area();
		a->reposition_entity_in_vector(id, -1);
	}
	else {
		Battle_Loop *l = GET_BATTLE_LOOP;
		if (l) {
			l->reposition_entity_in_vector(id, -1);
		}
	}

	return 0;
}

static int c_get_low_graphics(lua_State *stack)
{
	lua_pushboolean(stack, cfg.low_graphics);
	return 1;
}

static int c_start_battle(lua_State *stack)
{
	std::string level = lua_tostring(stack, 1);
	std::string script = lua_tostring(stack, 2);
	bool boss_battle = lua_toboolean(stack, 3);
	std::vector<std::string> enemies;

	for (int i = 4; i <= lua_gettop(stack); i++) {
		enemies.push_back(lua_tostring(stack, i));
	}

	Enemy_Avatar *enemy_avatar = new Enemy_Avatar(level, script, boss_battle, "null", enemies);
	Area_Loop *area_loop = GET_AREA_LOOP;
	area_loop->set_was_boss_battle(boss_battle);
	Area_Manager *area = area_loop->get_area();
	start_battle(enemy_avatar, true, area_loop->get_last_battle_screenshot(), area_loop->get_last_used_player_in_battle());
	area->set_early_break(true);

	return 0;
}

static int c_set_entity_stationary(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool stationary = lua_toboolean(stack, 2);

	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		Area_Manager *area = l->get_area();
		Map_Entity *e = area->get_entity(id);
		if (e) {
			e->set_stationary(stationary);
		}
	}

	return 0;
}

static int c_get_num_players(lua_State *stack)
{
	Area_Loop *al = GET_AREA_LOOP;
	if (al) {
		lua_pushnumber(stack, al->get_players().size());
		return 1;
	}
	else {
		Battle_Loop *bl = GET_BATTLE_LOOP;
		if (bl) {
			int num = 0;
			std::vector<Battle_Entity *> entities = bl->get_entities();
			for (size_t i = 0; i < entities.size(); i++) {
				Battle_Entity *e = entities[i];
				if (e) {
					if (General::is_hero(e->get_name())) {
						num++;
					}
				}
			}
			lua_pushnumber(stack, num);
			return 1;
		}
	}

	return 0;
}

static int c_get_player_id(lua_State *stack)
{
	int number = lua_tonumber(stack, 1);

	Area_Loop *al = GET_AREA_LOOP;
	if (al) {
		if (number == 0) {
			lua_pushnumber(stack, 0);
		}
		else {
			lua_pushnumber(stack, al->get_player_npcs()[number-1]->get_id());
		}
		return 1;
	}
	else {
		Battle_Loop *bl = GET_BATTLE_LOOP;
		if (bl) {
			int num = 0;
			std::vector<Battle_Entity *> entities = bl->get_entities();
			for (size_t i = 0; i < entities.size(); i++) {
				Battle_Player *p = dynamic_cast<Battle_Player *>(entities[i]);
				if (p) {
					if (num == number) {
						lua_pushnumber(stack, p->get_id());
						return 1;
					}
					num++;
				}
			}
		}
	}

	return 0;
}

static int c_set_entity_role_paused(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool paused = lua_toboolean(stack, 2);

	Area_Loop *al = GET_AREA_LOOP;
	if (al) {
		Area_Manager *area = al->get_area();
		if (area) {
			Map_Entity *e = area->get_entity(id);
			if (e) {
				e->set_role_paused(paused);
			}
		}
	}
	return 0;
}

static int c_add_ladder(lua_State *stack)
{
	int layer = lua_tonumber(stack, 1);
	int x = lua_tonumber(stack, 2);
	int y = lua_tonumber(stack, 3);
	int w = lua_tonumber(stack, 4);
	int h = lua_tonumber(stack, 5);

	Area_Loop  *al = GET_AREA_LOOP;
	if (al) {
		Area_Manager *area = al->get_area();
		area->add_ladder(
			layer,
			General::Point<float>(x, y),
			General::Point<float>(x+w, y+h)
		);
	}

	return 0;
}

static int c_do_whack_a_skunk(lua_State *stack)
{
	std::vector<Loop *> loops;
	Whack_a_Skunk_Loop *l = new Whack_a_Skunk_Loop(1000, 2000);
	l->set_area_loop(GET_AREA_LOOP);
	loops.push_back(l);
	engine->set_loops(loops, false);
	return 0;
}

static int c_do_item_shop(lua_State *stack)
{
	Shop_Loop *sl = new Shop_Loop(true);

	int end = lua_gettop(stack);

	for (int i = 0; i < end; i += 2) {
		std::string item_name = lua_tostring(stack, i+1);
		int price = lua_tonumber(stack, i+2);
		sl->add_item(item_name, price);
	}

	sl->end_inventory();

	std::vector<Loop *> loops;
	loops.push_back(sl);
	engine->set_loops(loops, false);

	return 0;
}

static int c_do_equipment_shop(lua_State *stack)
{
	Shop_Loop *sl = new Shop_Loop(false);
	
	int end = lua_gettop(stack);

	for (int i = 0; i < end; i += 6) {
		std::string name = lua_tostring(stack, i+1);
		int price = lua_tonumber(stack, i+2);
		Equipment::Equipment_Type type = Game_Specific_Globals::get_equipment_type(name);
		if (type == Equipment::WEAPON) {
			Equipment::Weapon w = Game_Specific_Globals::get_weapon_instance(name, 1);
			sl->add_equipment(type, name, w.attack, w.element, w.usable_by, price);
		}
		else if (type == Equipment::ARMOR) {
			Equipment::Armor a = Game_Specific_Globals::get_armor_instance(name);
			sl->add_equipment(type, name, a.defense, a.element, "", price);
		}
		else {
			Equipment::Accessory a = Game_Specific_Globals::get_accessory_instance(name);
			sl->add_equipment(type, name, 0, Equipment::ELEMENT_NONE, "", price);
		}
	}

	sl->end_inventory();

	std::vector<Loop *> loops;
	loops.push_back(sl);
	engine->set_loops(loops, false);

	return 0;
}

// Get lowest y value in battle level
static int c_get_highest_point(lua_State *stack)
{
	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		std::vector< std::vector< General::Point<int> > > &geo = bl->get_geometry();
		int lowest = INT_MAX;
		for (size_t plat = 0; plat < geo.size(); plat++) {
			for (size_t pt = 0; pt < geo[plat].size(); pt++) {
				if (geo[plat][pt].y < lowest) {
					lowest = geo[plat][pt].y;
				}
			}
		}
		lua_pushnumber(stack, lowest);
		return 1;
	}
	return 0;
}

static int c_battle_entity_is_colliding_with_area(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(bl->get_entity(id));
		if (be == NULL) {
			lua_pushboolean(stack, false);
			return 1;
		}
		std::vector<Bones::Bone> bones;
		be->collidable_get_bones(bones);
		General::Size<float> extents = bones[0].get_extents();
		General::Point<float> pos = be->get_position();
		General::Point<float> topleft(
			pos.x - extents.w/2,
			pos.y - extents.h
		);
		General::Point<float> bottomright(
			pos.x + extents.w/2,
			pos.y
		);
		std::vector< std::pair< General::Point<float>, General::Point<float> > > &sector1 = bl->get_sector(pos.x-extents.w/2 / Battle_Loop::SECTOR_SIZE);
		std::vector< std::pair< General::Point<float>, General::Point<float> > > &sector2 = bl->get_sector(pos.x+extents.w/2 / Battle_Loop::SECTOR_SIZE);
		std::vector< std::pair< General::Point<float>, General::Point<float> > > sector;
		sector.insert(sector.end(), sector1.begin(), sector1.end());
		sector.insert(sector.end(), sector2.begin(), sector2.end());
		int sz = sector.size();
		for (int i = 0; i < sz; i++) {
			if (checkcoll_line_box(sector[i].first, sector[i].second, topleft, bottomright)) {
				lua_pushboolean(stack, true);
				return 1;
			}
		}
	}
	
	lua_pushboolean(stack, false);
	return 1;
}

static int c_particle_is_colliding_with_area(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		Particle::Particle *p = Particle::get_particle(id);
		if (p == NULL) {
			lua_pushboolean(stack, false);
			return 1;
		}
		General::Point<float> pos = p->get_position();
		General::Point<int> offset;
		General::Size<int> size;
		p->collidable_get_box(offset, size);
		General::Point<float> topleft(pos.x+offset.x-size.w/2, pos.y+offset.y-size.h/2 + General::BOTTOM_SPRITE_PADDING);
		General::Point<float> bottomright(topleft.x+size.w, topleft.y+size.h);
		std::vector< std::pair< General::Point<float>, General::Point<float> > > &sector1 = bl->get_sector(topleft.x / Battle_Loop::SECTOR_SIZE);
		std::vector< std::pair< General::Point<float>, General::Point<float> > > &sector2 = bl->get_sector(bottomright.x / Battle_Loop::SECTOR_SIZE);
		std::vector< std::pair< General::Point<float>, General::Point<float> > > sector;
		sector.insert(sector.end(), sector1.begin(), sector1.end());
		sector.insert(sector.end(), sector2.begin(), sector2.end());
		int sz = sector.size();
		for (int i = 0; i < sz; i++) {
			if (checkcoll_line_box(sector[i].first, sector[i].second, topleft, bottomright)) {
				lua_pushboolean(stack, true);
				return 1;
			}
		}
	}
	
	lua_pushboolean(stack, false);
	return 1;
}

static int c_add_battle_enemy(lua_State *stack)
{
	std::string name = lua_tostring(stack, 1);

	Battle_Loop *bl = GET_BATTLE_LOOP;

	Battle_Enemy *be = new Battle_Enemy(bl, name);
	be->construct();
	be->set_id(bl->get_next_id());

	bl->add_entity(be);

	lua_pushnumber(stack, be->get_id());

	return 1;
}

static int c_add_battle_entity(lua_State *stack)
{
	std::string name = lua_tostring(stack, 1);

	Battle_Loop *bl = GET_BATTLE_LOOP;

	Battle_Entity *be = new Battle_Entity(bl, name);
	be->construct();
	be->set_id(bl->get_next_id());

	bl->add_entity(be);

	lua_pushnumber(stack, be->get_id());

	return 1;
}

static int c_set_enemy_aggressiveness(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int aggressiveness = lua_tonumber(stack, 2);

	Battle_Loop *bl = GET_BATTLE_LOOP;

	Battle_Enemy *e = dynamic_cast<Battle_Enemy *>(bl->get_entity(id));
	if (e) {
		e->get_ai()->set_aggressiveness(aggressiveness);
	}

	return 0;
}

static int c_get_battle_entity_hit_something_this_attack(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *bl = GET_BATTLE_LOOP;

	Battle_Enemy *e = dynamic_cast<Battle_Enemy *>(bl->get_entity(id));
	if (e) {
		lua_pushboolean(stack, e->get_hit_something_this_attack());
	}
	else {
		lua_pushboolean(stack, false);
	}

	return 1;
}

static int c_give_equipment(lua_State *stack)
{
	Equipment::Equipment_Type type = (Equipment::Equipment_Type)((int)lua_tonumber(stack, 1));
	std::string name = lua_tostring(stack, 2);
	int quantity = lua_tonumber(stack, 3);

	Game_Specific_Globals::give_equipment(type, name, quantity);

	return 0;
}

static int c_give_items(lua_State *stack)
{
	std::string name = lua_tostring(stack, 1);
	int quantity = lua_tonumber(stack, 2);

	Game_Specific_Globals::give_items(name, quantity);

	return 0;
}

static int c_get_random_start_platform(lua_State *stack)
{
	int min_entity_spacing = lua_tonumber(stack, 1);
	int min_edge_spacing = lua_tonumber(stack, 2);

	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		General::Point<float> p = bl->choose_random_start_platform(min_entity_spacing, min_edge_spacing);
		lua_pushnumber(stack, p.x);
		lua_pushnumber(stack, p.y);
		return 2;
	}
	return 0;
}

static int c_set_entity_immovable(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool immovable = lua_toboolean(stack, 2);
	
	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			be->set_immovable(immovable);
		}
	}
	
	return 0;
}

static int c_get_entity_immovable(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	
	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			lua_pushboolean(stack, be->is_immovable());
			return 1;
		}
	}
	
	return 0;
}

static int c_apply_force(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	Battle_Entity::Hit_Direction dir = (Battle_Entity::Hit_Direction)((int)lua_tonumber(stack, 2));
	bool right = lua_toboolean(stack, 3);
	float force_x = lua_tonumber(stack, 4);
	float force_y = lua_tonumber(stack, 5);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			be->apply_force(dir, right, General::Point<float>(force_x, force_y));
		}
	}

	return 0;
}

static int c_entity_is_on_ground(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			lua_pushnumber(stack, be->is_on_ground());
			return 1;
		}
	}

	return 0;
}

static int c_entity_is_solid_with_area(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	Area_Manager *area = loop->get_area();
	Map_Entity *entity = area->get_entity(id);
	if (entity) {
		lua_pushboolean(stack, entity->is_solid_with_area());
	}
	else {
		lua_pushboolean(stack, false);
	}

	return 1;
}

static int c_entity_is_solid_with_entities(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Area_Loop *loop = GET_AREA_LOOP;
	Area_Manager *area = loop->get_area();
	Map_Entity *entity = area->get_entity(id);
	if (entity) {
		lua_pushboolean(stack, entity->is_solid_with_entities());
	}
	else {
		lua_pushboolean(stack, false);
	}

	return 1;
}

static int c_entity_is_visible(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Map_Entity *entity = NULL;
	bool visible = false;
	Area_Loop *loop = GET_AREA_LOOP;
	if (loop) {
		Area_Manager *area = loop->get_area();
		if (area) {
			entity = area->get_entity(id);
		}
		if (entity) {
			visible = entity->is_visible();
		}
	}

	lua_pushboolean(stack, visible);

	return 1;
}

static int c_set_should_attack(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool should_attack = lua_toboolean(stack, 2);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			be->get_ai()->set_should_attack(should_attack);
		}
	}

	return 0;
}

static int c_set_battle_entity_jumping(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			be->set_on_ground(false);
			be->set_check_platform(true);
			be->set_platform(-1);
			be->set_jumping(true);
			be->set_jumps(2);
			General::Point<float> pos = be->get_position();
			be->set_position(General::Point<float>(pos.x, pos.y-1.0f));
		}
	}

	return 0;
}

static int c_get_height_from_ground(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			lua_pushnumber(stack, be->get_height_from_ground());
			return 1;
		}
	}

	return 0;
}

static int c_set_clipping_rectangle(lua_State *stack)
{
	int x = lua_tonumber(stack, 1);
	int y = lua_tonumber(stack, 2);
	int w = lua_tonumber(stack, 3);
	int h = lua_tonumber(stack, 4);
	
	General::set_clipping_rectangle(x, y, w, h);

	return 0;
}

static int c_get_clipping_rectangle(lua_State *stack)
{
	int x, y, w, h;

	al_get_clipping_rectangle(&x, &y, &w, &h);

	lua_pushnumber(stack, x);
	lua_pushnumber(stack, y);
	lua_pushnumber(stack, w);
	lua_pushnumber(stack, h);

	return 4;
}

static int c_set_battle_entity_unhittable(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool unhittable = lua_toboolean(stack, 2);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			be->set_unhittable(unhittable);
		}
	}

	return 0;
}

static int c_update_entity_animation(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			be->get_animation_set()->update();
		}
	}

	return 0;
}

static int c_get_hp(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			lua_pushnumber(stack, be->get_attributes().hp);
			return 1;
		}
	}
	return 0;
}

static int c_set_hp(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int hp = lua_tonumber(stack, 2);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			be->get_attributes().hp = hp;
		}
	}
	return 0;
}

static int c_drain_magic(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int num = lua_tonumber(stack, 2);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			if (be->get_attributes().equipment.accessory.name != "REDRING") {
				int mp = be->get_attributes().mp;
				mp -= num;
				if (mp < 0) mp = 0;
				be->get_attributes().mp = mp;
			}
		}
	}
	return 0;
}

static int c_get_mp(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			lua_pushnumber(stack, be->get_attributes().mp);
			return 1;
		}
	}
	return 0;
}

static int c_get_ability_cost(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	std::string ability = lua_tostring(stack, 2);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			lua_pushnumber(stack, Game_Specific_Globals::magic_cost(be->get_attributes(), ability));
			return 1;
		}
	}
	return 0;
}

static int c_get_distance_from_nearest_edge(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		lua_pushnumber(stack, l->get_distance_from_nearest_edge(id));
		return 1;
	}
	return 0;
}

static int c_add_no_enemy_zone(lua_State *stack)
{
	int x1 = lua_tonumber(stack, 1);
	int y1 = lua_tonumber(stack, 2);
	int x2 = lua_tonumber(stack, 3);
	int y2 = lua_tonumber(stack, 4);

	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		Area_Manager *area = l->get_area();
		General::Rectangle<int> r(x1, y1, x2, y2);
		area->add_no_enemy_zone(r);
	}

	return 0;
}

static int c_calculate_toss(lua_State *stack)
{
	float sx = lua_tonumber(stack, 1);
	float sy = lua_tonumber(stack, 2);
	float dx = lua_tonumber(stack, 3);
	float dy = lua_tonumber(stack, 4);
	float percent = lua_tonumber(stack, 5);

	float x, y;

	General::calculate_toss(sx, sy, dx, dy, percent, &x, &y);

	lua_pushnumber(stack, x);
	lua_pushnumber(stack, y);
	
	return 2;
}

static int c_add_crystals(lua_State *stack)
{
	int num = lua_tonumber(stack, 1);

	Game_Specific_Globals::crystals += num;

	return 0;
}

static int c_set_elapsed_time(lua_State *stack)
{
	double elapsed = lua_tonumber(stack, 1);

	Game_Specific_Globals::elapsed_time = elapsed;

	return 0;
}

static int c_add_cash(lua_State *stack)
{
	int amount = lua_tonumber(stack, 1);

	Game_Specific_Globals::cash += amount;

	return 0;
}

static int c_set_cash(lua_State *stack)
{
	int amount = lua_tonumber(stack, 1);

	Game_Specific_Globals::cash = amount;

	return 0;
}

static Saveable get_saveable(lua_State *stack)
{
	Saveable s;
	s.type = (Saveable_Type)((int)lua_tonumber(stack, 1));

	switch (s.type) {
		case SAVE_PLAYER: {
			s.player.layer = lua_tonumber(stack, 2);
			s.player.x = lua_tonumber(stack, 3);
			s.player.y = lua_tonumber(stack, 4);
			break;
		}
		case SAVE_CHOPPABLE: {
			s.choppable.name = lua_tostring(stack, 2);
			s.choppable.layer = lua_tonumber(stack, 3);
			s.choppable.x = lua_tonumber(stack, 4);
			s.choppable.y = lua_tonumber(stack, 5);
			break;
		}
		case SAVE_ENEMY: {
			s.enemy.level = lua_tostring(stack, 2);
			s.enemy.script = lua_tostring(stack, 3);
			s.enemy.layer = lua_tonumber(stack, 4);
			s.enemy.x = lua_tonumber(stack, 5);
			s.enemy.y = lua_tonumber(stack, 6);
			s.enemy.avatar = lua_tostring(stack, 7);
			int num = lua_tonumber(stack, 8);
			for (int i = 0; i < num; i++) {
				std::string name = lua_tostring(stack, 9+i);
				s.enemy.enemies.push_back(name);
			}
			break;
		}
	}

	return s;
}

static int c_restore_item(lua_State *stack)
{
	Saveable s = get_saveable(stack);

	saved_items.push_back(s);

	return 0;
}

static int c_save_item(lua_State *stack)
{
	Saveable s = get_saveable(stack);

	char line[2000];

	switch (s.type) {
		case SAVE_PLAYER: {
			snprintf(
				line,
				2000,
				"restore_item(SAVE_PLAYER, %d, %f, %f)\n",
				s.player.layer,
				s.player.x,
				s.player.y
			);
			break;
		}
		case SAVE_CHOPPABLE: {
			snprintf(
				line,
				2000,
				"restore_item(SAVE_CHOPPABLE, \"%s\", %d, %f, %f)\n",
				s.choppable.name.c_str(),
				s.choppable.layer,
				s.choppable.x,
				s.choppable.y
			);
			break;
		}
		case SAVE_ENEMY: {
			snprintf(
				line,
				2000,
				"restore_item(SAVE_ENEMY, \"%s\", \"%s\", %d, %f, %f, \"%s\"",
				s.enemy.level.c_str(),
				s.enemy.script.c_str(),
				s.enemy.layer,
				s.enemy.x,
				s.enemy.y,
				s.enemy.avatar.c_str()
			);
			snprintf(
				line+strlen(line),
				2000-strlen(line),
				", %d",
				(int)s.enemy.enemies.size()
			);
			for (size_t i = 0; i < s.enemy.enemies.size(); i++) {
				snprintf(
					line+strlen(line),
					2000-strlen(line),
					", \"%s\"",
					s.enemy.enemies[i].c_str()
				);
			}
			snprintf(
				line+strlen(line),
				2000-strlen(line),
				")\n"
			);
			break;
		}
	}

	saved_lua_lines.push_back(line);

	return 0;
}

static int c_set_area_name(lua_State *stack)
{
	saved_area_name = lua_tostring(stack, 1);
	return 0;
}

static int c_get_num_level_save_items(lua_State *stack)
{
	lua_pushnumber(stack, saved_items.size());
	return 1;
}

static int c_get_level_save_item_type(lua_State *stack)
{
	int index = lua_tonumber(stack, 1);
	lua_pushnumber(stack, saved_items[index].type);
	return 1;
}

static int c_get_level_save_item_data(lua_State *stack)
{
	int index = lua_tonumber(stack, 1);

	switch (saved_items[index].type) {
		case SAVE_PLAYER: {
			lua_pushnumber(stack, saved_items[index].player.layer);
			lua_pushnumber(stack, saved_items[index].player.x);
			lua_pushnumber(stack, saved_items[index].player.y);
			return 3;
		}
		case SAVE_CHOPPABLE: {
			lua_pushstring(stack, saved_items[index].choppable.name.c_str());
			lua_pushnumber(stack, saved_items[index].choppable.layer);
			lua_pushnumber(stack, saved_items[index].choppable.x);
			lua_pushnumber(stack, saved_items[index].choppable.y);
			return 4;
		}
		case SAVE_ENEMY: {
			lua_pushstring(stack, saved_items[index].enemy.level.c_str());
			lua_pushstring(stack, saved_items[index].enemy.script.c_str());
			lua_pushnumber(stack, saved_items[index].enemy.layer);
			lua_pushnumber(stack, saved_items[index].enemy.x);
			lua_pushnumber(stack, saved_items[index].enemy.y);
			lua_pushstring(stack, saved_items[index].enemy.avatar.c_str());
			lua_pushnumber(stack, saved_items[index].enemy.enemies.size());
			for (size_t i = 0; i < saved_items[index].enemy.enemies.size(); i++) {
				lua_pushstring(stack, saved_items[index].enemy.enemies[i].c_str());
			}
			return 7 + saved_items[index].enemy.enemies.size();
		}
	}

	return 0;
}

static int c_set_name(lua_State *stack)
{
	int index = lua_tonumber(stack, 1);
	std::string name = lua_tostring(stack, 2);

	battle_attributes[index].first = name;

	return 0;
}

static int c_set_attributes(lua_State *stack)
{
	int index = lua_tonumber(stack, 1);
	battle_attributes[index].second.hp = lua_tonumber(stack, 2) * cfg.difficulty_mult();
	battle_attributes[index].second.max_hp = lua_tonumber(stack, 3) * cfg.difficulty_mult();
	battle_attributes[index].second.mp = lua_tonumber(stack, 4);
	battle_attributes[index].second.max_mp = lua_tonumber(stack, 5);
	battle_attributes[index].second.attack = lua_tonumber(stack, 6);
	battle_attributes[index].second.defense = lua_tonumber(stack, 7);

	return 0;
}

static int c_set_status(lua_State *stack)
{
	int index = lua_tonumber(stack, 1);
	battle_attributes[index].second.status.name = lua_tostring(stack, 2);
	battle_attributes[index].second.status.count = 0;

	return 0;
}

static int c_set_weapon(lua_State *stack)
{
	int index = lua_tonumber(stack, 1);
	std::string name = lua_tostring(stack, 2);

	battle_attributes[index].second.equipment.weapon = Game_Specific_Globals::get_weapon_instance(name, 1);

	return 0;
}

static int c_set_weapon_attachment(lua_State *stack)
{
	int index = lua_tonumber(stack, 1);
	std::string name = lua_tostring(stack, 2);
	int quantity = lua_tonumber(stack, 3);

	battle_attributes[index].second.equipment.weapon.attachments.push_back(Game_Specific_Globals::get_weapon_instance(name, quantity));

	return 0;
}

static int c_set_armor(lua_State *stack)
{
	int index = lua_tonumber(stack, 1);
	std::string name = lua_tostring(stack, 2);
	
	battle_attributes[index].second.equipment.armor = Game_Specific_Globals::get_armor_instance(name);

	return 0;
}

static int c_set_accessory(lua_State *stack)
{
	int index = lua_tonumber(stack, 1);
	std::string name = lua_tostring(stack, 2);

	battle_attributes[index].second.equipment.accessory = Game_Specific_Globals::get_accessory_instance(name);

	return 0;
}

static int c_set_crystals(lua_State *stack)
{
	int index = lua_tonumber(stack, 1);
	battle_attributes[index].second.abilities.abilities[0] = lua_tonumber(stack, 2);
	battle_attributes[index].second.abilities.abilities[1] = lua_tonumber(stack, 3);
	battle_attributes[index].second.abilities.abilities[2] = lua_tonumber(stack, 4);
	battle_attributes[index].second.abilities.hp = lua_tonumber(stack, 5);
	battle_attributes[index].second.abilities.mp = lua_tonumber(stack, 6);

	return 0;
}

static int c_set_selected_abilities(lua_State *stack)
{
	int index = lua_tonumber(stack, 1);
	selected_abilities[index][0] = lua_tostring(stack, 2);
	selected_abilities[index][1] = lua_tostring(stack, 3);
	selected_abilities[index][2] = lua_tostring(stack, 4);
	selected_abilities[index][3] = lua_tostring(stack, 5);

	return 0;
}

static int c_set_battle_entity_attack(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int attack = lua_tonumber(stack, 2);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *be = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (be) {
			be->get_attributes().attack = attack;
		}
	}

	return 0;	
}

static int c_revive_everyone(lua_State *stack)
{
	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		std::vector<Player *> players = l->get_players();
		for (size_t i = 0; i < players.size(); i++) {
			Battle_Attributes &a = players[i]->get_battle_attributes();
			int max_hp = a.max_hp;
			int max_mp = a.max_mp;
			a.status.name = "";
			Game_Specific_Globals::get_accessory_effects(a.equipment.accessory.name, &max_hp, &max_mp, NULL, NULL);
			a.hp = max_hp;
			a.mp = max_mp;
		}
	}
	return 0;
}

static int c_increase_hp(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int amount = lua_tonumber(stack, 2) * cfg.difficulty_mult();

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		std::vector<Battle_Entity *> v = l->get_entities();
		for (size_t i = 0; i < v.size(); i++) {
			Battle_Entity *e = v[i];
			if (e && e->get_id() == id) {
				Battle_Attributes &attr = e->get_attributes();
				if (attr.hp > 0) {
					attr.hp += amount;
					attr.hp = MIN(attr.max_hp, attr.hp);
				}
				break;
			}
		}
	}

	return 0;
}

static int c_set_saved_players(lua_State *stack)
{
	saved_players.clear();
	for (int i = 1; i <= lua_gettop(stack); i++) {
		saved_players.push_back(lua_tostring(stack, i));
	}
	return 0;
}

static int c_set_battle_entity_speed_multiplier(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	float mult = lua_tonumber(stack, 2);

	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		Battle_Entity *e =
			dynamic_cast<Battle_Entity *>(bl->get_entity(id));
		if (e) {
			e->set_speed_multiplier(mult);
		}
	}

	return 0;
}

static int c_add_bisou(lua_State *stack)
{
	Area_Loop *al = GET_AREA_LOOP;
	al->add_bisou();
	return 0;
}

static int c_whack_a_skunk_was_played(lua_State *stack)
{
	Area_Loop *al =  GET_AREA_LOOP;
	if (al) {
		lua_pushboolean(stack, al->whack_a_skunk_was_played());
	}
	else {
		lua_pushboolean(stack, false);
	}

	return 1;
}

static int c_get_whack_a_skunk_score(lua_State *stack)
{
	Area_Loop *al = GET_AREA_LOOP;
	lua_pushnumber(stack, al->get_whack_a_skunk_score());
	return 1;
}

static int c_set_whack_a_skunk_played(lua_State *stack)
{
	bool played = lua_tonumber(stack, 1);
	Area_Loop *al = GET_AREA_LOOP;
	al->set_whack_a_skunk_played(played);
	return 0;
}

static int c_save_boss_save(lua_State *stack)
{
	engine->save_game(-1);
	return 0;
}

static int c_switch_characters(lua_State *stack)
{
	bool sound = lua_toboolean(stack, 1);

	Area_Loop *al = GET_AREA_LOOP;
	if (al) {
		al->switch_characters(sound);
	}

	return 0;
}

static int c_ramp_music_down(lua_State *stack)
{
	double time = lua_tonumber(stack, 1);

	Music::ramp_down(time);

	return 0;
}

static int c_ramp_music_up(lua_State *stack)
{
	double time = lua_tonumber(stack, 1);

	Music::ramp_up(time);

	return 0;
}

static int c_get_player_weapon_name(lua_State *stack)
{
	std::string player = lua_tostring(stack, 1);

	Battle_Loop *l = GET_BATTLE_LOOP;

	if (l) {
		std::vector<Battle_Entity *> entities = l->get_entities();
		for (size_t i = 0; i < entities.size(); i++) {
			Battle_Player *p = dynamic_cast<Battle_Player *>(entities[i]);
			if (p) {
				if (p->get_name() == player) {
					Battle_Attributes attr = p->get_attributes();
					lua_pushstring(stack, attr.equipment.weapon.name.c_str());
					return 1;
				}
			}
		}
	}
	return 0;
}

static int c_set_player_lost_weapon(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool lost = lua_toboolean(stack, 2);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *entity = l->get_entity(id);
		Battle_Player *p = dynamic_cast<Battle_Player *>(entity);
		if (p) {
			p->set_lost_weapon(lost);
		}
	}

	return 0;
}

static int c_add_battle_data(lua_State *stack)
{
	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		std::string name = lua_tostring(stack, 1);
		int size = lua_tonumber(stack, 2);

		int i = 3;
		double *data = new double[1];
		double *old_data = new double[1];

		for (int j = 0; j < size; j++) {
			double d = lua_tonumber(stack, i);
			if (j > 0) {
				memcpy(old_data, data, sizeof(double)*j);
				delete[] data;
				data = new double[j+1];
				memcpy(data, old_data, sizeof(double)*j);
				delete[] old_data;
				old_data = new double[j+1];
			}
			data[j] = d;
			i++;
		}

		delete[] old_data;

		l->add_battle_data(name, size, data);
	}
	return 0;
}

static int c_get_battle_data_size(lua_State *stack)
{
	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		std::string name = lua_tostring(stack, 1);
		lua_pushnumber(stack, l->get_battle_data_size(name));
		return 1;
	}

	return 0;
}

static int c_get_battle_data(lua_State *stack)
{
	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		std::string name = lua_tostring(stack, 1);
		int index = lua_tonumber(stack, 2);
		lua_pushnumber(stack, l->get_battle_data(name, index));
		return 1;
	}

	return 0;
}

static int c_count_battle_entities(lua_State *stack)
{
	std::string name = lua_tostring(stack, 1);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		std::vector<Battle_Entity *> v = l->get_entities();
		int count = 0;
		for (size_t i = 0; i < v.size(); i++) {
			Battle_Entity *e = v[i];
			if (e) {
				if (e->get_name() == name) {
					count++;
				}
			}
		}
		lua_pushnumber(stack, count);
		return 1;
	}

	return 0;
}

static int c_get_entity_bitmap(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *e = l->get_entity(id);
		if (e) {
			Animation_Set *anim_set = e->get_animation_set();
			lua_pushlightuserdata(stack, anim_set->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap());
			return 1;
		}
	}

	return 0;
}

static int c_get_save_state_version(lua_State *stack)
{
	lua_pushnumber(stack, save_state_version);
	return 1;
}

static int c_set_save_state_version(lua_State *stack)
{
	int version = lua_tonumber(stack, 1);
	save_state_version = version;
	return 0;
}

static int c_set_battle_entity_default_layer(lua_State *stack)
{
	int layer = lua_tonumber(stack, 1);

	Battle_Loop *l = GET_BATTLE_LOOP;

	if (l) {
		l->set_entity_layer(layer);
	}

	return 0;
}

static int c_set_battle_entity_layer(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	int layer = lua_tonumber(stack, 2);

	Battle_Loop *l = GET_BATTLE_LOOP;

	if (l) {
		Battle_Entity *e = l->get_entity(id);
		if (e) {
			e->set_layer(layer);
		}
	}

	return 0;
}

static int c_call_on_battle_script(lua_State *stack)
{
	Battle_Loop *l = GET_BATTLE_LOOP;

	if (!l) {
		return 0;
	}

	const char *func = lua_tostring(stack, 1);

	lua_State *battle_stack = l->get_lua_state();
	lua_getglobal(battle_stack, func);

	if (!lua_isfunction(battle_stack, -1)) {
		lua_pop(battle_stack, 1);
		return 0;
	}

	for (int i = 2; i <= lua_gettop(stack); i++) {
		if (lua_isnumber(stack, i)) {
			lua_pushnumber(battle_stack, lua_tonumber(stack, i));
		}
		else if (lua_isstring(stack, i)) {
			lua_pushstring(battle_stack, lua_tostring(stack, i));
		}
		else if (lua_isboolean(stack, i)) {
			lua_pushboolean(battle_stack, lua_toboolean(stack, i));
		}
	}

	if (lua_pcall(battle_stack, lua_gettop(stack)-1, 0, 0) != 0) {
		General::log_message("*** error running function: " + (func == NULL ? "(NULL)" : std::string(func)));
		dump_lua_stack(battle_stack);
	}

	return 0;
}

static int c_call_on_battle_entity_script(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	const char *func = lua_tostring(stack, 2);

	Battle_Loop *loop = GET_BATTLE_LOOP;

	if (loop) {
		Battle_Entity *entity = loop->get_entity(id);

		if (entity) {
			lua_State *entity_stack = entity->get_lua_state();
			lua_getglobal(entity_stack, func);

			if (!lua_isfunction(entity_stack, -1)) {
				lua_pop(entity_stack, 1);
				return 0;
			}

			for (int i = 3; i <= lua_gettop(stack); i++) {
				if (lua_isnumber(stack, i)) {
					lua_pushnumber(entity_stack, lua_tonumber(stack, i));
				}
				else if (lua_isstring(stack, i)) {
					lua_pushstring(entity_stack, lua_tostring(stack, i));
				}
				else if (lua_isboolean(stack, i)) {
					lua_pushboolean(entity_stack, lua_toboolean(stack, i));
				}
			}

			if (lua_pcall(entity_stack, lua_gettop(stack)-2, 0, 0) != 0) {
				General::log_message("*** error running function: " + (func == NULL ? "(NULL)" : std::string(func)));
				dump_lua_stack(entity_stack);
			}
		}
	}

	return 0;
}

static int c_get_num_platforms(lua_State *stack)
{
	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		std::vector< std::vector< General::Point<int> > > &geo = bl->get_geometry();
		lua_pushnumber(stack, geo.size());
		return 1;
	}

	return 0;
}

static int c_set_platform_solid(lua_State *stack)
{
	int platform = lua_tonumber(stack, 1);
	bool solid = lua_toboolean(stack, 2);

	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		std::vector<bool> &v = bl->get_platform_solid();
		v[platform] = solid;
	}

	return 0;
}

static int c_get_skeleton_animation_elapsed_time(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *e = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		Skeleton::Skeleton *skel = e->get_skeleton();
		int curr_anim = skel->get_curr_anim();
		std::vector<Skeleton::Animation *> &v = skel->get_animations();
		Skeleton::Animation *anim = v[curr_anim];
		int elapsed = 0;
		for (int i = 0; i < anim->curr_frame; i++) {
			elapsed += anim->delays[i];
		}
		elapsed += anim->delays[anim->curr_frame];
		lua_pushnumber(stack, elapsed);
		return 1;
	}

	return 0;
}

static int c_set_entity_stops_battle_end(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool stops_battle_end = lua_toboolean(stack, 2);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *e = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (e) {
			e->set_stops_battle_end(stops_battle_end);
		}
	}

	return 0;
}

static int c_get_level_collision(lua_State *stack)
{
	float x = lua_tonumber(stack, 1);
	float y = lua_tonumber(stack, 2);
	float radius = lua_tonumber(stack, 3);

	// FIXME: This does a box check but it's good enough for now

	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		General::Point<float> topleft(x-radius, y-radius);
		General::Point<float> bottomright(x+radius, y+radius);
		std::vector< std::pair< General::Point<float>, General::Point<float> > > &sector1 = bl->get_sector(topleft.x / Battle_Loop::SECTOR_SIZE);
		std::vector< std::pair< General::Point<float>, General::Point<float> > > &sector2 = bl->get_sector(bottomright.x / Battle_Loop::SECTOR_SIZE);
		std::vector< std::pair< General::Point<float>, General::Point<float> > > sector;
		sector.insert(sector.end(), sector1.begin(), sector1.end());
		sector.insert(sector.end(), sector2.begin(), sector2.end());
		int sz = sector.size();
		for (int i = 0; i < sz; i++) {
			General::Point<float> a = sector[i].first;
			General::Point<float> b = sector[i].second;
			if (checkcoll_line_box(a, b, topleft, bottomright)) {
				lua_pushnumber(stack, a.x);
				lua_pushnumber(stack, a.y);
				lua_pushnumber(stack, b.x);
				lua_pushnumber(stack, b.y);
				General::Point<float> result;
				dist_point_line_result(General::Point<float>(x, y), a, b, &result);
				float dist1 = General::distance(a.x, a.y, result.x, result.y);
				float dist2 = General::distance(b.x, b.y, result.x, result.y);
				float p = dist1 / (dist1+dist2); // percent along the line the collision happened at
				lua_pushnumber(stack, p);
				return 5;
			}
		}
	}

	return 0;
}

static int c_set_can_accelerate_quickly(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool accelerates_quickly = lua_toboolean(stack, 2);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *e = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (e) {
			e->set_can_accelerate_quickly(accelerates_quickly);
		}
	}

	return 0;
}

static int c_fade_out(lua_State *stack)
{
	engine->fade_out();
	return 0;
}

static int c_fade_in(lua_State *stack)
{
	engine->fade_in();
	return 0;
}

static int c_inc_num_jumping(lua_State *stack)
{
	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		int n = l->get_num_jumping();
		l->set_num_jumping(n+1);
	}
	return 0;
}

static int c_dec_num_jumping(lua_State *stack)
{
	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		int n = l->get_num_jumping();
		l->set_num_jumping(n-1);
	}
	return 0;
}

static int c_is_item(lua_State *stack)
{
	std::string name = lua_tostring(stack, 1);

	lua_pushboolean(stack, General::is_item(name));

	return 1;
}

static int c_get_entity_inputs(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		Map_Entity *e = l->get_area()->get_entity(id);
		if (e) {
			float *input = e->get_inputs();
			int i;
			for (i = 0; i < Map_Entity::NUM_INPUTS; i++) {
				lua_pushnumber(stack, input[i]);
			}
			return i;
		}
	}

	return 0;
}

static int c_set_entities_slide_on_entity(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);
	bool slide = lua_toboolean(stack, 2);

	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		Map_Entity *e = l->get_area()->get_entity(id);
		if (e) {
			e->set_entities_slide_on_self(slide);
		}
	}

	return 0;
}

static int c_get_entity_speed(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		Map_Entity *e = l->get_area()->get_entity(id);
		if (e) {
			lua_pushnumber(stack, e->get_current_speed());
			return 1;
		}
	}

	return 0;
}

static int c_point_in_area_bounds(lua_State *stack)
{
	int layer = lua_tonumber(stack, 1);
	float x = lua_tonumber(stack, 2);
	float y = lua_tonumber(stack, 3);

	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		lua_pushboolean(stack, l->get_area()->point_collides(layer, General::Point<float>(x, y)));
		return 1;
	}

	return 0;
}

static int c_get_ability_name(lua_State *stack)
{
	int player_id = lua_tonumber(stack, 1);
	bool battle = lua_toboolean(stack, 2);
	int button = lua_tonumber(stack, 3);

	Battle_Loop *bl = GET_BATTLE_LOOP;
	if (bl) {
		Battle_Player *battle_player = bl->get_player(player_id);
		if (battle_player) {
			Player *player = battle_player->get_player();
			if (player) {
				const std::vector<std::string> &v = player->get_selected_abilities(battle, dynamic_cast<Runner_Loop *>(bl), bl->is_cart_battle());
				lua_pushstring(stack, v[button].c_str());
				return 1;
			}
		}
	}

	return 0;
}

static int c_set_battle_was_event(lua_State *stack)
{
	int type = lua_tonumber(stack, 1);
	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		l->set_battle_was_event((Enemy_Avatar_Wander_Character_Role::Battle_Event_Type)type);
	}
	return 0;
}

static int c_is_burrowing(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *l = GET_BATTLE_LOOP;
	if (l) {
		Battle_Entity *e = dynamic_cast<Battle_Entity *>(l->get_entity(id));
		if (e) {
			lua_pushboolean(stack, e->is_burrowing());
			return 1;
		}
	}

	return 0;
}

static int c_start_attack(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		Map_Entity *e = l->get_area()->get_entity(id);
		if (e) {
			Player *p = dynamic_cast<Player *>(e);
			if (p) {
				p->start_attack();
			}
		}
	}

	return 0;
}

// Forces caching of new bitmaps
static int c_redraw(lua_State *stack)
{
	al_clear_to_color(al_map_rgb(0x00, 0x00, 0x00));
	engine->draw_all(engine->get_loops(), false);
	al_flip_display();
	return 0;
}

static int c_kill_enemy(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *loop = GET_BATTLE_LOOP;
	if (loop) {
		Battle_Entity *entity = loop->get_entity(id);
		if (entity) {
			entity->get_attributes().hp = 0;
			engine->play_sample("sfx/enemy_die.ogg");
			Lua::call_lua(entity->get_lua_state(), "die", ">");
			entity->set_death_animation_start(al_get_time());
			if (entity->get_skeleton()) {
				entity->get_skeleton()->set_curr_anim("death");
			}
			else {
				entity->get_animation_set()->set_sub_animation("death");
				entity->get_animation_set()->reset();
			}
			if (entity->get_weapon_animation_set()) {
				entity->get_weapon_animation_set()->set_sub_animation("death");
				entity->get_weapon_animation_set()->reset();
			}
		}
	}

	return 0;
}

static int c_hurt_enemy(lua_State *stack)
{
	int id = lua_tonumber(stack, 1);

	Battle_Loop *loop = GET_BATTLE_LOOP;
	if (loop) {
		Battle_Entity *entity = loop->get_entity(id);
		if (entity) {
			entity->set_hurt(true);
			entity->set_hurt_end(al_get_time() + 0.5);
		}
	}

	return 0;
}

static int c_clear_to_color(lua_State *stack)
{
	float r = lua_tonumber(stack, 1);
	float g = lua_tonumber(stack, 2);
	float b = lua_tonumber(stack, 3);
	float a = lua_tonumber(stack, 4);

	al_clear_to_color(al_map_rgba_f(r, g, b, a));

	return 0;
}

static int c_credits(lua_State *stack)
{
	std::vector<Loop *> loops;
	Credits_Loop *l = new Credits_Loop();
	l->init();
	loops.push_back(l);
	engine->do_blocking_mini_loop(loops, NULL);
	engine->set_done(true);
	return 0;
}

static int c_difficulty(lua_State *stack)
{
	cfg.difficulty = (Configuration::Difficulty)((int)lua_tonumber(stack, 0));
	return 0;
}

static int c_log_message(lua_State *stack)
{
	std::string msg = lua_tostring(stack, 1);

	General::log_message(msg);

	return 0;
}

void register_c_functions(lua_State *lua_state)
{
	#define REGISTER_FUNCTION(name) \
		lua_pushcfunction(lua_state, c_ ## name); \
		lua_setglobal(lua_state, #name);

	REGISTER_FUNCTION(add_entity);
	REGISTER_FUNCTION(add_polygon_entity);
	REGISTER_FUNCTION(add_npc);
	REGISTER_FUNCTION(set_character_role);
	REGISTER_FUNCTION(change_areas);
	REGISTER_FUNCTION(get_screen_size);
	REGISTER_FUNCTION(get_screen_scale);
	REGISTER_FUNCTION(set_entity_position);
	REGISTER_FUNCTION(set_entity_z);
	REGISTER_FUNCTION(set_entity_layer);
	REGISTER_FUNCTION(get_area_pixel_size);
	REGISTER_FUNCTION(get_area_tile_size);
	REGISTER_FUNCTION(add_floating_image);
	REGISTER_FUNCTION(add_floating_rectangle);
	REGISTER_FUNCTION(add_outline_point);
	REGISTER_FUNCTION(add_outline_split);
	REGISTER_FUNCTION(set_entity_solid_with_area);
	REGISTER_FUNCTION(entity_is_solid_with_area);
	REGISTER_FUNCTION(set_entity_solid_with_entities);
	REGISTER_FUNCTION(entity_is_solid_with_entities);
	REGISTER_FUNCTION(add_tween);
	REGISTER_FUNCTION(set_entity_input_disabled);
	REGISTER_FUNCTION(get_entity_input_disabled);
	REGISTER_FUNCTION(set_show_entity_shadow);
	REGISTER_FUNCTION(get_entity_shadow_is_shown);
	REGISTER_FUNCTION(play_music);
	REGISTER_FUNCTION(set_entity_animation);
	REGISTER_FUNCTION(get_entity_animation);
	REGISTER_FUNCTION(get_entity_animation_frame);
	REGISTER_FUNCTION(set_entity_animation_no_reset);
	REGISTER_FUNCTION(get_entity_animation_num_frames);
	REGISTER_FUNCTION(set_entity_animation_frame);
	REGISTER_FUNCTION(reset_entity_animation);
	REGISTER_FUNCTION(set_entity_direction);
	REGISTER_FUNCTION(get_entity_direction);
	REGISTER_FUNCTION(get_num_entities_in_area);
	REGISTER_FUNCTION(get_area_entity_id_by_number);
	REGISTER_FUNCTION(set_area_player_underlay_bitmap_add);
	REGISTER_FUNCTION(set_character_destination);
	REGISTER_FUNCTION(character_is_following_path);
	REGISTER_FUNCTION(reset_outline);
	REGISTER_FUNCTION(process_outline);
	REGISTER_FUNCTION(set_entity_animation_set_prefix);
	REGISTER_FUNCTION(get_entity_animation_set_prefix);
	REGISTER_FUNCTION(center_entity_cameras);
	REGISTER_FUNCTION(set_move_entity_cameras_while_input_disabled);
	REGISTER_FUNCTION(set_entity_speed);
	REGISTER_FUNCTION(set_shadow_color);
	REGISTER_FUNCTION(set_clear_color);
	REGISTER_FUNCTION(set_parallax_parameters);
	REGISTER_FUNCTION(get_area_top);
	REGISTER_FUNCTION(get_battle_top);
	REGISTER_FUNCTION(add_tile_group);
	REGISTER_FUNCTION(get_entity_iso_position);
	REGISTER_FUNCTION(area_is_isometric);
	REGISTER_FUNCTION(speak);
	REGISTER_FUNCTION(speak_top);
	REGISTER_FUNCTION(speak_force_t);
	REGISTER_FUNCTION(speak_force_b);
	REGISTER_FUNCTION(bit_and);
	REGISTER_FUNCTION(bit_rshift);
	REGISTER_FUNCTION(load_sample);
	REGISTER_FUNCTION(play_sample);
	REGISTER_FUNCTION(stop_sample);
	REGISTER_FUNCTION(adjust_sample);
	REGISTER_FUNCTION(destroy_sample);
	REGISTER_FUNCTION(change_tile_layer);
	REGISTER_FUNCTION(get_random_enemy_spawn_point);
	REGISTER_FUNCTION(add_enemy_avatar);
	REGISTER_FUNCTION(rand);
	REGISTER_FUNCTION(set_entity_visible);
	REGISTER_FUNCTION(entity_is_visible);
	REGISTER_FUNCTION(set_tile);
	REGISTER_FUNCTION(get_tile);
	REGISTER_FUNCTION(add_flash);
	REGISTER_FUNCTION(shake);
	REGISTER_FUNCTION(end_shake);
	REGISTER_FUNCTION(fade);
	REGISTER_FUNCTION(get_camera_position);
	REGISTER_FUNCTION(set_camera_offset);
	REGISTER_FUNCTION(get_camera_offset);
	REGISTER_FUNCTION(milestone_is_complete);
	REGISTER_FUNCTION(set_milestone_complete);
	REGISTER_FUNCTION(block_on_condition);
	REGISTER_FUNCTION(entity_animation_is_finished);
	REGISTER_FUNCTION(set_wander_pause_times);
	REGISTER_FUNCTION(set_wander_minimum_move_distance);
	REGISTER_FUNCTION(set_area_swiping_in);
	REGISTER_FUNCTION(get_entity_name);
	REGISTER_FUNCTION(push_entity_to_front);
	REGISTER_FUNCTION(push_entity_to_back);
	REGISTER_FUNCTION(get_entity_layer);
	REGISTER_FUNCTION(set_entity_stationary);
	REGISTER_FUNCTION(get_num_players);
	REGISTER_FUNCTION(get_player_id);
	REGISTER_FUNCTION(set_entity_role_paused);
	REGISTER_FUNCTION(add_ladder);
	REGISTER_FUNCTION(set_entity_right);
	REGISTER_FUNCTION(get_entity_right);
	REGISTER_FUNCTION(get_entity_animation_length);
	REGISTER_FUNCTION(add_no_enemy_zone);
	REGISTER_FUNCTION(restore_item);
	REGISTER_FUNCTION(save_item);
	REGISTER_FUNCTION(get_num_level_save_items);
	REGISTER_FUNCTION(get_level_save_item_type);
	REGISTER_FUNCTION(get_level_save_item_data);
	REGISTER_FUNCTION(revive_everyone);
	REGISTER_FUNCTION(increase_hp);
	REGISTER_FUNCTION(set_saved_players);
	REGISTER_FUNCTION(switch_characters);
	REGISTER_FUNCTION(ramp_music_down);
	REGISTER_FUNCTION(ramp_music_up);
	REGISTER_FUNCTION(inc_num_jumping);
	REGISTER_FUNCTION(dec_num_jumping);
	REGISTER_FUNCTION(get_entity_inputs);
	REGISTER_FUNCTION(set_entities_slide_on_entity);
	REGISTER_FUNCTION(get_entity_speed);
	REGISTER_FUNCTION(point_in_area_bounds);
	REGISTER_FUNCTION(set_battle_was_event);
	REGISTER_FUNCTION(start_attack);

	// Particles
	REGISTER_FUNCTION(add_particle_group);
	REGISTER_FUNCTION(delete_particle_group);
	REGISTER_FUNCTION(get_particle_group_blackboard);
	REGISTER_FUNCTION(set_particle_group_blackboard);
	REGISTER_FUNCTION(get_particle_group_alignment);
	REGISTER_FUNCTION(set_particle_group_alignment);
	REGISTER_FUNCTION(add_particle);
	REGISTER_FUNCTION(get_particle_size);
	REGISTER_FUNCTION(set_particle_size);
	REGISTER_FUNCTION(get_particle_tint);
	REGISTER_FUNCTION(set_particle_tint);
	REGISTER_FUNCTION(set_particle_draw_offset);
	REGISTER_FUNCTION(get_particle_bitmap_index);
	REGISTER_FUNCTION(set_particle_bitmap_index);
	REGISTER_FUNCTION(get_particle_blackboard);
	REGISTER_FUNCTION(set_particle_blackboard);
	REGISTER_FUNCTION(get_particle_group_id);
	REGISTER_FUNCTION(get_particle_position);
	REGISTER_FUNCTION(set_particle_position);
	REGISTER_FUNCTION(get_particle_bitmap_size);
	REGISTER_FUNCTION(get_particle_hidden);
	REGISTER_FUNCTION(set_particle_hidden);
	REGISTER_FUNCTION(get_particle_damage);
	REGISTER_FUNCTION(set_particle_damage);
	REGISTER_FUNCTION(get_particle_angle);
	REGISTER_FUNCTION(set_particle_angle);
	REGISTER_FUNCTION(get_particle_scale);
	REGISTER_FUNCTION(set_particle_scale);
	REGISTER_FUNCTION(set_particle_bullet_time);

	// Battle functions
	REGISTER_FUNCTION(add_parallax_bitmap);
	REGISTER_FUNCTION(ai_get);
	REGISTER_FUNCTION(set_battle_entity_attacking);
	REGISTER_FUNCTION(set_battle_entity_sub_animation);
	REGISTER_FUNCTION(reset_battle_entity_animation);
	REGISTER_FUNCTION(set_battle_entity_flying);
	REGISTER_FUNCTION(checkcoll_line_player);
	REGISTER_FUNCTION(get_battle_width);
	REGISTER_FUNCTION(get_battle_height);
	REGISTER_FUNCTION(start_battle);
	REGISTER_FUNCTION(get_highest_point);
	REGISTER_FUNCTION(battle_entity_is_colliding_with_area);
	REGISTER_FUNCTION(particle_is_colliding_with_area);
	REGISTER_FUNCTION(add_battle_enemy);
	REGISTER_FUNCTION(add_battle_entity);
	REGISTER_FUNCTION(set_enemy_aggressiveness);
	REGISTER_FUNCTION(get_battle_entity_hit_something_this_attack);
	REGISTER_FUNCTION(get_random_start_platform);
	REGISTER_FUNCTION(set_entity_immovable);
	REGISTER_FUNCTION(set_entity_immovable);
	REGISTER_FUNCTION(apply_force);
	REGISTER_FUNCTION(entity_is_on_ground);
	REGISTER_FUNCTION(set_should_attack);
	REGISTER_FUNCTION(set_battle_entity_jumping);
	REGISTER_FUNCTION(get_height_from_ground);
	REGISTER_FUNCTION(set_battle_entity_unhittable);
	REGISTER_FUNCTION(get_hp);
	REGISTER_FUNCTION(set_hp);
	REGISTER_FUNCTION(get_mp);
	REGISTER_FUNCTION(get_ability_cost);
	REGISTER_FUNCTION(get_distance_from_nearest_edge);
	REGISTER_FUNCTION(set_name);
	REGISTER_FUNCTION(set_attributes);
	REGISTER_FUNCTION(set_status);
	REGISTER_FUNCTION(set_weapon);
	REGISTER_FUNCTION(set_weapon_attachment);
	REGISTER_FUNCTION(set_armor);
	REGISTER_FUNCTION(set_accessory);
	REGISTER_FUNCTION(set_crystals);
	REGISTER_FUNCTION(set_selected_abilities);
	REGISTER_FUNCTION(set_battle_entity_attack);
	REGISTER_FUNCTION(set_battle_entity_speed_multiplier);
	REGISTER_FUNCTION(save_boss_save);
	REGISTER_FUNCTION(get_player_weapon_name);
	REGISTER_FUNCTION(set_player_lost_weapon);
	REGISTER_FUNCTION(get_entity_bone_size);
	REGISTER_FUNCTION(add_battle_data);
	REGISTER_FUNCTION(get_battle_data_size);
	REGISTER_FUNCTION(get_battle_data);
	REGISTER_FUNCTION(count_battle_entities);
	REGISTER_FUNCTION(get_entity_bitmap);
	REGISTER_FUNCTION(set_battle_entity_default_layer);
	REGISTER_FUNCTION(set_battle_entity_layer);
	REGISTER_FUNCTION(call_on_battle_script);
	REGISTER_FUNCTION(call_on_battle_entity_script);
	REGISTER_FUNCTION(get_num_platforms);
	REGISTER_FUNCTION(set_platform_solid);
	REGISTER_FUNCTION(get_num_entities_in_battle);
	REGISTER_FUNCTION(get_battle_entity_id_by_number);
	REGISTER_FUNCTION(set_entity_stops_battle_end);
	REGISTER_FUNCTION(get_level_collision);
	REGISTER_FUNCTION(set_can_accelerate_quickly);
	REGISTER_FUNCTION(drain_magic);
	REGISTER_FUNCTION(is_burrowing);
	REGISTER_FUNCTION(kill_enemy);
	REGISTER_FUNCTION(hurt_enemy);
	REGISTER_FUNCTION(difficulty);

	// Generic
	REGISTER_FUNCTION(set_area_loop_input_paused);
	REGISTER_FUNCTION(stop_entity);
	REGISTER_FUNCTION(remove_entity);
	REGISTER_FUNCTION(remove_particle);
	REGISTER_FUNCTION(get_entity_position);
	REGISTER_FUNCTION(get_entity_z);
	REGISTER_FUNCTION(get_entity_animation_size);
	REGISTER_FUNCTION(t);
	REGISTER_FUNCTION(hold_milestones);
	REGISTER_FUNCTION(get_time);
	REGISTER_FUNCTION(get_low_graphics);
	REGISTER_FUNCTION(give_equipment);
	REGISTER_FUNCTION(give_items);
	REGISTER_FUNCTION(calculate_toss);
	REGISTER_FUNCTION(add_crystals);
	REGISTER_FUNCTION(set_elapsed_time);
	REGISTER_FUNCTION(add_cash);
	REGISTER_FUNCTION(set_cash);
	REGISTER_FUNCTION(set_area_name);
	REGISTER_FUNCTION(add_bisou);
	REGISTER_FUNCTION(get_save_state_version);
	REGISTER_FUNCTION(set_save_state_version);
	REGISTER_FUNCTION(is_item);
	REGISTER_FUNCTION(get_ability_name);
	REGISTER_FUNCTION(log_message);

	// Graphics
	REGISTER_FUNCTION(load_bitmap)
	REGISTER_FUNCTION(destroy_bitmap)
	REGISTER_FUNCTION(get_bitmap_size)
	REGISTER_FUNCTION(get_bitmap_texture_size)
	REGISTER_FUNCTION(draw_bitmap)
	REGISTER_FUNCTION(draw_bitmap_yellow_glow)
	REGISTER_FUNCTION(draw_bitmap_additive)
	REGISTER_FUNCTION(draw_tinted_rotated_bitmap)
	REGISTER_FUNCTION(draw_tinted_rotated_bitmap_additive)
	REGISTER_FUNCTION(draw_bitmap_region)
	REGISTER_FUNCTION(get_work_bitmap);
	REGISTER_FUNCTION(set_target_bitmap);
	REGISTER_FUNCTION(set_target_allegro_bitmap);
	REGISTER_FUNCTION(get_target_bitmap);
	REGISTER_FUNCTION(load_video);
	REGISTER_FUNCTION(start_video);
	REGISTER_FUNCTION(update_video);
	REGISTER_FUNCTION(set_video_offset);
	REGISTER_FUNCTION(draw_video);
	REGISTER_FUNCTION(get_video_frame_num);
	REGISTER_FUNCTION(destroy_video);
	REGISTER_FUNCTION(create_shader);
	REGISTER_FUNCTION(set_shader_float);
	REGISTER_FUNCTION(use_shader);
	REGISTER_FUNCTION(destroy_shader);
	REGISTER_FUNCTION(set_clipping_rectangle);
	REGISTER_FUNCTION(get_clipping_rectangle);
	REGISTER_FUNCTION(update_entity_animation);
	REGISTER_FUNCTION(fade_out);
	REGISTER_FUNCTION(fade_in);
	REGISTER_FUNCTION(redraw);
	REGISTER_FUNCTION(clear_to_color);
	
	// Map
	REGISTER_FUNCTION(start_map);
	REGISTER_FUNCTION(add_map_location);
	REGISTER_FUNCTION(set_map_neighbors);

	// Shop
	REGISTER_FUNCTION(do_item_shop);
	REGISTER_FUNCTION(do_equipment_shop);

	// Mini-games
	REGISTER_FUNCTION(do_whack_a_skunk);
	REGISTER_FUNCTION(whack_a_skunk_was_played);
	REGISTER_FUNCTION(get_whack_a_skunk_score);
	REGISTER_FUNCTION(set_whack_a_skunk_played);

	// Skeletons
	REGISTER_FUNCTION(get_skeleton_animation_elapsed_time);

	REGISTER_FUNCTION(credits);

	#undef REGISTER_FUNCION
}

void load_global_scripts(lua_State *lua_state)
{
	unsigned char *bytes;

#ifdef LUAJIT
	bytes = General::slurp("scripts/5.2.lua");
	if (luaL_loadstring(lua_state, (char *)bytes)) {
		Lua::dump_lua_stack(lua_state);
		throw Error("Error loading 5.2.lua.\n");
	}
	delete[] bytes;

	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running 5.2.lua.");
	}
#endif

	bytes = General::slurp("scripts/global.lua");
	if (luaL_loadstring(lua_state, (char *)bytes)) {
		Lua::dump_lua_stack(lua_state);
		throw Error("Error loading global script.\n");
	}
	delete[] bytes;

	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running global script.");
	}
}

} // end namespace Lua

