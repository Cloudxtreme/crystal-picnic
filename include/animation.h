#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>

#include <vector>
#include <string>

#include "frame.h"
#include "general.h"

class Animation {
public:
	friend class Animation_Set;

	enum Loop_Mode {
		LOOP_NORMAL = 0,
		LOOP_PINGPONG
	};

	/* Tags */
	void set_tags(std::vector<std::string> tags);
	std::vector<std::string> get_tags()
	{
		return tags;
	}

	bool has_tag(std::string tagName) {
		for (int i = 0; i < (int)tags.size(); i++) {
			if (tags[i] == tagName)
				return true;
		}

		return false;
	}

	std::string get_tag(std::string tag_start) {
		for (int i = 0; i < (int)tags.size(); i++) {
			if (tags[i].substr(0, tag_start.length()) == tag_start)
				return tags[i];
		}

		return "";
	}

	/* Returns false on fail.
	 * frame is not copied so should not be destroyed.
	 */
	bool add_frame(Frame *frame);
	int get_length();

	/* -1 for current
	 */
	Frame *get_frame(int num)
	{
		return frames[num];
	}

	Frame *get_current_frame()
	{
		return frames[current_frame];
	}

	bool is_finished() {
		if (((!looping && ((loop_mode != LOOP_PINGPONG && current_frame == num_frames-1 &&
		count >= frames[current_frame]->get_delay()) ||
		((loop_mode == LOOP_PINGPONG) && (increment == -1) &&
			(current_frame == 0) && (count >= frames[0]->get_delay())))) /*||
		(num_frames == 1 && count >= 100)*/))
			return true;
		return false;
	}


	unsigned int get_current_frame_num()
	{
		return current_frame;
	}


	unsigned int get_num_frames()
	{
		return num_frames;
	}


	std::string get_name()
	{
		return name;
	}


	/* Returns how many frames passed
	 * Can go backwards.
	 * Step is in milliseconds.
	 */
	int update(int step);
	int update();

	void set_looping(bool l);
	void set_loop_mode(Loop_Mode m);
	void set_loop_start(int loop_start);
	bool is_looping() { return looping; }

	void reset();
	void set_frame(int frame);

	void draw(int x, int y, int flags);
	void draw_rotated(int x, int y, int flags);
	void draw_tinted(ALLEGRO_COLOR tint, int x, int y, int flags);
	void draw_add_tinted(ALLEGRO_COLOR tint, float p, int x, int y, int flags);
	void draw_tinted_rotated(ALLEGRO_COLOR tint, int x, int y, int flags);
	void draw_add_tinted_rotated(ALLEGRO_COLOR tint, float p, int x, int y, int flags);
	void draw_scaled(int sx, int sy, int sw, int sh, int dx, int dy,
		int dw, int dh, int flags);

	float get_angle() { return angle; }
	void set_angle(float a) { angle = a; }

	void set_angle_inc(float ai) { angle_inc = ai; }
	void set_rotation_center(General::Point<float> p) {
		rotation_center = p;
	}
	
	Animation(std::string name);
	/* Frames are destroyed
	 */
	~Animation();

protected:
	void wrap();

	std::vector<Frame *> frames;
	int num_frames;
	int current_frame;
	int count;
	std::string name;
	bool looping;
	Loop_Mode loop_mode;
	int increment;
	std::vector<std::string> tags;

	// for rotating animations
	bool rotating;
	float angle;
	float angle_inc;
	General::Point<float> rotation_center;
	
	int loop_start;
};

#endif

