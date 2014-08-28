#ifndef RUNNER_LOOP_H
#define RUNNER_LOOP_H

#include <allegro5/allegro.h>

#include "general.h"
#include "battle_loop.h"
#include "steering.h"
#include "shaders.h"
#include "player.h"

class Runner_Loop : public Battle_Loop {
public:
	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();
	void destroy_graphics();
	void reload_graphics();

	void start1();
	void start2();
	void stop();

	Runner_Loop(std::vector<Player *> bisou, Wrap::Bitmap **end_screenshot, std::string *tmp);
	virtual ~Runner_Loop();

private:
	struct Platform {
		int x, y;
		int length;
		int middle_start;
	};

	struct Bird {
		float x, y;
	};

	struct Cloud {
		Wrap::Bitmap *bitmap;
		float x;
		float y;
		float vx;
		bool star;
	};

	struct Coin {
		float x;
		float y;
	};

	void draw_platform(Platform *p);
	bool platform_visible(int i);
	void extra_drawing_hook(int layer);
	void get_player_hitbox(int *x1, int *y1, int *x2, int *y2);

	void get_dp(int x, int y, ALLEGRO_BITMAP *bmp, int *outx, int *outy);

	int num_middle_bmps;
	
	Platform *platforms;

	float camera_y;
	int jumps;

	int left_bmp_width, left_bmp_height;
	int right_bmp_width;
	int mid_bmp_width;

	General::Point<float> last_player_pos;

	Wrap::Bitmap *bird_bmp;
	std::vector<Bird> waiting_birds;
	std::vector<Bird> active_birds;

	enum SfxID {
		BIRD_CALL,
		BIRD_HIT_PLAYER,
		FALL_IN_PIT,
		HIT_ROCK,
		BEE_DRONE,
		DEATH_BIRD,
		FRUIT_POP,
		SHIELD_DROP,
		SHIELD_PICKUP,
		SLIDE
	};

	std::vector<Sound::Sample *> sfx;

	bool fell_in_pit;

	float last_x;
	bool hit_trunk;

	static const int NUM_BEES = 40;
	Steering::Boid bees[NUM_BEES];
	int bee_y_default;
	float bee_sine_wave;
	float bee_cosine_wave;

	int tree_bg_i;
	int tree_bg_w;
	int tree_bg_h;

	int tree_fg_i;
	int tree_fg_w;
	int tree_fg_h;

	Wrap::Bitmap *gradient;
	Animation_Set *bee_anim;

	bool bees_on_top; // draw bees sorted with player or on top of everything
	double dead_time;

	int shields;
	std::vector< General::Point<int> > shield_positions;
	Wrap::Bitmap *shield_bmp;
	Animation_Set *shield_barrier;

	double invincible_end;

	Wrap::Bitmap *cloud1, *cloud2, *cloud3, *star1, *star2, *star3, *star4;
	std::vector<Cloud *> clouds;
	void gencloud(Cloud *c, bool genx, bool star);

	std::vector<General::Point<int> > slide_trees;
	int tree_slide_i;
	int tree_slide_h;

	Animation_Set *poof_anim;

	Animation_Set *coin;
	std::vector< Coin > coins;
	int coin_w;
	int coin_h;
	int collected_coins;

	bool hitting_big_tree;

	int mountain_bg_w;
	int mountain_bg_h;

	Battle_Player *player;

	int bird_w, bird_h;

	int hit_rock_count;

	int start_hp;

	bool start_hitting_rocks;

	bool won;
	double win_time;

	std::vector< std::pair<double, int> > time_distance;
};

#endif // RUNNER_LOOP_H
