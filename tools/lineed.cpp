#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_native_dialog.h>
#include <vector>
#include <cmath>
#include <cstdio>

#define SCR_W 1024
#define SCR_H 650

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

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

int layer = -1;

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
	
	if(layer >= 0) {
		fprintf(f, "\t%d,\n", layer);
		printf("\t%d,\n", layer);
	} else {
		fprintf(f, "\t<layer>,\n");
		printf("\t<layer>,\n");
	}
	
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
				points[i][j].x/2,
				points[i][j].y/2
			);
			printf("\t%d, %d,\n",
				points[i][j].x/2,
				points[i][j].y/2
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
	al_set_new_display_flags(ALLEGRO_OPENGL);
	ALLEGRO_DISPLAY *display = al_create_display(SCR_W, SCR_H);
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

	al_register_event_source(q, al_get_display_event_source(display));
	al_register_event_source(q, al_get_keyboard_event_source());
	al_register_event_source(q, al_get_mouse_event_source());
	al_register_event_source(q, al_get_timer_event_source(timer));

	points.push_back(std::vector<Point>());

	if (argc == 3) {
		FILE *f = fopen(argv[2], "r");
		char line[200];
		int count = 0;
      
		fgets(line, 199, f); // skip outline
      
		fgets(line, 199, f); // do layer
		if(sscanf(line, "\t%i,\n", &layer) != 1) {
			printf("no layer? %i\n", layer);
			layer = -1;
		}
		else {
			printf("layer: %i\n", layer);
		}

		while (fgets(line, 199, f)) {
			int x, y;
			if (sscanf(line, "\t%d, %d,\n", &x, &y) == 2) {
				if (x == -1 && y == -1) {
					splits.push_back(count);
					poly_count++;
					points.push_back(std::vector<Point>());
				}
				else {
					Point p;
					p.x = x*2;
					p.y = y*2;
					points[poly_count].push_back(p);
					count++;
				}
			}
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
			if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				int ret = al_show_native_message_box(display, "Confirm Exit", "Really?", "Seriously?", 0, ALLEGRO_MESSAGEBOX_YES_NO);
				if(ret == 1) {
					goto done;
				}
			} else if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
				if (event.keyboard.keycode == ALLEGRO_KEY_S) {
					save();
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_Q) {
					int ret = al_show_native_message_box(display, "Confirm Exit", "Really?", "Seriously?", 0, ALLEGRO_MESSAGEBOX_YES_NO);
					if(ret == 1) {
						goto done;
					}
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_UP) {
					top_y -= 32;
					if (top_y < 0) top_y = 0;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
					top_y += 32;
					if (top_y + SCR_H > al_get_bitmap_height(bmp)) {
						top_y = al_get_bitmap_height(bmp)-SCR_H;
					}
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_LEFT) {
					top_x -= 32;
					if (top_x < 0) top_x = 0;
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
					top_x += 32;
					if (top_x + SCR_W > al_get_bitmap_width(bmp)) {
						top_x = al_get_bitmap_width(bmp)-SCR_W;
					}
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
					if (down) {
						points[poly_count].erase(points[poly_count].begin()+points[poly_count].size()-1);
						if (points[poly_count].size() <= 0) {
							down = false;
						}
						else {
							Point &p = points[poly_count][points[poly_count].size()-1];
							down_x = p.x;
							down_y = p.y;
						}
					}
					else {
						if (poly_count > 0) {
							points.erase(points.begin()+poly_count);
							poly_count--;
							Point &p = points[poly_count][points[poly_count].size()-1];
							down_x = p.x;
							down_y = p.y;
							down = true;
						}
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
					if (top_y+state.y < al_get_bitmap_height(bmp)/2)
						add(state.x, 0);
					else
						add(state.x, MIN(SCR_H, al_get_bitmap_height(bmp)-1));
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_Y) {
					ALLEGRO_MOUSE_STATE state;
					al_get_mouse_state(&state);
					if (top_x+state.x < al_get_bitmap_width(bmp)/2)
						add(0, top_y);
					else
						add(MIN(SCR_W, al_get_bitmap_width(bmp)-1), state.y);
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_1) {
					add(al_get_bitmap_width(bmp), 0);
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_2) {
					add(al_get_bitmap_width(bmp), al_get_bitmap_height(bmp));
				}
				else if (event.keyboard.keycode == ALLEGRO_KEY_3) {
					add(0, al_get_bitmap_height(bmp));
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
					if(mx > al_get_bitmap_width(bmp))
						mx = al_get_bitmap_width(bmp);
					if(my > al_get_bitmap_height(bmp))
						my = al_get_bitmap_height(bmp);
					
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
					if (i < poly_count) {
						// draw joining line
						Point &p1 = points[i][points[i].size()-1];
						Point &p2 = points[i][0];
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

