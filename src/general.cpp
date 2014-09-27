#include "crystalpicnic.h"
#include "general.h" 
#include "animation_set.h"
#include "astar.h"
#include "speech_loop.h"
#include "tls.h"
#include "battle_loop.h"
#include "shaders.h"
#include "game_specific_globals.h"

#ifdef WELL512
#include "well512.h"
#else
#include "mt.h"
#endif

#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>

#include <allegro5/allegro_opengl.h>

#include <cctype>

#include "hqm.h"

#ifdef ALLEGRO_ANDROID
#include <allegro5/allegro_android.h>
#include "android.h"
#endif

#ifdef ALLEGRO_MACOSX
extern "C" void LOG(const char *);
#endif

ALLEGRO_DEBUG_CHANNEL("CrystalPicnic")

static Wrap::Shader *lower_green_opacity_shader;

static Animation_Set *speech_arrow;
static Animation_Set *speech_window;
static General::Size<int> speech_window_sizes[3][3];

static ALLEGRO_FONT *fonts[General::NUM_FONTS];

#ifdef ALLEGRO_WINDOWS
#include <allegro5/allegro_native_dialog.h>
static ALLEGRO_TEXTLOG *textlog;
#endif

static std::vector<Wrap::Bitmap *> poison_bmps;
struct Poison_Bubble {
	General::Point<float> pos; // relative to draw pos
	float y_velocity;
	int frame;
	int count_to;
	int count;
};
static std::vector<Poison_Bubble> poison_bubbles;

static void add_poison_bubble()
{
	Poison_Bubble b;
	b.pos.x = (int)(General::rand() % 15) - 7;
	b.pos.y = -General::rand() % 5;
	b.y_velocity = -((General::rand() % 1000) / 1000.0f) * 0.1f;
	b.frame = 0;
	b.count_to = 2 + General::rand() % 4;
	b.count = 0;
	poison_bubbles.push_back(b);
}

static void update_poison_bubbles()
{
	for (size_t i = 0; i < poison_bubbles.size(); i++) {
		Poison_Bubble &b = poison_bubbles[i];
		b.pos.y += b.y_velocity;
		b.count++;
		if (b.count >= b.count_to) {
			b.frame++;
			if (b.frame >= (int)poison_bmps.size()) {
				poison_bubbles.erase(poison_bubbles.begin()+i);
				i--;
				add_poison_bubble();
			}
			else {
				b.count = 0;
			}
		}
	}
}

namespace General {

ALLEGRO_COLOR UI_GREEN;
ALLEGRO_COLOR UI_BLUE;

const float EPSILON = 0.0;

int default_bmp_format;
int noalpha_bmp_format;
int font_bmp_format;

int texture_size = 512;

const char *direction_strings[NUM_DIRECTIONS] = {
	"n",
	"ne",
	"e",
	"se",
	"s",
	"sw",
	"w",
	"nw"
};

/* Definitions */
int argc;
char **argv;
bool can_log_to_disk = false;

const char *LOG_FILENAME = "crystalpicnic.log";

unsigned char rgb_scale_4[16] = {
	0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255
};

#if defined ALLEGRO_IPHONE
static std::string user_resource_path_ios()
{
	const int MAX_PATH = 5000;
	static char path[MAX_PATH];

	ALLEGRO_PATH *user_path = al_get_standard_path(ALLEGRO_USER_DOCUMENTS_PATH);
	sprintf(path, "%s/", al_path_cstr(user_path, '/'));
	al_destroy_path(user_path);
	return path;
}
#endif

std::string get_user_resource(const char *fmt, ...)
{
	va_list ap;

#ifdef ALLEGRO_IPHONE
	const int MAX_PATH = 5000;
	char file[MAX_PATH];
	static char result[MAX_PATH];
	char old[MAX_PATH];

	sprintf(file, "%s/CrystalPicnic", user_resource_path_ios().c_str());
	if (!al_filename_exists(file))
		mkdir(file, 0755);

	va_start(ap, fmt);
	vsnprintf(file, MAX_PATH, fmt, ap);
	va_end(ap);

	sprintf(result, "%s/CrystalPicnic/%s", user_resource_path_ios().c_str(), file);
#else
#ifndef ALLEGRO_WINDOWS
	const int MAX_PATH = 5000;
#endif
	char s1[MAX_PATH];
	char s2[MAX_PATH];
	static char result[MAX_PATH];

	ALLEGRO_PATH *user_path = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);
	strcpy(s1, al_path_cstr(user_path, ALLEGRO_NATIVE_PATH_SEP));
	al_drop_path_tail(user_path);
	strcpy(s2, al_path_cstr(user_path, ALLEGRO_NATIVE_PATH_SEP));
	al_destroy_path(user_path);

	if (!al_filename_exists(s2))
		mkdir(s2, 0755);
	if (!al_filename_exists(s1))
		mkdir(s1, 0755);

	va_start(ap, fmt);
	vsnprintf(s2, MAX_PATH, fmt, ap);
	va_end(ap);

	sprintf(result, "%s/%s", s1, s2);
#endif
	
	return result;
}

std::string numtos(double d)
{
	static char buf[50];

#ifdef _MSC_VER
	_snprintf(buf, 50, "%g", d);
#else
	snprintf(buf, 50, "%g", d);
#endif

	return std::string(buf);
}

void log_message(std::string msg)
{
	std::string tmp = "[LOG] " + msg;

#ifdef DEBUG
	if (can_log_to_disk) {
		std::ofstream log_file(get_user_resource(std::string(LOG_FILENAME)).c_str(), std::ios_base::app);
		log_file << tmp << std::endl;
		log_file.close();
	}
#endif

#if defined ALLEGRO_MACOSX
	LOG(tmp.c_str());
#elif defined ALLEGRO_ANDROID
	ALLEGRO_DEBUG("%s\n", tmp.c_str());
#else
#ifdef ALLEGRO_WINDOWS
	if (find_arg("-log") > 0) {
		al_append_native_text_log(textlog, "%s\n", tmp.c_str());
	}
	else {
		std::cout << tmp << std::endl;
	}
#else
	std::cout << tmp << std::endl;
#endif
#endif
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

void init_textlog()
{
#ifdef ALLEGRO_WINDOWS
	if (al_init_native_dialog_addon() && find_arg("-log") > 0) {
		textlog = al_open_native_text_log("Log", ALLEGRO_TEXTLOG_MONOSPACE);
	}
#endif
}

void init()
{
	General::srand(time(NULL));

	ALLEGRO_DEBUG("Random number generator seeded");

	UI_GREEN = al_map_rgb(47, 102, 60);
	UI_BLUE = al_map_rgb(45, 82, 106);

	lower_green_opacity_shader = Shader::get("lower_green_opacity");

	ALLEGRO_DEBUG("lower_green_opacity_shader created");

	speech_window = new Animation_Set();
	speech_window->load("misc_graphics/speech_window");
	speech_window_sizes[0][0].w = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_width();
	speech_window_sizes[0][0].h = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_height();
	speech_window->set_sub_animation("top_middle");
	speech_window_sizes[1][0].w = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_width();
	speech_window_sizes[1][0].h = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_height();
	speech_window->set_sub_animation("top_right");
	speech_window_sizes[2][0].w = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_width();
	speech_window_sizes[2][0].h = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_height();
	speech_window->set_sub_animation("middle_left");
	speech_window_sizes[0][1].w = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_width();
	speech_window_sizes[0][1].h = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_height();
	speech_window->set_sub_animation("middle_middle");
	speech_window_sizes[1][1].w = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_width();
	speech_window_sizes[1][1].h = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_height();
	speech_window->set_sub_animation("middle_right");
	speech_window_sizes[2][1].w = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_width();
	speech_window_sizes[2][1].h = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_height();
	speech_window->set_sub_animation("bottom_left");
	speech_window_sizes[0][2].w = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_width();
	speech_window_sizes[0][2].h = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_height();
	speech_window->set_sub_animation("bottom_middle");
	speech_window_sizes[1][2].w = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_width();
	speech_window_sizes[1][2].h = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_height();
	speech_window->set_sub_animation("bottom_right");
	speech_window_sizes[2][2].w = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_width();
	speech_window_sizes[2][2].h = speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_height();

	speech_arrow = new Animation_Set();
	speech_arrow->load("misc_graphics/speech_arrow");

	load_fonts();

	for (int i = 0; i < 9; i++) {
		char fn[100];
		snprintf(fn, 100, "misc_graphics/poison/%d.cpi", i+1);
		poison_bmps.push_back(Wrap::load_bitmap(fn));
	}
	for (int i = 0; i < 5; i++) {
		add_poison_bubble();
	}

	Game_Specific_Globals::init();

	hqm_set_download_path(get_download_path().c_str());
}

void shutdown()
{
	Shader::destroy(lower_green_opacity_shader);
	delete speech_window;
	delete speech_arrow;
#ifdef ALLEGRO_WINDOWS
	if (find_arg("-log") > 0) {
		al_close_native_text_log(textlog);
	}
#endif
	for (size_t i = 0; i < poison_bmps.size(); i++) {
		Wrap::destroy_bitmap(poison_bmps[i]);
	}
	poison_bmps.clear();

	destroy_fonts();
}

// General purpose update function -- always called from every loop every logic frame
void logic()
{
	speech_arrow->update();
	update_poison_bubbles();
}

// Closes 'f'
unsigned char *slurp_from_file(ALLEGRO_FILE *f, int *ret_size, bool terminate_with_0, bool use_malloc)
{
	long size = al_fsize(f);
	unsigned char *bytes;
	int extra = terminate_with_0;
	
	if (size < 0) {
		std::vector<char> v;
		int c;
		while ((c = al_fgetc(f)) != EOF) {
			v.push_back(c);
		}
		if (use_malloc) {
			bytes = (unsigned char *)malloc(v.size()+extra);
		}
		else {
			bytes = new unsigned char[v.size()+extra];
		}
		for (unsigned int i = 0; i < v.size(); i++) {
			bytes[i] = v[i];
		}
	}
	else {
		if (use_malloc) {
			bytes = (unsigned char *)malloc(size+extra);
		}
		else {
			bytes = new unsigned char[size+extra];
		}
		al_fread(f, bytes, size);
	}
	al_fclose(f);
	if (extra) {
		bytes[size] = 0;
	}

	if (ret_size)
		*ret_size = size + extra;

	return bytes;
}

// Slurp from CPA
unsigned char *slurp(std::string filename, int *ret_size, bool terminate_with_0, bool use_malloc)
{
	ALLEGRO_FILE *f = engine->get_cpa()->load(filename);
	if (f == NULL) {
		return NULL;
	}
	return slurp_from_file(f, ret_size, terminate_with_0, use_malloc);
}

// Slurp a filesystem file
unsigned char *slurp_real_file(std::string filename, int *ret_size, bool terminate_with_0, bool use_malloc)
{
	ALLEGRO_FILE *f = al_fopen(filename.c_str(), "rb");
	if (f == NULL) {
		return NULL;
	}
	unsigned char *result = slurp_from_file(f, ret_size, terminate_with_0, use_malloc);
	return result;
}

void load_mask(std::string filename, int size, unsigned char **dest)
{
	ALLEGRO_FILE *f = engine->get_cpa()->load(filename);
	if (!f) {
		*dest = NULL;
		return;
	}
	*dest = new unsigned char[size];
	int total = 0;
	do {
		total += al_fread(f, *dest, size-total);
	} while (total < size);
	al_fclose(f);
}

float distance(float x1, float y1, float x2, float y2)
{
	float dx = x1-x2;
	float dy = y1-y2;
	return sqrtf(dx*dx + dy*dy);
}

float sign(float f)
{
	if (f < 0)
		return -1;
	else if (f > 0)
		return 1;
	else
		return 0;
}

uint32_t read32bits(ALLEGRO_FILE *f)
{
	int b1 = al_fgetc(f);
	int b2 = al_fgetc(f);
	int b3 = al_fgetc(f);
	int b4 = al_fgetc(f);
	return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}

std::string readNString(ALLEGRO_FILE *f)
{
	int len = read32bits(f);
	char *data = new char[len+1];
	data[len] = 0;
	for (int i = 0; i < len; i++) {
		data[i] = al_fgetc(f);
	}
	std::string s = std::string(data);
	delete[] data;
	return s;
}

void analog_to_digital_joystick(float *a, int *d)
{
    if (a[0] == 0 && a[1] == 0) {
        d[0] = 0;
        d[1] = 0;
        return;
    }
    
	int digital[16][2] = {
		{ 1, 0 },
		{ 1, 1 },
		{ 1, 1 },
		{ 0, 1},
		{ 0, 1 },
		{ -1, 1 },
		{ -1, 1 },
		{ -1, 0 },
		{ -1, 0 },
		{ -1, -1 },
		{ -1, -1 },
		{ 0, -1 },
		{ 0, -1 },
		{ 1, -1 },
		{ 1, -1 },
		{ 1, 0 }
	};

	float angle = atan2(a[1], a[0]);
	while (angle < 0)
		angle += M_PI*2;
	while (angle >= M_PI*2)
		angle -= M_PI*2;
	int section = (int)((angle / (M_PI*2)) * 16);

	d[0] = digital[section][0];
	d[1] = digital[section][1];
}

#if defined ALLEGRO_WINDOWS
const char *strcasestr(const char *s1, const char *s2)
{
	while (*s1) {
		int i;
		if (strlen(s2) > strlen(s1))
			return NULL;
		for (i = 0; i < (int)strlen(s2); i++) {
			if (toupper(*(s1+i)) != s2[i])
				break;
		}
		if (i != (int)strlen(s2))
			return s1;
	}

	return NULL;
}
#endif

int find_arg(const char *s)
{
	for (int i = 1; i < argc && argv[i]; i++) {
		if (!strcmp(argv[i], s))
			return i;
	}
	return -1;
}

bool triangle_is_clockwise(Triangulate::Triangle *tri)
{
	float x1 = tri->points[0].x;
	float y1 = tri->points[0].y;
	float x2 = tri->points[1].x;
	float y2 = tri->points[1].y;
	float x3 = tri->points[2].x;
	float y3 = tri->points[2].y;

	if (((x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1)) < 0.0)
		return false;
	return true;
}

void tile_bitmap(Wrap::Bitmap *bitmap, ALLEGRO_COLOR tint, int x, int y, int w, int h)
{
	int bmp_w = al_get_bitmap_width(bitmap->bitmap);
	int bmp_h = al_get_bitmap_height(bitmap->bitmap);

	int wt = w / bmp_w;
	if (w % bmp_w != 0) wt++;
	int ht = h / bmp_h;
	if (h % bmp_h != 0) ht++;

	int dy = y;
	int remain_h = h;

	bool held = al_is_bitmap_drawing_held();
	if (!held) {
		al_hold_bitmap_drawing(true);
	}

	for (int y = 0; y < ht; y++) {
		int dx = x;
		int remain_w = w;
		int this_h = remain_h > bmp_h ? bmp_h : remain_h;
		for (int x = 0; x < wt; x++) {
			int this_w = remain_w > bmp_w ? bmp_w : remain_w;
			al_draw_tinted_scaled_bitmap(
				bitmap->bitmap,
				tint,
				0.25f, 0.25f, this_w-0.25f, this_h-0.25f,
				dx, dy, this_w, this_h,
				0
			);
			dx += this_w;
			remain_w -= this_w;
		}
		dy += this_h;
		remain_h -= this_h;
	}

	if (!held) {
		al_hold_bitmap_drawing(false);
	}
}

void draw_speech_window(Speech_Type type, int wx, int wy, int ww, int wh, bool draw_star, ALLEGRO_COLOR color, float opacity)
{
	if (type == SPEECH_THOUGHTS || type == SPEECH_COLORED_ROUNDED) {
		al_draw_filled_rounded_rectangle(
			wx+1, wy+1, wx+ww-1, wy+wh-1,
			10, 10,
			color
		);
	}
	else if (type == SPEECH_COLORED) {
		al_draw_filled_rectangle(
			wx+1, wy+1, wx+ww-1, wy+wh-1,
			color
		);
	}
	else if (type == SPEECH_NORMAL) {
		if (opacity  != 1) {
			Shader::use(lower_green_opacity_shader);
			al_set_shader_float("opacity", opacity);
		}
		al_hold_bitmap_drawing(true);
		speech_window->set_sub_animation("top_left");
		speech_window->get_current_animation()->draw_tinted(color, wx, wy, 0);
		speech_window->set_sub_animation("top_right");
		speech_window->get_current_animation()->draw_tinted(color, (wx+ww)-speech_window_sizes[2][0].w, wy, 0);
		speech_window->set_sub_animation("bottom_left");
		speech_window->get_current_animation()->draw_tinted(color, wx, (wy+wh)-speech_window_sizes[0][2].h, 0);
		speech_window->set_sub_animation("bottom_right");
		speech_window->get_current_animation()->draw_tinted(color, (wx+ww)-speech_window_sizes[2][0].w,
			(wy+wh)-speech_window_sizes[0][2].h, 0);

		int w = ww - speech_window_sizes[0][0].w - speech_window_sizes[2][0].w;
		int h = wh - speech_window_sizes[0][0].h - speech_window_sizes[0][2].h;

		// top
		speech_window->set_sub_animation("top_middle");
		tile_bitmap(speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap(),
			color, wx+speech_window_sizes[0][0].w, wy, w, speech_window_sizes[0][0].h);
		// left
		speech_window->set_sub_animation("middle_left");
		tile_bitmap(speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap(),
			color, wx, wy+speech_window_sizes[0][0].h, speech_window_sizes[0][0].w, h);
		// right
		speech_window->set_sub_animation("middle_right");
		tile_bitmap(speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap(),
			color, wx+speech_window_sizes[0][0].w+w, wy+speech_window_sizes[0][0].h, speech_window_sizes[2][0].w, h);
		// bottom
		speech_window->set_sub_animation("bottom_middle");
		tile_bitmap(speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap(),
			color, wx+speech_window_sizes[0][0].w, wy+speech_window_sizes[0][0].h+h, w, speech_window_sizes[0][2].h);
		// middle
		speech_window->set_sub_animation("middle_middle");
		tile_bitmap(speech_window->get_current_animation()->get_current_frame()->get_bitmap()->get_bitmap(),
			color, wx+speech_window_sizes[0][0].w, wy+speech_window_sizes[0][0].h, w, h);
		al_hold_bitmap_drawing(false);
		if (opacity != 1) {
			Shader::use(NULL);
		}
	}

/*
	if (draw_star) {
		Animation *a = speech_arrow->get_current_animation();
		Frame *f = a->get_current_frame();
		a->draw(wx+ww-f->get_width()-8, wy+wh-f->get_height()-8, 0);
	}
*/
}

ALLEGRO_FONT *get_font(Font_Type font)
{
	return fonts[(int)font];
}

static void my_draw_text(ALLEGRO_FONT *f, ALLEGRO_COLOR c, float x, float y, int flags, const char *str)
{
	ALLEGRO_USTR_INFO info;
	const ALLEGRO_USTR *u = al_ref_cstr(&info, str);
	al_draw_ustr(f, c, x, y, flags, u);
}

void draw_text(Text_Border_Type border_type, std::string text, ALLEGRO_COLOR main_color, ALLEGRO_COLOR border_color, float x, float y, int flags, Font_Type font)
{
	ALLEGRO_FONT *f = get_font(font);
	const char *str = text.c_str();

	switch (border_type) {
		case TEXT_BORDER_SQUARE:
			my_draw_text(f, border_color, x-1, y-1, flags, str);
			my_draw_text(f, border_color, x, y-1, flags, str);
			my_draw_text(f, border_color, x+1, y-1, flags, str);
			my_draw_text(f, border_color, x-1, y, flags, str);
			my_draw_text(f, border_color, x+1, y, flags, str);
			my_draw_text(f, border_color, x-1, y+1, flags, str);
			my_draw_text(f, border_color, x, y+1, flags, str);
			my_draw_text(f, border_color, x+1, y+1, flags, str);
			break;
		default:
			break;
	}

	my_draw_text(f, main_color, x, y, flags, str);
}

void draw_text(ALLEGRO_FONT *font, std::string text, ALLEGRO_COLOR color, float x, float y, int flags)
{
	my_draw_text(font, color, x, y, flags, text.c_str());
}

void draw_text(std::string text, ALLEGRO_COLOR color, float x, float y, int flags, Font_Type font)
{
	my_draw_text(get_font(font), color, x, y, flags, text.c_str());
}

void draw_text_width(int width, std::string text, ALLEGRO_COLOR color, float x, float y, int flags, Font_Type font)
{
	int cx, cy, cw, ch;
	al_get_clipping_rectangle(&cx, &cy, &cw, &ch);
	int cx2 = cx + cw;
	int cy2 = cy + ch;
	int x1 = x * cfg.screens_w;
	int y1 = y * cfg.screens_h;
	int x2 = (x + width) * cfg.screens_w;
	int y2 = (y + get_font_line_height(font)) * cfg.screens_h;
	if (cx > x1) x1 = cx;
	if (cy > y1) y1 = cy;
	if (cx2 < x2) x2 = cx2;
	if (cy2 < y2) y2 = cy2;
	if (x2 <= x1 || y2 <= y1 || x1 >= x2 || y1 >= y2) return;
	al_set_clipping_rectangle(x1, y1, x2-x1, y2-y1);
	my_draw_text(get_font(font), color, x, y, flags, text.c_str());
	al_set_clipping_rectangle(cx, cy, cw, ch);
}

void draw_text(std::string text, float x, float y, int flags, Font_Type font)
{
	my_draw_text(get_font(font), al_color_name("lightgrey"), x, y, flags, text.c_str());
}

int get_text_width(ALLEGRO_FONT *font, std::string text)
{
	return al_get_text_width(font, text.c_str());
}

int get_text_width(Font_Type font, std::string text)
{
	return al_get_text_width(get_font(font), text.c_str());
}

int get_font_line_height(Font_Type font)
{
	return al_get_font_line_height(get_font(font));
}

bool check_mask(unsigned char *mask, int pitch, int x, int y)
{
	int o = y*(pitch*8) + x;
	int byte = o / 8;
	int bit = 7 - (x % 8);

	return mask[byte] & (1 << (7-bit));
}

void iso_project(int *x, int *y, General::Point<float> iso_offset)
{
	double xinc = -(*y) / 2.0;
	double yinc = (*x) / 4.0;
	
	*x = (int)(((float)(*x)/2) + xinc);
	*y = (int)(((float)(*y)/4) + yinc);

	*x += iso_offset.x;
	*y += iso_offset.y;
}

void reverse_iso_project(int *x, int *y, General::Point<float> iso_offset)
{
	*x -= iso_offset.x;
	*y -= iso_offset.y;

	double xinc = -2.0*(*y);
	double yinc = (*x) / 2.0;

	*x = (int)((*x) - xinc);
	*y = (int)(2 * ((*y) - yinc));
}

std::string itos(int i)
{
	char buf[20];
	sprintf(buf, "%d", i);
	return std::string(buf);
}

std::string ftos(float f)
{
	char buf[40];
	sprintf(buf, "%g", f);
	return std::string(buf);
}

// width and height are in checks
// must be POT 32x32 or higher
Wrap::Bitmap *create_checkerboard_bitmap(int check_w, int check_h, int width, int height, ALLEGRO_COLOR color1, ALLEGRO_COLOR color2)
{
	ALLEGRO_STATE state;
	al_store_state(&state, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_NEW_BITMAP_PARAMETERS | ALLEGRO_STATE_BLENDER);

	// FIXME: This is required due to a bug with nvidia drivers on Linux.
	// Because of this these checkerboard bitmaps can't be translucent
	al_set_new_bitmap_format(General::noalpha_bmp_format);

	Wrap::Bitmap *b = Wrap::create_bitmap(check_w*width, check_h*height);
	ALLEGRO_DEBUG("b=%p\n", b);
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
	al_set_target_bitmap(b->bitmap);
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			ALLEGRO_COLOR *c;
			if (y % 2 == 0) {
				if (x % 2 == 0) {
					c = &color1;
				}
				else {
					c = &color2;
				}
			}
			else {
				if (x % 2 == 0) {
					c = &color2;
				}
				else {
					c = &color1;
				}
			}
			al_draw_filled_rectangle(
				x*check_w, y*check_h,
				x*check_w+check_w, y*check_h+check_h,
				*c
			);
		}
	}
	al_restore_state(&state);

	return b;
}

void draw_textured_quad(float x1, float y1, float x2, float y2,
	ALLEGRO_COLOR color, Wrap::Bitmap *texture)
{
	ALLEGRO_VERTEX v[6];

	if (x2 < x1) {
		int tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if (y2 < y1) {
		int tmp = y1;
		y1 = y2;
		y2 = tmp;
	}

	int width = (x2-x1);
	int height = (y2-y1);

	v[0].x = x1;
	v[0].y = y1;
	v[0].z = 0;
	v[0].u = 0;
	v[0].v = 0;
	v[0].color = color;

	v[1].x = x2;
	v[1].y = y1;
	v[1].z = 0;
	v[1].u = width;
	v[1].v = 0;
	v[1].color = color;

	v[2].x = x1;
	v[2].y = y2;
	v[2].z = 0;
	v[2].u = 0;
	v[2].v = height;
	v[2].color = color;

	v[3].x = x2;
	v[3].y = y1;
	v[3].z = 0;
	v[3].u = width;
	v[3].v = 0;
	v[3].color = color;

	v[4].x = x1;
	v[4].y = y2;
	v[4].z = 0;
	v[4].u = 0;
	v[4].v = height;
	v[4].color = color;

	v[5].x = x2;
	v[5].y = y2;
	v[5].z = 0;
	v[5].u = width;
	v[5].v = height;
	v[5].color = color;

	al_draw_prim(v, 0, texture->bitmap, 0, 6, ALLEGRO_PRIM_TRIANGLE_LIST);
}

std::string get_language_string(Human_Language lang)
{
	std::string language_strings[NUM_LANGUAGES] = {
		"English",
		"French"
	};

	return language_strings[(unsigned int)lang];
}

uint32_t rand()
{
#ifdef WELL512
	return WELLRNG512();
#else
	return genrand_int32();
#endif
}

uint32_t rand(int min, int max_inclusive)
{
#ifdef WELL512
	uint32_t n = General::rand();
	double f = (double)n / (uint32_t)0xffffffff;
	if (f == 1) return max_inclusive;
	return f * (max_inclusive-min+1) + min;
#else
	double d = genrand_real1();
	if (d == 1) return max_inclusive;
	return d * (max_inclusive-min+1) + min;
#endif
}
	
void srand(uint32_t s)
{
#ifdef WELL512
	::srand(s);
	uint32_t state[16];
	for (int i = 0; i < 16; i++) {
		int r = ::rand();
		state[i] = r;
	}
	WELLRNG512SEEDCONST(state);
#else
	init_genrand(s);
#endif
}

void load_bitmap_array(Wrap::Bitmap **bmps, int num, std::string start, std::string ext)
{
	for (int i = 0; i < num; i++) {
		std::string name = start + itos(i) + "." + ext;
		bmps[i] = Wrap::load_bitmap(name);
	}
}
				
void best_fit_tinted(Wrap::Bitmap *bitmap, ALLEGRO_COLOR tint)
{
	int w = al_get_bitmap_width(bitmap->bitmap);
	int h = al_get_bitmap_height(bitmap->bitmap);
	float p1 = (float)w / cfg.screen_w;
	float p2 = (float)h / cfg.screen_h;
	int dw, dh;
	if (p1 >= p2) {
		dw = cfg.screen_w;
		dh = ((float)cfg.screen_w / w) * h;
	}
	else {
		dw = ((float)cfg.screen_h / h) * w;
		dh = cfg.screen_h;
	}
	al_draw_tinted_scaled_bitmap(
		bitmap->bitmap,
		tint,
		0, 0, w, h, 
		cfg.screen_w/2-dw/2,
		cfg.screen_h/2-dh/2,
		dw, dh,
		0
	);
}

void best_fit(Wrap::Bitmap *bitmap)
{
	best_fit_tinted(bitmap, al_color_name("white"));
}

bool exists(std::string path)
{
	return engine->get_cpa()->exists(path);
}

ALLEGRO_FONT *load_font(const char *filename, int size)
{
	int old_format = al_get_new_bitmap_format();
	al_set_new_bitmap_format(font_bmp_format);
	ALLEGRO_FILE *f = engine->get_cpa()->load(filename);
	ALLEGRO_FONT *font = al_load_ttf_font_f(f, filename, size, ALLEGRO_TTF_MONOCHROME);
	al_set_new_bitmap_format(old_format);
	return font;
}

void load_fonts()
{
	for (int i = 0; i < NUM_FONTS; i++) {
		fonts[i] = NULL;
	}

	fonts[FONT_LIGHT] = General::load_font("fonts/light.ttf", 8);
	fonts[FONT_HEAVY] = General::load_font("fonts/heavy.ttf", 8);

	ALLEGRO_DEBUG("Loaded fonts");
}

void destroy_fonts()
{
	for (int i = 0; i < NUM_FONTS; i++) {
		al_destroy_font(fonts[i]);
	}
}

std::vector<std::string> split(std::string s)
{
        std::vector<std::string> params;
        int pos = 0;
        int pos2 = 0;

	int nquotes = 0;
	while ((pos = s.find('"', pos)) != (int)std::string::npos) {
		nquotes++;
		pos++;
	}

	if (nquotes % 2 == 1) {
		params.clear();
		params.push_back("error");
		return params;
	}

	pos = 0;

	while (true) {
		if (s.at(pos) == '"') {
			pos2 = pos + 1;
			pos = s.find('"', pos2);
			if (pos == (int)std::string::npos) {
				params.clear();
				params.push_back("error");
				return params;
			}
			params.push_back(s.substr(pos2, pos-pos2));
			pos += 2;
			pos2 = pos;
		}
		else {
			pos = s.find(' ', pos);
			if (pos == (int)std::string::npos) {
				break;
			}
			params.push_back(s.substr(pos2, pos-pos2));
			pos++;
			pos2 = pos;
		}
		if (pos >= (int)s.length())
			break;
	}

        if (pos2 < (int)s.length()) {
                params.push_back(s.substr(pos2, std::string::npos));
        }

        return params;
}

/*
   Return whether a polygon in 2D is concave or convex
   return 0 for incomputables eg: colinear points
          CONVEX == 1
          CONCAVE == -1
   It is assumed that the polygon is simple
   (does not intersect itself or have holes)
*/
// From Paul Bourke
int polygon_is_convex(std::vector<float> v)
{
	int i;
	int x0, x1, x2;
	int y0, y1, y2;
	int flag = 0;
	double z;

	int n = v.size();

	if (n < 6)
		return(0);

	for (i = 0; i < n; i += 2) {
		x0 = v[i % n];
		y0 = v[(i + 1) % n];
		x1 = v[(i + 2) % n];
		y1 = v[(i + 3) % n];
		x2 = v[(i + 4) % n];
		y2 = v[(i + 5) % n];

		z = (x1 - x0) * (y2 - y1);
		z -= (y1 - y0) * (x2 - x1);

		if (z < 0)
			flag |= 1;
		else if (z > 0)
			flag |= 2;
		if (flag == 3)
			return(CONCAVE);
	}
	if (flag != 0)
		return(CONVEX);
	else
		return(0);
}

static bool real_polygon_is_clockwise(std::vector<float> v)
{
	/*
	//--
	// Shift verts over if any negative
	float lowest_x = INT_MAX;
	float lowest_y = INT_MAX;
	for (size_t i = 0; i < v.size()/2; i++) {
		float x = v[i*2+0];
		float y = v[i*2+1];
		if (x < lowest_x) lowest_x = x;
		if (y < lowest_y) lowest_y = y;
	}
	// move right/down to 0
	float addx = (lowest_x < 0) ? -lowest_x : 0;
	float addy = (lowest_y < 0) ? -lowest_y : 0;
	for (size_t i = 0; i < v.size()/2; i++) {
		v[i*2+0] += addx;
		v[i*2+1] += addy;
		printf(">>>>>>>> %g %g\n", v[i*2+0], v[i*2+1]);
	}
	//--
	*/

	int convex = polygon_is_convex(v);

	if (convex == 0) {
		return false;
	}
	
	int n = v.size();

	if (convex == CONVEX) {
		for (int i = 0; i < n/2; i++) {
			float x1, y1;
			if (i == 0) {
				x1 = v[n-2];
				y1 = v[n-1];
			}
			else {
				x1 = v[i*2-2];
				y1 = v[i*2-1];
			}
			float x2 = v[i*2+0];
			float y2 = v[i*2+1];
			float x3 = v[(i*2+2) % n];
			float y3 = v[(i*2+3) % n];
			float cross = (x2-x1)*(y3-y2)-(y2-y1)*(x3-x2);
			if (cross > 0) return false;
		}
		return true;
	}
	else {
		float sum = 0;
		for (int i = 0; i < n; i += 2) {
			float x1 = v[i % n];
			float y1 = v[(i + 1) % n];
			float x2 = v[(i + 2) % n];
			float y2 = v[(i + 3) % n];

			sum += (x1*y2) - (x2*y1);
		}
		sum /= 2;

		return sum < 0;
	}
}

// "real" function seems backwards, maybe because y axis is up in one case and down in another
bool polygon_is_clockwise(std::vector<float> v)
{
	return !real_polygon_is_clockwise(v);
}

bool polygon_is_clockwise(std::vector< General::Point<float> > v, int start, int end_exclusive)
{
	int nverts = (end_exclusive-start);
	std::vector<float> fv;

	for (int i = 0; i < nverts; i++) {
		fv.push_back(v[start+i].x);
		fv.push_back(v[start+i].y);
	}

	return polygon_is_clockwise(fv);
}

void draw_triangle(Triangulate::Triangle &t, ALLEGRO_COLOR c, float xoffs, float yoffs)
{
	for (int i = 0; i < 3; i++) {
		int j = (i+1) % 3;
		al_draw_line(
			t.points[i].x+xoffs, t.points[i].y+yoffs,
			t.points[j].x+xoffs, t.points[j].y+yoffs,
			c,
			1
		);
	}
}

float angle_difference_clockwise(float a1, float a2)
{
	while (a1 < 0) a1 += M_PI*2;
	while (a2 < 0) a2 += M_PI*2;
	while (a1 > M_PI*2) a1 -= M_PI*2;
	while (a2 > M_PI*2) a2 -= M_PI*2;
	if (a2 < a1) {
		return (M_PI * 2) - (a1 - a2);
	}
	else {
		return a2 - a1;
	}
}

float angle_difference_counter_clockwise(float a1, float a2)
{
	while (a1 < 0) a1 += M_PI*2;
	while (a2 < 0) a2 += M_PI*2;
	while (a1 > M_PI*2) a1 -= M_PI*2;
	while (a2 > M_PI*2) a2 -= M_PI*2;
	if (a2 > a1) {
		return (M_PI * 2) - (a2 - a1);
	}
	else {
		return a1 - a2;
	}
}

float angle_difference(float a1, float a2)
{
	return MIN(fabs(angle_difference_clockwise(a1, a2)), fabs(angle_difference_counter_clockwise(a1, a2)));
}

float direction_to_angle(Direction dir)
{
	switch (dir) {
		case DIR_N:
			return M_PI*3/2;
		case DIR_NE:
			return M_PI*3/2+M_PI/4;
		case DIR_E:
			return 0;
		case DIR_SE:
			return M_PI/4;
		case DIR_S:
			return M_PI/2;
		case DIR_SW:
			return M_PI*3/4;
		case DIR_W:
			return M_PI;
		default: //case DIR_NW:
			return M_PI+M_PI/4;
	}
}

Direction angle_to_direction(float angle)
{
	angle += M_PI/16.0f;
	while (angle < 0.0f) angle += M_PI*2;
	while (angle >= M_PI*2) angle -= M_PI*2;

	float radians_per_sector = (M_PI*2) / 8.0f;
	int sector = angle / radians_per_sector;
	if (sector > 7) sector = 7;

	Direction d = DIR_S;

	switch (sector) {
		case 0:
			d = DIR_E;
			break;
		case 1:
			d = DIR_SE;
			break;
		case 2:
			d = DIR_S;
			break;
		case 3:
			d = DIR_SW;
			break;
		case 4:
			d = DIR_W;
			break;
		case 5:
			d = DIR_NW;
			break;
		case 6:
			d = DIR_N;
			break;
		case 7:
			d = DIR_NE;
			break;
	}

	return d;
}

bool is_hero(std::string name)
{
	if (name == "egbert" || name == "frogbert" || name == "bisou") {
		return true;
	}
	else {
		return false;
	}
}

std::string get_hero_printable_name(std::string entity_name)
{
	if (entity_name == "egbert") {
		return t("EGBERT");
	}
	else if (entity_name == "frogbert") {
		return t("FROGBERT");
	}
	else if (entity_name == "bisou") {
		return t("BISOU");
	}
	
	return "noname";
}

bool set_clipping_rectangle(int x, int y, int width, int height)
{
	x *= cfg.screens_w;
	y *= cfg.screens_h;
	width *= cfg.screens_w;
	height *= cfg.screens_h;

	ALLEGRO_BITMAP *bmp = al_get_target_bitmap();
	int w = al_get_bitmap_width(bmp);
	int h = al_get_bitmap_height(bmp);

	if (x >= w) {
		return false;
	}
	if (y >= h) {
		return false;
	}

	if (x < 0) {
		width += x;
		x = 0;
	}
	if (y < 0) {
		height += y;
		y = 0;
	}
	if (x+width > w) {
		width = w-x;
	}
	if (y+height > h) {
		height = h-y;
	}

	if (width <= 0 || height <= 0) {
		return false;
	}

	al_set_clipping_rectangle(x, y, width, height);

	return true;
}

bool ability_is_universal(std::string name)
{
	if (name == "ATTACK" || name == "JUMP") {
		return true;
	}

	return false;
}

std::string get_time_string(double seconds)
{
	if (seconds < 60) {
		return itos(seconds) + "s";
	}
	else if (seconds < 60*60) {
		return itos(seconds/60) + "m";
	}
	else {
		int hours = seconds / (60*60);
		seconds -= hours * 60 * 60;
		int minutes = seconds / 60;

		char buf[100];
		sprintf(buf, "%dh%dm", hours, minutes);

		return std::string(buf);
	}
}

void draw_wrapped_text(std::string text, ALLEGRO_COLOR color, float x, float y, float width, Font_Type font)
{
	ALLEGRO_USTR *u = al_ustr_new(text.c_str());
	int len = al_ustr_length(u);

	int last_good = 0;
	int i;

	for (i = 0; i < len;) {
		int offset = al_ustr_offset(u, i);
		uint32_t c = al_ustr_get(u, offset);
		if (c == ' ') {
			ALLEGRO_USTR *u2 = al_ustr_dup_substr(u, 0, offset);
			int w = al_get_ustr_width(get_font(font), u2);
			al_ustr_free(u2);
			if (w > width) {
				ALLEGRO_USTR *u3 = al_ustr_dup_substr(u, 0, last_good);
				al_draw_ustr(
					get_font(font),
					color,
					x, y,
					0,
					u3
				);
				al_ustr_free(u3);
				al_ustr_remove_range(u, 0, last_good+1);
				last_good = 0;
				y += get_font_line_height(font);
				i = 0;
				len = al_ustr_length(u);
				continue;
			}
			else {
				last_good = offset;
			}
		}
		i++;
	}

	int w = al_get_ustr_width(get_font(font), u);
	if (w > width) {
		ALLEGRO_USTR *u3 = al_ustr_dup_substr(u, 0, last_good);
		al_draw_ustr(
			get_font(font),
			color,
			x, y,
			0,
			u3
		);
		al_ustr_free(u3);
		al_ustr_remove_range(u, 0, last_good+1);
		y += get_font_line_height(font);
	}
	al_draw_ustr(
		get_font(font),
		color,
		x, y,
		0,
		u
	);

	al_ustr_free(u);
}

bool is_slash(char c)
{
	return (c == '/' || c == '\\');
}

std::string tolower(std::string s)
{
	std::string ret = "";
	for (int i = 0; i < (int)s.length(); i++) {
		char buf[2];
		buf[1] = 0;
		buf[0] = ::tolower(s.c_str()[i]);
		ret += buf;
	}
	return ret;
}

void calculate_toss(
	float sx,
	float sy,
	float dx,
	float dy,
	float percent,
	float *outx,
	float *outy)
{
	float dist_x = dx - sx;
	float dist_y = dy - sy;
	float distance = sqrt(dist_x*dist_x + dist_y*dist_y);
	float diameter = distance;
	float start_angle = asin(0);
	float remain_angle = M_PI - start_angle;
	float want_angle = percent * remain_angle + start_angle;
	float angle = atan2(dist_y, dist_x);
	float real_start_x = cos(angle + M_PI) * (diameter - distance) + sx;
	float real_start_y = sin(angle + M_PI) * (diameter - distance) + sy;
	float center_x = (dx + real_start_x) / 2;
	float center_y = (dy + real_start_y) / 2;
	float in_x = real_start_x - center_x;
	float in_y = real_start_y - center_y;
	float in_z = 0;
	angle += M_PI / 2.0f;
	float vx = cos(angle);
	float vy = sin(angle);
	ALLEGRO_TRANSFORM t;
	al_identity_transform(&t);
	al_rotate_transform_3d(&t, vx, vy, 0, want_angle);
	float res_x = t.m[0][0] * in_x + t.m[1][0] * in_y + t.m[2][0] * in_z + t.m[3][0];
	float res_y = t.m[0][1] * in_x + t.m[1][1] * in_y + t.m[2][1] * in_z + t.m[3][1];
	float res_z = t.m[0][2] * in_x + t.m[1][2] * in_y + t.m[2][2] * in_z + t.m[3][2];
	*outx = res_x + center_x;
	*outy = res_y + center_y - res_z;
}

void draw_poison_bubbles(General::Point<float> pos)
{
	int bmp_w = al_get_bitmap_width(poison_bmps[0]->bitmap);
	int bmp_h = al_get_bitmap_height(poison_bmps[0]->bitmap);

	for (size_t i = 0; i < poison_bubbles.size(); i++) {
		Poison_Bubble &b = poison_bubbles[i];
		General::Point<float> dest(b.pos.x + pos.x, b.pos.y + pos.y);

		al_draw_bitmap(poison_bmps[b.frame]->bitmap, dest.x-bmp_w/2, dest.y-bmp_h/2, 0);
	}
}

bool is_item(std::string name)
{
	name = tolower(name);
	return
		name == "antidote" ||
		name == "bone" ||
		name == "coin0" ||
		name == "coin1" ||
		name == "coin2" ||
		name == "dirtysock" ||
		name == "healthflask" ||
		name == "healthjar" ||
		name == "healthvial" ||
		name == "magicflask" ||
		name == "magicjar" ||
		name == "magicvial" ||
		name == "tincan";
}

std::string get_save_filename(int number)
{
	char buf[2000];
	ALLEGRO_PATH *user_path = al_get_standard_path(ALLEGRO_USER_SETTINGS_PATH);
	if (number == -1) {
		sprintf(buf, "%s/boss.lua", al_path_cstr(user_path, ALLEGRO_NATIVE_PATH_SEP));
	}
	else {
		sprintf(buf, "%s/save%d.lua", al_path_cstr(user_path, ALLEGRO_NATIVE_PATH_SEP), number);
	}
	al_destroy_path(user_path);

	return buf;
}

std::string get_download_path()
{
#ifdef ALLEGRO_ANDROID
	std::string path = std::string(get_sdcarddir()) + "/ogg";
	return path;
#else
	return get_user_resource("ogg");
#endif
}

} // end namespace General
