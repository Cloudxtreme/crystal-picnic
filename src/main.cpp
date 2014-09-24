#include "crystalpicnic.h"
#include "main.h"
#include "area_loop.h"
#include "map_loop.h"
#include "widgets.h"
#include "game_specific_globals.h"
#include "saveload_loop.h"
#include "hqm_loop.h"
#include "runner_loop.h"
#include "settings_loop.h"

// FIXME:
#include "credits_loop.h"

#ifdef ALLEGRO_WINDOWS
#include "direct3d.h"
#endif

#ifdef ALLEGRO_ANDROID

#ifdef ALLEGRO_ANDROID
#include "android.h"
#endif

bool try_purchase(ALLEGRO_EVENT_QUEUE *event_queue)
{
	int purchased = checkPurchased();
	bool ret = false;
	if (purchased != 1) {
		doIAP();
		purchased = -1;
		while (purchased == -1) {
			purchased = isPurchased();
			al_rest(0.01);
			while (!al_event_queue_is_empty(event_queue)) {
				ALLEGRO_EVENT event;
				al_get_next_event(event_queue, &event);
				if (event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_OUT) {
					engine->switch_out();
				}
				else if (event.type == ALLEGRO_EVENT_DISPLAY_SWITCH_IN) {
					engine->switch_in();
				}
				else if (event.type == ALLEGRO_EVENT_DISPLAY_HALT_DRAWING) {
					engine->handle_halt(&event);
				}
			}
		}
	}
	if (purchased == 1) {
		ret = true;
		queryPurchased(); // This writes a receipt to permanent storage
	}
	al_rest(0.5);
	al_flush_event_queue(event_queue);
	return ret;
}
#endif

#define GET_NEXT (al_get_time() + 1.5)

static Wrap::Bitmap *title;
static Wrap::Bitmap *title_glow;
static double start = 0.0;
static double next = 0.0;

static void draw_main_menu()
{
	al_clear_to_color(al_color_name("black"));

	al_draw_bitmap(
		title->bitmap,
		cfg.screen_w - al_get_bitmap_width(title->bitmap),
		cfg.screen_h - al_get_bitmap_height(title->bitmap),
		0
	);

	ALLEGRO_STATE old_blend_state;
	al_store_state(&old_blend_state, ALLEGRO_STATE_BLENDER);
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
	double now = al_get_time();
	double duration = next - start;
	double start_save = start;
	if (now > next) {
		now = next;
		start = now;
		next = GET_NEXT;
	}
	double elapsed = now - start_save;
	float alpha;
	if (elapsed >= duration / 2.0f) {
		alpha = 1.0f - ((elapsed - duration / 2.0f) / (duration / 2.0f));
	}
	else {
		alpha = elapsed / (duration / 2.0f);
	}
	if (General::rand() % 15 == 0) {
		alpha *= (General::rand() % 1000) / 1000.0f;
	}
	alpha *= 0.5f;
	al_draw_tinted_bitmap(
		title_glow->bitmap,
		al_map_rgba_f(alpha, alpha, alpha, alpha),
		cfg.screen_w - al_get_bitmap_width(title_glow->bitmap),
		cfg.screen_h - al_get_bitmap_height(title_glow->bitmap),
		0
	);
	al_restore_state(&old_blend_state);

	tgui::draw();
}

class Drawer_Loop : public Loop {
public:
	bool init() { return true; }
	void top() {}
	bool handle_event(ALLEGRO_EVENT *event) { return false; }
	bool logic() { return false; }
	void draw() {
		al_clear_to_color(al_color_name("black"));

		al_draw_bitmap(
			title->bitmap,
			cfg.screen_w / 2 - al_get_bitmap_width(title->bitmap) / 2,
			cfg.screen_h / 2 - al_get_bitmap_height(title->bitmap) / 2,
			0
		);
	}
	Drawer_Loop() {}
	virtual ~Drawer_Loop() {}
};

static void fade(double time, bool out)
{
	double fade_start = al_get_time();

	while (true) {
		ALLEGRO_BITMAP *old_target = engine->set_draw_target(false);

		al_clear_to_color(al_color_name("black"));

		draw_main_menu();

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
}

Main::Main()
{
}

Main::~Main()
{
}

bool Main::init(void)
{
	engine = new Engine();
	if (!engine)
		return false;
	
	return engine->init();
}

void Main::execute()
{
	bool lost_boss_battle = false;

	while (true) {
		// Initial setup
		engine->fade(0, 0, al_map_rgba(0, 0, 0, 0));

#if defined OUYA || defined FIRETV
		if (!engine->get_purchased()) {
			queryPurchased();
			int purchased = -1;
			do {
				purchased = isPurchased();
			} while (purchased == -1);
			if (purchased == 1) {
				engine->set_purchased(true);
			}
		}
#endif

		Game_Specific_Globals::init();
		engine->hold_milestones(false);
		engine->clear_milestones();

		if (engine->game_is_over() && !lost_boss_battle && engine->get_continued_or_saved()) {
			double t = Game_Specific_Globals::elapsed_time;
			engine->load_game(engine->get_save_number_last_used());
			Game_Specific_Globals::elapsed_time = t;
			engine->set_game_just_loaded(false); // Trick Area_Manager into not calling load_level_state
		}
		else if (!lost_boss_battle) {
			// Title screen
			
			engine->set_game_over(false);

			bool add_continue;
			bool add_hqm;
			bool add_bonus;

			int button_inc = 0;

			W_Title_Screen_Button *buy_button = NULL;
			W_Title_Screen_Button *restore_button = NULL;

#if defined OUYA || defined FIRETV
			if (!engine->get_purchased()) {
				buy_button = new W_Title_Screen_Button(t("BUY_FULL_GAME"));
				buy_button->setX(20);
				buy_button->setY(132);
				add_continue = false;
				add_hqm = false;
				add_bonus = false;
			}
			else {
				add_continue = true;
				add_hqm = true;
				add_bonus = cfg.beat_game;
			}
#else
#ifdef DEMO
			add_continue = false;
			add_hqm = false;
			add_bonus = false;
			button_inc = 16;
#else
			add_continue = true;
			add_hqm = true;
			add_bonus = cfg.beat_game;
#endif
#endif
			
			button_inc += add_bonus ? -16 : 0;


			W_Button *config_button = new W_Button("misc_graphics/interface/gear_icon.cpi");
			config_button->setX(20);
			config_button->setY(20);
			
			W_Title_Screen_Button *new_game_button = new W_Title_Screen_Button(t("NEW_GAME"));
			new_game_button->setX(20);
			new_game_button->setY(100+button_inc+(add_continue ? 0 : 16));
			W_Title_Screen_Button *continue_button = new W_Title_Screen_Button(t("CONTINUE"));
			continue_button->setX(20);
			continue_button->setY(116+button_inc);

			W_Title_Screen_Button *bonus_game_button = NULL;
			W_Title_Screen_Button *hqm_button = new W_Title_Screen_Button(t("HQM"));

			if (add_bonus) {
				bonus_game_button = new W_Title_Screen_Button(t("BONUS_GAME"));
			}

			tgui::setNewWidgetParent(0);
#if !defined ALLEGRO_IPHONE
			tgui::addWidget(config_button);
#endif
			tgui::addWidget(new_game_button);
			if (add_continue) {
				tgui::addWidget(continue_button);
			}
			if (buy_button) {
				tgui::addWidget(buy_button);
			}
			if (restore_button) {
				tgui::addWidget(restore_button);
			}
			if (hqm_button && add_hqm) {
				hqm_button->setX(20);
				hqm_button->setY(132+button_inc);
				tgui::addWidget(hqm_button);
			}
			if (bonus_game_button) {
				bonus_game_button->setX(20);
				bonus_game_button->setY(132);
				tgui::addWidget(bonus_game_button);
			}
			tgui::setFocus(new_game_button);

			title = Wrap::load_bitmap("misc_graphics/title.cpi");
			title_glow = Wrap::load_bitmap("misc_graphics/title_glow.cpi");
			Music::play("music/title.mid");

			start = al_get_time();
			next = GET_NEXT;

			bool done = false;
			engine->set_continued_or_saved(false);

			ALLEGRO_EVENT_QUEUE *event_queue = engine->get_event_queue();

			while (true) {
				engine->set_touch_input_type(TOUCHINPUT_GUI);

				if (!Music::is_playing() && !engine->is_switched_out()) {
					Music::play(engine->get_music_name());
				}

				while (!al_event_queue_is_empty(event_queue)) {
					ALLEGRO_EVENT event;
					al_get_next_event(event_queue, &event);
#if !defined ALLEGRO_IPHONE && !defined ALLEGRO_ANDROID
					if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
						done = true;
						break;
					}
					else
#endif
#ifdef ALLEGRO_ANDROID
					if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_BACK) {
						done = true;
						break;
					}
#endif
					if (event.type == ALLEGRO_EVENT_JOYSTICK_CONFIGURATION) {
						al_reconfigure_joysticks();
						engine->choose_joystick();
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
#if !defined ALLEGRO_ANDROID
					else if (event.type == ALLEGRO_EVENT_KEY_DOWN && event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
						done = true;
						break;
					}
#endif
					else if (!engine->is_switched_out() && engine->get_send_tgui_events()) {
						tgui::handleEvent(&event);
					}
				}

				if (done) {
					break;
				}

				tgui::TGUIWidget *widget = tgui::update();

				if (widget == config_button) {
					Settings_Loop *l = new Settings_Loop();
					fade(1.0, true);
					tgui::hide();
					tgui::push(); // popped in ~Settings_Loop()
					std::vector<Loop *> loops;
					l->init();
					loops.push_back(l);
					engine->start_timers();
					engine->fade_in(loops);
					engine->do_blocking_mini_loop(loops, NULL);
					engine->stop_timers();
					fade(1.0, false);
				}
				else if (widget == new_game_button) {
					fade(1.0, true);
					Game_Specific_Globals::elapsed_time = 0;
					engine->reset_game();
					engine->set_game_just_loaded(false);
					engine->set_continued_or_saved(false);
					break;
				}
				else if (widget == continue_button) {
					fade(1.0, true);
					tgui::hide();
					tgui::push(); // popped in ~SaveLoad_Loop()
					std::vector<Loop *> loops;
					SaveLoad_Loop *l = new SaveLoad_Loop(false);
					l->init();
					loops.push_back(l);
					engine->start_timers();
					engine->fade_in(loops);
					engine->do_blocking_mini_loop(loops, NULL);
					engine->stop_timers();
					if (engine->get_continued_or_saved()) {
						break;
					}
					else {
						fade(1.0, false);
					}
				}
#if defined OUYA || defined FIRETV
				else if (widget && widget == buy_button) {
					if (try_purchase(event_queue)) {
						engine->set_purchased(true);
						tgui::setFocus(new_game_button);
						buy_button->remove();
						delete buy_button;
						buy_button = NULL;
						add_continue = true;
						tgui::addWidget(continue_button);
						new_game_button->setY(new_game_button->getY()-16);
						add_hqm = true;
						hqm_button->setX(20);
						hqm_button->setY(132);
						tgui::addWidget(hqm_button);
					}
					engine->switch_in();
					Music::play("music/title.ogg");
				}
#endif
				else if (widget && widget == hqm_button) {
					fade(1.0, true);
					tgui::hide();
					tgui::push(); // popped in ~HQM_Loop()
					std::vector<Loop *> loops;
					HQM_Loop *l = new HQM_Loop();
					l->init();
					loops.push_back(l);
					engine->start_timers();
					engine->fade_in(loops);
					engine->do_blocking_mini_loop(loops, NULL);
					engine->stop_timers();
					fade(1.0, false);
				}
				else if (widget && widget == bonus_game_button) {
					Music::play("music/runner.mid");
					fade(1.0, true);
					tgui::hide();
					tgui::push(); // popped in ~Runner_Loop()
					std::vector<Loop *> loops;
					Player *bisou = new Player("bisou");
					bisou->load();
					std::vector<Player *> v;
					v.push_back(bisou);
					Wrap::Bitmap *ss;
					std::string tmp;
					Runner_Loop *l = new Runner_Loop(v, &ss, &tmp);
					loops.push_back(l);
					engine->set_mini_loops(loops);
					l->init();
					l->top();
					engine->start_timers();
					engine->fade_in(loops);
					engine->do_blocking_mini_loop(loops, NULL);
					engine->stop_timers();
					Music::play("music/title.mid");
					fade(1.0, false);
					Wrap::destroy_bitmap(ss);
					delete bisou;
				}
				bool lost;
#ifdef ALLEGRO_WINDOWS
				engine->handle_display_lost();
				lost = Direct3D::lost;
#else
				lost = false;
#endif

				if (!lost) {
					ALLEGRO_BITMAP *old_target = engine->set_draw_target(false);

					al_clear_to_color(al_color_name("black"));

					draw_main_menu();
					engine->finish_draw(false, old_target);
					al_flip_display();
				}
				al_rest(1.0/60.0);
			}

			Wrap::destroy_bitmap(title);
			Wrap::destroy_bitmap(title_glow);
			config_button->remove();
			delete config_button;
			new_game_button->remove();
			delete new_game_button;
			continue_button->remove();
			delete continue_button;

			if (buy_button) {
				buy_button->remove();
				delete buy_button;
			}
			if (restore_button) {
				restore_button->remove();
				delete restore_button;
			}
			if (add_hqm) {
				hqm_button->remove();
			}
			delete hqm_button;
			if (bonus_game_button) {
				bonus_game_button->remove();
				delete bonus_game_button;
			}
			if (done) {
				break;
			}
		}
		else {
			engine->set_game_over(false);
			lost_boss_battle = false;
			engine->load_game(-1);
		}
		Loop *l;

		if (engine->get_game_just_loaded() && Lua::get_saved_area_name().substr(0, 4) == "map:") {
			l = new Map_Loop(Lua::get_saved_area_name().substr(4));
		}
		else {
			l = new Area_Loop();
		}

		std::vector<Loop *> loops;
		loops.push_back(l);
		engine->set_loops_only(loops);
		l->init();

		ALLEGRO_DEBUG("Created start loop");

		engine->do_event_loop();
		
		engine->destroy_loops();

		lost_boss_battle = engine->get_lost_boss_battle();
		engine->set_lost_boss_battle(false);
		if (lost_boss_battle) {
#if defined OUYA || defined FIRETV
			if (!engine->get_purchased()) {
				std::string music = Music::get_playing();
				std::vector<std::string> texts;
				texts.push_back(t("RETRY_BOSS_PURCHASE1"));
				texts.push_back(t("RETRY_BOSS_PURCHASE2"));
				bool buy = engine->yes_no_prompt(texts);
				if (buy) {
					if (try_purchase(engine->get_event_queue())) {
						engine->set_purchased(true);
					}
					engine->switch_in();
					Music::play(music);
				}
				if (buy && engine->get_purchased()) {
					lost_boss_battle = true;
				}
				else {
					lost_boss_battle = false;
				}
			}
			else
#elif defined DEMO
			if (true) {
				std::vector<std::string> texts;
				texts.push_back(t("NO_RETRY_BOSS_IN_DEMO"));
				engine->notify(texts);
				lost_boss_battle = false;
			}
			else
#endif
			{
				std::vector<std::string> texts;
				texts.push_back(t("RETRY_BOSS"));
				lost_boss_battle = engine->yes_no_prompt(texts);
			}
		}
	}
}


void Main::shutdown()
{
	engine->shutdown();
	delete engine;
}

