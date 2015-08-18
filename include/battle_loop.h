#ifndef BATTLE_LOOP_H
#define BATTLE_LOOP_H

#include <map>
#include <atlas.h>

#include "bitmap.h"
#include "frame.h"
#include "animation.h"
#include "xml.h"
#include "general.h"
#include "animation_set.h"
#include "loop.h"
#include "luainc.h"
#include "sound.h"
#include "offsetable.h"
#include "graphics.h"
#include "particle.h"

class Battle_Pathfinder;
struct Battle_Pathfinder_Edge;
struct Battle_Pathfinder_Node;
class Battle_Entity;
class Battle_Player;
class Enemy_Avatar;
class Player;

class Battle_Loop : public Loop, public Offsetable {
public:
	// Might have to adjust this for large enemies
	static const int SECTOR_SIZE = 16;

	enum Jump_Point_Type {
		JUMP_NONE = -1,
		JUMP_HORIZONTAL = 0,
		JUMP_UP = 1,
		JUMP_DOWN = 2
	};
	struct Jump_Point {
		Jump_Point_Type type;
		float x, y;
		// line index into geometry, platform inferred from outer index
		int point;
		int dest_platform;
		int dest_point;
	};

	General::Point<float> get_top();

	void get_player_screen_pos(General::Point<float> pos, int *px, int *py);
	void get_entity_draw_pos(General::Point<float> pos, Animation_Set *anim_set, int *tlx, int *tly);
	void get_entity_coordinates(General::Point<float> pos, Animation_Set *anim_set, int *tlx, int *tly);
	
	void get_area_offset(int *dx, int *dy);

	virtual bool init();
	virtual void top();
	virtual bool handle_event(ALLEGRO_EVENT *event);
	virtual bool logic();
	virtual void draw();
	void destroy_graphics();
	void reload_graphics();

	void construct();

	int ai_get(lua_State *stack, int entity_id, std::string cmd);

	std::vector< std::vector< General::Point<int> > > &get_geometry() { return geometry; }
	std::vector<bool> &get_platform_solid();
	std::vector< std::pair< General::Point<float>, General::Point<float> > > &get_sector(int n)
	{
		if (n < 0) {
			n = 0;
		}
		else if (n >= (int)sectors.size()) {
			n = sectors.size()-1;
		}
		return sectors[n];
	}
	int get_sector_platform(int sector, int n) {
		if (sector < 0) {
			sector = 0;
		}
		else if (sector >= (int)sectors.size()) {
			sector = sectors.size()-1;
		}
		return sector_platforms[sector][n];
	}

	int get_width() { return size.w; }
	int get_height() { return size.h; }
	void set_geometry(std::vector< std::vector< General::Point<int> > > &g) {
		geometry = g;
		jump_points.clear();
		make_jump_points();
		make_pathfinding_info();
		make_sectors();
	}

	Battle_Entity *get_entity(int entity_id);
	Battle_Player *get_player(int player_id);
	Battle_Player *get_active_player();
	std::vector<Battle_Entity *> &get_entities();
	
	std::vector<Battle_Pathfinder_Node *> find_closest_nodes(General::Point<float> pos, int platform = -1);
	std::vector<Battle_Pathfinder_Node> find_path(
		int start_platform, General::Point<float> start, int end_platform, General::Point<float> end);

	void add_parallax_bitmap(Wrap::Bitmap *bitmap, bool foreground);

	void set_entity_layer(int layer);
	int get_entity_layer();

	void draw_bullet_time(General::Point<float> start, General::Point<float> end, float width);
	void draw_bullet_time_v(General::Point<float> start, General::Point<float> end, float width);

	int get_next_id();

	lua_State *get_lua_state();

	General::Point<int> choose_random_start_platform(int entity_spacing, int edge_spacing);

	void set_switch_players(bool switch_players);

	bool battle_transition_is_done();

	void set_game_over_time(double game_over_time);

	float get_distance_from_nearest_edge(int id);
	void add_burrow_hole(General::Point<float> pos, double end_time);
	void add_static_burrow_hole(General::Point<float> pos, double end_time);

	void add_battle_data(std::string name, int size, double *data);
	int get_battle_data_size(std::string name);
	double get_battle_data(std::string name, int index);
	
	bool reposition_entity_in_vector(int id, int before); // before == -1 means end, returns true on success

	void add_particle(Particle::Particle *p);
	void remove_particle(int id);
	Particle::Particle *get_particle(int id);
	std::vector<Particle::Particle *> &get_particles() { return particles; }

	void add_entity(Battle_Entity *entity);

	bool is_cart_battle();

	void set_rumble_offset(General::Point<float> rumble_offset) { this->rumble_offset = rumble_offset; }

	Battle_Loop(std::vector<Player *> players, Enemy_Avatar *enemy_avatar, bool delete_enemy_avatar, Wrap::Bitmap *pre_battle_screen_bmp, Wrap::Bitmap **end_screenshot, std::string *end_player);
	virtual ~Battle_Loop();

protected:
	void draw_entities(int layer);

	struct Battle_Data {
		std::string name;
		int size;
		double *data;
	};
	int find_battle_data(std::string name);
	std::vector<Battle_Data> battle_data;

	virtual void extra_drawing_hook(int layer) {}

	struct Burrow_Hole {
		General::Point<float> pos;
		double start_time;
		double end_time;
		int pgid;
	};
	void destroy_burrow_hole(std::vector<Burrow_Hole> &v, int index);

	void init_lua();
	lua_State *lua_state;

	// Visual stuff	
	struct Tile {
		int index;
		int x;
		int y;
		std::vector<int> indices;
		int delay;
	};
	static General::Size<int> size;

	ATLAS *atlas;
	std::vector< std::vector<Tile> > tiles;

	std::vector< std::vector< General::Point<int> > > geometry;
	std::vector<bool> platform_solid;
	std::vector< std::vector< std::pair< General::Point<float>, General::Point<float> > > > sectors;
	std::vector< std::vector<int> > sector_platforms;

	std::vector<Battle_Entity *> entities;
	std::vector<Particle::Particle *> particles;
		
	// every platform connects to every other platform at the
	// closest vertical point, if none found then the closest
	// horizontally
	void make_jump_points();
	std::vector< std::vector<Jump_Point> > jump_points;

	// This is saved for the life of this battle area for all pathfinding.
	// The pathfinding algorithms clone this and return pointers to their
	// own copy.
	int num_pathfinding_nodes;
	int num_pathfinding_edges;
	Battle_Pathfinder_Node *pathfinding_nodes;
	Battle_Pathfinder_Edge *pathfinding_edges;
	void make_pathfinding_info();
	void make_sectors();

	void maybe_expand_vertex_cache(int needed);

	std::string level_name, script_name;

	std::vector<Wrap::Bitmap *> parallax_bmps;
	std::vector<bool> parallax_is_foreground;
	Wrap::Bitmap *cart_parallax[4];

	Wrap::Bitmap *pre_battle_screen_bmp;
	bool battle_transition_done;

	bool first_run;

	Enemy_Avatar *enemy_avatar;

	static float x_offset_ratio;

	int entity_layer;

	bool draw_interface;

	int active_player;

	bool move_camera_to_zero;
	
	std::vector<Player *> players;
	
	Wrap::Bitmap **end_screenshot;
	std::string *end_player;

	Wrap::Shader *shield_shader;
	Wrap::Shader *bullet_time_shader;
	Wrap::Shader *bullet_time_v_shader;

	bool making_screen_copy;
	Wrap::Bitmap *screen_copy_bmp;
	bool do_not_make_screen_copy;

	int active_players;

	int curr_id;

	bool switch_players;

	double game_over_time;

	bool boss_battle;

	Wrap::Bitmap *burrow_hole_bmps[4];
	std::vector<Burrow_Hole> burrow_holes;
	std::vector<Burrow_Hole> static_burrow_holes;
	int burrow_particle_count;

	bool delete_enemy_avatar;

	std::vector<Battle_Entity *> entities_to_add;

	bool cart_battle;

	double battle_start_time;

	Animation_Set *pumping_cart;

	bool script_has_logic;

	int cart_pixels_travelled;
	bool set_cart_transition_start;
	int cart_transition_start;
	bool added_antboss;
	bool cart_battle_enemy_dead;
	double cart_battle_enemy_dead_time;
	int cart_pixels_travelled_final;
	bool cart_crashed;
	Animation_Set *cart_wheel;
	bool cart_music_ramped;
	bool played_wheel_sample;

	General::Point<float> rumble_offset;

	bool runner_loop;

	ALLEGRO_VERTEX *vertex_cache;
	int vertex_cache_size;

	std::map<int, Battle_Player *> player_map;
};

#include "battle_entity.h"

void start_battle(Enemy_Avatar *enemy_avatar, bool delete_enemy_avatar, Wrap::Bitmap **end_screenshot, std::string *end_player);

#endif // _BATTLE_LOOP_H
