#include "collidable.h"
#include "collision_detection.h"
#include "bones.h"

bool Collidable::collides_with_point(
		General::Point<float> pos
	)
{
	if (collidable_get_type() == COLLIDABLE_POINT) {
		General::Point<float> this_pos;
		collidable_get_position(this_pos);
		return checkcoll_point_point(
			pos,
			this_pos
		);
	}
	else if (collidable_get_type() == COLLIDABLE_BOX) {
		General::Point<float> this_pos;
		General::Point<int> this_offset;
		General::Size<int> this_size;
		collidable_get_position(this_pos);
		collidable_get_box(this_offset, this_size);
		General::Point<float> topleft = General::Point<float>(
			this_pos.x + this_offset.x,
			this_pos.y + this_offset.y
		);
		General::Point<float> bottomright = General::Point<float>(
			topleft.x + this_size.w,
			topleft.y + this_size.h
		);
		return checkcoll_point_box(
			pos,
			topleft,
			bottomright
		);
	}
	else { // COLLIDABLE_BONES
		General::Point<float> this_pos;
		std::vector<Bones::Bone> bones;
		collidable_get_position(this_pos);
		collidable_get_bones(bones);
		for (size_t i = 0; i < bones.size(); i++) {
			std::vector< General::Point<float> > this_outline;
			this_outline = bones[i].get_outline();
			bool b = checkcoll_point_polygon(
				pos,
				this_outline,
				this_pos,
				0,
				this_outline.size()
			);
			if (b) {
				return true;
			}
		}
		return false;
	}
}

bool Collidable::collides_with_box(
		General::Point<float> pos,
		General::Point<int> offset,
		General::Size<int> size
	)
{
	if (collidable_get_type() == COLLIDABLE_POINT) {
		General::Point<float> this_pos;
		collidable_get_position(this_pos);
		General::Point<float> topleft = General::Point<float>(
			pos.x + offset.x,
			pos.y + offset.y
		);
		General::Point<float> bottomright = General::Point<float>(
			topleft.x + size.w,
			topleft.y + size.h
		);
		return checkcoll_point_box(
			this_pos,
			topleft,
			bottomright
		);
	}
	else if (collidable_get_type() == COLLIDABLE_BOX) {
		General::Point<float> this_pos;
		General::Point<int> this_offset;
		General::Size<int> this_size;
		collidable_get_position(this_pos);
		collidable_get_box(this_offset, this_size);
		General::Point<float> topleft1 = General::Point<float>(
			this_pos.x + this_offset.x,
			this_pos.y + this_offset.y
		);
		General::Point<float> bottomright1 = General::Point<float>(
			topleft1.x + this_size.w,
			topleft1.y + this_size.h
		);
		General::Point<float> topleft2 = General::Point<float>(
			pos.x + offset.x,
			pos.y + offset.y
		);
		General::Point<float> bottomright2 = General::Point<float>(
			topleft2.x + size.w,
			topleft2.y + size.h
		);
		return checkcoll_box_box(
			topleft1,
			bottomright1,
			topleft2,
			bottomright2
		);
	}
	else { // COLLIDABLE_BONES
		General::Point<float> this_pos;
		collidable_get_position(this_pos);
		std::vector<Bones::Bone> bones;
		collidable_get_bones(bones);
		General::Point<float> topleft = General::Point<float>(
			pos.x + offset.x,
			pos.y + offset.y
		);
		General::Point<float> bottomright = General::Point<float>(
			topleft.x + size.w,
			topleft.y + size.h
		);
		for (size_t i = 0; i < bones.size(); i++) {
			std::vector< General::Point<float> > this_outline = bones[i].get_outline();
			std::vector<Triangulate::Triangle> this_triangles = bones[i].get();
			bool b = checkcoll_box_polygon(
				topleft,
				bottomright,
				this_outline,
				this_pos,
				NULL
			);
			if (b) {
				return true;
			}
		}
		return false;
	}
}

bool Collidable::collides_with_bones(
		General::Point<float> pos,
		std::vector<Bones::Bone> &bones
	)
{
	if (collidable_get_type() == COLLIDABLE_POINT) {
		General::Point<float> this_pos;
		collidable_get_position(this_pos);
		std::vector<Bones::Bone> bones;
		collidable_get_bones(bones);
		for (size_t i = 0; i < bones.size(); i++) {
			std::vector< General::Point<float> > this_outline = bones[i].get_outline();
			bool b = checkcoll_point_polygon(
				this_pos,
				this_outline,
				pos,
				0,
				this_outline.size()
			);
			if (b) {
				return true;
			}
		}
	}
	else if (collidable_get_type() == COLLIDABLE_BOX) {
		General::Point<float> this_pos;
		General::Point<int> this_offset;
		General::Size<int> this_size;
		collidable_get_position(this_pos);
		collidable_get_box(this_offset, this_size);
		General::Point<float> topleft = General::Point<float>(
			this_pos.x + this_offset.x,
			this_pos.y + this_offset.y
		);
		General::Point<float> bottomright = General::Point<float>(
			topleft.x + this_size.w,
			topleft.y + this_size.h
		);
		std::vector<Bones::Bone> bones;
		collidable_get_bones(bones);
		for (size_t i = 0; i < bones.size(); i++) {
			std::vector< General::Point<float> > this_outline = bones[i].get_outline();
			std::vector<Triangulate::Triangle> this_triangles = bones[i].get();
			bool b = checkcoll_box_polygon(
				topleft,
				bottomright,
				this_outline,
				pos,
				NULL
			);
			if (b) {
				return true;
			}
		}
	}
	else { // COLLIDABLE_BONES
		General::Point<float> pos1;
		collidable_get_position(pos1);
		std::vector<Bones::Bone> bones1;
		collidable_get_bones(bones1);

		for (size_t i = 0; i < bones.size(); i++) {
			std::vector< General::Point<float> > outline2 = bones[i].get_outline();
			for (size_t j = 0; j < bones1.size(); j++) {
				std::vector< General::Point<float> > outline1 = bones1[i].get_outline();
				bool b = checkcoll_polygon_polygon(
					outline1, pos1,
					outline2, pos
				);
				if (b) {
					return true;
				}
			}
		}
	}

	return false;
}

bool Collidable::collides_with(Collidable &c)
{
	if (c.collidable_get_type() == COLLIDABLE_POINT) {
		General::Point<float> pos;
		c.collidable_get_position(pos);
		return collides_with_point(pos);
	}
	else if (c.collidable_get_type() == COLLIDABLE_BOX) {
		General::Point<float> pos;
		General::Point<int> offset;
		General::Size<int> size;
		c.collidable_get_position(pos);
		c.collidable_get_box(offset, size);
		return collides_with_box(pos, offset, size);
	}
	else { // COLLIDABLE_BONES
		General::Point<float> pos;
		std::vector<Bones::Bone> bones;
		c.collidable_get_position(pos);
		c.collidable_get_bones(bones);
		return collides_with_bones(pos, bones);
	}

	// Should never get here
	General::log_message("IMPOSSIBLE CONDITION: collides_with unknown");
	return false;
}

