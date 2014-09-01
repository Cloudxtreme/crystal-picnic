#ifndef MAP_ENTITY_H
#define MAP_ENTITY_H

#include <allegro5/allegro.h>

#include <vector>
#include <string>

#include "camera.h"
#include "animation_set.h"
#include "config.h"
#include "positioned_entity.h"
#include "visible_entity.h"
#include "collidable.h"
#include "luainc.h"
#include "bones.h"

class Area_Manager;
class Area_Loop;
namespace A_Star {
	struct A_Star_Thread_Info;
}

class Map_Entity : public Positioned_Entity, public Visible_Entity, public Collidable {
public:
	enum Input_Index {
		X = 0,
		Y,
		BUTTON1,
		NUM_INPUTS
	};

	virtual void handle_event(ALLEGRO_EVENT *event);
	virtual void logic();
	virtual void draw();
	virtual bool load();
	virtual void reset();
	virtual void set_role_paused(bool role_paused);
	virtual void set_position(General::Point<float> pos);

	ALLEGRO_THREAD *get_astar_thread();
	A_Star::A_Star_Thread_Info *get_ati();
	void set_ati(A_Star::A_Star_Thread_Info *ati);

	void init();

	void set_move_cameras_while_input_disabled(bool move_cameras);

	void update_direction(bool can_move);

	float get_speed();
	void set_speed(float speed);

	int get_layer();
	void set_layer(int layer);

	int get_sort_offset();
	void set_sort_offset(int sort_offset);

	General::Direction get_direction();
	void set_direction(General::Direction direction);
	void auto_set_direction();

	bool is_facing_right();

	void set_area_loop(Area_Loop *al);

	void add_camera(Camera *camera);
	void remove_camera(Camera *camera);

	bool is_solid_with_area();
	void set_solid_with_area(bool s);
	bool is_solid_with_entities();
	void set_solid_with_entities(bool s);

	bool input_is_disabled();
	void set_input_disabled(bool disabled);
	float *get_inputs();

	bool shadow_is_shown();
	void set_show_shadow(bool show);

	void center_cameras();

	bool collides_with_circle(float cx, float cy, float radius, float &out_distance);

	void get_astar_thread_info(ALLEGRO_THREAD **thread, A_Star::A_Star_Thread_Info **ati);
	void set_astar_thread_info(ALLEGRO_THREAD *thread, A_Star::A_Star_Thread_Info *ati);

	float get_current_speed();

	std::string get_name();

	bool is_visible();
	void set_visible(bool visible);

	// Collidable interface
	Collidable_Type collidable_get_type();
	void collidable_get_position(
		General::Point<float> &pos
	);
	void collidable_get_bones(
		std::vector< Bones::Bone > &bones
	);

	void face(General::Point<float> point);

	std::vector<Bones::Bone> &get_current_bones();

	void set_facing_right(General::Direction direction);
	
	float get_run_factor();

	bool is_stationary();
	void set_stationary(bool stationary);

	void construct(std::string name);

	bool get_collided_with_entity();
	void set_collided_with_entity(bool collided);

	void set_panning_to_entity(int ent);

	bool get_entities_slide_on_self() { return entities_slide; }
	void set_entities_slide_on_self(bool slide ) { entities_slide = slide; }

	Map_Entity(std::string name);
	virtual ~Map_Entity();

protected:
	void init_lua();
	void run_lua_logic();
	lua_State *lua_state;

	int layer;

	std::vector<Camera *> cameras;

	float input[NUM_INPUTS];

	float speed;

	std::string name;

	General::Direction direction;

	int sort_offset; // subtraced from y for sorting
	General::Point<int> bb_offset;
	General::Size<int> bb_size;

	//std::vector<Triangulate::Triangle> collision_polygon;
	//std::vector< General::Point<float> > polygon_outline;

	std::list<Map_Entity *> current_collisions;

	bool solid_with_area;
	bool solid_with_entities;

	bool input_disabled;
	bool move_cameras_while_input_disabled;

	bool show_shadow;

	ALLEGRO_THREAD *astar_thread;
	A_Star::A_Star_Thread_Info *ati;

	bool visible;

	Area_Loop *area_loop;
	Area_Manager *area;
	
	std::map< std::pair<std::string, int>, std::vector<Bones::Bone> > bones;

	bool facing_right;

	bool stationary;

	bool role_paused;
	bool collided_with_entity;
	bool colliding_with_ladder;

	bool moved_out_of_collision_start;

	int pan_to_entity;
	bool panning_back_to_player;

	bool entities_slide;
};

#endif // MAP_ENTITY_H
