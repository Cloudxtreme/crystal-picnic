#ifndef GENERAL_H
#define GENERAL_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#include <string>
#include <iostream>
#include <cmath>
#include <sys/stat.h>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <list>
#include <algorithm>
#include <cfloat>

#include <wrap.h>

#include "config.h"
#include "sound.h"
#include "general_types.h"
#include "speech_types.h"
#include "positioned_entity.h"
#include "particle.h"

#ifdef ALLEGRO_WINDOWS
#include <shlobj.h>
#endif

namespace Triangulate {
	struct Triangle;
}

#define D2R(a) ((a)/180.0*M_PI)
#define R2D(a) ((a)/M_PI*180.0)

#undef MIN
#undef MAX
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#include "snprintf.h"

namespace General {

const int LOGIC_RATE = 60;
const int LOGIC_MILLIS = 1000/LOGIC_RATE;

const int CHUNK_SIZE = 128;
const int TILE_SIZE = 16;
const int BOTTOM_SPRITE_PADDING = 16;

const int RENDER_W = 240;
const int RENDER_H = 160;

const int BIG_FLOAT = 1000000;
const int SMALL_FLOAT = -1000000;

enum Orientation {
	VERTICAL = 0,
	HORIZONTAL
};

enum Direction {
	DIR_N = 0,
	DIR_NE,
	DIR_E,
	DIR_SE,
	DIR_S,
	DIR_SW,
	DIR_W,
	DIR_NW,
	NUM_DIRECTIONS
};

enum Font_Type {
	FONT_LIGHT = 0,
	FONT_HEAVY,
	NUM_FONTS
};

enum Text_Border_Type {
	TEXT_BORDER_NONE,
	TEXT_BORDER_SQUARE
};

enum Text_Speed {
	TEXT_SPEED_SLOW = 0,
	TEXT_SPEED_MEDIUM,
	TEXT_SPEED_FAST
};

enum Human_Language {
	LANGUAGE_ENGLISH = 0, // must start at 0
	LANGUAGE_JAPANESE,
	LANGUAGE_FRENCH,
	LANGUAGE_KOREAN,
	LANGUAGE_CHINESE,
	LANGUAGE_GERMAN,
	LANGUAGE_SPANISH,
	LANGUAGE_PORTUGUESE,
	NUM_LANGUAGES // must be here and last
};

extern int default_bmp_format;
extern int noalpha_bmp_format;
extern int font_bmp_format;

extern const char *direction_strings[NUM_DIRECTIONS];

extern int argc;
extern char **argv;
extern bool can_log_to_disk; // has al_init succeeded yet?

extern unsigned char rgb_scale_4[16];

extern std::string user_resource_dir;

extern ALLEGRO_COLOR UI_GREEN;
extern ALLEGRO_COLOR UI_BLUE;

std::string get_user_resource(std::string relative_path);

float circle_line_distance(
	float x, float y, /* circle center */
	float x1, float y1, float x2, float y2 /* line */);

// Logging
std::string numtos(double d);
void log_message(std::string msg);

unsigned char *slurp_from_file(std::string filename, int *ret_size = NULL, bool terminate_with_0 = true, bool use_malloc = false);
unsigned char *slurp(std::string filename, int *ret_size = NULL, bool terminate_with_0 = true, bool use_malloc = false);
unsigned char *slurp_real_file(std::string filename, int *ret_size = NULL, bool terminate_with_0 = true, bool use_malloc = false);
void load_mask(std::string filename, int size, unsigned char **dest);

void init_textlog(); // call before creating display
void init(); // call after initing allegro
void shutdown();

float distance(float x1, float y1, float x2, float y2);
float sign(float f);

uint32_t read32bits(ALLEGRO_FILE *f);
std::string readNString(ALLEGRO_FILE *f);

void analog_to_digital_joystick(float *a, int *d);

template<class T> void erase_from_list(std::list<T> &l, std::list<T> &to_erase)
{
	typename std::list<T>::iterator it;
	for (it = to_erase.begin(); it != to_erase.end(); it++) {
		typename std::list<T>::iterator found = std::find(l.begin(), l.end(), *it);
		if (found != l.end()) {
			l.erase(found);
		}
	}
}

template<class T> void find_shared_in_lists(
		typename std::list<T>::iterator beg1,
		typename std::list<T>::iterator end1,
		typename std::list<T>::iterator beg2,
		typename std::list<T>::iterator end2,
		std::list<T> &dest)
{
	typename std::list<T>::iterator it;
	for (it = beg1; it != end1; it++) {
		typename std::list<T>::iterator found;
		found = std::find(beg2, end2, *it);
		if (found != end2) {
			dest.push_back(*it);
		}
	}
}

int find_arg(const char *s);

#ifdef ALLEGRO_WINDOWS
const char *strcasestr(const char *s1, const char *s2);
#endif

inline bool bxor(bool a, bool b) {
	return (a && !b) || (!a && b);
}

bool triangle_is_clockwise(Triangulate::Triangle *tri);

void draw_speech_window(Speech_Type type, int x, int y, int w, int h, bool draw_star, ALLEGRO_COLOR color, float green_opacity);

void draw_text(Text_Border_Type border_type, std::string text, ALLEGRO_COLOR main_color, ALLEGRO_COLOR border_color, float x, float y, int flags = 0, Font_Type font = FONT_LIGHT);
void draw_text(ALLEGRO_FONT *font, std::string text, ALLEGRO_COLOR color, float x, float y, int flags = 0);
void draw_text(std::string text, ALLEGRO_COLOR color, float x, float y, int flags = 0, Font_Type font = FONT_LIGHT);
void draw_text_width(int width, std::string text, ALLEGRO_COLOR color, float x, float y, int flags = 0, Font_Type font = FONT_LIGHT);
void draw_text(std::string text, float x, float y, int flags, Font_Type font = FONT_LIGHT);
void draw_wrapped_text(std::string text, ALLEGRO_COLOR color, float x, float y, float width, Font_Type font = FONT_LIGHT);
ALLEGRO_FONT *get_font(Font_Type font);
int get_text_width(ALLEGRO_FONT *font, std::string text);
int get_text_width(Font_Type font, std::string text);
int get_font_line_height(Font_Type font);

void logic();

extern int texture_size;

bool check_mask(unsigned char *mask, int pitch, int x, int y);

void iso_project(int *x, int *y, General::Point<float> iso_offset);
void reverse_iso_project(int *x, int *y, General::Point<float> iso_offset);

std::string itos(int i);
std::string ftos(float i);

Wrap::Bitmap *create_checkerboard_bitmap(int check_w, int check_h, int width, int height, ALLEGRO_COLOR color1, ALLEGRO_COLOR color2);
void draw_textured_quad(float x1, float y1, float x2, float y2,
	ALLEGRO_COLOR color, Wrap::Bitmap *texture);

std::string get_language_string(Human_Language lang);

void srand(uint32_t s);
uint32_t rand();
uint32_t rand(int min, int max_inclusive);

template<typename C, typename P> C find_in_vector(std::vector<P> v)
{
	for (size_t i = 0; i < v.size(); i++) {
		C p = dynamic_cast<C>(v[i]);
		if (p) {
			return p;
		}
	}
	
	return NULL;
}

static inline double interpolate(double a, double b, double percent)
{
	double diff = b - a;
	return a + (percent * diff);
}

void load_bitmap_array(Wrap::Bitmap **bmps, int num, std::string start, std::string ext);

void best_fit(Wrap::Bitmap *bitmap);
void best_fit_tinted(Wrap::Bitmap *bitmap, ALLEGRO_COLOR color);

bool exists(std::string path);
	
ALLEGRO_FONT *load_font(const char *filename, int size);
void load_fonts();
void destroy_fonts();

std::vector<std::string> split(std::string s);

const int CONVEX = 1;
const int CONCAVE = -1;

int polygon_is_convex(std::vector<float> v);
bool polygon_is_clockwise(std::vector<float> v);
bool polygon_is_clockwise(std::vector< General::Point<float> > v, int start, int end_exclusive);

void tile_bitmap(Wrap::Bitmap *bitmap, ALLEGRO_COLOR tint, int x, int y, int w, int h);

void draw_triangle(Triangulate::Triangle &t, ALLEGRO_COLOR c, float xoffs, float yoffs);

float angle_difference(float a1, float a2);
float angle_difference_clockwise(float a1, float a2);
float angle_difference_counter_clockwise(float a1, float a2);

float direction_to_angle(Direction dir);
Direction angle_to_direction(float angle);

bool is_hero(std::string name);
std::string get_hero_printable_name(std::string entity_name);

bool set_clipping_rectangle(int x, int y, int width, int height);

bool ability_is_universal(std::string name);

std::string get_time_string(double seconds);

uint8_t *slurp_file(std::string filename, int *sz);

bool is_slash(char c);

std::string tolower(std::string s);

void calculate_toss(
	float sx,
	float sy,
	float dx,
	float dy,
	float percent,
	float *outx,
	float *outy);

void draw_poison_bubbles(General::Point<float> pos);

bool is_item(std::string name);

std::string get_save_filename(int number);

std::string get_download_path();

} // end namespace General

#ifdef ALLEGRO_WINDOWS
using General::strcasestr;
#endif

#define ERASE_FROM_UNSORTED_VECTOR(v, i) \
	v[i] = v[v.size()-1]; \
	v.pop_back();

#endif
