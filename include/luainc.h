#ifndef LUAINC_H
#define LUAINC_H

#include <allegro5/allegro.h>
#include <string>
#include <vector>

class Area_Manager;
class Player;
class Area_Loop;

extern "C" {
#ifdef LUAJIT
#include <lua5.1/lua.h>
#include <lua5.1/lauxlib.h>
#include <lua5.1/lualib.h>
#else
#include <lua5.2/lua.h>
#include <lua5.2/lauxlib.h>
#include <lua5.2/lualib.h>
#endif
}

namespace Lua {

void reset_game();
std::vector<std::string> get_saved_players();
void init_battle_attributes();
void store_battle_attributes(std::vector<Player *> players);
void restore_battle_attributes(Player *p);
void add_battle_attributes_lines();
void clear_saved_lua_lines();
void clear_before_load();
void add_saved_lua_line(std::string line);
void write_saved_lua_lines(ALLEGRO_FILE *f);
std::string get_saved_area_name();
void open_lua_libs(lua_State *lua_state);
void register_c_functions(lua_State *lua_state);
void load_global_scripts(lua_State *stack);
void call_lua(lua_State* luaState, const char *func, const char *sig, ...);
void dump_lua_stack(lua_State *l);
int get_save_state_version();
void set_save_state_version(int version);

} // end namespace Lua

#endif
