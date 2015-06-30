#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>
#include <allegro5/allegro_image.h>

#include "tgui2.hpp"

#include <string>

class ExFrame : public tgui::TGUIWidget {
public:
	virtual void draw(int abs_x, int abs_y) {
		al_draw_filled_rectangle(
			abs_x,
			abs_y,
			abs_x+width,
			abs_y+height,
			color
		);
	}

	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (mb == 1) {
			down = true;
			downX = abs_x;
			downY = abs_y;
		}
	}

	virtual void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (mb == 1) {
			down = false;
		}
	}

	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {
		if (down) {
			int dx = abs_x - downX;
			int dy = abs_y - downY;
			downX = abs_x;
			downY = abs_y;
			x += dx;
			y += dy;
		}
	}

	void setSize(int w, int h)
	{
		width = w;
		height = h;
	}

	void setPosition(int x, int y)
	{
		this->x = x;
		this->y = y;
	}

	ExFrame(ALLEGRO_COLOR color) :
		down(false)
	{
		this->color = color;
	}

protected:
	ALLEGRO_COLOR color;
	bool down;
	int downX, downY;
};

class ExButton : public ExFrame {
public:
	static const int PADDING = 5;

	virtual void draw(int abs_x, int abs_y) {
		ExFrame::draw(abs_x, abs_y);

		al_draw_text(
			tgui::getFont(),
			al_map_rgb(0xff, 0xff, 0xff),
			abs_x+PADDING,
			abs_y+PADDING,
			0,
			text.c_str()
		);
	}

	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {
		if (rel_x >= 0)
			exit(0);
	}

	ExButton(std::string text, ALLEGRO_COLOR color) :
		ExFrame(color)
	{
		this->text = text;
		this->color = color;
		width = al_get_text_width(
			tgui::getFont(),
			text.c_str()
		) + PADDING*2;
		height = al_get_font_line_height(
			tgui::getFont()
		) + PADDING*2;
	}

protected:
	std::string text;
};

