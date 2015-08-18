#ifndef AREA_MANAGER_H
#define AREA_MANAGER_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_opengl.h>

#ifdef ALLEGRO_WINDOWS
#include <allegro5/allegro_direct3d.h>
#endif

#include <vector>
#include <list>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "layer.h"
#include "camera.h"
#include "positioned_entity.h"
#include "map_entity.h"
#include "config.h"
#include "astar.h"
#include "triangulate.h"
#include "luainc.h"
#include "offsetable.h"
#include "bones.h"
#include "shaders.h"
#include "particle.h"

class Area_Manager : public Camera, public Offsetable {
public:
	enum {
		TILE_GROUP_BUSHES = 1
	};

	static const int QUADRANT_SIZE = 256;

	struct Tile_Group {
		int layer;
		std::vector< General::Point<int> > tiles;
		General::Point<int> top_left_tile_pixel;
		General::Point<int> bottom_right_tile_pixel;
		General::Point<int> top_left;
		General::Point<int> bottom_right; // for iso
		General::Point<int> top_left2;
		General::Point<int> bottom_right2; // for iso
		General::Size<int> size; // for top down
		int flags;
		double duration;
	};

	struct Tile_Bounds {
		General::Point<int> top_left;
		General::Size<int> size;
	};

	std::string get_name() {
		return name;
	}

	bool logic();

	void draw();

	bool load(std::string name);

	// returns id. pass -1 for auto id.
	int add_entity(Map_Entity *entity);
	Map_Entity *get_entity(int id);

	General::Point<float> get_top();
	void set_top(General::Point<float> top);

	lua_State *get_lua_state();

	// Camera interface
	General::Point<float> get_camera_pos();
	bool move_camera(General::Point<float> delta, General::Point<float> entity_pos, bool tracking_entity);

	void start();

	bool area_is_colliding(int layer, General::Point<int> pos, std::vector<Bones::Bone> &bones, bool right, General::Line<float> *ret_line, bool *stop_entity = NULL);
	std::list<Map_Entity *> entity_is_colliding(Map_Entity *entity, General::Point<float> entity_pos, bool weapon);
	bool ladder_is_colliding(int layer, Map_Entity *entity);
	void shake_bushes(Map_Entity *entity, General::Point<float> entity_pos);

	void load_unload_surrounding(General::Point<float> player_pos, bool check_minmax);

	int get_width();
	int get_height();

	General::Size<int> get_pixel_size();

	int height_at(int x, int y);

	void add_floating_image(Wrap::Bitmap *bitmap, int layer, int offset_x, int offset_y, bool pre, bool subtractive);
	void add_floating_rectangle(ALLEGRO_COLOR tl, ALLEGRO_COLOR tr, ALLEGRO_COLOR br, ALLEGRO_COLOR bl, int width, int height, int layer, int offset_x, int offset_y, bool pre);

	void add_outline_point(int layer, int x, int y);
	void add_outline_split(int layer, int index);

	int get_num_entities();
	int get_entity_id_by_number(int number);

	bool reposition_entity_in_vector(int id, int before); // before == -1 means end, returns true on success

	// area takes ownership of this bitmap, so don't destroy
	void set_player_underlay_bitmap_add(Wrap::Bitmap *bitmap, int layer, bool top_also);

	std::vector<Triangulate::Triangle> *get_nav_meshes();
	A_Star::Navigation_Link ****get_nav_links();
	
	std::vector< General::Line<float> > *get_collision_lines();

	void process_outline(); // call before any a* stuff for npcs in start() (lua)

	bool is_dungeon();

	void shutdown();

	Map_Entity *activate(Map_Entity *activator, General::Point<float> pos, int box_width, int box_length);

	void set_shadow_color(float r, float g, float b);

	void set_clear_color(float r, float g, float b);
	ALLEGRO_COLOR get_clear_color();

	void set_parallax_params(bool x, bool y);

	void add_tile_group(Tile_Group group);
	General::Point<float> get_iso_offset();

	General::Point<int> get_entity_iso_pos(int id);
	bool is_isometric();
	
	void reset_top();

	std::vector<Tiled_Layer *>& get_tiled_layers();

	void reshape();
	void prepare_for_reshape();

	std::string get_path();

	void set_early_break(bool early_break);

	float get_height_offset_dampened() {
		return height_offset_dampened;
	}

	void toggle_layer(int layer);

	void reset_outline();

	void add_ladder(int layer, General::Point<float> topleft, General::Point<float> bottomright);

	void add_no_enemy_zone(General::Rectangle<int> zone);
	std::vector< General::Rectangle<int> > &get_no_enemy_zones();
	bool point_is_in_no_enemy_zone(int x, int y);

	General::Point<float> get_player_start_position();
	int get_player_start_layer();

	void remove_entity_from_vector(Entity *e);
	void remove_entity_from_vector(int id);

	std::vector< General::Line<float> > get_quadrant(int layer, General::Point<float> pos);

	void destroy_sheets();
	void load_sheets();

	bool get_in_speech_loop();
	void set_in_speech_loop(bool in_speech_loop);

	void add_particle(Particle::Particle *p);
	void remove_particle(int id);
	Particle::Particle *get_particle(int id);

	bool point_collides(int layer, General::Point<float> p);

	void set_rumble_offset(General::Point<float> rumble_offset);
	General::Point<float> get_rumble_offset() { return rumble_offset; }

	Area_Manager();
	virtual ~Area_Manager(); // should this be virtual ie deleted as Camera of Offsetable? doubt it

private:
	enum Area_Floating_Block_Type {
		FLOATING_IMAGE = 0,
		FLOATING_RECTANGLE
	};

	struct Area_Floating_Block {
		int layer;
		Area_Floating_Block_Type type;
		Wrap::Bitmap *bitmap;
		ALLEGRO_COLOR tl, tr, br, bl; // top left, top right etc
		                              // for rectangle gradients
		int offset_x;
		int offset_y;
		int width;
		int height;
		bool pre;
		bool subtractive;
	};

	struct Floating_Layer_Info {
		int layer;
		int total_width;
		int total_height;
		int min_x;
		int min_y;
	};

	struct Ladder {
		int layer;
		General::Point<float> topleft;
		General::Point<float> bottomright;
	};
	
	Area_Floating_Block create_floating_block(int layer, int offset_x, int offset_y, bool pre);
	void post_process_floating_block(Area_Floating_Block *b);

	void draw_layer_tiled_ungrouped(int layer);
	void draw_layer_tiled_groups(int layer);
	void draw_layer_tiled(int layer);
	void draw_layer_isometric_ungrouped(int layer);
	void draw_layer_isometric_groups(int layer);
	void draw_layer_isometric(int layer);
	void draw_layer(int layer);

	int tile_x(int number);
	int tile_y(int number);
	void add_tile_to_texture_opengl(
		Painted_Layer::Tile *tile,
		Wrap::Bitmap *bitmap
	);
#ifdef ALLEGRO_WINDOWS
	void add_tile_to_texture_direct3d(
		Painted_Layer::Tile *tile,
		Wrap::Bitmap *bitmap
	);
#endif
	void add_tile_to_texture(
		Painted_Layer::Tile *tile,
		Wrap::Bitmap *bitmap
	);
	void init_lua();
	void draw_floating_layer(int layer, bool pre);
	Floating_Layer_Info &find_floating_info(int layer);

	void link_triangles();

	int partitions_wide();
	int partitions_high();

//	void load_isometric_heightmap(std::string filename);
	Map_Entity *get_next_entity_in_layer(Map_Entity *start, int layer);
	void draw_entity(Map_Entity *entity, bool draw_shadow, General::Point<int> start_draw, General::Point<int> end_draw);
	inline void draw_iso_tile(int layer, int x, int y, int next_x, int next_y);
	inline bool tile_is_grouped(int layer, int x, int y);
	inline void next_valid_iso_tile(
		int x, int y, int start_x, int start_y, int width, int height, float curr_width, int end_y, int *outx, int *outy);
	
	void calc_tile_bounds(int sheet);
	
	void optimize_tiled_area();

	void create_outline_storage();
	void destroy_outline();

	void maybe_expand_vertex_cache(int needed);

	// Top left visible corner offset
	General::Point<float> top;
	int width;
	int height;
	General::Size<int> pixel_size;

	int num_layers;

	// tiled area stuff
	std::vector<Tiled_Layer *> tiled_layers;
	int width_in_tiles;
	int height_in_tiles;
	std::vector<Wrap::Bitmap *> tile_sheets;
	std::vector<int> sheet_used;
	bool load_tiled_area();

	std::string name;

	std::vector<Map_Entity *> entities;

	ALLEGRO_VERTEX *verts;

	std::string path;

	lua_State *lua_state;

	Wrap::Bitmap *normal_character_shadow_bmp;
	Wrap::Bitmap *big_character_shadow_bmp;
	
	unsigned char *heightmap;
	int player_height;

	std::vector<Area_Floating_Block> floating;
	std::vector<Floating_Layer_Info> floating_info;

	std::vector< General::Line<float> > *collision_lines;
	std::vector< General::Point<float> > *outline_points;
	std::vector<int> *outline_splits;
	std::vector<Triangulate::Triangle> *nav_meshes;
	A_Star::Navigation_Link ****nav_links;
	
        float height_offset_dampened;

	Wrap::Bitmap *player_underlay_bitmap_add;
	int player_underlay_layer;
	bool player_underlay_top_also;
	bool _is_dungeon;
	
	unsigned char *shadow_mask;
	int shadow_pitch;
	ALLEGRO_COLOR shadow_color;

	bool isometric;

	ALLEGRO_COLOR clear_color;

	bool parallax_x;
	bool parallax_y;

	std::vector< std::vector<Tile_Group *> > tile_groups;

	// iso stuff
	int curr_sheet; // draw_iso_tile uses this
	int iso_vert_count;
	General::Point<int> iso_tiles_in_screen;

	std::vector< std::vector<Tile_Bounds> > tile_bounds;
	
	std::vector< std::vector<int> > tile_swap_map;
	int remap_sheets;

	bool early_break;

	bool layer_toggled_off[10];

	std::vector<Map_Entity *> shaken_entities;
	std::vector< General::Point<float> > shaken_positions;

	std::vector< std::vector< std::vector< std::vector< std::pair< General::Point<float>, General::Point<float> > > > > > quadrants;

	std::vector<Ladder> ladders;

	bool ***is_grouped;
	General::Size<int> size;

	std::vector< General::Rectangle<int> > no_enemy_zones;

	General::Point<float> player_start_position;
	int player_start_layer;

	Wrap::Shader *water_shader;

	std::vector<Wrap::Bitmap *> water_sheets;

	bool in_speech_loop;

	bool **group_drawn;

	bool script_has_pre_draw;
	bool script_has_mid_draw;
	bool script_has_post_draw;

	std::vector<Particle::Particle *> particles;

	General::Point<float> rumble_offset;

	ALLEGRO_VERTEX *vertex_cache;
	int vertex_cache_size;
};

#endif // AREAMANAGER_H
