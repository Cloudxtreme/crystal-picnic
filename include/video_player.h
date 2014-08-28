#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#include <allegro5/allegro.h>

#include <string>

#include <wrap.h>

#include "general.h"

class Video_Player {
public:
	static const int FPS = 16; // FIXME?

	void start();
	bool draw(); // returns true when video is done
	void update();

	void set_offset(General::Point<float> offset);
	int get_current_frame_num();

	void reset_elapsed();

	Video_Player(std::string dirname, int buffer_size_in_frames);
	~Video_Player();

protected:
	struct Video_Frame {
		Wrap::Bitmap *bitmap;
		int frame_num;
	};

	void get_frames(int number);

	int buffer_size_in_frames;
	General::Point<float> offset;

	std::vector<Video_Frame> frames;
	int total_frames;
	int total_frames_loaded;
	double elapsed;

	bool started;

	std::string dirname;
};

#endif
