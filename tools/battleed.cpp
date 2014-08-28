#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <vector>
#include <cmath>
#include <cstdio>

int top_x = 0;
int top_y = 0;

struct Point {
	int x, y;
};

typedef Point Open;
typedef Point Closed;

std::vector< std::vector<Point> > points;
std::vector<int> splits;

bool down;
int down_x, down_y;
int first_down_x, first_down_y;
int poly_count = 0;

bool snap_to_grid = false;
int grid_w = 32;
int grid_h = 32;
bool iso_grid = true;
Point iso_offset;

void iso_project(int *x, int *y) {
	double xinc = -(*y) / 2.0;
	double yinc = (*x) / 4.0;
	
	*x = (int)(((float)(*x)/2) + xinc);
	*y = (int)(((float)(*y)/4) + yinc);

	*x += iso_offset.x;
	*y += iso_offset.y;
}

void reverse_iso_project(int *x, int *y) {
	*x -= iso_offset.x;
	*y -= iso_offset.y;

	double xinc = -2*(*y);
	double yinc = (*x) / 2;

	*x = (int)((*x) - xinc);
	*y = (int)(2 * ((*y) - yinc));
}

void get_iso_xy(int *xx, int *yy)
{
	int x = *xx;
	int y = *yy;

	reverse_iso_project(&x, &y);
	if (x % grid_w < grid_w/2) {
		x = x - (x % grid_w);
	}
	else {
		x = x + (grid_w - (x % grid_w));
	}
	if (y % grid_h < grid_h/2) {
		y = y - (y % grid_h);
	}
	else {
		y = y + (grid_h - (y % grid_h));
	}
	iso_project(&x, &y);

	*xx = x;
	*yy = y;
}

float circle_line_distance(
	float x, float y, /* circle center */
	float x1, float y1, float x2, float y2 /* line */)
{
	float A = x - x1;
	float B = y - y1;
	float C = x2 - x1;
	float D = y2 - y1;

	float dot = A * C + B * D;
	float len_sq = C * C + D * D;
	float param = dot / len_sq;

	float xx,yy;

	if(param < 0) {
		xx = x1;
		yy = y1;
	}
	else if (param > 1) {
		xx = x2;
		yy = y2;
	}
	else {
		xx = x1 + param * C;
		yy = y1 + param * D;
	}

	float dx = x - xx;
	float dy = y - yy;
	float dist = sqrt(dx*dx + dy*dy);

	return dist;
}

void save(void)
{
	FILE *f = fopen("out.txt", "w");
	int count = 0;
	fprintf(f, "outline%d = {\n", 0);
	printf("outline%d = {\n", 0);
	fprintf(f, "\t<layer>,\n");
	printf("\t<layer>,\n");
	for (int i = 0; i < (int)points.size(); i++) {
		if (i > 0) {
			count += points[i-1].size();
			//fprintf(f, "\tadd_outline_split(%d)\n", count);
			//printf("\tadd_outline_split(%d)\n", count);
			fprintf(f, "\t-1, -1,\n");
			printf("\t-1, -1,\n");
		}
		for (int j = 0; j < (int)points[i].size(); j++) {
			/*
			fprintf(f, "\tadd_outline_point(%d, %d)\n",
				points[i][j].x,
				points[i][j].y
			);
			printf("\tadd_outline_point(%d, %d)\n",
				points[i][j].x,
				points[i][j].y
			);
			*/
			fprintf(f, "\t%d, %d,\n",
				points[i][j].x,
				points[i][j].y
			);
			printf("\t%d, %d,\n",
				points[i][j].x,
				points[i][j].y
			);
		}
	}

	fprintf(f, "}\n");
	printf("}\n");

	fclose(f);
}

void add(int x, int y)
{
	Point p;
	p.x = x + top_x;
	p.y = y + top_y;
	points[poly_count].push_back(p);
	if (down) {
		down_x = p.x;
		down_y = p.y;

	}
	else {
		down = true;
		down_x = top_x + x;
		down_y = top_y + y;
		first_down_x = down_x;
		first_down_y = down_y;
	}
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: lineed <image> [input.txt]\n");
		return 0;
	}

	al_init();
	al_init_image_addon();
    al_init_primitives_addon();
	ALLEGRO_DISPLAY *display = al_create_display(640, 480);
	//al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	ALLEGRO_BITMAP *bmp = al_load_bitmap(argv[1]);
	ALLEGRO_EVENT_QUEUE *q = al_create_event_queue();
	al_install_keyboard();
	al_install_mouse();

	int width = al_get_bitmap_width(bmp);
	int height = al_get_bitmap_height(bmp);
	iso_offset.x = (width > height ? width : height) / 2;
	iso_offset.y = 0;
		
	ALLEGRO_TIMER *timer = al_create_timer(0.05);
	al_start_timer(timer);

	al_register_event_source(q, al_get_keyboard_event_source());
	al_register_event_source(q, al_get_mouse_event_source());
	al_register_event_source(q, al_get_timer_event_source(timer));

	points.push_back(std::vector<Point>());

	if (argc == 3) {
		FILE *f = fopen(argv[2], "r");
		char line[200];
		int count = 0;
		while (fgets(line, 199, f)) {
			int x, y;
			if (sscanf(line, "\tadd_outline_point(%d, %d)", &x, &y) == 2) {
				Point p;
				p.x = x;
				p.y = y;
				points[poly_count].push_back(p);
				count++;
			}
			else {
				int n;
				sscanf(line, "\tadd_outline_split(%d)", &n);
				splits.push_back(n);
				poly_count++;
				points.push_back(std::vector<Point>());
			}
		}
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
					save();
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_Q) {
					goto done;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_UP) {
					top_y -= 32;
					if (top_y < 0) top_y = 0;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
					top_y += 32;
					if (top_y + 480 > al_get_bitmap_height(bmp)) {
						top_y = al_get_bitmap_height(bmp)-480;
					}
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
					top_x -= 32;
					if (top_x < 0) top_x = 0;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
					top_x += 32;
					if (top_x + 640 > al_get_bitmap_width(bmp)) {
						top_x = al_get_bitmap_width(bmp)-640;
					}
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
					if (down) {
						points[poly_count].erase(points[poly_count].begin()+points[poly_count].size()-1);
						Point &p = points[poly_count][points[poly_count].size()-1];
						down_x = p.x;
						down_y = p.y;
					}
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_L) {
					// finish loop
					down = false;
					poly_count++;
					points.push_back(std::vector<Point>());
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_X) {
					ALLEGRO_MOUSE_STATE state;
					al_get_mouse_state(&state);
					if (state.y < al_get_display_height(display)/2)
						add(state.x, 0);
					else
						add(state.x, al_get_display_height(display)-1);
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_Y) {
					ALLEGRO_MOUSE_STATE state;
					al_get_mouse_state(&state);
					if (state.x < al_get_display_width(display)/2)
						add(0, state.y);
					else
						add(al_get_display_width(display)-1, state.y);
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_1) {
					add(al_get_display_width(display)-1, 0);
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_2) {
					add(al_get_display_width(display)-1, al_get_display_height(display)-1);
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_3) {
					add(0, al_get_display_height(display)-1);
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_4) {
					add(0, 0);
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_P) {
					snap_to_grid = !snap_to_grid;
				}
			}
			else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
				ALLEGRO_KEYBOARD_STATE state;
				al_get_keyboard_state(&state);
				if (al_key_down(&state, ALLEGRO_KEY_D)) {
					int mx = event.mouse.x+top_x;
					int my = event.mouse.y+top_y;
					for (int i = 0; i < (int)points.size(); i++) {
						for (int j = 0; j < (int)points.size(); j++) {
							Point &p1 = points[i][j];
							Point &p2 = points[i][j+1%(points[i].size())];
							if (circle_line_distance(
								mx, my,
								p1.x, p1.y, p2.x, p2.y) < 3) {
								points[i].erase(points[i].begin()+j);
								break;
							}
						}
					}
				}
				else {
					if (snap_to_grid) {
						int x = event.mouse.x;
						int y = event.mouse.y;
						get_iso_xy(&x, &y);
						add(x, y);
					}
					else {
						add(event.mouse.x, event.mouse.y);
					}
				}
			}
		}
		if (timer_ticks > 0) {
			timer_ticks = 0;
			al_clear_to_color(al_map_rgb(255, 255, 255));
			al_draw_bitmap(bmp, -top_x, -top_y, 0);
			for (int i = 0; i < (int)points.size(); i++) {
				if (points[i].size() > 1) {
					for (int j = 0; j < (int)points[i].size()-1; j++) {
						Point &p1 = points[i][j];
						Point &p2 = points[i][j+1];
						al_draw_line(
							p1.x-top_x,
							p1.y-top_y,
							p2.x-top_x,
							p2.y-top_y,
							al_map_rgb(255, 50, 50),
							1
						);
					}
				}
			}
			if (down) {
				ALLEGRO_MOUSE_STATE state;
				al_get_mouse_state(&state);
				int x = state.x;
				int y = state.y;
				if (snap_to_grid) {
					get_iso_xy(&x, &y);
				}
				al_draw_line(
					down_x-top_x,
					down_y-top_y,
					x,
					y,
					al_map_rgb(255, 0, 0),
					1
				);
			}
			al_flip_display();
		}
		al_rest(0.001);
	}
done:
	return 0;
}

