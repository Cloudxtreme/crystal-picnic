#ifndef SPEECH_LOOP_H
#define SPEECH_LOOP_H

#include "loop.h"
#include "general.h"
#include "area_loop.h"
#include "speech_types.h"

class Speech_Loop : public Loop {
public:
	static const int FADE_TIME = 150;
	static const int DEFAULT_CHAR_DELAY = 45;
	static const int LINES_AVAILABLE = 3;
	static const int INSET = 15;

	bool init();
	void top();
	bool handle_event(ALLEGRO_EVENT *event);
	bool logic();
	void draw();
	void post_draw();

	void set_colors(ALLEGRO_COLOR fgcolor, ALLEGRO_COLOR bgcolor);
	void set_height(int height); // -1 = full screen

	Speech_Loop(bool blocking, std::vector<int> ids, std::vector<std::string> gestures, std::vector<ALLEGRO_USTR *> text, Speech_Location loc, Speech_Type type, bool autoscroll, std::vector<Loop *> children, int entity_to_set_facing_of, General::Direction direction_entity_should_face);
	virtual ~Speech_Loop();

private:
	void process_lines();
	void set_y(int height);

	enum Window_State {
		FADING_IN,
		FADED_IN,
		FADING_OUT
	};

	enum Print_State {
		PRINTING,
		WAITING,
		DONE
	};

	enum Meta_Data_Type {
		UNDEFINED,
		DELAY,
		SPEED,
		SHAKE,
		BOLD,
		ITALICS,
		SIZE,
		COLOR
	};

	struct Meta_Data {
		Meta_Data_Type type;
		int start;
		int end;
		int milliseconds;
		ALLEGRO_COLOR color;
		int font_size;
		bool used;
	};

	Meta_Data_Type get_metadata_type(std::string s);
	void load_font(int size, bool bold);
	ALLEGRO_FONT *get_font(int size);
	std::vector<Meta_Data *> get_metadata(int index);
	int get_char_width(const char *cstr, std::vector<Meta_Data *> md,
		bool is_end_of_word);
	void draw_char(const char *str, int x, int y, std::vector<Meta_Data *> md, float alpha);
	ALLEGRO_FONT *get_font(std::vector<Meta_Data *> md);
	bool get_shake(std::vector<Meta_Data *> md);
	bool get_bold(std::vector<Meta_Data *> md);
	bool get_italic(std::vector<Meta_Data *> md);
	void handle_next(bool prev_next);
	void gesture(bool first);
	void ungesture();

	struct Font {
		int size;
		ALLEGRO_FONT *font;
	};

	Window_State wstate;
	Print_State pstate;

	int in_count;
	int out_count;
	int char_count;

	int block_num;
	int char_num;
	int char_start;
	std::vector<int> newlines;
	int line_num;
	int begin_line;

	Speech_Location loc;
	int char_delay;

	std::vector<int> ids;
	std::vector<Meta_Data> meta_data;
	std::vector<ALLEGRO_USTR *> text;
	std::vector<std::string> gestures;
	std::vector<Font> fonts;

	bool at_top;
	int y;
	int height;

	bool next;
	bool next_terminates;
	int next_touch_id;

	Speech_Type type;
	bool autoscroll;
	bool blocking;

	double end_block_time;

	Sound::Sample *dream_keys;

	int lines_available;

	std::vector<Loop *> children;

	ALLEGRO_COLOR fgcolor; // For COLORED and COLORED_ROUNDED
	ALLEGRO_COLOR bgcolor; // For COLORED and COLORED_ROUNDED

	int entity_to_set_facing_of;
	General::Direction direction_entity_should_face;

	std::string prev_gesture;
	std::string prev_entity_for_sound;
};

#endif // SPEECH_LOOP_H
