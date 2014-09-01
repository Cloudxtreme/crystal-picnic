#ifndef WHACK_A_SKUNK_LOOP_H
#define WHACK_A_SKUNK_LOOP_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_primitives.h>

#include <string>

#include "animation_set.h"
#include "loop.h"
#include "general.h"
#include "engine.h"
#include "sound.h"
#include "player.h"

class Whack_a_Skunk_Loop : public Loop {
public:
	static const int STAR_LIFETIME;
	static const int STARS_PER;
	static const float SHOOT_SPEED;
	static const int POW_LIFETIME;
	static const int GAME_LENGTH;

	enum Skunk_Type { REGULAR, SILVER, GOLD, FAKE };
	enum Skunk_Status { NIL, ALIVE, DYING, DEAD, TAUNTING, GOING_DOWN };

	struct Skunk_Hole {
		int x, y;
		int type;
		int status;
		int count;
		int miss_count;
		int dying_count;
		int taunting_count;
	};

	struct Star {
		float angle;
		float da;
		float x, y;
		float dx, dy;
		int count;
	};

	struct Pow {
		int x, y;
		int count;
		bool kratch;
	};

	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();

	void set_area_loop(Area_Loop *area_loop);

	Whack_a_Skunk_Loop(int min_next, int max_next);
	virtual ~Whack_a_Skunk_Loop();

private:
	bool get_skunk_info(int hole, Animation_Set **anim_set);
	void add_pow_effects(int hole, bool kratch);

	std::string name;
	Animation_Set *regular_skunk, *silver_skunk, *gold_skunk, *fake_skunk;
	General::Size<int> skunk_size;
	Wrap::Bitmap *bg_bitmap;
	Animation_Set *hand;
	std::vector< Skunk_Hole > holes;
	General::Point<int> top_offset;
	int min_next, max_next;
	int next_diff;
	int next_count;
	int misses;
	int hits;
 	int curr_hole;
	int axes[2];
	General::Size<int> hand_size;
	bool bashing;
	int bg_w, bg_h;
	bool done;
	int done_count;
	std::list<Star> stars;
	std::list<Pow> pows;
	Wrap::Bitmap *pow_bmp, *kratch_bmp, *star_bmp;
	int hits_in_a_row;
	Animation_Set *font;
	int font_w, font_h;
	int timer;
	double start_time;
	Wrap::Bitmap *mask_front, *mask_middle, *mask_back;
	Wrap::Bitmap *highlight_front, *highlight_middle, *highlight_back;
	Wrap::Bitmap *mask_fronthighlight, *mask_middlehighlight, *mask_backhighlight;

	Sound::Sample *hit_sample;
	Sound::Sample *miss_sample;
	Sound::Sample *taunt_sample;

	std::string old_music;

	Area_Loop *area_loop;
};

#endif // WHACK_A_SKUNK_LOOP_H
