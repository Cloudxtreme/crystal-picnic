#include "crystalpicnic.h"
#include "engine.h"
#include "main_menu_loop.h"

#include "area_loop.h"
#include "battle_loop.h"
#include "runner_loop.h"
#include "shop_loop.h"
#include "whack_a_skunk_loop.h"
#include "map_loop.h"
#include "direct3d.h"
#include "tls.h"
#include "particle.h"
#include "luainc.h"
#include "video_player.h"
#include "shaders.h"
#include "collision_detection.h"
#include "input_config_loop.h"

#ifdef ALLEGRO_ANDROID
#include "android.h"
#endif

#include <allegro5/allegro_opengl.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>

#include <bass.h>

#if defined __linux__ || defined ALLEGRO_MACOSX || defined ALLEGRO_IPHONE
	#define USE_FULLSCREEN_WINDOW 1
#endif

ALLEGRO_DEBUG_CHANNEL("CrystalPicnic")

#ifdef ALLEGRO_IPHONE
bool gamepadConnected()
{
	return false;
}
#endif

static void do_modal(
	ALLEGRO_EVENT_QUEUE *queue,
	ALLEGRO_COLOR clear_color, // if alpha == 1 then don't use background image
	ALLEGRO_BITMAP *background,
	bool (*callback)(tgui::TGUIWidget *widget),
	bool (*check_draw_callback)(),
	void (*before_flip_callback)(),
	void (*resize_callback)()
	)
{
	ALLEGRO_BITMAP *back;
	if (clear_color.a != 0) {
		back = NULL;
	}
	else if (background) {
		back = background;
	}
	else {
		back = Graphics::clone_target();
	}

	bool lost = false;

	int redraw = 0;
	ALLEGRO_TIMER *logic_timer = al_create_timer(1.0/60.0);
	al_register_event_source(queue, al_get_timer_event_source(logic_timer));
	al_start_timer(logic_timer);

	while (1) {
		ALLEGRO_EVENT event;

		while (!al_event_queue_is_empty(queue)) {
			al_wait_for_event(queue, &event);

			if (event.type == ALLEGRO_EVENT_JOYSTICK_AXIS && event.joystick.id && al_get_joystick_num_buttons((ALLEGRO_JOYSTICK *)event.joystick.id) == 0) {
				continue;
			}

			process_dpad_events(&event);

			if (event.type == ALLEGRO_EVENT_TIMER && event.timer.source != logic_timer) {
				continue;
			}
			else if (event.type == ALLEGRO_EVENT_TIMER) {
				redraw++;
			}
			else if (event.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION) {
				al_reconfigure_joysticks();
			}
			else if (resize_callback && event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
				resize_callback();
			}
			else if (event.type == ALLEGRO_EVENT_DISPLAY_LOST) {
				lost = true;
			}
			else if (event.type == ALLEGRO_EVENT_DISPLAY_FOUND) {
				lost = false;
			}
#if defined ALLEGRO_IPHONE || defined ALLEGRO_ANDROID
			else if (event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
				engine->switch_out();
			}
			else if (event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
				engine->switch_in();
			}
			else if (event.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING) {
				engine->handle_halt(&event);
			}
#endif

			if (engine->get_send_tgui_events()) {
				tgui::handleEvent(&event);
			}

			tgui::TGUIWidget *w = tgui::update();

			if (callback(w)) {
				goto done;
			}
		}

		if (!lost && redraw && (!check_draw_callback || check_draw_callback())) {
			redraw = 0;

			al_set_target_bitmap(engine->get_render_buffer()->bitmap);

			al_clear_to_color(al_map_rgb_f(0.0f, 0.0f, 0.0f));
			ALLEGRO_TRANSFORM t, backup;
			al_copy_transform(&backup, al_get_current_transform());
			al_identity_transform(&t);
			al_use_transform(&t);
			if (back) {
				al_draw_tinted_bitmap(back, al_map_rgb_f(0.5f, 0.5f, 0.5f), 0, 0, 0);
			}
			else {
				al_clear_to_color(clear_color);
			}
			al_use_transform(&backup);

			tgui::draw();

			if (before_flip_callback) {
				before_flip_callback();
			}

			al_flip_display();
		}
	}

done:
	if (clear_color.a == 0 && !background) {
		al_destroy_bitmap(back);
	}

	al_destroy_timer(logic_timer);
}

// This should be the only thing global in the game
Engine *engine;

std::string Engine::JUMP_SWITCH_INFO_MILESTONE = "jump_switch_info";

Engine::Engine() :
	tween_time_adjustment(0.0),
	switched_out(false),
	done(false),
	render_buffer(NULL),
	last_area(""),
	reset_count(false),
	game_over(false),
	lost_boss_battle(false),
	purchased(false),
	continued_or_saved(false),
	_loaded_video(NULL),
	send_tgui_events(true),
	save_number_last_used(0),
	draw_touch_controls(true),
	last_direction(-1),
	can_move(false),
	started_new_game(false)
{
        resource_manager = new Resource_Manager;

	fading = false;
	fade_color = al_map_rgba(0, 0, 0, 0);

	frames_drawn = 0;
	curr_fps = 0;

	translation = NULL;

	_unblock_mini_loop = false;

	milestones_held = false;

	work_bitmap = NULL;
	
	_shake.start = 0;
	_shake.duration = 0;
	shaking = false;

	touch_input_type = TOUCHINPUT_GUI;
	touch_input_on_bottom = true;
}

Engine::~Engine()
{
}

void Engine::delete_tweens()
{
	tweens.clear();
}

void Engine::add_tween(int id)
{
	Tween t;
	t.id = id;
	tweens.push_back(t);
}

void Engine::run_tweens()
{
	lua_State *lua_state;

	Area_Loop *area_loop = General::find_in_vector<Area_Loop *, Loop *>(loops);

	if (!area_loop) {
		Battle_Loop *bl = General::find_in_vector<Battle_Loop *, Loop *>(loops);
		if (!bl)
			return;
		lua_state = bl->get_lua_state();
	}
	else {
		Area_Manager *area = area_loop->get_area();
		lua_state = area->get_lua_state();
	}

	std::list<Tween>::iterator it;
	std::vector<Tween> tweens_to_delete;
	for (it = tweens.begin(); it != tweens.end();) {
		Tween &t = *it;
		Lua::call_lua(lua_state, "run_tween", "id>b", t.id, tween_time_adjustment);
		bool tween_done = lua_toboolean(lua_state, -1);
		lua_pop(lua_state, 1);
		if (tween_done)
			tweens_to_delete.push_back(t);
		it++;
	}

	tween_time_adjustment = 0.0;

	for (it = tweens.begin(); it != tweens.end();) {
		Tween &t = *it;
		bool found = false;
		for (size_t i = 0; i < tweens_to_delete.size(); i++) {
			if (t.id == tweens_to_delete[i].id) {
				Lua::call_lua(lua_state, "delete_tween", "i>", t.id);
				it = tweens.erase(it);
				found = true;
				break;
			}
		}
		if (!found) {
			it++;
		}
	}
}

std::vector<Loop *> &Engine::get_loops()
{
	return loops;
}

ALLEGRO_DISPLAY *Engine::get_display()
{
	return display;
}

void Engine::set_loops_delay_init(std::vector<Loop *> loops, bool destroy_old, bool delay_init)
{
	if (loops.size() == 0) {
		new_loops = old_loops[old_loops.size()-1];
		for (size_t i = 0; i < new_loops.size(); i++) {
			new_loops[i]->return_to_loop();
		}
		old_loops.pop_back();
		new_tweens = tween_stack[tween_stack.size()-1];
		tween_stack.pop_back();
		tween_time_adjustment = al_get_time() - tween_pause_times[tween_pause_times.size()-1];
		tween_pause_times.pop_back();
	}
	else {
		new_loops = loops;
		if (!delay_init) {
			for (size_t i = 0; i < new_loops.size(); i++) {
				new_loops[i]->init();
			}
		}
		new_tweens = std::list<Tween>();
	}
	if (!destroy_old) {
		old_loops.push_back(this->loops);
		tween_stack.push_back(tweens);
		/* Save current time so timing is preserved on backed up tweens */
		tween_pause_times.push_back(al_get_time());
	}
	destroy_old_loop = destroy_old;
}

void Engine::set_loops(std::vector<Loop *> loops, bool destroy_old)
{
	set_loops_delay_init(loops, destroy_old, false);
}

void Engine::set_loops_only(std::vector<Loop *> loops)
{
	this->loops = loops;
}

void Engine::remove_loop(Loop *loop, bool delete_it)
{
	for (size_t i = 0; i < loops.size(); i++) {
		if (loops[i] == loop) {
			loops.erase(loops.begin()+i);
			if (delete_it)
				loops_to_delete.push_back(loop);
			return;
		}
	}
}

void Engine::add_loop(Loop *loop)
{
	loops.push_back(loop);
}

#ifdef ALLEGRO_WINDOWS
void Engine::load_d3d_resources()
{
	if (al_get_display_flags(display) & ALLEGRO_DIRECT3D) {
		Wrap::reload_loaded_shaders();
		for (size_t i = 0; i < old_loops.size(); i++) {
			for (size_t j = 0; j < old_loops[i].size(); j++) {
				old_loops[i][j]->reload_graphics();
			}
		}
		for (size_t i = 0; i < loops.size(); i++) {
			loops[i]->reload_graphics();
		}
		Wrap::reload_loaded_bitmaps();
	}
}

void Engine::destroy_d3d_resources()
{
	if (al_get_display_flags(display) & ALLEGRO_DIRECT3D) {
		Wrap::destroy_loaded_shaders();
		for (size_t i = 0; i < old_loops.size(); i++) {
			for (size_t j = 0; j < old_loops[i].size(); j++) {
				old_loops[i][j]->destroy_graphics();
			}
		}
		for (size_t i = 0; i < loops.size(); i++) {
			loops[i]->destroy_graphics();
		}
		Wrap::destroy_loaded_bitmaps();
	}
}
#endif

void Engine::setup_screen_size()
{
	float w = al_get_display_width(display);
	float h = al_get_display_height(display);

	tgui::setScreenSize(w, h);

	if (cfg.fullscreen) {
		cfg.save_screen_w = w;
		cfg.save_screen_h = h;
	}
	
	float r = h / General::RENDER_H;
	float r2 = w / h;
	
	if (cfg.linear_filtering) {
		r = (int)r;
	}

	cfg.screen_w = General::RENDER_H * r2;
	cfg.screen_h = General::RENDER_H;

	if (cfg.linear_filtering) {
		cfg.screens_w = r;
		cfg.screens_h = r;
#ifdef OUYA
		// On OUYA we use setFixedSize(1280, 720) in Java but that doesn't change mouse coords, this fixes it.
		tgui::setScale(1920.0f/cfg.screen_w, 1080.0f/cfg.screen_h);
#else
		tgui::setScale(w/cfg.screen_w, h/cfg.screen_h);
#endif
	}
	else {
		r += r * 0.5f / cfg.screen_w;
		cfg.screens_w = r;
		cfg.screens_h = r;
		tgui::setScale(r, r);
	}

	tgui::setOffset(0, 0);

	if (render_buffer) {
		Wrap::destroy_bitmap(render_buffer);
		render_buffer = NULL;
	}

	if (cfg.linear_filtering) {
		int flags = al_get_new_bitmap_flags();
		al_set_new_bitmap_flags(ALLEGRO_NO_PRESERVE_TEXTURE | ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
		render_buffer = Wrap::create_bitmap_no_preserve(
			cfg.screen_w * cfg.screens_w,
			cfg.screen_h * cfg.screens_h
		);
		al_set_new_bitmap_flags(flags);
	}

	ALLEGRO_BITMAP *target = al_get_target_bitmap();
	if (render_buffer) {
		al_set_target_bitmap(render_buffer->bitmap);
	}
	ALLEGRO_TRANSFORM t;
	al_identity_transform(&t);
	al_scale_transform(&t, r, r);
	al_use_transform(&t);
	al_clear_to_color(al_color_name("black"));
	al_set_target_bitmap(target);

	if (work_bitmap) {
		Wrap::destroy_bitmap(work_bitmap);
		work_bitmap = NULL;
	}
	if (render_buffer) {
		work_bitmap = Wrap::create_bitmap_no_preserve(
			al_get_bitmap_width(render_buffer->bitmap),
			al_get_bitmap_height(render_buffer->bitmap)
		);
	}
	else {
		work_bitmap = Wrap::create_bitmap_no_preserve(
			w, h
		);
	}
}

bool Engine::init_allegro()
{
	ALLEGRO_DEBUG("Calling al_init");

	al_init();

	General::init_textlog();

	first_frame_time = al_get_time();

	ALLEGRO_DEBUG("Allegro init done");

	al_set_org_name("Nooskewl");
	al_set_app_name("Crystal Picnic");

	ALLEGRO_DEBUG("App name set");

#if !defined ALLEGRO_IPHONE && !defined ALLEGRO_ANDROID && !defined ALLEGRO_RASPBERRYPI
	al_install_mouse();
#elif defined ALLEGRO_IPHONE || defined ALLEGRO_ANDROID
	al_install_touch_input();
#endif

	al_install_keyboard();

	TLS::init();

	ALLEGRO_DEBUG("TLS initialized");

	cfg.load();

	// Process command line arguments
	int index;
	if ((index = General::find_arg("-adapter")) > 0) {
		cfg.adapter = atoi(General::argv[index+1]);
		General::log_message("Using adapter " + General::itos(cfg.adapter));
		al_set_new_display_adapter(cfg.adapter);
	}
	if (General::find_arg("-windowed") > 0) {
		cfg.fullscreen = false;
	}
	else if (General::find_arg("-no-windowed") > 0) {
		cfg.fullscreen = true;
	}
	if (General::find_arg("-opengl") > 0) {
		cfg.force_opengl = true;
	}
	if (General::find_arg("-no-opengl") > 0) {
		cfg.force_opengl = false;
	}
	if (General::find_arg("-linear-filtering") > 0) {
		cfg.linear_filtering = true;
	}
	if (General::find_arg("-no-linear-filtering") > 0) {
		cfg.linear_filtering = false;
	}
	if (General::find_arg("-reverb") > 0) {
		cfg.reverb = true;
	}
	if (General::find_arg("-no-reverb") > 0) {
		cfg.reverb = false;
	}
	if ((index = General::find_arg("-width")) > 0) {
		cfg.screen_w = atoi(General::argv[index+1]);
	}
	if ((index = General::find_arg("-height")) > 0) {
		cfg.screen_h = atoi(General::argv[index+1]);
	}

	ALLEGRO_DEBUG("Configuration loaded");

	if (al_install_joystick()) {
		General::log_message("Joystick driver installed.");
	}

	ALLEGRO_DEBUG("Input drivers installed");

	load_cpa();

	al_init_primitives_addon();
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();

	ALLEGRO_DEBUG("Addons initialized");

	if (cfg.vsync) {
		al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);
	}
	else {
		al_set_new_display_option(ALLEGRO_VSYNC, 0, ALLEGRO_SUGGEST);
	}

#if defined ALLEGRO_IPHONE || defined ALLEGRO_ANDROID
	al_set_new_display_option(ALLEGRO_SUPPORTED_ORIENTATIONS, ALLEGRO_DISPLAY_ORIENTATION_LANDSCAPE, ALLEGRO_REQUIRE);
#endif

	al_set_new_display_flags(
		al_get_new_display_flags()
		| ALLEGRO_PROGRAMMABLE_PIPELINE
		| (cfg.force_opengl ? ALLEGRO_OPENGL : 0)
#if defined USE_FULLSCREEN_WINDOW
		| (cfg.fullscreen ? ALLEGRO_FULLSCREEN_WINDOW : 0)
#else
		| (cfg.fullscreen ? ALLEGRO_FULLSCREEN : 0)
#endif
	);

	ALLEGRO_DEBUG("Display flags set");

#ifdef ALLEGRO_ANDROID
	al_set_new_display_option(ALLEGRO_COLOR_SIZE, 16, ALLEGRO_REQUIRE);
	display = al_create_display(1280, 720);
#else
#if defined ALLEGRO_RASPBERRYPI
	al_set_new_display_option(ALLEGRO_COLOR_SIZE, 16, ALLEGRO_REQUIRE);
#else
	al_set_new_display_option(ALLEGRO_COLOR_SIZE, 32, ALLEGRO_REQUIRE);
#endif
	display = al_create_display(cfg.screen_w, cfg.screen_h);
	if (!display) {
		cfg.screen_w = 1024;
		cfg.screen_h = 576;
		cfg.fullscreen = false;
		al_set_new_display_flags(al_get_new_display_flags() & ~ALLEGRO_FULLSCREEN);
		al_set_new_display_flags(al_get_new_display_flags() & ~ALLEGRO_FULLSCREEN_WINDOW);
		display = al_create_display(cfg.screen_w, cfg.screen_h);
	}
#endif
			
	if (!display) {
		General::log_message("Unable to create display.");
		return false;
	}

#if defined OUYA
	// We use setFixedSize on OUYA for performance
	cfg.screen_w = 1280;
	cfg.screen_h = 720;
#elif defined USE_FULLSCREEN_WINDOW
	if (cfg.fullscreen) {
		cfg.screen_w = al_get_display_width(display);
		cfg.screen_h = al_get_display_height(display);
	}
#endif

#ifdef ALLEGRO_MACOSX
	// Allegro fullscreen window on Mac is broken
	if (cfg.fullscreen) {
		al_toggle_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, false);
		al_toggle_display_flag(display, ALLEGRO_FULLSCREEN_WINDOW, true);
	}
#endif

	al_clear_to_color(al_map_rgb_f(0, 0, 0));
	al_flip_display();

	General::log_message("OpenGL: " + General::itos(cfg.force_opengl));
	
	//al_register_bitmap_loader_f(".cpi", load_cpi_f);
	//al_register_bitmap_loader(".cpi", load_cpi);

#ifdef ALLEGRO_WINDOWS
	// With OpenGL driver, crashes if icon is not a memory bitmap
	int flags = al_get_new_bitmap_flags();
	al_set_new_bitmap_flags(flags | ALLEGRO_MEMORY_BITMAP);
	Wrap::Bitmap *icon_tmp = Wrap::load_bitmap("misc_graphics/icon.cpi");
	al_set_display_icon(display, icon_tmp->bitmap);
	Wrap::destroy_bitmap(icon_tmp);
	al_set_new_bitmap_flags(flags);
#endif

	al_set_new_bitmap_flags(ALLEGRO_NO_PRESERVE_TEXTURE);

#ifdef ALLEGRO_WINDOWS
	if (al_get_display_flags(display) & ALLEGRO_DIRECT3D) {
		al_set_d3d_device_release_callback(Direct3D::lost_callback);
		al_set_d3d_device_restore_callback(Direct3D::found_callback);
	}
#endif

	halt_mutex = al_create_mutex();
	halt_cond = al_create_cond();

	al_set_window_title(display, "Egbert and Frogbert's Crystal Picnic");

	tgui::init(display);
	ALLEGRO_DEBUG("TGUI initialized");

	setup_screen_size();

#ifdef ALLEGRO_WINDOWS
	if (al_get_display_flags(display) & ALLEGRO_OPENGL) {
		load_d3d_resources();
	}
#endif

	if (al_get_display_flags(display) & ALLEGRO_OPENGL) {
		General::noalpha_bmp_format = ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE;
		General::font_bmp_format = ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE;
		General::default_bmp_format = ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE;
		cfg.opengl = true;
	}
	else {
		General::noalpha_bmp_format = ALLEGRO_PIXEL_FORMAT_XRGB_8888;
		General::font_bmp_format = ALLEGRO_PIXEL_FORMAT_ARGB_8888;
		General::default_bmp_format = ALLEGRO_PIXEL_FORMAT_ARGB_8888;
		cfg.opengl = false;
	}

	al_set_new_bitmap_format(General::default_bmp_format);

	ALLEGRO_DEBUG("Display setup done");

	event_queue = al_create_event_queue();

	timer = al_create_timer(1.0/60.0);

	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_display_event_source(display));
#if !defined ALLEGRO_IPHONE && !defined ALLEGRO_ANDROID && !defined ALLEGRO_RASPBERRYPI
	if (al_is_mouse_installed()) {
		al_register_event_source(event_queue, al_get_mouse_event_source());
	}
#elif defined ALLEGRO_IPHONE || defined ALLEGRO_ANDROID
	al_register_event_source(event_queue, al_get_touch_input_event_source());
#endif
	al_register_event_source(event_queue, al_get_keyboard_event_source());
#if !defined ALLEGRO_IPHONE
	al_register_event_source(event_queue, al_get_joystick_event_source());
#endif

	Music::init();

	// Create user settings dir
	ALLEGRO_PATH *user_path = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);
	al_drop_path_tail(user_path);
	if (!al_filename_exists(al_path_cstr(user_path, ALLEGRO_NATIVE_PATH_SEP))) {
		mkdir(al_path_cstr(user_path, ALLEGRO_NATIVE_PATH_SEP), 0755);
	}
	al_destroy_path(user_path);
	user_path = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);
	if (!al_filename_exists(al_path_cstr(user_path, ALLEGRO_NATIVE_PATH_SEP))) {
		mkdir(al_path_cstr(user_path, ALLEGRO_NATIVE_PATH_SEP), 0755);
	}
	al_destroy_path(user_path);

	ALLEGRO_DEBUG("Allegro initialized");

	return true;
}

void Engine::load_sfx()
{
	Sample s;

	s.count = 1;
	s.sample = Sound::load("sfx/chest_open.ogg");;
	sfx["sfx/chest_open.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/error.ogg");;
	sfx["sfx/error.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/menu_select.ogg");;
	sfx["sfx/menu_select.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/main_menu.ogg");;
	sfx["sfx/main_menu.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/quack.ogg");;
	sfx["sfx/quack.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/ribbit.ogg");;
	sfx["sfx/ribbit.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/pyou.ogg");;
	sfx["sfx/pyou.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/bisou.ogg");;
	sfx["sfx/bisou.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/open_door.ogg");;
	sfx["sfx/open_door.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/item_found.ogg");;
	sfx["sfx/item_found.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/exit_battle.ogg");;
	sfx["sfx/exit_battle.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/enter_area.ogg");;
	sfx["sfx/enter_area.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/land.ogg");;
	sfx["sfx/land.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/use_item.ogg");;
	sfx["sfx/use_item.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/switch_to_bisou.ogg");;
	sfx["sfx/switch_to_bisou.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/switch_to_egbert.ogg");;
	sfx["sfx/switch_to_egbert.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/switch_to_frogbert.ogg");;
	sfx["sfx/switch_to_frogbert.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/healing_item.ogg");;
	sfx["sfx/healing_item.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/single_jump.ogg");;
	sfx["sfx/single_jump.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/enemy_alerted.ogg");;
	sfx["sfx/enemy_alerted.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/trip.ogg");;
	sfx["sfx/trip.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/slip.ogg");;
	sfx["sfx/slip.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/npc.ogg");;
	sfx["sfx/npc.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/coin.ogg");;
	sfx["sfx/coin.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/throw.ogg");;
	sfx["sfx/throw.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/swing_weapon.ogg");;
	sfx["sfx/swing_weapon.ogg"] = s;

	s.count = 1;
	s.sample = Sound::load("sfx/poison_again.ogg");;
	sfx["sfx/poison_again.ogg"] = s;
}

void Engine::destroy_sfx()
{
	std::map<std::string, Sample>::iterator it;
	for (it = sfx.begin(); it != sfx.end(); it++) {
		std::pair<const std::string, Sample> &p = *it;
		if (p.second.sample) {
			Sound::destroy(p.second.sample);
		}
	}
}

bool Engine::init()
{	
	if (!init_allegro())
		return false;

	Graphics::init();

	reset_game();

	General::init();
	ALLEGRO_DEBUG("General::init done");

	load_translation();
	ALLEGRO_DEBUG("Translation loaded");

	al_init_user_event_source(&event_source);
	add_event_source((ALLEGRO_EVENT_SOURCE *)&event_source);

	ALLEGRO_DEBUG("Created user event source");

	load_sfx();

	ALLEGRO_DEBUG("Loaded sfx");

	ALLEGRO_DEBUG("Emoticons loaded");

	hero_shadow_bmp = Wrap::load_bitmap("misc_graphics/normal_character_shadow.cpi");
	big_shadow_bmp = Wrap::load_bitmap("misc_graphics/big_character_shadow.cpi");

#if defined ALLEGRO_ANDROID || defined ALLEGRO_IPHONE
	touch_bitmaps[TOUCH_ACTIVATE] = Wrap::load_bitmap("misc_graphics/interface/touch_ui/activate.cpi");
	touch_bitmaps[TOUCH_ANALOGSTICK] = Wrap::load_bitmap("misc_graphics/interface/touch_ui/analogstick.cpi");
	touch_bitmaps[TOUCH_MENU] = Wrap::load_bitmap("misc_graphics/interface/touch_ui/menu.cpi");
	touch_bitmaps[TOUCH_SPECIAL] = Wrap::load_bitmap("misc_graphics/interface/touch_ui/special.cpi");
	touch_bitmaps[TOUCH_SWITCH] = Wrap::load_bitmap("misc_graphics/interface/touch_ui/switch.cpi");
	touch_bitmaps[TOUCH_USE] = Wrap::load_bitmap("misc_graphics/interface/touch_ui/use.cpi");
	touch_bitmaps[TOUCH_ADVANCE] = Wrap::load_bitmap("misc_graphics/interface/touch_ui/advance.cpi");
	touch_bitmaps[TOUCH_ANALOGBASE] = Wrap::load_bitmap("misc_graphics/interface/touch_ui/analogbase.cpi");
	touch_bitmaps[TOUCH_ATTACK] = Wrap::load_bitmap("misc_graphics/interface/touch_ui/attack.cpi");
	touch_bitmaps[TOUCH_JUMP] = Wrap::load_bitmap("misc_graphics/interface/touch_ui/jump.cpi");
	helping_hand = Wrap::load_bitmap("misc_graphics/interface/touch_ui/helping_hand.cpi");
#endif

	return true;
}

void Engine::destroy_loops()
{
	for (size_t i = 0; i < loops.size(); i++) {
		delete loops[i];
	}
	loops.clear();
	for (size_t i = 0; i < old_loops.size(); i++) {
		for (size_t j = 0; j < old_loops[i].size(); j++) {
			delete old_loops[i][j];
		}
	}
	old_loops.clear();
}

void Engine::shutdown()
{
	Graphics::shutdown();

	destroy_sfx();

	Wrap::destroy_bitmap(hero_shadow_bmp);
	Wrap::destroy_bitmap(big_shadow_bmp);

#if defined ALLEGRO_ANDROID || defined ALLEGRO_IPHONE
	for (int i = 0; i < TOUCH_NONE; i++) {
		Wrap::destroy_bitmap(touch_bitmaps[i]);
	}
	Wrap::destroy_bitmap(helping_hand);
#endif

	remove_event_source((ALLEGRO_EVENT_SOURCE *)&event_source);
	al_destroy_user_event_source(&event_source);
	
	if (render_buffer) {
		Wrap::destroy_bitmap(render_buffer);
		render_buffer = NULL;
	}

	Wrap::destroy_bitmap(work_bitmap);

	tgui::shutdown();

	General::shutdown();
	
	delete resource_manager;

	Music::stop();
	Music::shutdown();

	al_shutdown_ttf_addon();
	al_shutdown_primitives_addon();
	al_destroy_display(display);

	al_destroy_config(translation);

	delete cpa;

	al_destroy_mutex(halt_mutex);
	al_destroy_cond(halt_cond);
}

static void destroy_event(ALLEGRO_USER_EVENT *u)
{
	delete (User_Event_Data *)u->data1;
	delete (double *)u->data2;
	delete (ALLEGRO_EVENT *)u->data3;
}

void Engine::top()
{
	if (!switched_out && !Music::is_playing()) {
		Music::play(music_name);
	}

	for (size_t i = 0; i < loops.size(); i++) {
		loops[i]->top();
	}
}

// Handle Direct3D lost/found
void Engine::handle_display_lost()
{
#ifdef ALLEGRO_WINDOWS
	if (al_get_display_flags(display) & ALLEGRO_DIRECT3D) {
		if (Direct3D::got_display_lost) {
			Direct3D::got_display_lost = false;
		}
		if (Direct3D::got_display_found) {
			Direct3D::got_display_found = false;
			General::destroy_fonts();
			destroy_d3d_resources();
			setup_screen_size();
			General::load_fonts();
			load_translation();
			load_d3d_resources();
		}
	}
#endif
}

void Engine::do_event_loop()
{
	fade_color = al_map_rgba(0, 0, 0, 0);
	_shake.start = al_get_time();
	_shake.duration = 0.0;
	shaking = false;

	done = false;

	logic_count = 0;
	int draw_count = 0;
	
	engine->start_timers();

	while (!done) {
		top();

		bool do_acknowledge_resize = false;

loop_top:
		ALLEGRO_EVENT event;
		al_wait_for_event(event_queue, &event);

		if (event.type == ALLEGRO_EVENT_JOYSTICK_AXIS && event.joystick.id && al_get_joystick_num_buttons((ALLEGRO_JOYSTICK *)event.joystick.id) == 0) {
			continue;
		}

		process_dpad_events(&event);

#if !defined ALLEGRO_IPHONE && !defined ALLEGRO_ANDROID
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			goto end;
		}
		else
#endif
		if (event.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION) {
			al_reconfigure_joysticks();
		}

		process_touch_input(&event);
		if (send_tgui_events) {
			tgui::handleEvent_pretransformed(&event);
		}

		for (size_t i = 0; i < loops.size(); i++) {
			if (loops[i]->handle_event(&event)) {
				break;
			}
		}
		for (size_t i = 0; i < loops.size(); i++) {
			bool bail = false;
			for (size_t j = 0; j < extra_events.size(); j++) {
				if (loops[i]->handle_event(&extra_events[j])) {
					bail = true;
					break;
				}
			}
			if (bail) {
				break;
			}
		}
		extra_events.clear();
		if (new_loops.size() > 0) {
			goto loop_end;
		}

		if (event.type == ALLEGRO_EVENT_TIMER) {
			Game_Specific_Globals::elapsed_time += General::LOGIC_MILLIS / 1000.0;
			logic_count++;
			draw_count++;
			/* pump user events */
			std::list<ALLEGRO_EVENT *>::iterator it;
			it = timed_events.begin();
			while (it != timed_events.end()) {
				ALLEGRO_EVENT *e = *it;
				double event_time;
				event_time = *((double *)e->user.data2);
				if (al_get_time() >= event_time) {
					al_emit_user_event(&event_source, e, destroy_event);
					it = timed_events.erase(it);
				}
				else {
					break;
				}
			}
		}
#if defined ALLEGRO_IPHONE || defined ALLEGRO_ANDROID
		if (event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
			switch_out();
		}
		else if (event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
			switch_in();
		}
		else if (event.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING) {
			handle_halt(&event);
		}
		else if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
			do_acknowledge_resize = true;
		}
#endif

#ifdef TRANSLATION_BUILD
		if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_F1) {
			load_translation();
		}
#endif

		// NOT else if
		if (event.type >= ALLEGRO_GET_EVENT_TYPE('B', 'A', 'R', 'Y')) {
			al_unref_user_event(&event.user);
		}

		if (!al_event_queue_is_empty(event_queue)) {
			goto loop_top;
		}

loop_end:

		if (reset_count) {
			logic_count = General::sign(logic_count);
			reset_count = false;
		}

		if (do_acknowledge_resize) {
			al_acknowledge_resize(display);
			do_acknowledge_resize = false;
		}

		if (do_acknowledge_resize) {
			al_acknowledge_resize(display);
			do_acknowledge_resize = false;
		}

		if (game_over) {
			for (size_t i = 0; i < new_loops.size(); i++) {
				delete new_loops[i];
			}
			new_loops.clear();
			break;
		}

		bool stop_draw = false;

		can_move = false;

		if (new_loops.size() > 0) {
			stop_draw = true;
			logic_count = 0;
		}
		else {
			if (cfg.debugmode && logic_count > 1) {
				logic_count = 1;
			}
			while (logic_count > 0) {
				logic();
				run_tweens(); // before logic or after?
				logic_count--;
				General::logic();
				bool bail = false;
				for (size_t i = 0; i < loops.size(); i++) {
					if ((stop_draw = loops[i]->logic())) {
						bail = true;
						break;
					}
				}
				if (bail) break;
				if (new_loops.size() > 0)
					break;
			}
		}

		delete_pending_loops();

		bool lost;
#ifdef ALLEGRO_WINDOWS
		handle_display_lost();
		lost = Direct3D::lost;
#else
		lost = false;
#endif

		if (!lost && !switched_out && !stop_draw && draw_count > 0 && new_loops.size() == 0) {
			draw_count = 0;

			al_clear_to_color(al_color_name("black"));

			draw_all(loops, false);
			
			al_flip_display();

			if (_loaded_video) {
				_loaded_video->reset_elapsed();
				_loaded_video = NULL;
			}

			frames_drawn++;
		}

		if (new_loops.size() > 0) {
			if (destroy_old_loop) {
				for (size_t i = 0; i < loops.size(); i++) {
					delete loops[i];
				}
			}

			loops = new_loops;
			new_loops.clear();

			tweens = new_tweens;
			new_tweens.clear();

			/* NOTE: Each loop should guard against being inited twice! */
			for (size_t i = 0; i < loops.size(); i++) {
				loops[i]->init();
			}
		}
	}

#if !defined ALLEGRO_IPHONE && !defined ALLEGRO_ANDROID
end:;
#endif

	if (milestone_is_complete("beat_game")) {
		cfg.beat_game = true;
	}
}

void Engine::unblock_mini_loop()
{
	_unblock_mini_loop = true;
}

void Engine::do_blocking_mini_loop(std::vector<Loop *> loops, const char *cb)
{
	_in_mini_loop = true;

	bool cb_exists = false;
	lua_State *lua_state = NULL;

	Area_Loop *al = General::find_in_vector<Area_Loop *, Loop *>(loops);
	if (al) {
		lua_state = al->get_area()->get_lua_state();
		lua_getglobal(lua_state, cb);
		cb_exists = !lua_isnil(lua_state, -1);
		lua_pop(lua_state, 1);
	}

	logic_count = 0;
	int draw_count = 0;

	while (true) {
		top();

		bool do_acknowledge_resize = false;

loop_top:
		ALLEGRO_EVENT event;
		al_wait_for_event(event_queue, &event);

		if (event.type == ALLEGRO_EVENT_JOYSTICK_AXIS && event.joystick.id && al_get_joystick_num_buttons((ALLEGRO_JOYSTICK *)event.joystick.id) == 0) {
			continue;
		}

		process_dpad_events(&event);

#if !defined ALLEGRO_IPHONE && !defined ALLEGRO_ANDROID
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			done = true;
			goto end;
		}
		else
#endif
		if (event.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION) {
			al_reconfigure_joysticks();
		}

		process_touch_input(&event);
		if (send_tgui_events) {
			tgui::handleEvent_pretransformed(&event);
		}

		for (size_t i = 0; i < loops.size(); i++) {
			if (loops[i]->handle_event(&event)) {
				break;
			}
		}
		for (size_t i = 0; i < loops.size(); i++) {
			bool bail = false;
			for (size_t j = 0; j < extra_events.size(); j++) {
				if (loops[i]->handle_event(&extra_events[j])) {
					bail = true;
					break;
				}
			}
			if (bail) {
				break;
			}
		}
		extra_events.clear();
		if (new_loops.size() > 0) {
			goto loop_end;
		}

		if (event.type == ALLEGRO_EVENT_TIMER) {
			Game_Specific_Globals::elapsed_time += General::LOGIC_MILLIS / 1000.0;
			logic_count++;
			draw_count++;
		}
#if defined ALLEGRO_IPHONE || defined ALLEGRO_ANDROID
		if (event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
			switch_out();
		}
		else if (event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
			switch_in();
		}
		else if (event.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING) {
			handle_halt(&event);
		}
		else if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
			do_acknowledge_resize = true;
		}
#endif

#ifdef TRANSLATION_BUILD
		if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_F1) {
			load_translation();
		}
#endif

		if (event.type >= ALLEGRO_GET_EVENT_TYPE('B', 'A', 'R', 'Y')) {
			al_unref_user_event(&event.user);
		}

		if (!al_event_queue_is_empty(event_queue)) {
			goto loop_top;
		}

loop_end:

		if (reset_count) {
			logic_count = General::sign(logic_count);
			reset_count = false;
		}

		if (do_acknowledge_resize) {
			al_acknowledge_resize(display);
			do_acknowledge_resize = false;
		}

		if (do_acknowledge_resize) {
			al_acknowledge_resize(display);
			do_acknowledge_resize = false;
		}

		can_move = false;
		if (cfg.debugmode && logic_count > 1) {
			logic_count = 1;
		}
		while (logic_count > 0) {
			logic();
			run_tweens(); // before logic or after?
			logic_count--;
			General::logic();
			for (size_t i = 0; i < loops.size(); i++) {
				loops[i]->logic();
			}
			Area_Loop *al = General::find_in_vector<Area_Loop *, Loop *>(loops);
			if (al) {
				if (cb_exists) {
					Lua::call_lua(lua_state, cb, ">b");
					bool done = lua_toboolean(lua_state, -1);
					lua_pop(lua_state, 1);
					if (done) {
						goto end;
					}
				}
			}
			if (_unblock_mini_loop) {
				_unblock_mini_loop = false;
				goto end;
			}
		}

		bool lost;
#ifdef ALLEGRO_WINDOWS
		handle_display_lost();
		lost = Direct3D::lost;
#else
		lost = false;
#endif

		if (!lost && !switched_out && draw_count > 0 && new_loops.size() == 0) {
			draw_count = 0;

			al_clear_to_color(al_color_name("black"));

			draw_all(loops, false);

			al_flip_display();
			
			if (_loaded_video) {
				_loaded_video->reset_elapsed();
				_loaded_video = NULL;
			}

			frames_drawn++;
		}
	}

end:

	for (size_t i = 0; i < loops.size(); i++) {
		delete loops[i];
	}

	mini_loops.clear();
	_in_mini_loop = false;
}

void Engine::add_event_source(ALLEGRO_EVENT_SOURCE *evt_source)
{
	al_register_event_source(event_queue, evt_source);
}

void Engine::remove_event_source(ALLEGRO_EVENT_SOURCE *evt_source)
{
	al_unregister_event_source(event_queue, evt_source);
}

static bool compare_user_events(ALLEGRO_EVENT *a, ALLEGRO_EVENT *b)
{
	double a_t = *((double *)a->user.data2);
	double b_t = *((double *)b->user.data2);
	return a_t < b_t;
}

void Engine::push_event(int type, void *data, double time)
{
	ALLEGRO_EVENT *event = new ALLEGRO_EVENT;
	event->user.type = type;
	event->user.data1 = (intptr_t)data;
	event->user.data2 = (intptr_t)(new double);
	*((double *)event->user.data2) = time;
	event->user.data3 = (intptr_t)event;
	timed_events.push_back(event);
	timed_events.sort(compare_user_events);
}

void Engine::push_event(int type, void *data)
{
	ALLEGRO_EVENT *event = new ALLEGRO_EVENT;
	event->user.type = type;
	event->user.data1 = (intptr_t)data;
	event->user.data2 = 0;
	event->user.data3 = (intptr_t)event;
	al_emit_user_event(&event_source, event, destroy_event);
}

void Engine::load_sample(std::string name, bool loop)
{
	std::map<std::string, Sample>::iterator it;
	for (it = sfx.begin(); it != sfx.end(); it++) {
		std::pair<const std::string, Sample> &p = *it;
		if (p.first == name) {
			p.second.count++;
			return;
		}
	}
	
	Sample s;
	s.count = 1;
	s.sample = Sound::load(name, loop);
	if (s.sample != NULL) {
		sfx[name] = s;
	}
}

void Engine::play_sample(std::string name, float vol, float pan, float speed)
{
	if (sfx.find(name) == sfx.end()) {
		return;
	}
	Sample &s = sfx[name];
	if (s.sample) {
		Sound::play(s.sample, vol, pan, speed);
		if (s.sample->loop) {
			s.looping = true;
		}
	}
}

void Engine::stop_sample(std::string name)
{
	if (sfx.find(name) == sfx.end()) {
		return;
	}
	Sample &s = sfx[name];
	if (s.sample) {
		Sound::stop(s.sample);
		if (s.sample->loop) {
			s.looping = false;
		}
	}
}

void Engine::adjust_sample(std::string name, float vol, float pan, float speed)
{
	if (sfx.find(name) == sfx.end()) {
		return;
	}
	Sample &s = sfx[name];
	if (s.sample) {
		Sound::adjust(s.sample, vol, pan, speed);
	}
}

void Engine::destroy_sample(std::string name)
{
	std::map<std::string, Sample>::iterator it;
	for (it = sfx.begin(); it != sfx.end(); it++) {
		std::pair<const std::string, Sample> &p = *it;
		if (p.first == name) {
			p.second.count--;
			if (p.second.count == 0) {
				Sound::destroy(p.second.sample);
				sfx.erase(it);
				return;
			}
		}
	}
}

void Engine::add_flash(double start, double up, double stay, double down, ALLEGRO_COLOR color)
{
	Flash f;
	f.start = al_get_time() + start;
	f.up = up;
	f.stay = stay;
	f.down = down;
	f.color = color;
	flashes.push_back(f);
}

void Engine::shake(double start, double duration, int amount)
{
	_shake.start = start + al_get_time();
	_shake.duration = duration;
	_shake.amount = amount;
	shaking = true;
}

void Engine::end_shake()
{
	_shake.start = 0;
	_shake.duration = 0;
	Area_Loop *l = GET_AREA_LOOP;
	if (l) {
		l->get_area()->set_rumble_offset(General::Point<float>(0, 0));
	}
	else {
		Battle_Loop *loop = GET_BATTLE_LOOP;
		if (loop) {
			loop->set_rumble_offset(General::Point<float>(0, 0));
		}
	}
	shaking = false;
}

void Engine::fade(double start, double duration, ALLEGRO_COLOR color)
{
	Fade f;
	f.fade_start = start + al_get_time();
	f.fade_duration = duration;
	f.fading_to = color;
	fades.push_back(f);
}

void Engine::hold_milestones(bool hold)
{
	if (hold) {
		held_milestones = milestones;
	}
	else {
		milestones = held_milestones;
	}
	milestones_held = hold;
}

void Engine::clear_milestones()
{
	milestones.clear();
}

bool Engine::milestone_is_complete(std::string name)
{
	if (milestones_held) {
		if (held_milestones.find(name) == held_milestones.end()) {
			return false;
		}
		return held_milestones[name];
	}
	else {
		if (milestones.find(name) == milestones.end()) {
			return false;
		}
		return milestones[name];
	}
}

void Engine::set_milestone_complete(std::string name, bool complete)
{
	if (milestones_held) {
		if (complete == false) {
			std::map<std::string, bool>::iterator it;
			if ((it = held_milestones.find(name)) != held_milestones.end()) {
				held_milestones.erase(it);
			}
		}
		else {
			held_milestones[name] = complete;
		}
	}
	else {
		if (complete == false) {
			std::map<std::string, bool>::iterator it;
			if ((it = milestones.find(name)) != milestones.end()) {
				milestones.erase(it);
			}
		}
		else {
			milestones[name] = complete;
		}
	}
}

void Engine::process_touch_input(ALLEGRO_EVENT *event)
{
#ifndef ALLEGRO_RASPBERRYPI
#if defined ALLEGRO_ANDROID || defined ALLEGRO_IPHONE
	if (
	event->type == ALLEGRO_EVENT_TOUCH_BEGIN ||
	event->type == ALLEGRO_EVENT_TOUCH_END ||
	event->type == ALLEGRO_EVENT_TOUCH_MOVE) {
		int tmp_x = event->touch.x;
		int tmp_y = event->touch.y;
		tgui::convertMousePosition(&tmp_x, &tmp_y);
		event->touch.x = tmp_x;
		event->touch.y = tmp_y;
		if (touch_input_type == TOUCHINPUT_GUI) {
			if (event->type == ALLEGRO_EVENT_TOUCH_BEGIN) {
				event->type = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
			}
			else if (event->type == ALLEGRO_EVENT_TOUCH_END) {
				event->type = ALLEGRO_EVENT_MOUSE_BUTTON_UP;
			}
			else {
				event->type = ALLEGRO_EVENT_MOUSE_AXES;
			}
			event->mouse.x = event->touch.x;
			event->mouse.y = event->touch.y;
		}
		else {
			update_touch(event);
		}
	}
#else
	if (
	event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
	event->type == ALLEGRO_EVENT_MOUSE_BUTTON_UP ||
	event->type == ALLEGRO_EVENT_MOUSE_AXES) {
		int tmp_x = event->mouse.x;
		int tmp_y = event->mouse.y;
		tgui::convertMousePosition(&tmp_x, &tmp_y);
		event->mouse.x = tmp_x;
		event->mouse.y = tmp_y;
	}
#endif
#endif
}

void Engine::post_draw()
{
	// draw flashes
	double now = al_get_time();
	for (size_t i = 0; i < flashes.size(); i++) {
		Flash &f = flashes[i];
		double end_time =
			f.start + f.up + f.stay + f.down;
		if (f.start > now)
			continue;
		else if (end_time < now) {
			flashes.erase(flashes.begin()+i);
			i--;
			continue;
		}
		float a;
		float r, g, b, target_a;
		al_unmap_rgba_f(
			f.color,
			&r, &g, &b, &target_a
		);
		if (now < f.start + f.up) {
			double elapsed = now - f.start;
			a = elapsed / f.up;
			a *= target_a;
		}
		else if (now < f.start + f.up + f.stay) {
			a = target_a;
		}
		else {
			double elapsed = now - (f.start + f.up + f.stay);
			a = 1.0f - (elapsed / f.down);
			a *= target_a;
		}
		r *= a;
		g *= a;
		b *= a;
		al_draw_filled_rectangle(
			0, 0,
			cfg.screen_w, cfg.screen_h,
			al_map_rgba_f(r, g, b, a)
		);
	}

	now = al_get_time();
	Fade fade = { 0, };
	if (fades.size() > 0)
		fade = fades[0];
	if (fading && now > fade.fade_start+fade.fade_duration) {
		fading = false;
		fade_color = fade.fading_to;
		fades.erase(fades.begin());
	}
	if (fades.size() > 0) {
		fade = fades[0];
		fading = true;
	}
	ALLEGRO_COLOR fade_tmp;
	if (fading && now >= fade.fade_start) {
		float p = (now - fade.fade_start) / fade.fade_duration;
		float r1, g1, b1, a1;
		float r2, g2, b2, a2;
		al_unmap_rgba_f(fade_color, &r1, &g1, &b1, &a1);
		al_unmap_rgba_f(fade.fading_to, &r2, &g2, &b2, &a2);
		float dr = r2 - r1;
		float dg = g2 - g1;
		float db = b2 - b1;
		float da = a2 - a1;
		float r, g, b, a;
		r = r1 + (p * dr);
		g = g1 + (p * dg);
		b = b1 + (p * db);
		a = a1 + (p * da);
		fade_tmp = al_map_rgba_f(r, g, b, a);
	}
	else
		fade_tmp = fade_color;
	float r, g, b, a;
	al_unmap_rgba_f(fade_tmp, &r, &g, &b, &a);
	r *= a;
	g *= a;
	b *= a;

	if (a <= 0)
		return;

	al_draw_filled_rectangle(
		0, 0,
		cfg.screen_w, cfg.screen_h,
		fade_tmp
	);
}

ALLEGRO_BITMAP *Engine::set_draw_target(bool force_no_target_change)
{
	ALLEGRO_BITMAP *old_target = NULL;

	if (!force_no_target_change) {
		old_target = al_get_target_bitmap();
		if (render_buffer) {
			al_set_target_bitmap(render_buffer->bitmap);
		}
	}

	return old_target;
}

void Engine::finish_draw(bool force_no_target_change, ALLEGRO_BITMAP *old_target)
{
	if (!force_no_target_change) {
		if (render_buffer) {
			int w = al_get_bitmap_width(old_target);
			int h = al_get_bitmap_height(old_target);
			al_set_target_bitmap(old_target);
			al_draw_scaled_bitmap(
				render_buffer->bitmap,
				0, 0,
				al_get_bitmap_width(render_buffer->bitmap),
				al_get_bitmap_height(render_buffer->bitmap),
				0, 0,
				w, h,
				0
			);
#if defined ALLEGRO_ANDROID || defined ALLEGRO_IPHONE
			if (draw_touch_controls && !gamepadConnected()) {
				switch (touch_input_type) {
					case TOUCHINPUT_GUI:
						// nothing
						break;
					case TOUCHINPUT_SPEECH:
						draw_touch_input_button(3, TOUCH_ADVANCE);
						break;
					case TOUCHINPUT_BATTLE: {
						Battle_Loop *loop = GET_BATTLE_LOOP;
						if (!loop) {
							loop = General::find_in_vector<Battle_Loop *, Loop *>(mini_loops);
						}
						if (loop) {
							for (int i = 0; i < 4; i++) {
								Player *player = loop->get_active_player()->get_player();
								if (!player) {
									continue;
								}
								TouchType type;
								std::string ability = player->get_selected_abilities(true, dynamic_cast<Runner_Loop *>(loop), loop->is_cart_battle())[i];
								if (ability == "") {
									continue;
								}
								if (ability == "ATTACK") {
									type = TOUCH_ATTACK;
								}
								else if (ability == "USE") {
									type = TOUCH_USE;
								}
								else if (ability == "JUMP") {
									type = TOUCH_JUMP;
								}
								else {
									type = TOUCH_SPECIAL;
								}
								draw_touch_input_button(i, type);
							}
							if (!dynamic_cast<Runner_Loop *>(loop) && !loop->is_cart_battle()) {
								draw_touch_input_button(5, TOUCH_SWITCH);
							}
							draw_touch_input_stick();
						}

						// Draw "help" for first battle

						Battle_Loop *bl = GET_BATTLE_LOOP;

						if (bl && !dynamic_cast<Runner_Loop *>(bl) && !engine->milestone_is_complete("heading_west_guards")) {
							float f = fmod(al_get_time(), 4);
							int dx, dy;
							if (f < 1) {
								dx = 0;
								dy = -1;
							}
							else if (f < 2) {
								dx = 1;
								dy = 0;
								f -= 1;
							}
							else if (f < 3) {
								dx = 0;
								dy = 1;
								f -= 2;
							}
							else {
								dx = -1;
								dy = 0;
								f -= 3;
							}

							float x = 50 + dx * 50 * f;
							float y = 100 + dy * 50 * f;

							Wrap::Bitmap *bmp = helping_hand;
							int bmpw = al_get_bitmap_width(bmp->bitmap);
							int bmph = al_get_bitmap_height(bmp->bitmap);

							float screen_w = al_get_display_width(display);
							float screen_h = al_get_display_height(display);
							float scalex = screen_w / cfg.screen_w;
							float scaley = screen_h / cfg.screen_h;

							int height = scaley * TOUCH_BUTTON_RADIUS * 2;

							ALLEGRO_COLOR tint = al_map_rgba_f(0.5f, 0.5f, 0.5f, 0.5f);

							al_draw_tinted_scaled_bitmap(
								bmp->bitmap,
								tint,
								0, 0, bmpw, bmph,
								x * scalex,
								y * scaley,
								scaley * TOUCH_BUTTON_RADIUS * 2,
								height,
								0
							);
						}

						break;
					}
					case TOUCHINPUT_MAP: {
						draw_touch_input_button(3, TOUCH_ADVANCE);
						draw_touch_input_button(4, TOUCH_MENU);
						draw_touch_input_stick();
						break;
					}
					case TOUCHINPUT_AREA: {
						if (can_move) {
							draw_touch_input_button(1, TOUCH_SPECIAL);
							draw_touch_input_button(3, TOUCH_ACTIVATE);
							draw_touch_input_button(4, TOUCH_MENU);
							draw_touch_input_button(5, TOUCH_SWITCH);
							draw_touch_input_stick();
						}
						break;
					}
				}
			}
#endif
		}
	}
}

void Engine::draw_to_display(ALLEGRO_BITMAP *cfg_screen_sized_bitmap)
{
	ALLEGRO_BITMAP *save = al_get_target_bitmap();
	al_set_target_backbuffer(display);

	ALLEGRO_BITMAP *old_target = set_draw_target(false);

	al_draw_bitmap(cfg_screen_sized_bitmap, 0, 0, 0);

	finish_draw(false, old_target);

	al_set_target_bitmap(save);
}

void Engine::draw_all(std::vector<Loop *> loops, bool force_no_target_change)
{
	Area_Loop *l = GET_AREA_LOOP;
	double now = al_get_time();
        if (now >= _shake.start && now <= _shake.start+_shake.duration) {
		int x = General::rand() % (_shake.amount);
		int y = General::rand() % (_shake.amount);
		if (l) {
			l->get_area()->set_rumble_offset(General::Point<float>(x, y));
		}
		else {
			Battle_Loop *loop = GET_BATTLE_LOOP;
			if (loop) {
				loop->set_rumble_offset(General::Point<float>(x, y));
			}
		}
	}
	else if (shaking && now >= _shake.start) {
		if (l) {
			l->get_area()->set_rumble_offset(General::Point<float>(0, 0));
		}
		else {
			Battle_Loop *loop = GET_BATTLE_LOOP;
			if (loop) {
				loop->set_rumble_offset(General::Point<float>(0, 0));
			}
		}
		shaking = false;
	}

	ALLEGRO_BITMAP *old_target = set_draw_target(force_no_target_change);

	draw_loops(loops);
	post_draw();
	post_draw_loops(loops);

	// draw fps
	if (cfg.show_fps) {
		now = al_get_time();
		double elapsed = now - first_frame_time;
		if (elapsed >= 3) {
			curr_fps = round(frames_drawn / elapsed);
			frames_drawn = 0;
			first_frame_time = now;
		}
		General::draw_text(
			General::itos(curr_fps),
			1, 1,
			0,
			General::FONT_HEAVY
		);
	}

	finish_draw(force_no_target_change, old_target);

	l = GET_AREA_LOOP;
	if (l) {
		l->get_area()->set_rumble_offset(General::Point<float>(0, 0));
	}
}

void Engine::draw_loops(std::vector<Loop *> loops)
{
	for (size_t i = 0; i < loops.size(); i++) {
		loops[i]->draw();
	}
}

void Engine::post_draw_loops(std::vector<Loop *> loops)
{
	for (size_t i = 0; i < loops.size(); i++) {
		loops[i]->post_draw();
	}
}

void Engine::delete_pending_loops()
{
	if (loops_to_delete.size() > 0) {
		for (size_t i = 0; i < loops_to_delete.size(); i++) {
			delete loops_to_delete[i];
		}
		loops_to_delete.clear();
	}
}

void Engine::load_translation()
{
	char modded[5000];
	ALLEGRO_USTR *all = al_ustr_new("");

	ALLEGRO_DEBUG("In load_translation");

	if (translation) {
		al_destroy_config(translation);
	}

#ifdef TRANSLATION_BUILD
	ALLEGRO_FILE *f = al_fopen("TRANSLATION.utf8", "rb");
#else
	ALLEGRO_FILE *f = cpa->load("text/" + cfg.language + ".utf8");
#endif
	translation = al_load_config_file_f(f);
	al_fclose(f);
	f = cpa->load("text/" + cfg.language + ".utf8");
	ALLEGRO_CONFIG *copy = al_load_config_file_f(f);
	al_fclose(f);

	ALLEGRO_CONFIG_ENTRY *it;
	char const *key = al_get_first_config_entry(
		copy,
		NULL,
		&it
	);

	while (key) {
		const char *txt = al_get_config_value(
			translation,
			NULL,
			key
		);

		al_ustr_append_cstr(all, txt);

		if (txt[0] == '"') {
			strcpy(modded, txt+1);
			if (modded[strlen(modded)-1] == '"') {
				modded[strlen(modded)-1] = '\0';
				al_set_config_value(
					translation,
					NULL,
					key,
					modded
				);
			}
		}
		key = al_get_next_config_entry(&it);
	}

	al_destroy_config(copy);

	al_get_ustr_width(General::get_font(General::FONT_LIGHT), all);
	al_get_ustr_width(General::get_font(General::FONT_HEAVY), all);

	al_ustr_free(all);
}

// Below doesn't want convenience 't' defined (see engine.h at the end)
#undef t

const char *Engine::t(const char *tag)
{
	static char missing[100];

	if (tag[0] == 0 || translation == NULL) {
		missing[0] = 0;
		return missing;
	}

	const char *str = al_get_config_value(
		translation,
		NULL,
		tag
	);

	if (str) {
		return str;
	}

	snprintf(missing, 100, "***%s***", tag);
	return missing;
}

static bool halt_ended = false;
struct Wait_Info {
	ALLEGRO_EVENT_QUEUE *queue;
	Engine *engine;
};

#ifdef ALLEGRO_ANDROID
void *wait_for_drawing_resume(void *arg)
{
	Wait_Info *i = (Wait_Info *)arg;
	ALLEGRO_EVENT_QUEUE *queue = i->queue;
	Engine *engine = i->engine;

	while (1) {
		ALLEGRO_EVENT event;
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING) {
			break;
		}
		else {
			if (event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
				engine->switch_in();
			}
			else if (event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
				engine->switch_out();
			}
		}
	}

	halt_ended = true;
	al_broadcast_cond(engine->get_halt_cond());

	return NULL;
}
#else
void *wait_for_drawing_resume(void *arg)
{
	Wait_Info *i = (Wait_Info *)arg;
	ALLEGRO_EVENT_QUEUE *queue = i->queue;

	while (true) {
		ALLEGRO_EVENT event;
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING) {
			break;
		}
	}

	halt_ended = true;
	al_broadcast_cond(engine->get_halt_cond());

	return NULL;
}
#endif

void Engine::switch_out()
{
#if defined ALLEGRO_ANDROID || defined ALLEGRO_IPHONE
	cfg.save();
#endif

	switched_out = true;

	stop_timers();

	music_name = Music::get_playing();
	Music::stop();
}

void Engine::switch_in()
{
	switched_out = false;

	start_timers();
}

void Engine::handle_halt(ALLEGRO_EVENT *event)
{
	stop_timers();

#ifdef ALLEGRO_ANDROID
	Wrap::destroy_loaded_shaders();
	for (size_t i = 0; i < old_loops.size(); i++) {
		for (size_t j = 0; j < old_loops[i].size(); j++) {
			old_loops[i][j]->destroy_graphics();
		}
	}
	for (size_t i = 0; i < loops.size(); i++) {
		loops[i]->destroy_graphics();
	}
	General::destroy_fonts();
	Wrap::destroy_loaded_bitmaps();
	switch_music_out();

	Wrap::destroy_bitmap(render_buffer);
	render_buffer = NULL;
	Wrap::destroy_bitmap(work_bitmap);
	work_bitmap = NULL;
#endif

	al_acknowledge_drawing_halt(display);

	Wait_Info i;
	i.queue = event_queue;
	i.engine = this;

	al_run_detached_thread(wait_for_drawing_resume, &i);

	al_lock_mutex(halt_mutex);
	while (!halt_ended) {
		al_wait_cond(halt_cond, halt_mutex);
	}
	al_unlock_mutex(halt_mutex);

	halt_ended = false;

	al_acknowledge_drawing_resume(display);

	start_timers();

#ifdef ALLEGRO_ANDROID
	setup_screen_size();

	Wrap::reload_loaded_shaders();
	for (size_t i = 0; i < old_loops.size(); i++) {
		for (size_t j = 0; j < old_loops[i].size(); j++) {
			old_loops[i][j]->reload_graphics();
		}
	}
	for (size_t i = 0; i < loops.size(); i++) {
		loops[i]->reload_graphics();
	}
	General::load_fonts();
	load_translation();
	Wrap::reload_loaded_bitmaps();
	switch_music_in();
#endif
}

void Engine::stop_timers()
{
	al_stop_timer(timer);
}

void Engine::start_timers()
{
	al_start_timer(timer);
}

int Engine::add_particle_group(std::string type, int layer, int align, std::vector<std::string> bitmap_names)
{
	Particle::Particle_Group *pg = new Particle::Particle_Group(type, layer, align, bitmap_names);
	particle_groups.push_back(pg);
	return pg->id;
}

void Engine::delete_particle_group(int id)
{
	for (size_t i = 0; i < particle_groups.size(); i++) {
		if (particle_groups[i]->id == id) {
			Particle::Particle_Group *pg = particle_groups[i];
			particle_groups.erase(particle_groups.begin()+i);
			delete pg;
		}
	}
}

Particle::Particle_Group *Engine::get_particle_group(int id)
{
	for (size_t i = 0; i < particle_groups.size(); i++) {
		if (particle_groups[i]->id == id) {
			return particle_groups[i];
		}
	}

	return NULL;
}

// -1 == boss save
void Engine::save_game(int number)
{
	General::log_message("Saving game " + General::itos(number));

	// FIXME: update save state version here
	Lua::set_save_state_version(1);

	bool is_map;
	std::string area_name;
	Area_Manager *area = NULL;

	/* Saving game happens from menu so Area_Loop is in the old_loops stack.
	 * Search it from back (newest) to front
	 */
	Area_Loop *l = NULL;
	Map_Loop *ml = NULL;
	for (int i = old_loops.size()-1; i >= 0; i--) {
		l = General::find_in_vector<Area_Loop *, Loop *>(old_loops[i]);
		if (l) {
			is_map = false;
			area = l->get_area();
			area_name = area->get_name();
			break;
		}
	}

	if (l == NULL) {
		l = General::find_in_vector<Area_Loop *, Loop *>(loops);
		
		if (l == NULL) {
			// We must be in a map, get its name
			is_map = true;
			for (size_t i = old_loops.size()-1; i >= 0; i--) {
				ml = General::find_in_vector<Map_Loop *, Loop *>(old_loops[i]);
				if (ml) {
					break;
				}
			}
			if (!ml) {
				ml = General::find_in_vector<Map_Loop *, Loop *>(loops);
			}
			if (ml) {
				area_name = "map:" + ml->get_current_location_name();
			}
		}
		else {
			is_map = false;
			area = l->get_area();
			area_name = area->get_name();
		}
	}

	Lua::clear_saved_lua_lines();
	
	char line[1000];

	// NOTE: Only Egbert+Frogbert and Egbert+Frogbert+Bisou (in any order) supported
	std::vector<Player *> players;
	
	if (is_map) {
		players = ml->get_players();
	}
	else {
		players = l->get_players();
	}

	if (players.size() == 2) {
		snprintf(line, 1000, "set_saved_players(\"%s\", \"%s\")\n",
			players[0]->get_name().c_str(), players[1]->get_name().c_str());
	}
	else {
		snprintf(line, 1000, "set_saved_players(\"%s\", \"%s\", \"%s\")\n",
			players[0]->get_name().c_str(), players[1]->get_name().c_str(), players[2]->get_name().c_str());
	}
	Lua::add_saved_lua_line(line);

	snprintf(line, 1000, "set_elapsed_time(%f)\n", Game_Specific_Globals::elapsed_time);
	Lua::add_saved_lua_line(line);
	snprintf(line, 1000, "set_cash(%d)\n", Game_Specific_Globals::cash);
	Lua::add_saved_lua_line(line);
	snprintf(line, 1000, "add_crystals(%d)\n", Game_Specific_Globals::crystals);
	Lua::add_saved_lua_line(line);

	std::vector<Game_Specific_Globals::Item> &items = Game_Specific_Globals::get_items();
	std::vector<Equipment::Weapon> &weapons = Game_Specific_Globals::get_weapons();
	std::vector<Equipment::Armor> &armor = Game_Specific_Globals::get_armor();
	std::vector<Equipment::Accessory> &accessories = Game_Specific_Globals::get_accessories();

	for (size_t i = 0; i < items.size(); i++) {
		snprintf(line, 1000, "give_items(\"%s\", %d)\n", items[i].name.c_str(), items[i].quantity);
		Lua::add_saved_lua_line(line);
	}
	for (size_t i = 0; i < weapons.size(); i++) {
		snprintf(line, 1000, "give_equipment(WEAPON, \"%s\", %d)\n", weapons[i].name.c_str(), weapons[i].quantity);
		Lua::add_saved_lua_line(line);
	}
	for (size_t i = 0; i < armor.size(); i++) {
		snprintf(line, 1000, "give_equipment(ARMOR, \"%s\", %d)\n", armor[i].name.c_str(), 1);
		Lua::add_saved_lua_line(line);
	}
	for (size_t i = 0; i < accessories.size(); i++) {
		snprintf(line, 1000, "give_equipment(ACCESSORY, \"%s\", %d)\n", accessories[i].name.c_str(), 1);
		Lua::add_saved_lua_line(line);
	}

	snprintf(line, 1000, "difficulty(%d)\n", (int)cfg.difficulty);
	Lua::add_saved_lua_line(line);

	Lua::store_battle_attributes(players);
	Lua::add_battle_attributes_lines();

	if (!is_map) {
		lua_State *lua_state = area->get_lua_state();

		lua_getglobal(lua_state, "is_dungeon");
		bool exists = !lua_isnil(lua_state, -1);
		lua_pop(lua_state, 1);
		
		General::Point<float> pos;

		if (exists) {
			Map_Entity *player = area->get_entity(0);
			pos = player->get_position();
			snprintf(line, 1000, "restore_item(SAVE_PLAYER, %d, %f, %f)\n", player->get_layer(), pos.x, pos.y);
			Lua::add_saved_lua_line(line);
			/* NOTE: When this is called, the area loop is not in main loops so most lua callbacks won't work */
			Lua::call_lua(area->get_lua_state(), "save_level_state", ">");
		}
		else {
			pos = area->get_player_start_position();
			snprintf(line, 1000, "restore_item(SAVE_PLAYER, %d, %f, %f)\n", area->get_player_start_layer(), pos.x, pos.y);
			Lua::add_saved_lua_line(line);
		}
	}

	std::string save_filename = General::get_save_filename(number);

	ALLEGRO_FILE *f = al_fopen(save_filename.c_str(), "w");

	if (f) {
		char buf[2000];
		snprintf(buf, 2000, "set_save_state_version(%d)\n", Lua::get_save_state_version());
		al_fputs(f, buf);
		
		snprintf(buf, 2000, "set_area_name(\"%s\")\n", area_name.c_str());
		al_fputs(f, buf);

		std::map<std::string, bool>::iterator it;
		for (it = milestones.begin(); it != milestones.end(); it++) {
			std::pair<const std::string, bool> &p = *it;
			snprintf(buf, 2000, "set_milestone_complete(\"%s\", %s)\n",
				p.first.c_str(),
				p.second ? "true" : "false"
			);
			al_fputs(f, buf);
		}
		Lua::write_saved_lua_lines(f);
		al_fclose(f);
	}

	if (number != -1) {
		save_number_last_used = number;
	}
}

// -1 == boss save
void Engine::load_game(int number)
{
	General::log_message("Loading game " + General::itos(number));

	// If there isn't a line, we need to set version 1
	Lua::set_save_state_version(1);

	Lua::clear_before_load();

	// Initialize stuff

	std::string save_filename = General::get_save_filename(number);

	lua_State *lua_state = luaL_newstate();
	Lua::open_lua_libs(lua_state);
	Lua::register_c_functions(lua_state);

	Lua::load_global_scripts(lua_state);

	if (!luaL_loadfile(lua_state, save_filename.c_str())) {
		started_new_game = false;
		if (lua_pcall(lua_state, 0, 0, 0)) {
			Lua::dump_lua_stack(lua_state);
		}
	}
	else {
		started_new_game = true;
	}

	lua_close(lua_state);

	game_just_loaded = true;

	if (number != -1) {
		save_number_last_used = number;
	}
}
	
void Engine::reset_game()
{
	Lua::reset_game();
	Lua::init_battle_attributes();
}

void Engine::logic()
{
}

void Engine::flush_event_queue()
{
	al_flush_event_queue(event_queue);
}

Wrap::Bitmap *Engine::get_hero_shadow()
{
	return hero_shadow_bmp;
}

Wrap::Bitmap *Engine::get_big_shadow()
{
	return big_shadow_bmp;
}

Wrap::Bitmap *Engine::get_work_bitmap()
{
	return work_bitmap;
}

static void before_flip_callback()
{
	ALLEGRO_BITMAP *target = al_get_target_bitmap();
	al_set_target_backbuffer(engine->get_display());
	al_draw_scaled_bitmap(
		target,
		0, 0,
		al_get_bitmap_width(target),
		al_get_bitmap_height(target),
		0, 0,
		al_get_display_width(engine->get_display()),
		al_get_display_height(engine->get_display()),
		0
	);
	al_set_target_bitmap(target);
}

static bool ok_callback(tgui::TGUIWidget *widget)
{
	return widget != NULL;
}

void Engine::notify(std::vector<std::string> texts, std::vector<Loop *> *loops_to_draw)
{
	ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
	int th = General::get_font_line_height(General::FONT_LIGHT);
	int w = 200;
	int h = (4+texts.size())*th+4;
	Wrap::Bitmap *frame_bmp = Wrap::create_bitmap(w+10, h+10);
	al_set_target_bitmap(work_bitmap->bitmap);
	al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));
	al_draw_filled_rectangle(
		5, 5,
		5+w, 5+h,
		al_color_name("black")
	);
	al_set_target_bitmap(frame_bmp->bitmap);
	al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));
	Graphics::draw_bitmap_shadow_region_no_intermediate(work_bitmap, 0, 0, 10+w, 10+h, 0, 0);
	ALLEGRO_COLOR main_color = al_color_name("lightgrey");
	ALLEGRO_COLOR dark_color = Graphics::change_brightness(main_color, 0.5f);
	al_draw_filled_rectangle(
		5, 5,
		5+w, 5+h,
		main_color
	);
	al_draw_rectangle(
		5+0.5f, 5+0.5f,
		5+w-0.5f, 5+h-0.5f,
		dark_color,
		1
	);
	for (size_t i = 0; i < texts.size(); i++) {
		int x = w / 2 + 5;
		int y = th+i*th + 5;
		General::draw_text(texts[i], al_color_name("black"), x, y, ALLEGRO_ALIGN_CENTER);
	}
	main_color = Graphics::change_brightness(main_color, 1.1f);
	dark_color = Graphics::change_brightness(dark_color, 1.1f);
	Wrap::Bitmap *ok_bitmap = Wrap::create_bitmap(50, th+4);
	al_set_target_bitmap(ok_bitmap->bitmap);
	al_clear_to_color(dark_color);
	al_draw_filled_rectangle(
		1, 1,
		49, th+3,
		main_color
	);
	General::draw_text(t("OK"), al_color_name("black"), 25, 2, ALLEGRO_ALIGN_CENTER);
	al_set_target_bitmap(old_target);

	W_Icon *frame = new W_Icon(frame_bmp);
	frame->setX(cfg.screen_w/2-5-w/2);
	frame->setY(cfg.screen_h/2-5-h/2);
	
	int framex = frame->getX()+5;
	int framey = frame->getY()+5;

	W_Icon *ok_icon = new W_Icon(ok_bitmap);
	ok_icon->setX(framex+w/2-25);
	ok_icon->setY(framey+th*(2+texts.size()));
	W_Button *ok_button = new W_Button(
		ok_icon->getX(),
		ok_icon->getY(),
		ok_icon->getWidth(),
		ok_icon->getHeight()
	);
	
	tgui::push();

	tgui::addWidget(frame);
	tgui::addWidget(ok_icon);
	tgui::addWidget(ok_button);
	tgui::setFocus(ok_button);

	ALLEGRO_BITMAP *bg;
	int flags = al_get_new_bitmap_flags();
	draw_touch_controls = false;
	if (cfg.linear_filtering) {
		int no_preserve_flag;
#ifdef ALLEGRO_ANDROID
		no_preserve_flag = 0;
#else
#ifdef ALLEGRO_WINDOWS
		if (al_get_display_flags(engine->get_display()) & ALLEGRO_DIRECT3D) {
			no_preserve_flag = 0;
		}
		else
#endif
		{
			no_preserve_flag = ALLEGRO_NO_PRESERVE_TEXTURE;
		}
#endif
		al_set_new_bitmap_flags(no_preserve_flag | ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
		bg = al_create_bitmap(
			al_get_bitmap_width(render_buffer->bitmap),
			al_get_bitmap_height(render_buffer->bitmap)
		);
		al_set_target_bitmap(bg);
		draw_all(loops_to_draw == NULL ? loops : *loops_to_draw, false);
	}
	else {
		al_set_new_bitmap_flags(flags & ~ALLEGRO_NO_PRESERVE_TEXTURE);
		bg = al_create_bitmap(
			al_get_display_width(display),
			al_get_display_height(display)
		);
		al_set_target_bitmap(bg);
		ALLEGRO_TRANSFORM t;
		al_identity_transform(&t);
		float scale = (float)al_get_display_width(display) / cfg.screen_w;
		al_scale_transform(&t, scale, scale);
		al_use_transform(&t);
		draw_all(loops_to_draw == NULL ? loops : *loops_to_draw, false);
	}
	draw_touch_controls = true;
	al_set_new_bitmap_flags(flags);

	void (*bfc)();
	if (render_buffer) {
		al_set_target_bitmap(render_buffer->bitmap);
		bfc = before_flip_callback;
	}
	else {
		al_set_target_backbuffer(display);
		bfc = NULL;
	}

	do_modal(
		event_queue,
		al_map_rgba_f(0, 0, 0, 0),
		bg,
		ok_callback,
		NULL,
		bfc,
		NULL
	);

	al_destroy_bitmap(bg);
	al_set_target_bitmap(old_target);

	frame->remove();
	ok_button->remove();
	ok_icon->remove();
	delete frame;
	delete ok_button;
	delete ok_icon;

	Wrap::destroy_bitmap(frame_bmp);
	Wrap::destroy_bitmap(ok_bitmap);

	tgui::pop();

	al_flush_event_queue(event_queue);
}

static W_Button *button1;
static W_Button *button2;
static int prompt_result;
static bool prompt_callback(tgui::TGUIWidget *widget)
{
	if (widget == button1) {
		prompt_result = 0;
		return true;
	}
	if (widget == button2) {
		prompt_result = 1;
		return true;
	}
	return false;
}

int Engine::prompt(std::vector<std::string> texts, std::string text1, std::string text2, std::vector<Loop *> *loops_to_draw)
{
	ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
	int th = General::get_font_line_height(General::FONT_LIGHT);
	int w = 200;
	int h = (4+texts.size())*th+4;
	Wrap::Bitmap *frame_bmp = Wrap::create_bitmap(w+10, h+10);
	al_set_target_bitmap(work_bitmap->bitmap);
	al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));
	al_draw_filled_rectangle(
		5, 5,
		5+w, 5+h,
		al_color_name("black")
	);
	al_set_target_bitmap(frame_bmp->bitmap);
	al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));
	Graphics::draw_bitmap_shadow_region_no_intermediate(work_bitmap, 0, 0, 10+w, 10+h, 0, 0);
	ALLEGRO_COLOR main_color = al_color_name("lightgrey");
	ALLEGRO_COLOR dark_color = Graphics::change_brightness(main_color, 0.5f);
	al_draw_filled_rectangle(
		5, 5,
		5+w, 5+h,
		main_color
	);
	al_draw_rectangle(
		5+0.5f, 5+0.5f,
		5+w-0.5f, 5+h-0.5f,
		dark_color,
		1
	);
	for (size_t i = 0; i < texts.size(); i++) {
		int x = w / 2 + 5;
		int y = th+i*th + 5;
		General::draw_text(texts[i], al_color_name("black"), x, y, ALLEGRO_ALIGN_CENTER);
	}
	main_color = Graphics::change_brightness(main_color, 1.1f);
	dark_color = Graphics::change_brightness(dark_color, 1.1f);
	Wrap::Bitmap *bitmap1 = Wrap::create_bitmap(50, th+4);
	al_set_target_bitmap(bitmap1->bitmap);
	al_clear_to_color(dark_color);
	al_draw_filled_rectangle(
		1, 1,
		49, th+3,
		main_color
	);
	General::draw_text(text1, al_color_name("black"), 25, 2, ALLEGRO_ALIGN_CENTER);
	Wrap::Bitmap *bitmap2 = Wrap::create_bitmap(50, th+4);
	al_set_target_bitmap(bitmap2->bitmap);
	al_clear_to_color(dark_color);
	al_draw_filled_rectangle(
		1, 1,
		49, th+3,
		main_color
	);
	General::draw_text(text2, al_color_name("black"), 25, 2, ALLEGRO_ALIGN_CENTER);
	al_set_target_bitmap(old_target);

	W_Icon *frame = new W_Icon(frame_bmp);
	frame->setX(cfg.screen_w/2-5-w/2);
	frame->setY(cfg.screen_h/2-5-h/2);
	
	int framex = frame->getX()+5;
	int framey = frame->getY()+5;

	W_Icon *icon1 = new W_Icon(bitmap1);
	icon1->setX(framex+w/2-50-5);
	icon1->setY(framey+th*(2+texts.size()));
	button1 = new W_Button(
		icon1->getX(),
		icon1->getY(),
		icon1->getWidth(),
		icon1->getHeight()
	);
	
	W_Icon *icon2 = new W_Icon(bitmap2);
	icon2->setX(framex+w/2+5);
	icon2->setY(framey+th*(2+texts.size()));
	button2 = new W_Button(
		icon2->getX(),
		icon2->getY(),
		icon2->getWidth(),
		icon2->getHeight()
	);

	tgui::push();

	tgui::addWidget(frame);
	tgui::addWidget(icon1);
	tgui::addWidget(button1);
	tgui::addWidget(icon2);
	tgui::addWidget(button2);
	tgui::setFocus(button1);

	ALLEGRO_BITMAP *bg;
	int flags = al_get_new_bitmap_flags();
	draw_touch_controls = false;
	if (cfg.linear_filtering) {
		int no_preserve_flag;
#ifdef ALLEGRO_ANDROID
		no_preserve_flag = 0;
#else
#ifdef ALLEGRO_WINDOWS
		if (al_get_display_flags(engine->get_display()) & ALLEGRO_DIRECT3D) {
			no_preserve_flag = 0;
		}
		else
#endif
		{
			no_preserve_flag = ALLEGRO_NO_PRESERVE_TEXTURE;
		}
#endif
		al_set_new_bitmap_flags(no_preserve_flag | ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
		bg = al_create_bitmap(
			al_get_bitmap_width(render_buffer->bitmap),
			al_get_bitmap_height(render_buffer->bitmap)
		);
		al_set_target_bitmap(bg);
		draw_all(loops_to_draw == NULL ? loops : *loops_to_draw, false);
	}
	else {
		al_set_new_bitmap_flags(flags & ~ALLEGRO_NO_PRESERVE_TEXTURE);
		bg = al_create_bitmap(
			al_get_display_width(display),
			al_get_display_height(display)
		);
		al_set_target_bitmap(bg);
		ALLEGRO_TRANSFORM t;
		al_identity_transform(&t);
		float scale = (float)al_get_display_width(display) / cfg.screen_w;
		al_scale_transform(&t, scale, scale);
		al_use_transform(&t);
		draw_all(loops_to_draw == NULL ? loops : *loops_to_draw, false);
	}
	draw_touch_controls = true;
	al_set_new_bitmap_flags(flags);
	
	void (*bfc)();
	if (render_buffer) {
		al_set_target_bitmap(render_buffer->bitmap);
		bfc = before_flip_callback;
	}
	else {
		al_set_target_backbuffer(display);
		bfc = NULL;
	}

	do_modal(
		event_queue,
		al_map_rgba_f(0, 0, 0, 0),
		bg,
		prompt_callback,
		NULL,
		bfc,
		NULL
	);

	al_destroy_bitmap(bg);
	al_set_target_bitmap(old_target);

	frame->remove();
	button1->remove();
	button2->remove();
	icon1->remove();
	icon2->remove();
	delete frame;
	delete button1;
	delete button2;
	delete icon1;
	delete icon2;

	Wrap::destroy_bitmap(frame_bmp);
	Wrap::destroy_bitmap(bitmap1);
	Wrap::destroy_bitmap(bitmap2);

	tgui::pop();

	al_flush_event_queue(event_queue);

	return prompt_result;
}

static W_Button *yes_button;
static W_Button *no_button;
static bool yes_no_result;
static bool yes_no_callback(tgui::TGUIWidget *widget)
{
	if (widget == yes_button) {
		yes_no_result = true;
		return true;
	}
	if (widget == no_button) {
		yes_no_result = false;
		return true;
	}
	return false;
}

bool Engine::yes_no_prompt(std::vector<std::string> texts, std::vector<Loop *> *loops_to_draw)
{
	ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
	int th = General::get_font_line_height(General::FONT_LIGHT);
	int w = 200;
	int h = (4+texts.size())*th+4;
	Wrap::Bitmap *frame_bmp = Wrap::create_bitmap(w+10, h+10);
	al_set_target_bitmap(work_bitmap->bitmap);
	al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));
	al_draw_filled_rectangle(
		5, 5,
		5+w, 5+h,
		al_color_name("black")
	);
	al_set_target_bitmap(frame_bmp->bitmap);
	al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));
	Graphics::draw_bitmap_shadow_region_no_intermediate(work_bitmap, 0, 0, 10+w, 10+h, 0, 0);
	ALLEGRO_COLOR main_color = al_color_name("lightgrey");
	ALLEGRO_COLOR dark_color = Graphics::change_brightness(main_color, 0.5f);
	al_draw_filled_rectangle(
		5, 5,
		5+w, 5+h,
		main_color
	);
	al_draw_rectangle(
		5+0.5f, 5+0.5f,
		5+w-0.5f, 5+h-0.5f,
		dark_color,
		1
	);
	for (size_t i = 0; i < texts.size(); i++) {
		int x = w / 2 + 5;
		int y = th+i*th + 5;
		General::draw_text(texts[i], al_color_name("black"), x, y, ALLEGRO_ALIGN_CENTER);
	}
	main_color = Graphics::change_brightness(main_color, 1.1f);
	dark_color = Graphics::change_brightness(dark_color, 1.1f);
	Wrap::Bitmap *yes_bitmap = Wrap::create_bitmap(50, th+4);
	al_set_target_bitmap(yes_bitmap->bitmap);
	al_clear_to_color(dark_color);
	al_draw_filled_rectangle(
		1, 1,
		49, th+3,
		main_color
	);
	General::draw_text(t("YES"), al_color_name("black"), 25, 2, ALLEGRO_ALIGN_CENTER);
	Wrap::Bitmap *no_bitmap = Wrap::create_bitmap(50, th+4);
	al_set_target_bitmap(no_bitmap->bitmap);
	al_clear_to_color(dark_color);
	al_draw_filled_rectangle(
		1, 1,
		49, th+3,
		main_color
	);
	General::draw_text(t("NO"), al_color_name("black"), 25, 2, ALLEGRO_ALIGN_CENTER);
	al_set_target_bitmap(old_target);

	W_Icon *frame = new W_Icon(frame_bmp);
	frame->setX(cfg.screen_w/2-5-w/2);
	frame->setY(cfg.screen_h/2-5-h/2);
	
	int framex = frame->getX()+5;
	int framey = frame->getY()+5;

	W_Icon *yes_icon = new W_Icon(yes_bitmap);
	yes_icon->setX(framex+w/2-50-5);
	yes_icon->setY(framey+th*(2+texts.size()));
	yes_button = new W_Button(
		yes_icon->getX(),
		yes_icon->getY(),
		yes_icon->getWidth(),
		yes_icon->getHeight()
	);
	
	W_Icon *no_icon = new W_Icon(no_bitmap);
	no_icon->setX(framex+w/2+5);
	no_icon->setY(framey+th*(2+texts.size()));
	no_button = new W_Button(
		no_icon->getX(),
		no_icon->getY(),
		no_icon->getWidth(),
		no_icon->getHeight()
	);

	tgui::push();

	tgui::addWidget(frame);
	tgui::addWidget(yes_icon);
	tgui::addWidget(yes_button);
	tgui::addWidget(no_icon);
	tgui::addWidget(no_button);
	tgui::setFocus(yes_button);

	ALLEGRO_BITMAP *bg;
	int flags = al_get_new_bitmap_flags();
	draw_touch_controls = false;
	if (cfg.linear_filtering) {
		int no_preserve_flag;
#ifdef ALLEGRO_ANDROID
		no_preserve_flag = 0;
#else
#ifdef ALLEGRO_WINDOWS
		if (al_get_display_flags(engine->get_display()) & ALLEGRO_DIRECT3D) {
			no_preserve_flag = 0;
		}
		else
#endif
		{
			no_preserve_flag = ALLEGRO_NO_PRESERVE_TEXTURE;
		}
#endif
		al_set_new_bitmap_flags(no_preserve_flag | ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
		bg = al_create_bitmap(
			al_get_bitmap_width(render_buffer->bitmap),
			al_get_bitmap_height(render_buffer->bitmap)
		);
		al_set_target_bitmap(bg);
		draw_all(loops_to_draw == NULL ? loops : *loops_to_draw, false);
	}
	else {
		al_set_new_bitmap_flags(flags & ~ALLEGRO_NO_PRESERVE_TEXTURE);
		bg = al_create_bitmap(
			al_get_display_width(display),
			al_get_display_height(display)
		);
		al_set_target_bitmap(bg);
		ALLEGRO_TRANSFORM t;
		al_identity_transform(&t);
		float scale = (float)al_get_display_width(display) / cfg.screen_w;
		al_scale_transform(&t, scale, scale);
		al_use_transform(&t);
		draw_all(loops_to_draw == NULL ? loops : *loops_to_draw, false);
	}
	draw_touch_controls = true;
	al_set_new_bitmap_flags(flags);
	
	void (*bfc)();
	if (render_buffer) {
		al_set_target_bitmap(render_buffer->bitmap);
		bfc = before_flip_callback;
	}
	else {
		al_set_target_backbuffer(display);
		bfc = NULL;
	}

	do_modal(
		event_queue,
		al_map_rgba_f(0, 0, 0, 0),
		bg,
		yes_no_callback,
		NULL,
		bfc,
		NULL
	);

	al_destroy_bitmap(bg);
	al_set_target_bitmap(old_target);

	frame->remove();
	yes_button->remove();
	no_button->remove();
	yes_icon->remove();
	no_icon->remove();
	delete frame;
	delete yes_button;
	delete no_button;
	delete yes_icon;
	delete no_icon;

	Wrap::destroy_bitmap(frame_bmp);
	Wrap::destroy_bitmap(yes_bitmap);
	Wrap::destroy_bitmap(no_bitmap);

	tgui::pop();

	al_flush_event_queue(event_queue);

	return yes_no_result;
}

static int get_number_min;
static int get_number_max;
static W_Button *ok_button;
static W_Button *cancel_button;
static W_Button *get_number_up_button;
static W_Button *get_number_down_button;
static W_Integer *number_widget;
static int get_number_result;
static bool get_number_callback(tgui::TGUIWidget *widget)
{
	if (widget == ok_button) {
		get_number_result = number_widget->get_number();
		return true;
	}
	else if (widget == cancel_button) {
		get_number_result = -1;
		return true;
	}
	else if (widget == get_number_up_button) {
		int n = number_widget->get_number();
		if (n < get_number_max) {
			n++;
			number_widget->set_number(n);
		}
	}
	else if (widget == get_number_down_button) {
		int n = number_widget->get_number();
		if (n > get_number_min) {
			n--;
			number_widget->set_number(n);
		}
	}
	return false;
}

int Engine::get_number(std::vector<std::string> texts, int low, int high, int start, std::vector<Loop *> *loops_to_draw)
{
	get_number_min = low;
	get_number_max = high;
	ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
	int th = General::get_font_line_height(General::FONT_LIGHT);
	int w = 200;
	int h = (6+texts.size())*th+4;
	Wrap::Bitmap *frame_bmp = Wrap::create_bitmap(w+10, h+10);
	al_set_target_bitmap(work_bitmap->bitmap);
	al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));
	al_draw_filled_rectangle(
		5, 5,
		5+w, 5+h,
		al_color_name("black")
	);
	al_set_target_bitmap(frame_bmp->bitmap);
	al_clear_to_color(al_map_rgba_f(0, 0, 0, 0));
	Graphics::draw_bitmap_shadow_region_no_intermediate(work_bitmap, 0, 0, 10+w, 10+h, 0, 0);
	ALLEGRO_COLOR main_color = al_color_name("lightgrey");
	ALLEGRO_COLOR dark_color = Graphics::change_brightness(main_color, 0.5f);
	al_draw_filled_rectangle(
		5, 5,
		5+w, 5+h,
		main_color
	);
	al_draw_rectangle(
		5+0.5f, 5+0.5f,
		5+w-0.5f, 5+h-0.5f,
		dark_color,
		1
	);
	for (size_t i = 0; i < texts.size(); i++) {
		int x = (w+10) / 2;
		int y = th+i*th + 5;
		General::draw_text(texts[i], al_color_name("black"), x, y, ALLEGRO_ALIGN_CENTER);
	}
	
	W_Icon *frame = new W_Icon(frame_bmp);
	frame->setX(cfg.screen_w/2-5-w/2);
	frame->setY(cfg.screen_h/2-5-h/2);

	int framex = frame->getX()+5;
	int framey = frame->getY()+5;

	number_widget = new W_Integer(start);
	number_widget->setX(framex+w/2);
	number_widget->setY(framey+th*(2+texts.size())+th/2);
	get_number_down_button = new W_Button("misc_graphics/interface/small_down_arrow.cpi");
	tgui::centerWidget(get_number_down_button, framex+w/2 - 25, framey+th*(2+texts.size())+th/2);
	get_number_up_button = new W_Button("misc_graphics/interface/small_up_arrow.cpi");
	tgui::centerWidget(get_number_up_button, framex+w/2 + 25, framey+th*(2+texts.size())+th/2);

	main_color = Graphics::change_brightness(main_color, 1.1f);
	dark_color = Graphics::change_brightness(dark_color, 1.1f);
	Wrap::Bitmap *ok_bitmap = Wrap::create_bitmap(50, th+4);
	al_set_target_bitmap(ok_bitmap->bitmap);
	al_clear_to_color(dark_color);
	al_draw_filled_rectangle(
		1, 1,
		49, th+3,
		main_color
	);
	General::draw_text(t("OK"), al_color_name("black"), 25, 2, ALLEGRO_ALIGN_CENTER);
	Wrap::Bitmap *cancel_bitmap = Wrap::create_bitmap(50, th+4);
	al_set_target_bitmap(cancel_bitmap->bitmap);
	al_clear_to_color(dark_color);
	al_draw_filled_rectangle(
		1, 1,
		49, th+3,
		main_color
	);
	General::draw_text(t("CANCEL"), al_color_name("black"), 25, 2, ALLEGRO_ALIGN_CENTER);
	al_set_target_bitmap(old_target);

	W_Icon *ok_icon = new W_Icon(ok_bitmap);
	ok_icon->setX(framex+w/2-5-50);
	ok_icon->setY(framey+th*(4+texts.size()));
	ok_button = new W_Button(
		ok_icon->getX(),
		ok_icon->getY(),
		ok_icon->getWidth(),
		ok_icon->getHeight()
	);
	
	W_Icon *cancel_icon = new W_Icon(cancel_bitmap);
	cancel_icon->setX(framex+w/2+5);
	cancel_icon->setY(framey+th*(4+texts.size()));
	cancel_button = new W_Button(
		cancel_icon->getX(),
		cancel_icon->getY(),
		cancel_icon->getWidth(),
		cancel_icon->getHeight()
	);

	tgui::push();

	tgui::addWidget(frame);
	tgui::addWidget(get_number_down_button);
	tgui::addWidget(number_widget);
	tgui::addWidget(get_number_up_button);
	tgui::addWidget(ok_icon);
	tgui::addWidget(ok_button);
	tgui::addWidget(cancel_icon);
	tgui::addWidget(cancel_button);

	tgui::setFocus(ok_button);

	ALLEGRO_BITMAP *bg;
	int flags = al_get_new_bitmap_flags();
	draw_touch_controls = false;
	if (cfg.linear_filtering) {
		int no_preserve_flag;
#ifdef ALLEGRO_ANDROID
		no_preserve_flag = 0;
#else
#ifdef ALLEGRO_WINDOWS
		if (al_get_display_flags(engine->get_display()) & ALLEGRO_DIRECT3D) {
			no_preserve_flag = 0;
		}
		else
#endif
		{
			no_preserve_flag = ALLEGRO_NO_PRESERVE_TEXTURE;
		}
#endif
		al_set_new_bitmap_flags(no_preserve_flag | ALLEGRO_MIN_LINEAR | ALLEGRO_MAG_LINEAR);
		bg = al_create_bitmap(
			al_get_bitmap_width(render_buffer->bitmap),
			al_get_bitmap_height(render_buffer->bitmap)
		);
		al_set_target_bitmap(bg);
		draw_all(loops_to_draw == NULL ? loops : *loops_to_draw, false);
	}
	else {
		al_set_new_bitmap_flags(flags & ~ALLEGRO_NO_PRESERVE_TEXTURE);
		bg = al_create_bitmap(
			al_get_display_width(display),
			al_get_display_height(display)
		);
		al_set_target_bitmap(bg);
		ALLEGRO_TRANSFORM t;
		al_identity_transform(&t);
		float scale = (float)al_get_display_width(display) / cfg.screen_w;
		al_scale_transform(&t, scale, scale);
		al_use_transform(&t);
		draw_all(loops_to_draw == NULL ? loops : *loops_to_draw, false);
	}
	draw_touch_controls = true;
	al_set_new_bitmap_flags(flags);
		
	void (*bfc)();
	if (render_buffer) {
		al_set_target_bitmap(render_buffer->bitmap);
		bfc = before_flip_callback;
	}
	else {
		al_set_target_backbuffer(display);
		bfc = NULL;
	}

	do_modal(
		event_queue,
		al_map_rgba_f(0, 0, 0, 0),
		bg,
		get_number_callback,
		NULL,
		bfc,
		NULL
	);

	al_destroy_bitmap(bg);
	al_set_target_bitmap(old_target);

	frame->remove();
	get_number_down_button->remove();
	number_widget->remove();
	get_number_up_button->remove();
	ok_button->remove();
	cancel_button->remove();
	ok_icon->remove();
	cancel_icon->remove();
	delete frame;
	delete get_number_down_button;
	delete number_widget;
	delete get_number_up_button;
	delete ok_button;
	delete cancel_button;
	delete ok_icon;
	delete cancel_icon;

	Wrap::destroy_bitmap(frame_bmp);
	Wrap::destroy_bitmap(ok_bitmap);
	Wrap::destroy_bitmap(cancel_bitmap);

	tgui::pop();
	
	al_flush_event_queue(event_queue);

	return get_number_result;
}

std::string Engine::get_last_area()
{
	return last_area;
}

void Engine::set_last_area(std::string last_area)
{
	this->last_area = last_area;
}

static void fade(std::vector<Loop *> loops, double time, bool out)
{
	double fade_start = al_get_time();

	while (true) {
		ALLEGRO_BITMAP *old_target = engine->set_draw_target(false);

		al_clear_to_color(al_color_name("black"));

		engine->draw_all(loops, true);

		bool done = false;

		double elapsed = (al_get_time() - fade_start) / time;
		if (elapsed > 1) {
			elapsed = 1;
			done = true;
		}

		if (out) {
			al_draw_filled_rectangle(0,  0, cfg.screen_w, cfg.screen_h,
				al_map_rgba_f(0, 0, 0, elapsed));
		}
		else {
			float p = 1.0f - (float)elapsed;
			al_draw_filled_rectangle(0,  0, cfg.screen_w, cfg.screen_h,
				al_map_rgba_f(0, 0, 0, p));
		}

		engine->finish_draw(false, old_target);

		al_flip_display();

		if (done) {
			break;
		}

		al_rest(1.0/60.0);
	}

	tgui::releaseKeysAndButtons();
	engine->flush_event_queue();
}

void Engine::fade_out(double time)
{
	::fade(engine->get_loops(), time, true);
}

void Engine::fade_in(double time)
{
	::fade(engine->get_loops(), time, false);
}

void Engine::fade_out(std::vector<Loop *> loops, double time)
{
	::fade(loops, time, true);
}

void Engine::fade_in(std::vector<Loop *> loops, double time)
{
	::fade(loops, time, false);
}

void Engine::set_game_over(bool game_over)
{
	this->game_over = game_over;
}

bool Engine::game_is_over()
{
	return game_over;
}

bool Engine::can_use_crystals()
{
	return milestone_is_complete("pyou_intro");
}

void Engine::load_cpa()
{
#if defined ALLEGRO_ANDROID
	// Loaded by PHYSFS
	cpa = new CPA("assets/data.cpa.uncompressed");
#elif defined ALLEGRO_IPHONE || defined ALLEGRO_RASPBERRYPI
	cpa = new CPA("data.cpa.uncompressed");
#else
	cpa = new CPA("data.cpa");
#endif
}

void Engine::set_done(bool done)
{
	this->done = done;
}

bool Engine::get_done()
{
	return done;
}

void Engine::set_purchased(bool purchased)
{
	this->purchased = purchased;
}

bool Engine::get_purchased()
{
	return purchased;
}

ALLEGRO_EVENT_QUEUE *Engine::get_event_queue()
{
	return event_queue;
}

bool Engine::is_switched_out()
{
	return switched_out;
}

bool Engine::get_continued_or_saved()
{
	return continued_or_saved;
}

void Engine::set_continued_or_saved(bool continued_or_saved)
{
	this->continued_or_saved = continued_or_saved;
}

void Engine::loaded_video(Video_Player *v)
{
	_loaded_video = v;
}

std::string Engine::get_music_name()
{
	return music_name;
}

void Engine::set_send_tgui_events(bool send_tgui_events)
{
	this->send_tgui_events = send_tgui_events;
	if (send_tgui_events == false) {
		tgui::releaseKeysAndButtons();
	}
}

bool Engine::get_send_tgui_events()
{
	return send_tgui_events;
}

int Engine::get_save_number_last_used()
{
	return save_number_last_used;
}

bool Engine::is_render_buffer(ALLEGRO_BITMAP *bmp)
{
	return render_buffer && render_buffer->bitmap == bmp;
}

bool Engine::in_mini_loop()
{
	return _in_mini_loop;
}

std::vector<Loop *> Engine::get_mini_loops()
{
	return mini_loops;
}

void Engine::set_mini_loops(std::vector<Loop *> loops)
{
	mini_loops = loops;
}

TouchInputType Engine::get_touch_input_type()
{
	return touch_input_type;
}

void Engine::set_touch_input_type(TouchInputType type)
{
	touch_input_type = type;
}

void Engine::get_touch_input_button_position(int location, int *x, int *y)
{
	// offset from middle to top button
	int y_offset = (cfg.screen_h/2 - TOUCH_BUTTON_RADIUS*5) / 2;
	int stick_y_offset = (cfg.screen_h/2 - TOUCH_STICK_RADIUS*2) / 2;
	// actual y offset of top button (top or bottom)
	int base_y = touch_input_on_bottom ? cfg.screen_h / 2 + y_offset : y_offset;

	int base_x = cfg.screen_w - 10  - TOUCH_BUTTON_RADIUS * 5;

	int square_y = touch_input_on_bottom ? 10 : cfg.screen_h - 10 - TOUCH_BUTTON_RADIUS * TOUCH_SQUARE_PERCENT;

	int button_x, button_y;

	switch (location) {
		case -1: // stick
			button_x = 10 + TOUCH_STICK_TRAVEL;
			button_y = touch_input_on_bottom ? cfg.screen_h/2 + stick_y_offset : stick_y_offset;
			break;
		case 4:
			button_x = base_x;
			button_y = square_y;
			break;
		case 5:
			button_x = base_x + TOUCH_BUTTON_RADIUS * 3;
			button_y = square_y;
			break;
		case 0:
			button_x = base_x + TOUCH_BUTTON_RADIUS * 1.5f;
			button_y = base_y;
			break;
		case 1:
			button_x = base_x;
			button_y = base_y + TOUCH_BUTTON_RADIUS * 1.5f;
			break;
		case 2:	
			button_x = base_x + TOUCH_BUTTON_RADIUS * 3;
			button_y = base_y + TOUCH_BUTTON_RADIUS * 1.5f;
			break;
		default:
			button_x = base_x + TOUCH_BUTTON_RADIUS * 1.5f;
			button_y = base_y + TOUCH_BUTTON_RADIUS * 3;
			break;
	}

	*x = button_x;
	*y = button_y;
}

void Engine::draw_touch_input_button(int location, TouchType bitmap)
{
	int x, y;

	get_touch_input_button_position(location, &x, &y);

	Wrap::Bitmap *bmp = touch_bitmaps[bitmap];
	int bmpw = al_get_bitmap_width(bmp->bitmap);
	int bmph = al_get_bitmap_height(bmp->bitmap);

	float screen_w = al_get_display_width(display);
	float screen_h = al_get_display_height(display);
	float scalex = screen_w / cfg.screen_w;
	float scaley = screen_h / cfg.screen_h;

	ALLEGRO_STATE state;
	al_store_state(&state, ALLEGRO_STATE_BLENDER);

	bool pressed = false;
	for (size_t i = 0; i < touches.size(); i++) {
		if (touches[i].location == location) {
			pressed = true;
			break;
		}
	}

	int height;
	if (location == 4 || location == 5) {
		height = scaley * TOUCH_BUTTON_RADIUS * TOUCH_SQUARE_PERCENT;
	}
	else {
		height = scaley * TOUCH_BUTTON_RADIUS * 2;
	}

	ALLEGRO_COLOR tint = al_map_rgba_f(0.5f, 0.5f, 0.5f, 0.5f);

	al_draw_tinted_scaled_bitmap(
		bmp->bitmap,
		tint,
		0, 0, bmpw, bmph,
		x * scalex,
		y * scaley,
		scaley * TOUCH_BUTTON_RADIUS * 2,
		height,
		0
	);
	
	if (pressed) {
		al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
		al_draw_tinted_scaled_bitmap(
			bmp->bitmap,
			tint,
			0, 0, bmpw, bmph,
			x * scalex,
			y * scaley,
			scaley * TOUCH_BUTTON_RADIUS * 2,
			height,
			0
		);
	}
	
	al_restore_state(&state);
}

bool Engine::touch_is_on_button(ALLEGRO_EVENT *event, int location)
{
	int x, y;
	get_touch_input_button_position(location, &x, &y);
	
	if (location == 4 || location == 5) {
		General::Point<float> p(event->touch.x, event->touch.y);
		General::Point<float> topleft(x, y);
		General::Point<float> bottomright(x+TOUCH_BUTTON_RADIUS*2, y+TOUCH_BUTTON_RADIUS*TOUCH_SQUARE_PERCENT);
		if (checkcoll_point_box(p, topleft, bottomright)) {
			return true;
		}
		return false;
	}

	x += TOUCH_BUTTON_RADIUS;
	y += TOUCH_BUTTON_RADIUS;

	if (General::distance(x, y, event->touch.x, event->touch.y) <= TOUCH_BUTTON_RADIUS) {
		return true;
	}
	return false;
}

void Engine::add_touch(ALLEGRO_EVENT *event, int location)
{
	Touch t;
	t.id = event->touch.id;
	t.x = event->touch.x;
	t.y = event->touch.y;
	t.location = location;
	touches.push_back(t);
}

int Engine::find_touch(int id)
{
	int index = -1;

	for (size_t i = 0; i < touches.size(); i++) {
		if (touches[i].id == id) {
			index = i;
			break;
		}
	}

	return index;
}

void Engine::update_touch(ALLEGRO_EVENT *event)
{
	// This moves the controls to top or bottom but we don't want that anymore.
	/*
	if (event->type == ALLEGRO_EVENT_TOUCH_BEGIN) {
		if (event->touch.x >= cfg.screen_w/2-25 && event->touch.x <= cfg.screen_w/2+25) {
			if (event->touch.y < cfg.screen_h/2) {
				touch_input_on_bottom = false;
			}
			else {
				touch_input_on_bottom = true;
			}
			return;
		}
	}
	*/

	// Battle stick code
	if (GET_BATTLE_LOOP && !dynamic_cast<Runner_Loop *>(GET_BATTLE_LOOP)) {
		if (event->type == ALLEGRO_EVENT_TOUCH_BEGIN || event->type == ALLEGRO_EVENT_TOUCH_MOVE) {
			bool go = true;
			for (int i = 0; i < 6; i++) {
				if (touch_is_on_button(event, i)) {
					go = false;
					break;
				}
			}
			if (go) {
				go = false;

				if (event->type == ALLEGRO_EVENT_TOUCH_BEGIN) {
					for (size_t i = 0; i < touches.size(); i++) {
						if (touches[i].location == -1) {
							// Don't put multiple touches on stick
							return;
						}
					}
					add_touch(event, -1);

					Area_Loop *al = GET_AREA_LOOP;
					Battle_Loop *bl = GET_BATTLE_LOOP;
					if (bl) {
						Battle_Player *player = bl->get_active_player();
						if (player->is_facing_right()) {
							last_direction = 0;
						}
						else {
							last_direction = M_PI;
						}
					}
					else if (al) {
						Area_Manager *area = al->get_area();
						Map_Entity *player = area->get_entity(0);
						General::Direction dir = player->get_direction();
						switch (dir) {
							case General::DIR_N:
								last_direction = M_PI * 1.5;
								break;
							case General::DIR_NE:
								last_direction = M_PI * 1.5 + M_PI / 4;
								break;
							case General::DIR_E:
								last_direction = 0;
								break;
							case General::DIR_SE:
								last_direction = M_PI / 4;
								break;
							case General::DIR_S:
								last_direction = M_PI / 2;
								break;
							case General::DIR_SW:
								last_direction = M_PI / 2 + M_PI / 4;
								break;
							case General::DIR_W:
								last_direction = M_PI;
								break;
							default:
								last_direction = M_PI + M_PI / 4;
								break;
						}
					}
					else {
						last_direction = 0;
					}
				}
				else {
					int index = find_touch(event->touch.id);
					if (index >= 0 && touches[index].location == -1) {
						float old_x = touches[index].x;
						float old_y = touches[index].y;
						float x = event->touch.x;
						float y = event->touch.y;

						float dx = x - old_x;
						float dy = y - old_y;

						int dw = al_get_display_width(display);
						int dh = al_get_display_height(display);
						int sm = MIN(dw, dh);

						if (General::distance(x, y, old_x, old_y) >= (sm * (GET_BATTLE_LOOP ? 0.005f: 0.01f))) {
							touches[index].x = x;
							touches[index].y = y;

							float a = atan2(dy, dx);

							while (a < 0) a += M_PI*2;
							while (a >= M_PI*2) a -= M_PI*2;

							if (GET_BATTLE_LOOP) {
								if (a >= M_PI*1.5 + M_PI/4 || a < M_PI/4) {
									a = 0;
								}
								else if (a < M_PI/4*3) {
									a = M_PI/2;
								}
								else if (a < M_PI/4*5) {
									a = M_PI;
								}
								else {
									a = M_PI*1.5;
								}
							}
							else {
								if (GET_AREA_LOOP && GET_AREA_LOOP->get_area()->get_entity(0)->get_inputs()[Map_Entity::X] == 0 && GET_AREA_LOOP->get_area()->get_entity(0)->get_inputs()[Map_Entity::Y] == 0) {
									if (General::angle_difference(a, 0) <= M_PI/4) {
										a = 0;
									}
									else if (General::angle_difference(a, M_PI/2) <= M_PI/4) {
										a = M_PI/2;
									}
									else if (General::angle_difference(a, M_PI) <= M_PI/4) {
										a = M_PI;
									}
									else {
										a = M_PI * 1.5;
									}
								}
								else {
									if (General::angle_difference(last_direction, a) < M_PI/4) {
										a = last_direction;
									}
									else if (General::angle_difference_clockwise(last_direction, a) < M_PI/2) {
										a = last_direction + M_PI / 4;
									}
									else if (General::angle_difference_counter_clockwise(last_direction, a) < M_PI/2) {
										a = last_direction - M_PI / 4;
									}
									else {
										if (General::angle_difference(a, 0) <= M_PI/4) {
											a = 0;
										}
										else if (General::angle_difference(a, M_PI/2) <= M_PI/4) {
											a = M_PI/2;
										}
										else if (General::angle_difference(a, M_PI) <= M_PI/4) {
											a = M_PI;
										}
										else {
											a = M_PI * 1.5;
										}
									}
								}
							}

							while (a < 0) a += M_PI*2;
							while (a >= M_PI*2) a -= M_PI*2;

							last_direction = a;

							float xx = cos(a);
							float yy = sin(a);

							event->type = ALLEGRO_EVENT_JOYSTICK_AXIS;
							event->joystick.id = 0;
							event->joystick.stick = 0;
							event->joystick.axis = 0;
							event->joystick.pos = xx;
							ALLEGRO_EVENT extra;
							extra.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
							extra.joystick.id = 0;
							extra.joystick.stick = 0;
							extra.joystick.axis = 1;
							extra.joystick.pos = yy;
							extra_events.push_back(extra);
						}
					}
				}

				return;
			}
		}
		else {
			int index = find_touch(event->touch.id);
			if (index >= 0 && touches[index].location == -1) {
				touches.erase(touches.begin() + index);
				event->type = ALLEGRO_EVENT_JOYSTICK_AXIS;
				event->joystick.id = 0;
				event->joystick.stick = 0;
				event->joystick.axis = 0;
				event->joystick.pos = 0;
				ALLEGRO_EVENT extra;
				extra.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
				extra.joystick.id = 0;
				extra.joystick.stick = 0;
				extra.joystick.axis = 1;
				extra.joystick.pos = 0;
				extra_events.push_back(extra);
				last_direction = -1;
				return;
			}
		}
	}
	else { // Other stick code
		if (event->type == ALLEGRO_EVENT_TOUCH_BEGIN || event->type == ALLEGRO_EVENT_TOUCH_MOVE) {
			int x, y;
			get_touch_input_button_position(-1, &x, &y);
			x += TOUCH_STICK_RADIUS;
			y += TOUCH_STICK_RADIUS;
			bool go = false;
			if (event->type == ALLEGRO_EVENT_TOUCH_BEGIN) {
				if (General::distance(x, y, event->touch.x, event->touch.y) <= TOUCH_STICK_RADIUS+TOUCH_STICK_TRAVEL) {
					for (size_t i = 0; i < touches.size(); i++) {
						if (touches[i].location == -1) {
							// Don't put multiple touches on stick
							return;
						}
					}
					add_touch(event, -1);
					go = true;
				}
			}
			else {
				int index = find_touch(event->touch.id);
				if (index >= 0 && touches[index].location == -1) {
					touches[index].x = event->touch.x;
					touches[index].y = event->touch.y;
					go = true;
				}
			}

			if (go) {
				float xx = event->touch.x - x;
				float yy = event->touch.y - y;
				xx /= (TOUCH_STICK_RADIUS+TOUCH_STICK_TRAVEL)/2;
				yy /= (TOUCH_STICK_RADIUS+TOUCH_STICK_TRAVEL)/2;
				if (xx < -1) xx = -1;
				if (xx > 1) xx = 1;
				if (yy < -1) yy = -1;
				if (yy > 1) yy = 1;
				event->type = ALLEGRO_EVENT_JOYSTICK_AXIS;
				event->joystick.id = 0;
				event->joystick.stick = 0;
				event->joystick.axis = 0;
				event->joystick.pos = xx;
				ALLEGRO_EVENT extra;
				extra.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
				extra.joystick.id = 0;
				extra.joystick.stick = 0;
				extra.joystick.axis = 1;
				extra.joystick.pos = yy;
				extra_events.push_back(extra);
			
				return;
			}
		}
		else {
			int index = find_touch(event->touch.id);
			if (index >= 0 && touches[index].location == -1) {
				touches.erase(touches.begin() + index);
				event->type = ALLEGRO_EVENT_JOYSTICK_AXIS;
				event->joystick.id = 0;
				event->joystick.stick = 0;
				event->joystick.axis = 0;
				event->joystick.pos = 0;
				ALLEGRO_EVENT extra;
				extra.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
				extra.joystick.id = 0;
				extra.joystick.stick = 0;
				extra.joystick.axis = 1;
				extra.joystick.pos = 0;
				extra_events.push_back(extra);
				return;
			}
		}
	}
	
	Battle_Loop *battle_loop = GET_BATTLE_LOOP;
	if (!battle_loop) {
		battle_loop = General::find_in_vector<Battle_Loop *, Loop *>(mini_loops);
	}

	if (event->type == ALLEGRO_EVENT_TOUCH_BEGIN) {
		if (touch_input_type == TOUCHINPUT_SPEECH) {
			if (touch_is_on_button(event, 3)) {
				add_touch(event, 3);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_ability[3];
			}
		}
		else if (touch_input_type == TOUCHINPUT_BATTLE) {
			bool processed = false;
			for (int i = 0; i < 4; i++) {
				if (touch_is_on_button(event, i)) {
					add_touch(event, i);
					event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
					event->joystick.button = cfg.joy_ability[i];
					processed = true;
					break;
				}
			}
			if (battle_loop && !dynamic_cast<Runner_Loop *>(battle_loop) && !battle_loop->is_cart_battle() && !processed && touch_is_on_button(event, 5)) {
				add_touch(event, 5);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_switch;
			}
		}
		else if (touch_input_type == TOUCHINPUT_MAP) {
			if (touch_is_on_button(event, 3)) {
				add_touch(event, 3);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_ability[3];
			}
			else if (touch_is_on_button(event, 4)) {
				add_touch(event, 4);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_menu;
			}
		}
		else if (touch_input_type == TOUCHINPUT_AREA) {
			if (touch_is_on_button(event, 1)) {
				add_touch(event, 1);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_ability[1];
			}
			else if (touch_is_on_button(event, 3)) {
				add_touch(event, 3);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_ability[3];
			}
			else if (touch_is_on_button(event, 4)) {
				add_touch(event, 4);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_menu;
			}
			else if (touch_is_on_button(event, 5)) {
				add_touch(event, 5);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_switch;
			}
		}
	}
	else if (event->type == ALLEGRO_EVENT_TOUCH_END) {
		int index = find_touch(event->touch.id);
		if (index >= 0) {
			if (touch_input_type == TOUCHINPUT_SPEECH) {
				touches.erase(touches.begin() + index);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
				event->joystick.button = cfg.joy_ability[3];
			}
			else if (touch_input_type == TOUCHINPUT_BATTLE) {
				int b = touches[index].location;
				touches.erase(touches.begin() + index);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
				if (b == 5) {
					event->joystick.button = cfg.joy_switch;
				}
				else {
					event->joystick.button = cfg.joy_ability[b];
				}
			}
			else if (touch_input_type == TOUCHINPUT_MAP) {
				int b = touches[index].location;
				touches.erase(touches.begin() + index);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
				if (b == 4) {
					event->joystick.button = cfg.joy_menu;
				}
				else {
					event->joystick.button = cfg.joy_ability[3];
				}
			}
			else if (touch_input_type == TOUCHINPUT_AREA) {
				int b = touches[index].location;
				touches.erase(touches.begin() + index);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
				if (b == 4) {
					event->joystick.button = cfg.joy_menu;
				}
				else if (b == 5) {
					event->joystick.button = cfg.joy_switch;
				}
				else {
					event->joystick.button = cfg.joy_ability[b];
				}
			}
		}
	}
	else {
		int index = find_touch(event->touch.id);
		if (touch_input_type == TOUCHINPUT_SPEECH) {
			if (index < 0 && touch_is_on_button(event, 3)) {
				add_touch(event, 3);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_ability[3];
			}
			else if (index >= 0 && touches[index].location == 3 && !touch_is_on_button(event, 3)) {
				touches.erase(touches.begin() + index);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
				event->joystick.button = cfg.joy_ability[3];
			}
		}
		else if (touch_input_type == TOUCHINPUT_BATTLE) {
			bool processed = false;
			for (int i = 0; i < 4; i++) {
				if (index < 0 && touch_is_on_button(event, i)) {
					add_touch(event, i);
					event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
					event->joystick.button = cfg.joy_ability[i];
					processed = true;
					break;
				}
				else if (index >= 0 && touches[index].location == i && !touch_is_on_button(event, i)) {
					touches.erase(touches.begin() + index);
					event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
					event->joystick.button = cfg.joy_ability[i];
					processed = true;
					break;
				}
			}
			if (!processed && battle_loop && !dynamic_cast<Runner_Loop *>(battle_loop) && !battle_loop->is_cart_battle()) {
				if (index < 0 && touch_is_on_button(event, 5)) {
					add_touch(event, 5);
					event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
					event->joystick.button = cfg.joy_switch;
				}
				else if (index >= 0 && touches[index].location == 5 && !touch_is_on_button(event, 5)) {
					touches.erase(touches.begin() + index);
					event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
					event->joystick.button = cfg.joy_switch;
				}
			}
		}
		else if (touch_input_type == TOUCHINPUT_MAP) {
			if (index < 0 && touch_is_on_button(event, 3)) {
				add_touch(event, 3);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_ability[3];
			}
			else if (index >= 0 && touches[index].location == 3 && !touch_is_on_button(event, 3)) {
				touches.erase(touches.begin() + index);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
				event->joystick.button = cfg.joy_ability[3];
			}
			else if (index < 0 && touch_is_on_button(event, 4)) {
				add_touch(event, 4);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_menu;
			}
			else if (index >= 0 && touches[index].location == 4 && !touch_is_on_button(event, 4)) {
				touches.erase(touches.begin() + index);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
				event->joystick.button = cfg.joy_menu;
			}
		}
		else if (touch_input_type == TOUCHINPUT_AREA) {
			if (index < 0 && touch_is_on_button(event, 1)) {
				add_touch(event, 1);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_ability[1];
			}
			else if (index >= 0 && touches[index].location == 1 && !touch_is_on_button(event, 1)) {
				touches.erase(touches.begin() + index);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
				event->joystick.button = cfg.joy_ability[1];
			}
			else if (index < 0 && touch_is_on_button(event, 3)) {
				add_touch(event, 3);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_ability[3];
			}
			else if (index >= 0 && touches[index].location == 3 && !touch_is_on_button(event, 3)) {
				touches.erase(touches.begin() + index);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
				event->joystick.button = cfg.joy_ability[3];
			}
			else if (index < 0 && touch_is_on_button(event, 4)) {
				add_touch(event, 4);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_menu;
			}
			else if (index >= 0 && touches[index].location == 4 && !touch_is_on_button(event, 4)) {
				touches.erase(touches.begin() + index);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
				event->joystick.button = cfg.joy_menu;
			}
			else if (index < 0 && touch_is_on_button(event, 5)) {
				add_touch(event, 5);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN;
				event->joystick.button = cfg.joy_switch;
			}
			else if (index >= 0 && touches[index].location == 5 && !touch_is_on_button(event, 5)) {
				touches.erase(touches.begin() + index);
				event->type = ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
				event->joystick.button = cfg.joy_switch;
			}
		}
	}
}

void Engine::draw_touch_input_stick()
{
	if (GET_BATTLE_LOOP) {
		return;
	}

	int index = -1;

	for (size_t i = 0; i < touches.size(); i++) {
		if (touches[i].location == -1) {
			index = i;
			break;
		}
	}

	int x, y;
	get_touch_input_button_position(-1, &x, &y);

	Wrap::Bitmap *bmp = touch_bitmaps[TOUCH_ANALOGBASE];
	int bmpw = al_get_bitmap_width(bmp->bitmap);
	int bmph = al_get_bitmap_height(bmp->bitmap);

	float screen_w = al_get_display_width(display);
	float screen_h = al_get_display_height(display);
	float scalex = screen_w / cfg.screen_w;
	float scaley = screen_h / cfg.screen_h;

	ALLEGRO_COLOR tint = al_map_rgba_f(0.5f, 0.5f, 0.5f, 0.5f);

	al_draw_tinted_scaled_bitmap(
		bmp->bitmap,
		tint,
		0, 0, bmpw, bmph,
		x * scalex,
		y * scaley,
		TOUCH_STICK_RADIUS * 2 * scaley,
		TOUCH_STICK_RADIUS * 2 * scaley,
		0
	);

	if (index >= 0) {
		float x2 = touches[index].x - TOUCH_STICK_RADIUS;
		float y2 = touches[index].y - TOUCH_STICK_RADIUS;
		float dx = x2 - x;
		float dy = y2 - y;
		float a = atan2(dy, dx);
		float length = sqrt(dx*dx + dy*dy);
		if (length > TOUCH_STICK_RADIUS) {
			x = x + cos(a) * TOUCH_STICK_RADIUS;
			y = y + sin(a) * TOUCH_STICK_RADIUS;
		}
		else {
			x = x2;
			y = y2;
		}
	}
	
	bmp = touch_bitmaps[TOUCH_ANALOGSTICK];
	bmpw = al_get_bitmap_width(bmp->bitmap);
	bmph = al_get_bitmap_height(bmp->bitmap);

	al_draw_tinted_scaled_bitmap(
		bmp->bitmap,
		tint,
		0, 0, bmpw, bmph,
		x * scalex,
		y * scaley,
		TOUCH_STICK_RADIUS * 2 * scaley,
		TOUCH_STICK_RADIUS * 2 * scaley,
		0
	);
}

void Engine::clear_touches()
{
	touches.clear();

}

void Engine::switch_music_out()
{
	Music::stop();
	std::map<std::string, Sample>::iterator it;
	for (it = sfx.begin(); it != sfx.end(); it++) {
		std::pair<const std::string, Sample> &p = *it;
		if (p.second.looping) {
			Sound::stop(p.second.sample);
		}
	}
}

void Engine::switch_music_in()
{
	std::map<std::string, Sample>::iterator it;
	for (it = sfx.begin(); it != sfx.end(); it++) {
		std::pair<const std::string, Sample> &p = *it;
		if (p.second.looping) {
			play_sample(p.first);
		}
	}
}

void Engine::set_can_move(bool can_move)
{
	this->can_move = can_move;
}

bool Engine::get_started_new_game()
{
	return started_new_game;
}

void Engine::add_extra_event(ALLEGRO_EVENT event)
{
	extra_events.push_back(event);
}

void process_dpad_events(ALLEGRO_EVENT *event)
{
	if (dont_process_dpad_events) {
		return;
	}

	if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN && event->joystick.button == cfg.joy_dpad_l) {
		event->joystick.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
		event->joystick.id = 0;
		event->joystick.stick = 0;
		event->joystick.axis = 0;
		event->joystick.pos = -1;
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN && event->joystick.button == cfg.joy_dpad_r) {
		event->joystick.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
		event->joystick.id = 0;
		event->joystick.stick = 0;
		event->joystick.axis = 0;
		event->joystick.pos = 1;
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN && event->joystick.button == cfg.joy_dpad_u) {
		event->joystick.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
		event->joystick.id = 0;
		event->joystick.stick = 0;
		event->joystick.axis = 1;
		event->joystick.pos = -1;
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN && event->joystick.button == cfg.joy_dpad_d) {
		event->joystick.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
		event->joystick.id = 0;
		event->joystick.stick = 0;
		event->joystick.axis = 1;
		event->joystick.pos = 1;
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP && event->joystick.button == cfg.joy_dpad_l) {
		event->joystick.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
		event->joystick.id = 0;
		event->joystick.stick = 0;
		event->joystick.axis = 0;
		event->joystick.pos = 0;
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP && event->joystick.button == cfg.joy_dpad_r) {
		event->joystick.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
		event->joystick.id = 0;
		event->joystick.stick = 0;
		event->joystick.axis = 0;
		event->joystick.pos = 0;
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP && event->joystick.button == cfg.joy_dpad_u) {
		event->joystick.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
		event->joystick.id = 0;
		event->joystick.stick = 0;
		event->joystick.axis = 1;
		event->joystick.pos = 0;
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_BUTTON_UP && event->joystick.button == cfg.joy_dpad_d) {
		event->joystick.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
		event->joystick.id = 0;
		event->joystick.stick = 0;
		event->joystick.axis = 1;
		event->joystick.pos = 0;
	}
	else if (event->type != ALLEGRO_EVENT_JOYSTICK_AXIS) {
		return;
	}
}

