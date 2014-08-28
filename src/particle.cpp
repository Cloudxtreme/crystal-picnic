#include <allegro5/allegro.h>

#include "particle.h"
#include "luainc.h"
#include "area_loop.h"
#include "area_manager.h"
#include "battle_loop.h"
#include "engine.h"
#include "resource_manager.h"

static void generic_add_particle(Particle::Particle *p)
{
	Battle_Loop *loop = GET_BATTLE_LOOP;
	if (loop) {
		loop->add_particle(p);
	}
	else {
		Area_Loop *al = GET_AREA_LOOP;
		if (al) {
			al->get_area()->add_particle(p);
		}
	}
}

namespace Particle {

static int particle_group_id = 1;

Particle_Group::Particle_Group(std::string type, int layer, int align, std::vector<std::string> bitmap_names)
{
	this->type = type;
	id = particle_group_id++;
	this->layer = layer;
	num_bitmaps = 0;
	alignment = align;
	reference_count = 0;
	this->bitmap_names = bitmap_names;
	init(type);
}

Particle_Group::~Particle_Group()
{
	Lua::call_lua(lua_state, "stop", ">", id);
	lua_close(lua_state);

	if (num_bitmaps > 0) {
		for (int i = 0; i < num_bitmaps; i++) {
			resource_manager->release_bitmap("misc_graphics/particles/" + bitmap_names[i] + ".cpi");
		}
		delete[] bitmaps;
	}
}

void Particle_Group::init(std::string type)
{
	init_lua(type);
	init_bitmaps(type);
}

// Each type of particle has its own script
void Particle_Group::init_lua(std::string type)
{
	lua_state = luaL_newstate();
	Lua::open_lua_libs(lua_state);
	Lua::register_c_functions(lua_state);

	Lua::load_global_scripts(lua_state);

	unsigned char *bytes;

	bytes = General::slurp("scripts/particles/global.lua");
	if (luaL_loadstring(lua_state, (char *)bytes)) {
		Lua::dump_lua_stack(lua_state);
		throw Error("Error loading global particle script.\n");
	}
	delete[] bytes;
	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running global particle script.");
	}

	bytes = General::slurp("scripts/particles/" + type + ".lua");
	if (luaL_loadstring(lua_state, (char *)bytes)) {
		Lua::dump_lua_stack(lua_state);
		throw Error("Error loading particle script.\n");
	}
	delete[] bytes;
	if (lua_pcall(lua_state, 0, 0, 0)) {
		Lua::dump_lua_stack(lua_state);
		lua_close(lua_state);
		throw Error("Error running particle script.");
	}

	Lua::call_lua(lua_state, "start", "i>", id);
}

void Particle_Group::init_bitmaps(std::string type)
{
	num_bitmaps = bitmap_names.size();
	bitmaps = new Wrap::Bitmap *[num_bitmaps];
	for (int i = 0; i < num_bitmaps; i++) {
		bitmaps[i] = resource_manager->reference_bitmap("misc_graphics/particles/" + bitmap_names[i] + ".cpi");
	}
}

void Particle::init()
{
	Lua::call_lua(particle_group->lua_state, "start_particle", "i>", id);
}

void Particle::logic()
{
	if (delete_me) {
		return;
	}
	
	Lua::call_lua(particle_group->lua_state, "logic", "i>", id);
}

void Particle::draw()
{
	int topx, topy;

	Battle_Loop *battle_loop = GET_BATTLE_LOOP;
	if (battle_loop) {
		General::Point<float> top = battle_loop->get_top();
		topx = top.x;
		topy = top.y;
	}
	else {
		Area_Loop *area_loop = GET_AREA_LOOP;
		if (area_loop) {
			General::Point<float> top = area_loop->get_area()->get_top();
			topx = top.x;
			topy = top.y;
		}
		else {
			return;
		}
	}

	if (battle_loop && bullet_time_len > 0.0f && bullet_time_start.x > General::SMALL_FLOAT) {
		float dx = pos.x - bullet_time_start.x;
		float dy = pos.y - bullet_time_start.y;
		float len = sqrt(dx*dx + dy*dy);
		General::Point<float> start;
		float angle = atan2(dy, dx);
		if (len > bullet_time_len) {
			start = General::Point<float>(
				bullet_time_start.x + cos(angle) * (len-bullet_time_len),
				bullet_time_start.y + sin(angle) * (len-bullet_time_len)
			);
			len = bullet_time_len;
		}
		else {
			start = bullet_time_start;
		}

		float perp = M_PI/2.0f;

		start.x += cos(perp)*-2.0f;
		start.y += sin(perp)*-2.0f;
		start.x -= topx;
		start.y -= topy;
		General::Point<float> end(
			pos.x+cos(perp)*2.0f-topx,
			pos.y+sin(perp)*2.0f-topy
		);

		al_hold_bitmap_drawing(false);
		battle_loop->draw_bullet_time(start, end, 4.0f);
		al_hold_bitmap_drawing(true);
	}

	if (bitmap_index >= 0) {
		if (angle != 0.0f) {
			ALLEGRO_BITMAP *bmp = particle_group->bitmaps[bitmap_index]->bitmap;
			al_draw_tinted_scaled_rotated_bitmap(
				bmp,
				tint,
				width/2, height/2,
				pos.x-topx+draw_offset.x, pos.y-topy+draw_offset.y,
				xscale, yscale,
				angle,
				(facing_right ? 0 : ALLEGRO_FLIP_HORIZONTAL) | extra_draw_flags
			);
		}
		else {
			al_draw_tinted_bitmap(
				particle_group->bitmaps[bitmap_index]->bitmap,
				tint,
				pos.x-topx-al_get_bitmap_width(particle_group->bitmaps[bitmap_index]->bitmap)/2+draw_offset.x,
				pos.y-topy-al_get_bitmap_height(particle_group->bitmaps[bitmap_index]->bitmap)/2+draw_offset.y,
				(facing_right ? 0 : ALLEGRO_FLIP_HORIZONTAL) | extra_draw_flags
			);
		}
	}
	else {
		al_draw_filled_circle(pos.x-topx+draw_offset.x, pos.y-topy+draw_offset.y, (width+height)/4, tint);
	}
}

Particle::Particle(
		int group, float width, float height, ALLEGRO_COLOR tint, int bitmap_index, int hit_dir, bool facing_right, bool hard_hitting
	)
{
	group_id = group;
	Particle_Group *pg = engine->get_particle_group(group);
	
	if (pg) {
		particle_group = pg;
		this->width = width;
		this->height = height;
		this->tint = tint;
		this->bitmap_index = bitmap_index;
		this->hit_dir = hit_dir;
		this->facing_right = facing_right;
		this->hard_hitting = hard_hitting;
		hidden = false;
		pg->reference_count++;
	}
	else {
		delete_me = true;
	}

	bullet_time_len = 0.0f;
	bullet_time_start = General::Point<float>(General::SMALL_FLOAT, General::SMALL_FLOAT);

	angle = 0.0f;

	high = false;

	xscale = yscale = 1;

	draw_offset = General::Point<int>(0, 0);

	extra_draw_flags = 0;

	damage = 1;
}

Particle::~Particle()
{
	particle_group->reference_count--;
	if (particle_group->reference_count <= 0) {
		engine->delete_particle_group(group_id);
	}
}

Collidable_Type Particle::collidable_get_type() 
{
	return COLLIDABLE_BOX;
}

void Particle::collidable_get_position(
		General::Point<float> &pos
	)
{
	pos = this->pos;
}

void Particle::collidable_get_box(
		General::Point<int> &offset,
		General::Size<int> &size
	)
{
	offset.x = -this->width/2;
	offset.y = -this->height;
	size.w = this->width;
	size.h = this->height;
}

int Particle::get_hit_direction()
{
	return hit_dir;
}

bool Particle::is_facing_right()
{
	return facing_right;
}

bool Particle::is_hard_hitting()
{
	return hard_hitting;
}

void Particle::set_position(General::Point<float> pos)
{
	Positioned_Entity::set_position(pos);
	if (bullet_time_len > 0.0f && bullet_time_start.x == General::SMALL_FLOAT) {
		bullet_time_start = pos;
	}
}

Particle *add_particle(int group, float width, float height, ALLEGRO_COLOR tint, int bitmap_index, int hit_dir, bool facing_right, bool hard_hitting)
{
	Particle *p = new Particle(
		group,
		width, height,
		tint,
		bitmap_index,
		hit_dir,
		facing_right,
		hard_hitting
	);

	generic_add_particle(p);

	p->init();

	return p;
}

void remove_particle(int id)
{
	Area_Loop *al = GET_AREA_LOOP;
	if (al) {
		al->get_area()->remove_particle(id);
	}
	else {
		Battle_Loop *bl = GET_BATTLE_LOOP;
		if (bl) {
			bl->remove_particle(id);
		}
	}
}

Particle *get_particle(int id)
{
	Area_Loop *al = GET_AREA_LOOP;
	if (al) {
		return al->get_area()->get_particle(id);
	}
	else {
		Battle_Loop *loop = GET_BATTLE_LOOP;
		if (loop) {
			return loop->get_particle(id);
		}
	}

	return NULL;
}

} // end namespace Particle
