#ifndef SKELETON_H
#define SKELETON_H

#include <allegro5/allegro.h>

#include <map>

#include <wrap.h>

#include "general.h"
#include "bones.h"

namespace Skeleton {

enum Type {
	TRANSLATION = 0,
	ROTATION,
	SCALE,
	BITMAP
};

struct Transform {
	Type type;
	float x, y; // For translation
	float angle; // For rotation
	float scale_x, scale_y; // For scaling
	int bitmap_index;
};

class Part
{
public:
	const std::string &get_name();
	Wrap::Bitmap *get_bitmap(int index = -1); // default: current bitmap
	std::vector<Wrap::Bitmap *> &get_bitmaps() { return bitmaps; }
	std::vector<std::string> &get_bitmap_names() { return bitmap_names; }
	int get_curr_bitmap() { return curr_bitmap; }
	std::vector<Transform *> &get_transforms();
	std::vector< std::vector<Bones::Bone> > &get_bones() { return bones; }
	std::vector< std::vector<Bones::Bone> > &get_transformed_bones() { return transformed_bones; }
	void add_bone(std::vector<Bones::Bone> b) { bones.push_back(b); transformed_bones.push_back(std::vector<Bones::Bone>()); }
	int get_layer() { return layer; }
	void set_layer(int layer) { this->layer = layer; }

	General::Point<float> get_position() { return pos; }
	void set_position(General::Point<float> pos) { this->pos = pos; }

	Part *clone();

	void update();

	void print(int tabs);

	Part(std::string name, std::vector<Transform *> transforms, std::vector<Wrap::Bitmap *> bitmaps);

	~Part();

private:
	int curr_bitmap;

	std::string name;

	/* Transformation 0 should be pivot if there is one */
	std::vector<Transform *> transforms;

	std::vector<Wrap::Bitmap *> bitmaps;
	std::vector<std::string> bitmap_names;

	/* For collision detection */
	std::vector< std::vector<Bones::Bone> > bones;
	std::vector< std::vector<Bones::Bone> > transformed_bones;

	int layer;

	General::Point<float> pos;
};

struct Link
{
	int num_children;
	Link **children;
	Part *part;
};

struct Animation
{
	int curr_frame;
	int curr_time; // in milliseconds
	std::string name;
	std::vector<Link *> frames;
	std::vector<int> delays;
	Link *work;
	int loops;
};

void destroy_links(Link *l);
void interpolate(float ratio, Skeleton::Link *a, Skeleton::Link *b, Skeleton::Link *result);
Transform *clone_transform(Transform *t);
Link *new_link();
void clone_link(Link *clone_to, Link *to_clone);

class Skeleton
{
public:
	bool load();

	void set_curr_anim(int index);
	void set_curr_anim(const std::string &name);
	int get_curr_anim();
	std::string get_curr_anim_name();
	std::vector<Animation *> &get_animations();
	void reset_current_animation();

	void draw(General::Point<float> offset, bool flipped, ALLEGRO_COLOR tint);
	void transform(General::Point<float> offset, bool flipped); // Update all transformed_bones
	void update(int millis);

	bool is_reversed() { return reversed; }

	int get_loops();

	Skeleton();
	Skeleton(const std::string &filename);

	~Skeleton();

private:
	void do_recurse(General::Point<float> offset, bool is_draw, bool flip, ALLEGRO_COLOR tint);
	void interpolate_now();

	int curr_anim;
	std::vector<Animation *> animations;

	std::string filename;

	bool reversed;

	int transform_count;
};

} // End namespace Skeleton

#endif // SKELETON_H
