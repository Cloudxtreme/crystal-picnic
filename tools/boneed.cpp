#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <vector>
#include <cmath>
#include <cstdio>
#include <string>

const int SCALE = 2;
const int SCR_W = 800;
const int SCR_H = 600;

int top_x = 0;
int top_y = 0;
unsigned int count = 0;

enum Bone_Type {
	BONE_NORMAL = 0, // can take damage
	BONE_ATTACK, // deals damage
	BONE_RESISTANT, // takes no damage
	BONE_WEAK // takes lots of damage
};

ALLEGRO_COLOR get_color(Bone_Type type)
{
	switch (type) {
		case BONE_NORMAL:
			return al_map_rgb(180, 180, 180);
		case BONE_ATTACK:
			return al_map_rgb(255, 0, 0);
		case BONE_RESISTANT:
			return al_map_rgb(70, 70, 70);
		case BONE_WEAK:
			return al_map_rgb(255, 255, 0);
	}

	return al_map_rgb(0, 0, 255); // ?
}

Bone_Type curr_type;

struct Point {
	int x, y;
};

struct Bone {
	std::string image_name;
	Bone_Type type;
	std::vector<Point> points;
};

struct Image {
	ALLEGRO_BITMAP *bmp;
	std::string name;
};

int curr_image = 0;
std::vector<Image> images;
std::string txt_name = "";
std::vector<Bone> bones;

bool down;
int down_x, down_y;

void save(const char *argv1)
{
	FILE *f = fopen("out.txt", "w");
	for (unsigned int i = 0; i < bones.size(); i++) {
		Bone &b = bones[i];
		printf("<%d>\n", (int)b.type);
		fprintf(f, "<%d>\n", (int)b.type);
		printf("\t<sheet>%s</sheet>\n", b.image_name.c_str());
		fprintf(f, "\t<sheet>%s</sheet>\n", b.image_name.c_str());
		for (unsigned int j = 0; j < b.points.size(); j++) {
			printf("\t<%d><x>%d</x><y>%d</y></%d>\n", j, b.points[j].x, b.points[j].y, j);
			fprintf(f, "\t<%d><x>%d</x><y>%d</y></%d>\n", j, b.points[j].x, b.points[j].y, j);
		}
		printf("</%d>\n", (int)b.type);
		fprintf(f, "</%d>\n", (int)b.type);
	}
	fclose(f);
}

void add(int x, int y)
{
	Point p;
	p.x = (x/SCALE) + top_x;
	p.y = (y/SCALE) + top_y;

	down_x = p.x;
	down_y = p.y;

	if (!down) {
		down = true;
		Bone b;
		b.type = curr_type;
		b.image_name = images[curr_image].name;
		bones.push_back(b);
	}

	bones[bones.size()-1].points.push_back(p);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: boneed <images...> [input.txt]\n");
		return 0;
	}

	al_init();
	al_init_image_addon();
	al_init_primitives_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_create_display(SCR_W, SCR_H);
	ALLEGRO_EVENT_QUEUE *q = al_create_event_queue();
	al_install_keyboard();
	al_install_mouse();

	for (int i = 1; i < argc; i++) {
		const char *s = argv[i];
		if (strstr(s, ".txt")) {
			txt_name = std::string(s);
		}
		else {
			ALLEGRO_BITMAP *b = al_load_bitmap(s);
			Image i;
			i.bmp = b;
			i.name = std::string(s);
			images.push_back(i);
		}
	}

	ALLEGRO_BITMAP *bmp = images[curr_image].bmp;

	ALLEGRO_PATH *p = al_create_path(argv[0]);
	al_set_path_filename(p, "DejaVuSans.ttf");
	ALLEGRO_FONT *font = al_load_ttf_font(al_path_cstr(p, '/'), 14, 0);
	al_destroy_path(p);

	ALLEGRO_TIMER *timer = al_create_timer(0.05);
	al_start_timer(timer);

	al_register_event_source(q, al_get_keyboard_event_source());
	al_register_event_source(q, al_get_mouse_event_source());
	al_register_event_source(q, al_get_timer_event_source(timer));

	if (txt_name != "") {
		FILE *f = fopen(txt_name.c_str(), "r");
		char buf[1000];
		while (1) {
			Bone b;
			if (fgets(buf, 1000, f) == NULL)
				break;
			sscanf(buf, "<%d>", (int *)&b.type);
			// sheet name
			fgets(buf, 1000, f);
			int c = 0, c2 = 0;
			char buf2[1000];
			while (buf[c] != '>') c++;
			c++;
			while (buf[c] != '<') buf2[c2++] = buf[c++];
			buf2[c2] = 0;
			b.image_name = std::string(buf2);
			while (1) {
				int x, y, n1, n2;
				if (fgets(buf, 1000, f) == NULL)
					break;
				if (sscanf(buf, "\t<%d><x>%d</x><y>%d</y></%d>", &n1, &x, &y, &n2) < 4)
					break;
				Point p;
				p.x = x;
				p.y = y;
				b.points.push_back(p);
			}
			bones.push_back(b);
			count++;
		}
		fclose(f);
	}

	int timer_ticks = 0;

	while (1) {
		while (!al_event_queue_is_empty(q)) {
			ALLEGRO_EVENT event;
			al_get_next_event(q, &event);
			if (event.type == ALLEGRO_EVENT_TIMER) {
				timer_ticks++;
			}
			else if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
				if (event.keyboard.keycode == ALLEGRO_KEY_S) {
					save(argv[1]);
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_Q) {
					goto done;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_UP) {
					top_y -= (32);
					if (top_y < 0) top_y = 0;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
					top_y += (32);
					if (top_y + (SCR_H/SCALE) > (al_get_bitmap_height(bmp))) {
						top_y = (al_get_bitmap_height(bmp))-(SCR_H/SCALE);
					}
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
					top_x -= (32);
					if (top_x < 0) top_x = 0;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
					top_x += (32);
					if (top_x + (SCR_W/SCALE) > (al_get_bitmap_width(bmp))) {
						top_x = (al_get_bitmap_width(bmp))-(SCR_W/SCALE);
					}
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
					if (down) {
						Bone &b = bones[bones.size()-1];
						b.points.erase(b.points.begin()+b.points.size()-1);
						if (b.points.size() == 0) {
							down = false;
						}
						else {
							Point &p = b.points[b.points.size()-1];
							down_x = p.x;
							down_y = p.y;
						}
					}
					else {
						if (count > 0) {
							count--;
							Bone &b = bones[count];
							Point &p = b.points[b.points.size()-1];
							down_x = p.x;
							down_y = p.y;
							down = true;
						}
					}
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_L) {
					// finish loop
					down = false;
					count++;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_1) {
					curr_type = (Bone_Type)0;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_2) {
					curr_type = (Bone_Type)1;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_3) {
					curr_type = (Bone_Type)2;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_4) {
					curr_type = (Bone_Type)3;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_OPENBRACE) {
					curr_image--;
					if (curr_image < 0)
						curr_image = images.size()-1;
					bmp = images[curr_image].bmp;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_CLOSEBRACE) {
					curr_image++;
					curr_image %= images.size();
					bmp = images[curr_image].bmp;
				}
			}
			else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
				add(event.mouse.x, event.mouse.y);
			}
		}
		if (timer_ticks > 0) {
			timer_ticks = 0;
			al_clear_to_color(al_map_rgb(0, 0, 0));
			al_draw_scaled_bitmap(bmp,
				0, 0, al_get_bitmap_width(bmp), al_get_bitmap_height(bmp),
				-top_x*SCALE, -top_y*SCALE, al_get_bitmap_width(bmp)*SCALE, al_get_bitmap_height(bmp)*SCALE,
				0);
			for (unsigned int i = 0; i < bones.size(); i++) {
				Bone &b = bones[i];
				if (b.image_name != images[curr_image].name)
					continue;
				if (b.points.size() > 1) {
					for (unsigned int j = 0; j < b.points.size()-1; j++) {
						Point &p1 = b.points[j];
						Point &p2 = b.points[j+1];
						al_draw_line(
							(p1.x-top_x)*SCALE,
							(p1.y-top_y)*SCALE,
							(p2.x-top_x)*SCALE,
							(p2.y-top_y)*SCALE,
							get_color(b.type),
							1
						);
					}
					if (i < count) {
						// draw joining line
						Point &p1 = b.points[b.points.size()-1];
						Point &p2 = b.points[0];
						al_draw_line(
							(p1.x-top_x)*SCALE,
							(p1.y-top_y)*SCALE,
							(p2.x-top_x)*SCALE,
							(p2.y-top_y)*SCALE,
							get_color(b.type),
							1
						);
					}
					Point &p = b.points[0];
					al_draw_circle(
						(p.x-top_x)*SCALE, (p.y-top_y)*SCALE, 3, al_map_rgb(0, 255, 255), 1
					);
				}
			}
			if (down) {
				ALLEGRO_MOUSE_STATE state;
				al_get_mouse_state(&state);
				int x = state.x;
				int y = state.y;
				al_draw_line(
					(down_x-top_x)*SCALE,
					(down_y-top_y)*SCALE,
					x,
					y,
					al_map_rgb(255, 255, 255),
					1
				);
			}
			al_draw_text(font, al_map_rgb(255, 255, 255), SCR_W/2, 0, ALLEGRO_ALIGN_CENTRE, images[curr_image].name.c_str());
			al_draw_text(font, al_map_rgb(255, 255, 255), SCR_W/2, SCR_H-15, ALLEGRO_ALIGN_CENTRE, "Press S to save and Q to quit");
			al_flip_display();
		}
		al_rest(0.001);
	}
done:

	return 0;
}

