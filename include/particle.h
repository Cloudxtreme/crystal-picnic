#ifndef PARTICLE_H
#define PARTICLE_H

#include <string>

#include <wrap.h>

#include "collidable.h"
#include "positioned_entity.h"

#define PARTICLE_HURT_ENEMY 1
#define PARTICLE_HURT_PLAYER 2
#define PARTICLE_HURT_BOTH 3
#define PARTICLE_HURT_NONE 4
#define PARTICLE_HURT_EGBERT 5

struct lua_State;

namespace Particle {

class Particle_Group {
public:
	Particle_Group(std::string type, int layer, int align, std::vector<std::string> bitmap_names);
	~Particle_Group();

	std::string type;

	// used to add to group
	int id;

	// where to draw/collide (in battle there are no layers currently)
	int layer;

	// The "brain" of the particles
	// These two loaded based on type in constructor
	lua_State *lua_state;
	Wrap::Bitmap **bitmaps;

	// blackboard
	float data[100];

	int reference_count;

	int alignment;

private:
	void init(std::string type);
	void init_lua(std::string type);
	void init_bitmaps(std::string type);

	std::vector<std::string> bitmap_names;
	int num_bitmaps;
};

class Particle : public Positioned_Entity, public Collidable {
public:
	void init();
	void logic();
	void draw();

	void set_position(General::Point<float> pos);

	Particle(
		int group, float width, float height, ALLEGRO_COLOR tint, int bitmap_index, int hit_dir, bool facing_right, bool hard_hitting
	);
	virtual ~Particle();

	// Collidable interface
	Collidable_Type collidable_get_type() ;
	void collidable_get_position(
		General::Point<float> &pos
	);
	void collidable_get_box(
		General::Point<int> &offset,
		General::Size<int> &size
	);

	int get_hit_direction();
	bool is_facing_right();
	bool is_hard_hitting();

	float width;
	float height;
	ALLEGRO_COLOR tint;
	int bitmap_index; // into Particle_Group

	// Can get/set by index
	// NOTE: velocity/whatever can use data
	float data[100];

	// One of these shared by all particles in a group, last one destroys
	Particle_Group *particle_group;
	int group_id;

	int hit_dir;
	bool facing_right;
	bool hard_hitting;

	bool hidden;

	float bullet_time_len;
	General::Point<float> bullet_time_start;

	float angle;

	bool high;

	float xscale;
	float yscale;

	General::Point<int> draw_offset;

	int extra_draw_flags;

	int damage;
};

Particle *add_particle(int group, float width, float height, ALLEGRO_COLOR tint, int bitmap_index, int hit_dir, bool facing_right, bool hard_hitting);
void remove_particle(int id);
Particle *get_particle(int id);

} // end namespace Particle

#endif
