#include <allegro5/allegro_color.h>
#include <allegro5/allegro_opengl.h>
#include <cfloat>

#include "runner_loop.h"
#include "collision_detection.h"
#include "engine.h"
#include "battle_pathfind.h"

const int BEE_LAYER = 1;
const int PLAYER_LAYER = 4;
const int LAST_LAYER = 5;
const int NUM_PLATFORMS = 40;
const float MIN_PLATFORM_DISTANCE = 50;
const float MAX_PLATFORM_DISTANCE = 100;
const float MOVE_SPEED = 0.05f;
const float ROCK_SLOWDOWN = 0.005f;
const float BIRD_SPEED = 3.0f;

static bool sliding;

static int coin_offset(void)
{
	float f = fmod(al_get_time(), 1.0);
	if (f < 0.5)
		return -f * 10;
	else
		return -(0.5 - (f-0.5)) * 10;
}

void Runner_Loop::gencloud(Cloud *c, bool genx, bool star) {
	c->star = star;
	if (star) {
		uint32_t r = General::rand() % 4;
		if (r == 0) c->bitmap = star1;
		else if (r == 1) c->bitmap = star2;
		else if (r == 2) c->bitmap = star3;
		else c->bitmap = star4;
	}
	else {
		uint32_t r = General::rand() % 3;
		if (r == 0) c->bitmap = cloud1;
		else if (r == 1) c->bitmap = cloud2;
		else c->bitmap = cloud3;
	}
	if (genx) {
		c->x = (General::rand() % (cfg.screen_w+al_get_bitmap_width(c->bitmap->bitmap))) - al_get_bitmap_width(c->bitmap->bitmap);
		c->vx = (General::rand(0, 9)+9) / 10.0;
		c->vx = -c->vx;
	}
	else {
		c->x = cfg.screen_w;
	}
	c->y = General::rand() % (cfg.screen_h-al_get_bitmap_height(c->bitmap->bitmap));
}

void Runner_Loop::get_player_hitbox(int *x1, int *y1, int *x2, int *y2)
{
	General::Point<float> player_pos = player->get_position();
	*x1 = player_pos.x - 10;
	*x2 = player_pos.x + 10;
	if (sliding) {
		*y1 = player_pos.y - 12;
	}
	else {
		*y1 = player_pos.y - 24;
	}
	*y2 = player_pos.y;
}

bool Runner_Loop::init(void)
{
	if (inited) {
		return true;
	}

	if (!Battle_Loop::init())
		return false;
	
	player = get_active_player();
	start_hp = player->get_attributes().hp;

	start2();
	
	return true;
}

void Runner_Loop::top(void)
{
	Battle_Loop::top();
}

bool Runner_Loop::handle_event(ALLEGRO_EVENT *event)
{
	if (event->type == ALLEGRO_EVENT_KEY_DOWN && (event->keyboard.keycode == ALLEGRO_KEY_LEFT || event->keyboard.keycode == ALLEGRO_KEY_RIGHT)) {
		return true;
	}
	else if (event->type == ALLEGRO_EVENT_JOYSTICK_AXIS && event->joystick.axis == 0) {
		return true;
	}
	return Battle_Loop::handle_event(event);
}

bool Runner_Loop::logic(void)
{
	if (!won && dead_time > al_get_time()) {
		player->get_input()[Battle_Entity::X] = 1;
	}

	if (Battle_Loop::logic())
		return true;

	double now = al_get_time();
	std::pair<double, int> &pair = time_distance[time_distance.size()-1];
	if (now > pair.first+0.1) {
		time_distance.push_back(std::pair<double, int>(now, player->get_position().x));
	}
	if (time_distance[0].first < now-3) {
		time_distance.erase(time_distance.begin());
	}

	if (!won && (time_distance.size() >= 29 && player->get_position().x-time_distance[0].second < 16) && dead_time > al_get_time()) {
		engine->play_sample("sfx/boingboingboing.ogg");
		player->get_attributes().hp = 0;
		player->get_animation_set()->set_sub_animation("surprised");
		player->get_animation_set()->reset();
		bees_on_top = true;
		engine->stop_sample("sfx/slide.ogg");
		dead_time = al_get_time();
	}
	
	if (!won && player->get_position().x >= size.w - cfg.screen_w/2) {
		won = true;
		win_time = al_get_time();
		player->get_input().clear();
		player->set_velocity(General::Point<float>(0, 0));
		player->set_accel(General::Point<float>(0, 0));
	}
	
	if (battle_transition_done) {
		if (player->get_position().x > cfg.screen_w / 2) {
			start_hitting_rocks = true;
		}
	}

	if (dead_time < al_get_time()-3) {
		engine->stop_sample("sfx/slide.ogg");
		std::vector<std::string> v;
		v.push_back(t("TRY_AGAIN"));
		std::vector<Loop *> mini_loops = engine->get_mini_loops();
		if (engine->prompt(v, t("YES"), t("NO"), &mini_loops) == 0) {
			start_hitting_rocks = false;
			stop();
			delete[] pathfinding_nodes;
			delete[] pathfinding_edges;
			start1();
			start2();
			return false;
		}
		else {
			std::vector<Loop *> loops;
			loops.push_back(this);
			engine->fade_out(loops);
			engine->unblock_mini_loop();
			return true;
		}
	}
	else if (won) {
		if (win_time < al_get_time() - 10) {
			std::vector<Loop *> loops;
			loops.push_back(this);
			engine->fade_out(loops);
			engine->unblock_mini_loop();
			return true;
		}
	}

	bool sliding_now = player->get_input()[Battle_Entity::Y] > 0  && player->is_on_ground();
	if (sliding && !sliding_now) {
		engine->stop_sample("sfx/slide.ogg");
	}
	else if (!sliding && sliding_now) {
		engine->play_sample("sfx/slide.ogg");
	}
	sliding = sliding_now;

	if (dead_time <= al_get_time()) {
		float alpha = 1.0 - (al_get_time() - dead_time);
		if (alpha < 0) alpha = 0;
		player->set_tint(al_map_rgba_f(alpha, alpha, alpha, alpha));
	}

	// move clouds
	for (size_t i = 0; i < clouds.size(); i++) {
		clouds[i]->x += clouds[i]->vx * player->get_velocity().x;
		if (clouds[i]->x <= -al_get_bitmap_width(clouds[i]->bitmap->bitmap)) {
			gencloud(clouds[i], false, clouds[i]->star);
		}
	}
	
	coin->update(General::LOGIC_MILLIS);
	bee_anim->update(General::LOGIC_MILLIS);
	shield_barrier->update(General::LOGIC_MILLIS);
	poof_anim->update(General::LOGIC_MILLIS);

	if (start_hitting_rocks && !won && dead_time > al_get_time()) {
		if (fabs(player->get_position().x-last_player_pos.x) < 1 || hitting_big_tree) {
			if (x_offset_ratio > 0.1f) {
				x_offset_ratio -= ROCK_SLOWDOWN;
				if (x_offset_ratio < 0.1f)
					x_offset_ratio = 0.1f;
			}
		}
		else {
			if (x_offset_ratio < 1.0f) {
				x_offset_ratio += ROCK_SLOWDOWN*2;
				if (x_offset_ratio > 1.0f)
					x_offset_ratio = 1.0f;
			}
		}
	}

	hitting_big_tree = false;

	// check collisions with big trees
	for (size_t i = 0; i < slide_trees.size(); i++) {
		General::Point<int> &p = slide_trees[i];
		float px = player->get_position().x;
		if (px >= p.x+30 && px <= p.x+50 && !sliding) {
			player->set_position(
				General::Point<float>(
					last_player_pos.x-1,
					player->get_position().y+0.5
				)
			);
			hitting_big_tree = true;
		}
	}

	last_player_pos = player->get_position();

	while (waiting_birds.size() > 0) {
		Bird &b = waiting_birds[0];
		if (b.x > player->get_position().x+cfg.screen_w*2.0f) {
			break;
		}
		engine->play_sample("sfx/bird_tweet.ogg");
		active_birds.push_back(b);
		waiting_birds.erase(waiting_birds.begin());
	}

	// check collisions with birds
	int x1, y1, x2, y2;
	get_player_hitbox(&x1, &y1, &x2, &y2);

	for (unsigned int i = 0; i < active_birds.size();) {
		Bird &b = active_birds[i];
		if (b.x < player->get_position().x - cfg.screen_w*1.5f) {
			ERASE_FROM_UNSORTED_VECTOR(active_birds, i);
		}
		else {
			b.x -= BIRD_SPEED;
			if (invincible_end <= al_get_time() && player->get_attributes().hp > 0 && checkcoll_box_box(General::Point<float>(x1, y1), General::Point<float>(x2, y2), General::Point<float>(b.x, b.y), General::Point<float>(b.x+bird_w, b.y+bird_h))) {
				// hit a bird, dead unless shields
				if (shields > 0) {
					engine->play_sample("sfx/hit_bird.ogg");
					shields--;
					invincible_end = al_get_time() + 2;
				}
				else {
					engine->play_sample("sfx/hit_bird.ogg");
					player->get_attributes().hp = 0;
					player->get_animation_set()->set_sub_animation("hit");
					Steering::set_bee_chase(false);
					bees_on_top = true;
					engine->stop_sample("sfx/slide.ogg");
					dead_time = al_get_time();
				}
			}
			i++;
		}
	}

	// check collisions with shields
	int shield_w = al_get_bitmap_width(shield_bmp->bitmap);
	int shield_h = al_get_bitmap_height(shield_bmp->bitmap);
	for (size_t i = 0; i < shield_positions.size(); i++) {
		General::Point<int> &p = shield_positions[i];
		if (checkcoll_box_box(General::Point<float>(x1, y1), General::Point<float>(x2, y2), General::Point<float>(p.x, p.y-shield_h), General::Point<float>(p.x+shield_w, p.y))) {
			if (shields < 3) {
				engine->play_sample("sfx/item_found.ogg");
				shields++;
			}
			shield_positions.erase(shield_positions.begin()+i);
			break;
		}
	}
	
	// check collisions with coins
	for (size_t i = 0; i < coins.size();) {
		General::Point<int> p = General::Point<int>(coins[i].x, coins[i].y+coin_offset());
		if (checkcoll_box_box(General::Point<float>(x1, y1), General::Point<float>(x2, y2), p, General::Point<float>(p.x+coin_w, p.y+coin_h))) {
			engine->play_sample("sfx/coin.ogg");
			collected_coins++;
			ERASE_FROM_UNSORTED_VECTOR(coins, i);
		}
		else
			i++;
		
	}


	// check for fall in pit
	if (!fell_in_pit) {
		for (size_t i = 0; i < tiles[2].size(); i++) {
			if (tiles[2][i].x > player->get_position().x) {
				if (tiles[2][i].y+400 < player->get_position().y) {
					engine->play_sample("sfx/poof.ogg");
					fell_in_pit = true;
					engine->stop_sample("sfx/slide.ogg");
					dead_time = al_get_time();
					player->get_input().clear();
					player->set_velocity(General::Point<float>(0, 0));
					player->set_accel(General::Point<float>(0, 0));
				}
				break;
			}
		}
	}

	// check for hit trunk
	if (!won && !hit_trunk && !player->is_jumping() && start_hitting_rocks) {
		if (last_x == player->get_position().x) {
			engine->play_sample("sfx/thud.ogg");
			hit_trunk = true;
		}
	}
	else {
		if (last_x != player->get_position().x) {
			hit_trunk = false;
		}
	}
	last_x = player->get_position().x;

	sort_bees(bees);
	for (int i = 0; i < NUM_BEES; i++) {
		bees[i].x -= 10;
		update_bee(i, bees);
		bees[i].x += 10;
	}
	bee_sine_wave += 0.015;
	bee_cosine_wave += 0.02;

	return false;
}

/* draw the bees sorted with the layers like the player */
void Runner_Loop::extra_drawing_hook(int layer)
{
	bool held = al_is_bitmap_drawing_held();
	if (held) al_hold_bitmap_drawing(false);

	General::Point<float> pp = player->get_position();
	General::Point<float> top = get_top();

	if (layer == PLAYER_LAYER) {
		// draw shields
		al_hold_bitmap_drawing(true);
		for (size_t i = 0; i < shield_positions.size(); i++) {
			if (shield_positions[i].x < pp.x-cfg.screen_w/2-100 || shield_positions[i].x > pp.x+cfg.screen_w/2)
				continue;
			al_draw_bitmap(shield_bmp->bitmap, shield_positions[i].x-top.x, shield_positions[i].y-top.y-al_get_bitmap_height(shield_bmp->bitmap), 0);
		}
		al_hold_bitmap_drawing(false);

		// draw coins
		for (size_t i = 0; i < coins.size(); i++) {
			if (coins[i].x < pp.x-cfg.screen_w/2-coin_w || coins[i].x > pp.x+cfg.screen_w/2)
				continue;
			Animation *a = coin->get_current_animation();
			Wrap::Bitmap *bmp = a->get_current_frame()->get_bitmap()->get_bitmap();
			al_draw_bitmap(bmp->bitmap, coins[i].x-top.x, coins[i].y-top.y+coin_offset(), 0);
		}

		// draw shield on player
		if (shields > 0 && dead_time > al_get_time()) {
			General::Point<float> p = player->get_position();
			int x = p.x - top.x;
			int y = p.y - top.y;
			int x1 = x - 32;
			int y1 = y - 48;
			al_draw_tinted_bitmap(shield_barrier->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap()->bitmap,
				al_map_rgba_f(0.64f, 0.64f, 0.64f, 0.64f),
				x1, y1, 0);
		}

		// draw birds
		al_hold_bitmap_drawing(true);
		for (unsigned int i = 0; i < active_birds.size(); i++) {
			Bird &b = active_birds[i];
			if (b.x < pp.x-cfg.screen_w/2-150 || b.x > pp.x+cfg.screen_w/2)
				continue;
			al_draw_bitmap(bird_bmp->bitmap, b.x-top.x, b.y-top.y, 0);
		}
		al_hold_bitmap_drawing(false);
	}

	// draw bees
	if (layer == (bees_on_top ? LAST_LAYER : BEE_LAYER)) {
		// draw bees
		double elapsed_since_dead = al_get_time() - dead_time;
		if (elapsed_since_dead > 1) elapsed_since_dead = 1;
		float add_y = -10 + 10 * sin(bee_sine_wave);
		float add_x = -32 + 10 * cos(bee_cosine_wave);
		if (won) {
			int add_plusplus = (al_get_time() - win_time) * 100;
			if (add_plusplus > 300) {
				engine->stop_sample("sfx/bees.ogg");
			}
			add_x -= add_plusplus;
		}
		int bee_w = bee_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_width();
		int bee_h = bee_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_height();
		ALLEGRO_VERTEX verts[6*NUM_BEES];
		for (int i = 0; i < NUM_BEES; i++) {
			int c = (bees[i].z+100) / 200.0 * 128 + 127;
			if (c > 255) {
				c = 255;
			}
			else if (c < 0) {
				c = 0;
			}
			verts[6*i+0].x = add_x + bees[i].x;
			verts[6*i+0].y = add_y + bees[i].y;
			verts[6*i+0].z = 0;
			verts[6*i+0].u = 0;
			verts[6*i+0].v = 0;
			verts[6*i+0].color = al_map_rgb(
				238, c, 238
			);
			verts[6*i+1].x = add_x + bees[i].x+bee_w;
			verts[6*i+1].y = add_y + bees[i].y;
			verts[6*i+1].z = 0;
			verts[6*i+1].u = bee_w;
			verts[6*i+1].v = 0;
			verts[6*i+1].color = al_map_rgb(
				238, c, 238
			);
			verts[6*i+2].x = add_x + bees[i].x;
			verts[6*i+2].y = add_y + bees[i].y+bee_h;
			verts[6*i+2].z = 0;
			verts[6*i+2].u = 0;
			verts[6*i+2].v = bee_h;
			verts[6*i+2].color = al_map_rgb(
				238, c, 238
			);
			verts[6*i+3].x = add_x + bees[i].x+bee_w;
			verts[6*i+3].y = add_y + bees[i].y;
			verts[6*i+3].z = 0;
			verts[6*i+3].u = bee_w;
			verts[6*i+3].v = 0;
			verts[6*i+3].color = al_map_rgb(
				238, c, 238
			);
			verts[6*i+4].x = add_x + bees[i].x;
			verts[6*i+4].y = add_y + bees[i].y+bee_h;
			verts[6*i+4].z = 0;
			verts[6*i+4].u = 0;
			verts[6*i+4].v = bee_h;
			verts[6*i+4].color = al_map_rgb(
				238, c, 238
			);
			verts[6*i+5].x = add_x + bees[i].x+bee_w;
			verts[6*i+5].y = add_y + bees[i].y+bee_h;
			verts[6*i+5].z = 0;
			verts[6*i+5].u = bee_w;
			verts[6*i+5].v = bee_h;
			verts[6*i+5].color = al_map_rgb(
				238, c, 238
			);
		}
		al_draw_prim(verts, 0, bee_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap()->bitmap, 0, 6*NUM_BEES, ALLEGRO_PRIM_TRIANGLE_LIST);
	}
	
	if (held) al_hold_bitmap_drawing(true);
}

void Runner_Loop::draw(void)
{
	if (won) {
		player->get_animation_set()->set_sub_animation("idle-down");
	}

	al_draw_bitmap(gradient->bitmap, 0, 0, 0);

	// draw clouds
	for (size_t i = 0; i < clouds.size(); i++) {
		al_draw_bitmap(clouds[i]->bitmap->bitmap, clouds[i]->x, clouds[i]->y, 0);
	}

	Battle_Loop::draw();

	// draw shield icons (how many shields you have)
	for (int i = 0; i < shields; i++) {
		al_draw_bitmap(shield_bmp->bitmap, 10+i*al_get_bitmap_width(shield_bmp->bitmap)+i*5, 10, 0);
	}

	std::string coinsS = General::itos(collected_coins);
	int w = General::get_text_width(General::FONT_HEAVY, coinsS);
	int fh = General::get_font_line_height(General::FONT_HEAVY);
	int ty = 10 + MAX(fh/2, coin_h/2) - fh/2;
	int cy = 10 + MAX(fh/2, coin_h/2) - coin_h/2;
	General::draw_text(coinsS, cfg.screen_w-10-coin_w-5-w, ty, 0, General::FONT_HEAVY);
	coin->get_current_animation()->draw(cfg.screen_w-10-coin_w, cy, 0);

	// draw poof
	if (dead_time <= al_get_time()) {
		float alpha = 1.0 - (al_get_time() - dead_time);
		if (alpha < 0) alpha = 0;
		int ox, oy;
		get_area_offset(&ox, &oy);
		int w = 32 + alpha*32;
		int h = 100 + alpha*32*100.0f/32.0f;
		int x = player->get_position().x-w/2;
		int y = player->get_position().y-h;
		Wrap::Bitmap *b = poof_anim->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap();
		al_draw_tinted_scaled_bitmap(
			b->bitmap,
			al_map_rgba_f(alpha, alpha, alpha, alpha),
			0, 0,
			al_get_bitmap_width(b->bitmap),
			al_get_bitmap_height(b->bitmap),
			ox+x,
			oy+y,
			w, h,
			0
		);
	}
}

void Runner_Loop::start1()
{
	start_hitting_rocks = false;
	sliding = false;
	fell_in_pit = false;
	hit_trunk = false;
	dead_time = FLT_MAX;
	shields = 0;
	collected_coins = 0;
	hitting_big_tree = false;
	x_offset_ratio = 1.0f;

	hit_rock_count = 0;

	int seed = 0;
	General::srand(seed);

	invincible_end = al_get_time();

	bees_on_top = false;

	for (int i = 0; i < 5; i++) {
		Cloud *c = new Cloud;
		gencloud(c, true, false);
		clouds.push_back(c);
	}

	for (int i = 0; i < 5; i++) {
		Cloud *c = new Cloud;
		gencloud(c, true, true);
		clouds.push_back(c);
	}
	
	platforms = new Platform[NUM_PLATFORMS];
	int platx = 0;
	int platy = 800;
	int platlen = 20;
	camera_y = platy - (cfg.screen_h - left_bmp_width);
	int end_x = 0;
	std::vector< std::vector< General::Point<int> > > geo;

	// Add tiles and generate level
	tiles.push_back(std::vector<Tile>());
	tiles.push_back(std::vector<Tile>());
	tiles.push_back(std::vector<Tile>());
	tiles.push_back(std::vector<Tile>());
	tiles.push_back(std::vector<Tile>());
	tiles.push_back(std::vector<Tile>());
	set_entity_layer(PLAYER_LAYER);

	int rock_idx = 0;
	int next_obstacle = 1;

	int since_bird = 0;

	for (int i = 0; i < NUM_PLATFORMS; i++) {
		next_obstacle--;

		int pixlen = left_bmp_width + right_bmp_width + mid_bmp_width*platlen;

		// add shields every 5 platforms (not on 0)
		if (i && i % 5 == 0) {
			int x = General::rand(0, pixlen-50);
			shield_positions.push_back(General::Point<int>(platx+x, platy));
		}
		
		bool has_bird = i != 0 && i < NUM_PLATFORMS-3 && (General::rand() % 2 == 0 || since_bird >= 3);
		bool added_coins = false;

		// add tiles
		for (int j = 0; j < platlen+2; j++) {
			Tile tile;
			int second = 0;
			tile.x = platx + left_bmp_width*j;
			if (j == 0) {
				tile.id = 0;
				second = 4;
			}
			else if (j == platlen+1) {
				tile.id = 1;
				second = 6;
			}
			else {
				tile.id = 7 + (General::rand() % num_middle_bmps);
				second = 5;
			}
			tile.y = platy;
			tiles[2].push_back(tile);
			tile.id = second;
			tile.y += left_bmp_height;
			tiles[3].push_back(tile);
			// add coins to birdless platforms
			int num = General::rand() % 5 + 3;
			if (!has_bird && next_obstacle < -5 && j < platlen-num && General::rand() % 8 == 0 && !added_coins) {
				int xx = platx + left_bmp_width*j;
				int yy;
				if (General::rand() % 2 == 0) {
					yy = platy - coin_h - 32;
				}
				else {
					yy = platy - coin_h;
				}
				for (int k = 0; k < num; k++) {
					Coin c;
					c.x = xx;
					c.y = yy;
					coins.push_back(c);
					xx += coin_w+15;
				}
				added_coins = true;
			}
			// rocks, stumps, big trees (slide)
			if (j > 0 && j < platlen && (General::rand() % 30) == 0 && next_obstacle < 0) {
				if (General::rand() % 2 == 0 && j < platlen-6) {
					tile.id = tree_slide_i;
					tile.y = platy - tree_slide_h + 4;
					tiles[PLAYER_LAYER-1].push_back(tile);
					tile.id++;
					tiles[PLAYER_LAYER+1].push_back(tile);
					slide_trees.push_back(
						General::Point<int>(
							tile.x,
							tile.y
						)
					);
					next_obstacle = 3;
				}
				else {
					if (General::rand() % 2) {
						tile.id = 2;
					}
					else {
						tile.id = 3;
					}
					tile.y = platy - left_bmp_height + 2;
					tiles[4].push_back(tile);
					next_obstacle = 1;
				}
			}
		}
		
		platforms[i].x = platx;
		platforms[i].y = platy;
		platforms[i].length = platlen;
		end_x = platx + pixlen;
		std::vector< General::Point<int> > line;
		
		int sz = tiles[2].size();
		int idx = sz-(platlen+2);
		int line_y = platy + 3;
		for (int j = 0; j < platlen+2; j++) {
			if (tiles[4].size() > (unsigned)rock_idx && tiles[4][rock_idx].x == tiles[2][idx+j].x) {
				if (tiles[4][rock_idx].id == 3) { // stump
					line.push_back(General::Point<int>(tiles[4][rock_idx].x, line_y));
					line.push_back(General::Point<int>(tiles[4][rock_idx].x, line_y-32));
					line.push_back(General::Point<int>(tiles[4][rock_idx].x+left_bmp_width, line_y-32));
				}
				else if (tiles[4][rock_idx].id == 2) { // rock
					line.push_back(General::Point<int>(tiles[4][rock_idx].x, line_y));

					line.push_back(General::Point<int>(tiles[4][rock_idx].x, line_y-32));
					line.push_back(General::Point<int>(tiles[4][rock_idx].x+left_bmp_width, line_y-32));
				}
				rock_idx++;
			}
			else {
				line.push_back(General::Point<int>(tiles[2][idx+j].x, line_y));
			}
		}
		line.push_back(General::Point<int>(platx+pixlen, line_y));
		geo.push_back(line);
		platform_solid.push_back(true);
		// if remaining to be added, generate another
		if (i >= NUM_PLATFORMS-1)
			break;
		// set platlen for next platform and add birds randomly
		int extra = (General::rand() % 6 ? 15 : 0);
		// birds on big platforms
		if (has_bird) {
		//if (General::rand() % 2 == 0 || since_bird >= 3) {
			since_bird = 0;
			// 0 - one flying head height
			// 1 - three flying head height and up
			// 2 - one flying ground level
			int pattern = General::rand() % 3;
			Bird b;
			if (pattern == 0) {
				b.x = platx + pixlen - 64;
				b.y = platy - bird_h - 16;
				waiting_birds.push_back(b);
			}
			else if (pattern == 1) {
				for (int j = 0; j < 3; j++) {
					b.x = platx + pixlen - 64 + (j*16);
					b.y = platy - bird_h - 16 - (16*j);
					waiting_birds.push_back(b);
				}
			}
			else {
				b.x = platx + pixlen - 64;
				b.y = platy - bird_h;
				waiting_birds.push_back(b);
			}
			// add coins
			int startx = b.x - 192;
			Coin c;
			c.x = startx;
			c.y = platy-coin_h;
			coins.push_back(c);

			// on ground
			if (pattern == 0 || pattern == 1) {
				for (int i2 = 0; i2 < 5; i2++) {
					c.x = startx+((i2+1)*(coin_w+15));
					c.y = platy - coin_h;
					coins.push_back(c);
				}
			}
			// in arc in the air
			if (pattern == 1 || pattern == 2) {
				float angle = M_PI + M_PI/12;
				float centerx = 72; // FIXME values
				float centery = 0;
				float dist = centerx;
				for (int i2 = 0; i2 < 5; i2++) {
					int x = cos(angle) * dist + centerx;
					int y = sin(angle) * dist + centery;
					c.x = startx+x;
					c.y = platy+y-coin_h;
					coins.push_back(c);
					angle += M_PI/12;
				}
			}
		}
		else {
			since_bird++;
		}

		// add some decorative trees
		int ntrees = 10 + General::rand() % 10 + (pixlen / tree_fg_w);
		for (int i2 = 0; i2 < ntrees; i2++) {
			int x = General::rand() / (float)UINT_MAX * (pixlen-mountain_bg_w);
			Tile t;
			t.x = x+platx;

			if (General::rand() % 10 == 0) {
				// foreground tree
				if (i < NUM_PLATFORMS-2) {
					t.y = platy - tree_fg_h + 2;
					t.id = General::rand() % 2 + tree_fg_i;
					tiles[5].push_back(t);
				}
			}
			else {
				// background mountain
				if (General::rand() % 10 == 0) {
					t.y = platy - mountain_bg_h + 2;
					t.id = tree_bg_i+2;
					tiles[0].push_back(t);
				}
				// background tree
				else {
					t.y = platy - tree_bg_h + 2;
					t.id = General::rand() % 2 + tree_bg_i;
					tiles[1].push_back(t);
				}
			}
		}

		size.w = platx + pixlen;

		platlen = General::rand() % 20 + 8 + extra;

		// set platx platy for next platform
		// 0 - no y change, 1 - x and y change
		int type = General::rand() % 3;
		float dist = General::rand() % (int)(MAX_PLATFORM_DISTANCE-MIN_PLATFORM_DISTANCE) + MIN_PLATFORM_DISTANCE;

		if (type == 0) {
			platx = end_x + dist;
			// platy stays the same
		}
		else {
			float avg_dist = sqrt((dist*dist)/2); // if both sides are equal
			int half = avg_dist / 2;
			int r = General::rand() % half;
			platx = end_x + r + half;
			int direction;
			if (platy > 1300)
				direction = -1;
			else if (platy < 300)
				direction = 1;
			else
				direction = (General::rand() % 2) ? -1 : 1;
			platy += (dist - (r + half)) * direction;
		}
	}

	size.h = 1500;

	set_geometry(geo);
	
	time_distance.push_back(std::pair<double, int>(al_get_time(), cfg.screen_w/2));
}

void Runner_Loop::start2()
{
	player->get_attributes().hp = start_hp;
	player->set_tint(al_map_rgba_f(1, 1, 1, 1));
	player->set_show_shadow(false);
	player->get_input().clear();

	float xv, xa;
	player->get_physics(
		&xa,
		NULL,
		NULL,
		&xv,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	);

	player->set_velocity(General::Point<float>(xv, 0));
	player->set_accel(General::Point<float>(xa, 0));

	player->set_physics(
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		0,
		0,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1
	);

	player->set_speed_multiplier(1.5f);
	player->set_position(General::Point<float>(cfg.screen_w/2, General::BIG_FLOAT));
	player->move_up(get_geometry());
	last_x = player->get_position().x-1;

	last_player_pos = player->get_position();

	Steering::set_boids(NUM_BEES, bees);
	bee_y_default = cfg.screen_h/2;
	Steering::set_bee_target(General::Point<float>(5, bee_y_default));
	bee_sine_wave = 0.0;
	bee_cosine_wave = 0.0;
	for (int i = 0; i < NUM_BEES; i++) {
		Steering::fill_bee(&bees[i]);
	}
	Steering::set_bee_chase(true);

	engine->reset_logic_count();
}

void Runner_Loop::stop()
{
	time_distance.clear();

	delete[] platforms;

	waiting_birds.clear();
	active_birds.clear();
	shield_positions.clear();
	slide_trees.clear();
	coins.clear();
	tiles.clear();
	platform_solid.clear();
	for (size_t i = 0; i < clouds.size(); i++) {
		delete clouds[i];
	}
	clouds.clear();
}

Runner_Loop::Runner_Loop(std::vector<Player *> bisou, Wrap::Bitmap **end_screenshot, std::string *tmp) :
	Battle_Loop(bisou, NULL, false, NULL, end_screenshot, tmp),
	won(false)
{
	draw_interface = false;

	// Make Battle_Loop not clear screen
	bird_bmp = Wrap::load_bitmap(
		"mini_games/runner/bird.cpi"
	);
	bird_w = al_get_bitmap_width(bird_bmp->bitmap);
	bird_h = al_get_bitmap_height(bird_bmp->bitmap);

	// load tiles into atlas
	// left right rock #middles
	std::vector<Wrap::Bitmap *> bmps;
	atlas = atlas_create(
		General::texture_size, General::texture_size, ATLAS_REPEAT_EDGES, 1, false
	);
	Wrap::Bitmap *bmp = Wrap::load_bitmap(
		"mini_games/runner/left.cpi"
	);
	atlas_add(atlas, bmp, 0);
	bmps.push_back(bmp);
	left_bmp_width = al_get_bitmap_width(bmp->bitmap);
	left_bmp_height = al_get_bitmap_height(bmp->bitmap);
	bmp = Wrap::load_bitmap(
		"mini_games/runner/right.cpi"
	);
	atlas_add(atlas, bmp, 1);
	bmps.push_back(bmp);
	right_bmp_width = al_get_bitmap_width(bmp->bitmap);
	bmp = Wrap::load_bitmap(
		"mini_games/runner/rock.cpi"
	);
	atlas_add(atlas, bmp, 2);
	bmps.push_back(bmp);
	bmp = Wrap::load_bitmap(
		"mini_games/runner/stump.cpi"
	);
	atlas_add(atlas, bmp, 3);
	bmps.push_back(bmp);
	bmp = Wrap::load_bitmap(
		"mini_games/runner/left-bottom.cpi"
	);
	atlas_add(atlas, bmp, 4);
	bmps.push_back(bmp);
	bmp = Wrap::load_bitmap(
		"mini_games/runner/mid0-bottom.cpi"
	);
	atlas_add(atlas, bmp, 5);
	bmps.push_back(bmp);
	bmp = Wrap::load_bitmap(
		"mini_games/runner/right-bottom.cpi"
	);
	atlas_add(atlas, bmp, 6);
	bmps.push_back(bmp);
	num_middle_bmps = 0;
	for (; num_middle_bmps < 16; num_middle_bmps++) {
		char name[1000];
		sprintf(name, "mini_games/runner/mid%d.cpi", num_middle_bmps);
		if (!engine->get_cpa()->exists(name))
			break;
	}

	for (int i = 0; i < num_middle_bmps; i++) {
		Wrap::Bitmap *b = Wrap::load_bitmap("mini_games/runner/mid" + General::itos(i) + ".cpi");
		atlas_add(atlas, b, i+7);
		bmps.push_back(b);
		mid_bmp_width = al_get_bitmap_width(b->bitmap);
	}

	tree_bg_i = num_middle_bmps+7;
	bmp = Wrap::load_bitmap(
		"mini_games/runner/tree_bg.cpi"
	);
	atlas_add(atlas, bmp, tree_bg_i);
	bmps.push_back(bmp);
	bmp = Wrap::load_bitmap(
		"mini_games/runner/tree_bg2.cpi"
	);
	atlas_add(atlas, bmp, tree_bg_i+1);
	bmps.push_back(bmp);
	tree_bg_w = al_get_bitmap_width(bmp->bitmap);
	tree_bg_h = al_get_bitmap_height(bmp->bitmap);
	bmp = Wrap::load_bitmap(
		"mini_games/runner/mountain1.cpi"
	);
	atlas_add(atlas, bmp, tree_bg_i+2);
	bmps.push_back(bmp);
	mountain_bg_w = al_get_bitmap_width(bmp->bitmap);
	mountain_bg_h = al_get_bitmap_height(bmp->bitmap);

	tree_fg_i = tree_bg_i + 3;
	bmp = Wrap::load_bitmap(
		"mini_games/runner/tree_fg.cpi"
	);
	atlas_add(atlas, bmp, tree_fg_i);
	bmps.push_back(bmp);
	bmp = Wrap::load_bitmap(
		"mini_games/runner/tree_fg2.cpi"
	);
	atlas_add(atlas, bmp, tree_fg_i+1);
	bmps.push_back(bmp);
	tree_fg_w = al_get_bitmap_width(bmp->bitmap);
	tree_fg_h = al_get_bitmap_height(bmp->bitmap);

	tree_slide_i = tree_fg_i + 2;
	bmp = Wrap::load_bitmap(
		"mini_games/runner/tree_slide.cpi"
	);
	tree_slide_h = al_get_bitmap_height(bmp->bitmap);
	atlas_add(atlas, bmp, tree_slide_i);
	bmps.push_back(bmp);
	bmp = Wrap::load_bitmap(
		"mini_games/runner/tree_slide_upper.cpi"
	);
	atlas_add(atlas, bmp, tree_slide_i+1);
	bmps.push_back(bmp);

	atlas_finish(atlas);

	for (unsigned int i = 0; i < bmps.size(); i++) {
		Wrap::destroy_bitmap(bmps[i]);
	}

	// Coin size is needed in generation
	coin = new Animation_Set();
	coin->load("mini_games/runner/coin");
	
	coin_w = coin->get_current_animation()->get_current_frame()->get_width();
	coin_h = coin->get_current_animation()->get_current_frame()->get_height();

	gradient = Wrap::load_bitmap("mini_games/runner/colorgradient.cpi");
	bee_anim = new Animation_Set();
	bee_anim->load("mini_games/runner/bee");
	shield_bmp = Wrap::load_bitmap("mini_games/runner/shield.cpi");
	shield_barrier = new Animation_Set();
	shield_barrier->load("mini_games/runner/shield_barrier");

	cloud1 = Wrap::load_bitmap("mini_games/runner/clouds1.cpi");
	cloud2 = Wrap::load_bitmap("mini_games/runner/clouds2.cpi");
	cloud3 = Wrap::load_bitmap("mini_games/runner/clouds3.cpi");
	star1 = Wrap::load_bitmap("mini_games/runner/stara1.cpi");
	star2 = Wrap::load_bitmap("mini_games/runner/stara2.cpi");
	star3 = Wrap::load_bitmap("mini_games/runner/starb1.cpi");
	star4 = Wrap::load_bitmap("mini_games/runner/starb2.cpi");

	poof_anim = new Animation_Set();
	poof_anim->load("mini_games/runner/poof");

	reload_graphics();
	
	engine->load_sample("sfx/thud.ogg");
	engine->load_sample("sfx/bird_tweet.ogg");
	engine->load_sample("sfx/hit_bird.ogg");
	engine->load_sample("sfx/poof.ogg");
	engine->load_sample("sfx/bees.ogg", true);
	engine->load_sample("sfx/slide.ogg", true);
	engine->load_sample("sfx/boingboingboing.ogg");

	engine->play_sample("sfx/bees.ogg");
	
	start1();
}


Runner_Loop::~Runner_Loop(void)
{
	stop();

	for (size_t i = 0; i < sfx.size(); i++) {
		Sound::destroy(sfx[i]);
	}

	Wrap::destroy_bitmap(bird_bmp);
	Wrap::destroy_bitmap(gradient);
	delete bee_anim;
	Wrap::destroy_bitmap(shield_bmp);
	delete shield_barrier;
	Wrap::destroy_bitmap(cloud1);
	Wrap::destroy_bitmap(cloud2);
	Wrap::destroy_bitmap(cloud3);
	Wrap::destroy_bitmap(star1);
	Wrap::destroy_bitmap(star2);
	Wrap::destroy_bitmap(star3);
	Wrap::destroy_bitmap(star4);
	delete poof_anim;
	delete coin;

	for (size_t i = 0; i < clouds.size(); i++) {
		delete clouds[i];
	}

	General::srand(time(NULL));

	engine->stop_sample("sfx/bees.ogg");
	
	engine->destroy_sample("sfx/thud.ogg");
	engine->destroy_sample("sfx/bird_tweet.ogg");
	engine->destroy_sample("sfx/bees.ogg");
	engine->destroy_sample("sfx/hit_bird.ogg");
	engine->destroy_sample("sfx/poof.ogg");
	engine->destroy_sample("sfx/slide.ogg");
	engine->destroy_sample("sfx/boingboingboing.ogg");

	player->get_attributes().hp = start_hp;

	tgui::pop(); // pushed beforehand
	tgui::unhide();
}

void Runner_Loop::destroy_graphics()
{
	Wrap::destroy_bitmap(screen_copy_bmp);
}

void Runner_Loop::reload_graphics()
{
	screen_copy_bmp = Wrap::create_bitmap_no_preserve(cfg.screen_w, cfg.screen_h);
	making_screen_copy = true;
}

