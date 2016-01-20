#ifndef ENGINE_H
#define ENGINE_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#include <string>
#include <map>
#include <set>

#ifdef ALLEGRO_WINDOWS
#include <d3d9.h>
#endif

#include "config.h"
#include "music.h"
#include "sound.h"
#include "loop.h"
#include "animation.h"
#include "animation_set.h"
#include "particle.h"
#include "cpa.h"
#include "video_player.h"

#include <tgui2.hpp>

enum TouchInputType {
	TOUCHINPUT_GUI = 0,
	TOUCHINPUT_SPEECH = 1,
	TOUCHINPUT_BATTLE = 2,
	TOUCHINPUT_MAP = 3,
	TOUCHINPUT_AREA = 4
};

const int TOUCH_BUTTON_RADIUS = 15;
const int TOUCH_STICK_RADIUS = 25;
const int TOUCH_STICK_TRAVEL = 0;
const float TOUCH_SQUARE_PERCENT = 1.35f;

enum TouchType {
	TOUCH_ACTIVATE = 0, // has to be 0, these are used as indexes into bitmaps
	TOUCH_ANALOGSTICK,
	TOUCH_MENU,
	TOUCH_SPECIAL,
	TOUCH_SWITCH,
	TOUCH_USE,
	TOUCH_ADVANCE,
	TOUCH_ANALOGBASE,
	TOUCH_ATTACK,
	TOUCH_JUMP,
	TOUCH_NONE
};

struct Touch {
	int id;
	float x, y;
	int location;
};

class Engine {
public:
	static std::string JUMP_SWITCH_INFO_MILESTONE;

	struct Sample {
		Sound::Sample *sample;
		int count;
		bool looping;
	};

	friend void *wait_for_drawing_resume(void *arg);

	bool init();
	void destroy_loops();
	void shutdown();
	void draw_all(std::vector<Loop *> loops, bool force_no_bitmap_change);
	void post_draw();
	void draw_loops(std::vector<Loop *> loops);
	void post_draw_loops(std::vector<Loop *> loops);
	void process_touch_input(ALLEGRO_EVENT *event);
	void do_event_loop();

	void do_blocking_mini_loop(std::vector<Loop *> loops, const char *callback);
	void unblock_mini_loop();

	std::vector<Loop *> &get_loops();
	void set_loops_delay_init(std::vector<Loop *> loops, bool destroy_old, bool delay_init);
	void set_loops(std::vector<Loop *> loops, bool destroy_old);
	void set_loops_only(std::vector<Loop *> loops);
	void remove_loop(Loop *loop, bool delete_it);
	void add_loop(Loop *loop);

	ALLEGRO_DISPLAY *get_display();

	void delete_tweens();
	void add_tween(int id);

	void add_event_source(ALLEGRO_EVENT_SOURCE *evt_source);
	void remove_event_source(ALLEGRO_EVENT_SOURCE *evt_source);
	void push_event(int type, void *data);
	void push_event(int type, void *data, double time);

	void load_sample(std::string name, bool loop = false);
	void play_sample(std::string name, float vol = 1.0, float pan = 0.0, float speed = 1.0);
	void stop_sample(std::string name);
	void adjust_sample(std::string name, float vol = 1.0, float pan = 0.0, float speed = 1.0);
	void destroy_sample(std::string name);

	void add_flash(double start, double up, double stay, double down, ALLEGRO_COLOR color);
	void shake(double start, double duration, int amount);
	void end_shake();
	void fade(double start, double duration, ALLEGRO_COLOR color);

	bool milestone_is_complete(std::string name);
	void set_milestone_complete(std::string name, bool complete);
	void clear_milestones();
	void hold_milestones(bool hold); // like al_hold_bitmap_drawing

	const char *t(const char *tag); // translation lookup

	void stop_timers();
	void start_timers();

	ALLEGRO_COND *get_halt_cond() { return halt_cond; }

	void top();

	int add_particle_group(std::string type, int layer, int align, std::vector<std::string> bitmap_names);
	void delete_particle_group(int id);
	Particle::Particle_Group *get_particle_group(int id);

	void load_translation();

	void flush_event_queue();

	Wrap::Bitmap *get_hero_shadow();
	Wrap::Bitmap *get_big_shadow();
	
	Wrap::Bitmap *get_work_bitmap();

	void notify(std::vector<std::string> texts, std::vector<Loop *> *loops_to_draw = NULL);
	int prompt(std::vector<std::string> texts, std::string button1, std::string button2, std::vector<Loop *> *loops_to_draw = NULL);
	bool yes_no_prompt(std::vector<std::string> texts, std::vector<Loop *> *loops_to_draw = NULL);
	// returns -1 on cancel else low->high
	int get_number(std::vector<std::string> texts, int low, int high, int start, std::vector<Loop *> *loops_to_draw = NULL);

	void draw_to_display(ALLEGRO_BITMAP *cfg_screen_sized_bitmap);

	std::string get_last_area();
	void set_last_area(std::string last_area);

	ALLEGRO_BITMAP *set_draw_target(bool force_no_target_change);
	void finish_draw(bool force_no_target_change, ALLEGRO_BITMAP *old_target);

	void fade_out(double time = 0.5);
	void fade_in(double time = 0.5);
	void fade_out(std::vector<Loop *> loops, double time = 0.5);
	void fade_in(std::vector<Loop *> loops, double time = 0.5);

	CPA *get_cpa() { return cpa; }
	void load_cpa();

	void save_game(int number);
	void load_game(int number);
	void reset_game();

	bool get_game_just_loaded() {
		return game_just_loaded;
	}
	void set_game_just_loaded(bool game_just_loaded) {
		this->game_just_loaded = game_just_loaded;
	}

	void reset_logic_count() { reset_count = true; }

	void set_game_over(bool game_over);
	bool game_is_over();
	
	bool get_lost_boss_battle() { return lost_boss_battle; }
	void set_lost_boss_battle(bool lost_boss_battle) { this->lost_boss_battle = lost_boss_battle; }

	bool can_use_crystals();

	void set_done(bool done);
	bool get_done();
	void set_purchased(bool purchased);
	bool get_purchased();
	
	void switch_out();
	void switch_in();
	void handle_halt(ALLEGRO_EVENT *event);
	bool is_switched_out();

	ALLEGRO_EVENT_QUEUE *get_event_queue();

	void handle_display_lost();

	bool get_continued_or_saved();
	void set_continued_or_saved(bool continued_or_saved);

	void loaded_video(Video_Player *video);
	
	std::string get_music_name();

	void set_send_tgui_events(bool send_tgui_events);
	bool get_send_tgui_events();

	int get_save_number_last_used();

	bool is_render_buffer(ALLEGRO_BITMAP *bmp);
	Wrap::Bitmap *get_render_buffer() { return render_buffer; }
	void destroy_render_buffer();
	void create_render_buffer();

	bool in_mini_loop();
	std::vector<Loop *> get_mini_loops();
	void set_mini_loops(std::vector<Loop *> loops);

	TouchInputType get_touch_input_type();
	void set_touch_input_type(TouchInputType type);
	void clear_touches();
	void set_draw_touch_controls(bool draw) { draw_touch_controls = draw; }

	void switch_music_out();
	void switch_music_in();

	void set_can_move(bool can_move);

	bool get_started_new_game();

	void add_extra_event(ALLEGRO_EVENT event);

	Engine();
	~Engine();

protected:
	struct Tween {
		int id;
	};

	void get_touch_input_button_position(int location, int *x, int *y);
	void draw_touch_input_button(int location, TouchType bitmap);
	bool touch_is_on_button(ALLEGRO_EVENT *event, int location);
	void update_touch(ALLEGRO_EVENT *event);
	int find_touch(int id);
	void add_touch(ALLEGRO_EVENT *event, int location);
	void draw_touch_input_stick();

	bool init_allegro();
	void run_tweens();
	void load_sfx();
	void destroy_sfx();
	void delete_pending_loops();

	void logic();

	ALLEGRO_DISPLAY *display;
	ALLEGRO_THREAD *bgloader_thread;
	ALLEGRO_EVENT_QUEUE *event_queue;
	ALLEGRO_TIMER *timer;

	std::vector<Loop *> loops;
	std::vector<Loop *> new_loops;
	std::vector< std::vector<Loop *> > old_loops;
	bool destroy_old_loop;

	std::list<Tween> tweens;
	std::vector< std::list<Tween> > tween_stack;
	std::list<Tween> new_tweens;
	std::vector<double> tween_pause_times;
	double tween_time_adjustment;

	ALLEGRO_EVENT_SOURCE event_source;

	std::map<std::string, Sample> sfx;
	std::map<std::string, Wrap::Bitmap *> bmps;

	std::list<ALLEGRO_EVENT *> timed_events;

	struct Flash {
		double start;
		double up;
		double stay;
		double down;
		ALLEGRO_COLOR color;
	};
	std::vector<Flash> flashes;

	struct Shake {
		double start;
		double duration;
		int amount;
	};
	Shake _shake;
	bool shaking;

	struct Fade {
		double fade_start;
		double fade_duration;
		ALLEGRO_COLOR fading_to;
	};
	bool fading;
	ALLEGRO_COLOR fade_color;
	std::vector<Fade> fades;

	std::map<std::string, bool> milestones;
	std::map<std::string, bool> held_milestones;
	bool milestones_held;

	void setup_screen_size();

#ifdef ALLEGRO_WINDOWS
	void load_d3d_resources();
	void destroy_d3d_resources();
#endif

	bool switched_out;

	bool done;
	bool _unblock_mini_loop;

	std::vector<Loop *> loops_to_delete;

	ALLEGRO_CONFIG *translation;

	ALLEGRO_MUTEX *halt_mutex;
	ALLEGRO_COND *halt_cond;

	std::string music_name;

	std::vector<Particle::Particle_Group *> particle_groups;

	int frames_drawn;
	double first_frame_time;
	int curr_fps;
	int slow_frames;
	bool running_slow;

	Wrap::Bitmap *hero_shadow_bmp;
	Wrap::Bitmap *big_shadow_bmp;

	Wrap::Bitmap *work_bitmap;

	Wrap::Bitmap *render_buffer;
	float actual_scale;

	std::string last_area;

	CPA *cpa;

	bool game_just_loaded;

	int logic_count;
	bool reset_count;

	bool game_over;
	bool lost_boss_battle;

	bool purchased;

	bool continued_or_saved;

	Video_Player *_loaded_video;

	bool send_tgui_events;

	int save_number_last_used;

	bool _in_mini_loop;
	std::vector<Loop *> mini_loops;

	TouchInputType touch_input_type;
	bool touch_input_on_bottom;
	Wrap::Bitmap *touch_bitmaps[10];
	std::vector<Touch> touches;
	bool draw_touch_controls;
	float last_direction;
	Wrap::Bitmap *helping_hand;

	std::vector<ALLEGRO_EVENT> extra_events;

	bool can_move;

	// This is only set from the Continue screen (NOT New Game)
	bool started_new_game;

	float switch_out_volume;
};

extern Engine *engine;

// Convenience
#define t(s) engine->t(s)

void process_dpad_events(ALLEGRO_EVENT *event);

#endif // ENGINE_H
