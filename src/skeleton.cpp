#include <allegro5/allegro_color.h>

#include "skeleton.h"
#include "xml.h"
#include "battle_loop.h"
#include "engine.h"
#include "resource_manager.h"

#include <cmath>

// FIXME: set to 1 for skeled
#ifdef ALLEGRO_RASPBERRYPI
#define TRANSFORM_UPDATE 3
#else
#define TRANSFORM_UPDATE 1
#endif

static int curr_part_name = 0;

static void update_parts(Skeleton::Link *l)
{
	if (l->part) {
		l->part->update();
	}

	for (int i = 0; i < l->num_children; i++) {
		update_parts(l->children[i]);
	}
}

static ALLEGRO_TRANSFORM *combine_transforms(ALLEGRO_TRANSFORM *start, std::vector<Skeleton::Transform *> transforms)
{
	static ALLEGRO_TRANSFORM t;

	al_identity_transform(&t);

	for (size_t i = 0; i < transforms.size(); i++) {
		Skeleton::Transform *transform = transforms[i];
		switch (transform->type) {
			case Skeleton::TRANSLATION:
				al_translate_transform(&t, transform->x, transform->y);
				break;
			case Skeleton::ROTATION:
				al_rotate_transform(&t, transform->angle);
				break;
			case Skeleton::SCALE:
				al_scale_transform(&t, transform->scale_x, transform->scale_y);
				break;
			default:
				break;
		}
	}
	
	if (start) {
		al_compose_transform(&t, start);
	}

	return &t;
}

static void recurse(General::Point<float> offset, Skeleton::Link *l, ALLEGRO_TRANSFORM t, bool draw_it, bool flip_it, int layer, ALLEGRO_COLOR tint, bool reversed)
{
	if (l->part == NULL) {
		return;
	}

	Wrap::Bitmap *bitmap = l->part->get_bitmap();

	ALLEGRO_TRANSFORM bone_trans;

	if (!draw_it) {
		al_copy_transform(&l->new_trans, combine_transforms(&t, l->part->get_transforms()));

		al_identity_transform(&l->final_trans);
		al_compose_transform(&l->final_trans, &l->new_trans);
		if (flip_it) {
			al_scale_transform(&l->final_trans, -1, 1);
		}
		al_copy_transform(&bone_trans, &l->final_trans);
		al_translate_transform(&l->final_trans, offset.x, offset.y);
	}

	if (l->part->get_layer() == layer) {
		if (draw_it) {
			ALLEGRO_TRANSFORM t, backup;

			al_copy_transform(&backup, al_get_current_transform());

			al_copy_transform(&t, &l->final_trans);
			al_compose_transform(&t, al_get_current_transform());
			al_use_transform(&t);

			al_draw_tinted_bitmap(bitmap->bitmap, tint, 0, 0, 0);

			al_use_transform(&backup);

			// KEEPME draw bones
			/*
			Battle_Loop *bl = GET_BATTLE_LOOP;
			if (bl) {
				General::Point<float> top = bl->get_top();
				std::vector< std::vector<Bones::Bone> > &transformed_bones = l->part->get_transformed_bones();
				std::vector<Bones::Bone> &b = transformed_bones[l->part->get_curr_bitmap()];
				for (size_t i = 0; i < b.size(); i++) {
					Bones::Bone &bone = b[i];
					std::vector<Triangulate::Triangle> tris;
					tris = bone.get();
					for (size_t j = 0; j < tris.size(); j++) {
						Triangulate::Triangle &t = tris[j];
						ALLEGRO_COLOR color;
						if (bone.type == Bones::BONE_ATTACK) {
							color = al_color_name("green");
						}
						else {
							color = al_color_name("white");
						}
						al_draw_triangle(
							t.points[0].x+offset.x,
							t.points[0].y+offset.y,
							t.points[1].x+offset.x,
							t.points[1].y+offset.y,
							t.points[2].x+offset.x,
							t.points[2].y+offset.y,
							color,
							1
						);
					}
				}
			}
			*/
		}
		else {
			std::vector< std::vector<Bones::Bone> > &bones = l->part->get_bones();
			std::vector< std::vector<Bones::Bone> > &transformed_bones = l->part->get_transformed_bones();

			for (size_t i = 0; i < bones.size(); i++) {
				std::vector<Bones::Bone> &src = bones[i];
				std::vector<Bones::Bone> &dst = transformed_bones[i];

				for (size_t j = 0; j < src.size(); j++) {
					Bones::Bone &b = src[j];
					Bones::Bone &b2 = dst[j];
					std::vector< General::Point<float> > &outline = b.get_outline();
					std::vector< General::Point<float> > &new_outline = b2.get_outline();
					
					int bmp_w = al_get_bitmap_width(l->part->get_bitmap(j)->bitmap);
					int bmp_h = al_get_bitmap_height(l->part->get_bitmap(j)->bitmap);

					for (size_t k = 0; k < outline.size(); k++) {
						General::Point<float> &p = outline[k];
						General::Point<float> &p2 = new_outline[k];
						p2.x = p.x;
						p2.y = p.y;
						p2.x += bmp_w/2;
						p2.y += bmp_h;
						al_transform_coordinates(&bone_trans, &p2.x, &p2.y);
					}

					std::vector<Triangulate::Triangle> &triangles = b.get();
					std::vector<Triangulate::Triangle> &new_triangles = b2.get();

					for (size_t k = 0; k < triangles.size(); k++) {
						for (int l = 0; l < 3; l++) {
							General::Point<float> &p = triangles[k].points[l];
							General::Point<float> &p2 = new_triangles[k].points[l];
							p2.x = p.x;
							p2.y = p.y;
							p2.x += bmp_w/2;
							p2.y += bmp_h;
							al_transform_coordinates(&bone_trans, &p2.x, &p2.y);
						}
					}
				}
			}
		}
	}
	
	for (int i = 0; i < l->num_children; i++) {
		recurse(offset, l->children[i], l->new_trans, draw_it, flip_it, layer, tint, reversed);
	}
}

static void interpolate_part(float ratio, Skeleton::Part *a, Skeleton::Part *b, Skeleton::Part *result)
{
	std::vector<Skeleton::Transform *> &ta = a->get_transforms();
	std::vector<Skeleton::Transform *> &tb = b->get_transforms();
	std::vector<Skeleton::Transform *> &tresult = result->get_transforms();

	for (size_t i = 0; i < ta.size(); i++) {
		Skeleton::Transform *tr_a = ta[i];
		Skeleton::Transform *tr_b = tb[i];
		Skeleton::Transform *tr_result = tresult[i];
		switch (tr_a->type) {
			case Skeleton::TRANSLATION:
				tr_result->x = General::interpolate(tr_a->x, tr_b->x, ratio);
				tr_result->y = General::interpolate(tr_a->y, tr_b->y, ratio);
				break;
			case Skeleton::ROTATION:
				tr_result->angle = General::interpolate(tr_a->angle, tr_b->angle, ratio);
				break;
			case Skeleton::SCALE:
				tr_result->scale_x = General::interpolate(tr_a->scale_x, tr_b->scale_x, ratio);
				tr_result->scale_y = General::interpolate(tr_a->scale_y, tr_b->scale_y, ratio);
				break;
			case Skeleton::BITMAP:
				tr_result->bitmap_index = tr_a->bitmap_index;
				break;
		}
	}

	result->update();
}

static void find_used_layers(Skeleton::Link *l, std::vector<int> &v)
{
	if (l->part == NULL) {
		return;
	}
	
	int layer = l->part->get_layer();

	if (std::find(v.begin(), v.end(), layer) == v.end()) {
		v.push_back(layer);
	}

	for (int i = 0; i < l->num_children; i++) {
		find_used_layers(l->children[i], v);
	}
}

namespace Skeleton {

Link *new_link()
{
	Link *l = new Link;
	l->part = NULL;
	l->num_children = 0;
	l->children= NULL;
	return l;
}

void clone_link(Link *clone_to, Link *to_clone)
{
	if (to_clone->part) {
		clone_to->part = to_clone->part->clone();
	}
	else {
		clone_to->part = NULL;
	}

	clone_to->num_children = to_clone->num_children;
	clone_to->children = new Link *[clone_to->num_children];

	for (int i = 0; i < clone_to->num_children; i++) {
		clone_to->children[i] = new_link();
		clone_link(clone_to->children[i], to_clone->children[i]);
	}
}

Transform *clone_transform(Transform *t)
{
	Transform *cloned = new Transform;
	memcpy(cloned, t, sizeof(Transform));
	return cloned;
}

void interpolate(float ratio, Link *a, Link *b, Link *result)
{
	if (a->part == NULL || b->part == NULL || result->part == NULL) {
		return;
	}

	interpolate_part(ratio, a->part, b->part, result->part);

	for (int i = 0; i < a->num_children; i++) {
		interpolate(ratio, a->children[i], b->children[i], result->children[i]);
	}
}

void destroy_links(Link *l)
{
	if (l == NULL) {
		return;
	}

	delete l->part;

	for (int i = 0; i < l->num_children; i++) {
		destroy_links(l->children[i]);
	}

	delete l;
}

void Skeleton::do_recurse(General::Point<float> offset, bool is_draw, bool flip, ALLEGRO_COLOR tint)
{
	if (animations.size() <= 0 || animations[curr_anim]->frames.size() <= 0) {
		return;
	}

	Animation *anim = animations[curr_anim];

	if (anim->work == NULL) {
		return;
	}

	if (reversed) {
		flip = !flip;
	}

	std::vector<int> used_layers;
	find_used_layers(anim->work, used_layers);
	std::sort(used_layers.begin(), used_layers.end());

	ALLEGRO_TRANSFORM t;
	al_identity_transform(&t);

	for (size_t i = 0; i < used_layers.size(); i++) {
		recurse(offset, anim->work, t, is_draw, flip, used_layers[i], tint, reversed);
	}
}

void Skeleton::draw(General::Point<float> offset, bool flip, ALLEGRO_COLOR tint)
{
	do_recurse(offset, true, flip, tint);
}

void Skeleton::transform(General::Point<float> offset, bool flip)
{
	transform_count++;
	if (transform_count == TRANSFORM_UPDATE) {
		transform_count = 0;
		do_recurse(offset, false, flip, al_color_name("white"));
	}

	last_transform_offset = offset;
	last_transform_flip = flip;
}

void Skeleton::update(int millis)
{
	if (animations.size() <= 0 || animations[curr_anim]->frames.size() <= 0) {
		return;
	}

	animations[curr_anim]->curr_time += millis;

	while (true) {
		int delay = animations[curr_anim]->delays[animations[curr_anim]->curr_frame];
		if (animations[curr_anim]->curr_time >= delay) {
			animations[curr_anim]->curr_time -= delay;
			animations[curr_anim]->curr_frame++;
			animations[curr_anim]->curr_frame %= animations[curr_anim]->frames.size();
			if (animations[curr_anim]->curr_frame == 0) {
				animations[curr_anim]->loops++;
			}
		}
		else {
			break;
		}
	}

	interpolate_now();

	update_parts(animations[curr_anim]->work);
}

Skeleton::Skeleton()
{
	curr_anim = 0;
	reversed = false;
	transform_count = 0;
}

Skeleton::Skeleton(const std::string &filename)
{
	curr_anim = 0;
	reversed = false;
	transform_count = 0;

	this->filename = "skeletons/xml/" + filename;
}

Skeleton::~Skeleton()
{
	for (size_t i = 0; i < animations.size(); i++) {
		Animation *anim = animations[i];
		for (size_t j = 0; j < anim->frames.size(); j++) {
			destroy_links(anim->frames[j]);
		}
		destroy_links(anim->work);
	}
}

void read_xml(XMLData *xmlpart, Link *link)
{
	std::string partname = xmlpart->find("name")->get_value();

	XMLData *xml_layer = xmlpart->find("layer");
	int layer;
	if (xml_layer) {
		layer = atoi(xml_layer->get_value().c_str());
	}
	else {
		layer = 0;
	}
	
	XMLData *xmlbmp = xmlpart->find("bitmaps");
	std::string xmlbmpvalue = xmlbmp->get_value();

	std::vector<Wrap::Bitmap *> bitmaps;
	std::vector<std::string> bitmap_names;
	std::vector< std::vector<Bones::Bone> > bones;
	std::vector<Bones::Bone> bone;

	std::string name = General::itos(curr_part_name++);

	size_t current;
	size_t next = -1;
	do {
		current = next + 1;
		next = xmlbmpvalue.find_first_of(",", current);
		std::string name = xmlbmpvalue.substr(current, next - current);
		Wrap::Bitmap *bitmap = resource_manager->reference_bitmap("skeletons/parts/" + name);
		bitmaps.push_back(bitmap);
		bitmap_names.push_back(name);
		std::string bones_filename = "skeletons/parts/bones/" + name;
		bones_filename = bones_filename.replace(bones_filename.length()-3, 3, "xml");
		bone.clear();
		Bones::load(bone, al_get_bitmap_width(bitmap->bitmap), al_get_bitmap_height(bitmap->bitmap), bones_filename);
		bones.push_back(bone);
	} while (next != std::string::npos);

	std::vector<Transform *> transforms;

	XMLData *xmltransforms = xmlpart->find("transforms");
	if (xmltransforms) {
		std::list<XMLData *> &nodes = xmltransforms->get_nodes();

		std::list<XMLData *>::iterator it = nodes.begin();
		for (; it != nodes.end(); it++) {
			XMLData *node = *it;
			XMLData *type = node->find("type");
			std::string typeS = type->get_value();
			Type t = (Type)atoi(typeS.c_str());
			Transform *transform = new Transform;
			transform->type = t;
			switch (t) {
				case TRANSLATION: {
						XMLData *x = node->find("x");
						XMLData *y = node->find("y");
						transform->x = atof(x->get_value().c_str());
						transform->y = atof(y->get_value().c_str());
					}
					break;
				case ROTATION: {
						XMLData *angle = node->find("angle");
						transform->angle = atof(angle->get_value().c_str());
					}
					break;
				case SCALE: {
						XMLData *scale_x = node->find("scale_x");
						XMLData *scale_y = node->find("scale_y");
						transform->scale_x = atof(scale_x->get_value().c_str());
						transform->scale_y = atof(scale_y->get_value().c_str());
					}
					break;
				case BITMAP: {
						XMLData *bitmap_index = node->find("bitmap_index");
						transform->bitmap_index = atoi(bitmap_index->get_value().c_str());
					}
					break;
			}
			transforms.push_back(transform);
		}
	}

	Part *part = new Part(partname, transforms, bitmaps);

	part->set_layer(layer);

	part->get_bitmap_names() = bitmap_names;

	for (size_t i = 0; i < bones.size(); i++) {
		part->add_bone(bones[i]);
	}

	link->part = part;

	XMLData *xmlchildren = xmlpart->find("children");
	if (xmlchildren == NULL) {
		link->num_children = 0;
	}
	else {
		std::list<XMLData *> &childnodes = xmlchildren->get_nodes();
		link->num_children = childnodes.size();

		std::list<XMLData *>::iterator it = childnodes.begin();

		link->children = new Link *[childnodes.size()];
		int i = 0;

		for (; it != childnodes.end(); it++) {
			XMLData *child = *it;
			link->children[i] = new Link;
			read_xml(child, link->children[i++]);
		}
	}
}

void print_links(Link *l, int tabs)
{
	for (int i = 0; i < tabs; i++) {
		printf("\t");
	}
	printf("%p\n", l);
	l->part->print(tabs);
	for (int i = 0; i < l->num_children; i++) {
		print_links(l->children[i], tabs+1);
	}
}

bool Skeleton::load()
{
	const ALLEGRO_FILE_INTERFACE *fs = al_get_new_file_interface();
	al_set_standard_file_interface();

	XMLData *xml = new XMLData(filename);

	if (xml->failed()) {
		General::log_message("Loading skeleton failed (XMLData->failed == true)");
		delete xml;
		al_set_new_file_interface(fs);
		return false;
	}

	std::list<XMLData *> &xmlanims = xml->get_nodes();
	std::list<XMLData *>::iterator it = xmlanims.begin();

	for (; it != xmlanims.end(); it++) {
		XMLData *node = *it;
		if (node->get_name() == "reversed") {
			reversed = atoi(node->get_value().c_str());
			continue;
		}
		Animation *anim = new Animation;
		anim->curr_frame = 0;
		anim->curr_time = 0;
		std::string name = node->get_name();
		anim->name = name;
		std::list<XMLData *> &xmldata = node->get_nodes();
		std::list<XMLData *>::iterator it2 = xmldata.begin();

		for (; it2 != xmldata.end(); it2++) {
			XMLData *node2 = *it2;
			std::string tagname = node2->get_name();
			if (tagname == "delays") {
				std::list<XMLData *> &xmldelays = node2->get_nodes();
				std::list<XMLData *>::iterator it3 = xmldelays.begin();
				for (; it3 != xmldelays.end(); it3++) {
					XMLData *node3 = *it3;
					int delay = atof(node3->get_value().c_str());
					anim->delays.push_back(delay);
				}
			}
			else if (tagname == "part") {
				Link *link = new Link;
				read_xml(node2, link);
				anim->frames.push_back(link);
				if (anim->frames.size() == 1) {
					anim->work = new_link();
					animations.push_back(anim);
				}
				read_xml(node2, anim->work);
			}
		}
	}

	al_set_new_file_interface(fs);

	for (size_t i = 0; i < animations.size(); i++) {	
		Animation *anim = animations[curr_anim];
		anim->curr_frame = 0;
		anim->curr_time = 0;
		anim->loops = 0;
	}

	return true;
}

void Skeleton::set_curr_anim(int index)
{
	curr_anim = (index >= 0 && index < (int)animations.size()) ? index : 0;

	interpolate_now();

	transform(last_transform_offset, last_transform_flip);
}

void Skeleton::set_curr_anim(const std::string &name)
{
	for (size_t i = 0; i < animations.size(); i++) {
		if (animations[i]->name == name) {
			curr_anim = i;

			interpolate_now();

			transform(last_transform_offset, last_transform_flip);

			return;
		}
	}
}

int Skeleton::get_curr_anim()
{
	return curr_anim;
}

std::string Skeleton::get_curr_anim_name()
{
	return animations[curr_anim]->name;
}

std::vector<Animation *> &Skeleton::get_animations()
{
	return animations;
}

void Skeleton::reset_current_animation()
{
	Animation *anim = animations[curr_anim];
	anim->curr_frame = 0;
	anim->curr_time = 0;
	anim->loops = 0;

	interpolate_now();
}

int Skeleton::get_loops()
{
	Animation *anim = animations[curr_anim];
	return anim->loops;
}

void Skeleton::interpolate_now()
{
	int next_frame = (animations[curr_anim]->curr_frame + 1) % (animations[curr_anim]->frames.size());
	float ratio = (float)animations[curr_anim]->curr_time / animations[curr_anim]->delays[animations[curr_anim]->curr_frame];

	interpolate(
		ratio,
		animations[curr_anim]->frames[animations[curr_anim]->curr_frame],
		animations[curr_anim]->frames[next_frame],
		animations[curr_anim]->work
	);
}

/* Should only be called on 'work' */
void Part::update()
{
	for (size_t i = 0; i < transforms.size(); i++) {
		if (transforms[i]->type == BITMAP) {
			curr_bitmap = transforms[i]->bitmap_index;
			return;
		}
	}
}

Part::Part(
	std::string name,
	std::vector<Transform *> transforms,
	std::vector<Wrap::Bitmap *> bitmaps
	) :
	curr_bitmap(0),
	name(name),
	transforms(transforms),
	bitmaps(bitmaps),
	layer(0)
{
}

Part::~Part()
{
	for (size_t i = 0; i < bitmaps.size(); i++) {
		resource_manager->release_bitmap(bitmaps[i]->filename);
	}

	for (size_t i = 0; i < transforms.size(); i++) {
		delete transforms[i];
	}
}

void Part::print(int tabs)
{
	for (size_t i = 0; i < transforms.size(); i++) {
		for (int j = 0; j < tabs; j++) {
			printf("\t");
		}
		printf("%d ", transforms[i]->type);
		switch (transforms[i]->type) {
			case TRANSLATION:
				printf("%g %g\n", transforms[i]->x, transforms[i]->y);
				break;
			case ROTATION:
				printf("%g\n", transforms[i]->angle);
				break;
			case SCALE:
				printf("%g %g\n", transforms[i]->scale_x, transforms[i]->scale_y);
				break;
			case BITMAP:
				printf("%d\n", transforms[i]->bitmap_index);
				break;
		}
	}
}
	
std::vector<Transform *> &Part::get_transforms(void)
{
	return transforms;
}

Wrap::Bitmap *Part::get_bitmap(int index)
{
	if (index < 0) index = curr_bitmap;
	return bitmaps[index];
}

const std::string &Part::get_name()
{
	return name;
}

Part *Part::clone()
{
	std::vector<Transform *> new_transforms;
	for (size_t i = 0; i < transforms.size(); i++) {
		Transform *t = clone_transform(transforms[i]);
		new_transforms.push_back(t);
	}
	std::vector<Wrap::Bitmap *> new_bitmaps;
	for (size_t i = 0; i < bitmaps.size(); i++) {
		// FIXME: LOAD HERE NOT CREATE
		Wrap::Bitmap *b = Wrap::create_bitmap(al_get_bitmap_width(bitmaps[i]->bitmap), al_get_bitmap_height(bitmaps[i]->bitmap));
		ALLEGRO_STATE state;
		al_store_state(&state, ALLEGRO_STATE_BLENDER | ALLEGRO_STATE_TARGET_BITMAP);
		al_set_target_bitmap(b->bitmap);
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
		al_draw_bitmap(bitmaps[i]->bitmap, 0, 0, 0);
		al_restore_state(&state);
		new_bitmaps.push_back(b);
	}
	Part *p = new Part(name, new_transforms, new_bitmaps);
	p->set_layer(layer);
	p->get_bitmap_names() = bitmap_names;
	p->curr_bitmap = curr_bitmap;
	p->bones = bones;
	p->transformed_bones = transformed_bones;
	return p;
}

} // end namespace Skeleton
