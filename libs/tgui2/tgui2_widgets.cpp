#define ALLEGRO_STATICLINK

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_color.h>

#include "tgui2.hpp"
#include "tgui2_widgets.hpp"

#include <cstdio>


static void getVisibleSubMenus(TGUI_Splitter *root, std::vector<TGUI_SubMenuItem *> &subMenus, int depth, int maxDepth);

static ALLEGRO_COLOR fore;
static ALLEGRO_COLOR back;
static ALLEGRO_COLOR back_hilite;
static ALLEGRO_COLOR back_darker;
bool colors_set = false;

static void setDefaultColors()
{
	if (colors_set) {
		return;
	}

	tguiWidgetsSetColors(al_map_rgb(0xff, 0xff, 0x00), al_map_rgb(0x80, 0x00, 0x80));
}

TGUI_Extended_Widget::TGUI_Extended_Widget() :
	resizable(false)
{
	x = 0;
	y = 0;
	tampering = true;
}

// --
	
void TGUI_Checkbox::draw(int abs_x, int abs_y)
{
	al_draw_filled_rectangle(abs_x, abs_y, abs_x+width, abs_y+height, back);
	al_draw_rectangle(abs_x+0.5f, abs_y+0.5f, abs_x+width-0.5f, abs_y+height-0.5f, fore, 1);
	if (checked) {
		al_draw_line(abs_x+0.5f, abs_y+0.5f, abs_x+width-0.5f, abs_y+height-0.5f, fore, 1);
		al_draw_line(abs_x+0.5f, abs_y+height-0.5f, abs_x+width-0.5f, abs_y+0.5f, fore, 1);
	}
}

void TGUI_Checkbox::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (rel_x >= 0) {
		checked = !checked;
	}
}

bool TGUI_Checkbox::getChecked()
{
	return checked;
}

void TGUI_Checkbox::setChecked(bool checked)
{
	this->checked = checked;
}

TGUI_Checkbox::TGUI_Checkbox(int x, int y, int w, int h, bool checked)
{
	this->x = x;
	this->y = y;
	this->width = w;
	this->height = h;
	this->checked = checked;
}

TGUI_Checkbox::~TGUI_Checkbox()
{
}

// --

void TGUI_Icon::keyDown(int keycode)
{
	if (this == tgui::getFocussedWidget() && keycode == ALLEGRO_KEY_ENTER) {
		clicked = true;
	}
}

void TGUI_Icon::joyButtonDown(int button)
{
	if (this == tgui::getFocussedWidget()) {
		clicked = true;
	}
}

bool TGUI_Icon::acceptsFocus()
{
	return true;
}

void TGUI_Icon::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	float r, g, b, a;
	al_unmap_rgba_f(clear_color, &r, &g, &b, &a);
	if (a != 0) {
		al_clear_to_color(clear_color);
	}

	al_draw_bitmap(image, abs_x+x, abs_y+y, flags);
}

void TGUI_Icon::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (rel_x >= 0 && rel_y >= 0)
		clicked = true;
}

tgui::TGUIWidget *TGUI_Icon::update()
{
	if (clicked) {
		clicked = false;
		return this;
	}

	return NULL;
}

void TGUI_Icon::setClearColor(ALLEGRO_COLOR c)
{
	clear_color = c;
}

TGUI_Icon::TGUI_Icon(ALLEGRO_BITMAP *image, int x, int y, int flags) :
	flags(flags),
	image(image),
	clicked(false)
{
	this->x = x;
	this->y = y;
	if (image) {
		this->width = al_get_bitmap_width(image);
		this->height = al_get_bitmap_height(image);
	}
	clear_color = back;
}

TGUI_Icon::~TGUI_Icon()
{
	if (image)
		al_destroy_bitmap(image);
}

// --

void TGUI_IconButton::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	al_draw_filled_rectangle(abs_x, abs_y, abs_x+width, abs_y+height,
		back);
	al_draw_rectangle(abs_x+0.5, abs_y+0.5, abs_x-0.5+width,
		abs_y-0.5+height, fore, 1);

	TGUI_Icon::draw(abs_x+ico_ofs_x, abs_y+ico_ofs_y);
}


TGUI_IconButton::TGUI_IconButton(ALLEGRO_BITMAP *image, int x, int y, 
		int width, int height, int ico_ofs_x, int ico_ofs_y, int flags)
	: TGUI_Icon(image, x, y, flags),
	ico_ofs_x(ico_ofs_x),
	ico_ofs_y(ico_ofs_y)
{
	this->width = width;
	this->height = height;
}

TGUI_IconButton::~TGUI_IconButton()
{
}

// --

void TGUI_Splitter::addCollidingChildrenToVector(std::vector<tgui::TGUIWidget *> &v, tgui::TGUIWidget *exception, int x1, int y1, int x2, int y2)
{
	for (size_t i = 0; i < widgets.size(); i++) {
		tgui::TGUIWidget *w = widgets[i];
		w->addCollidingChildrenToVector(v, exception, x1, y1, x2, y2);
		if (w == exception || !w->acceptsFocus()) {
			continue;
		}
		int _x1, _y1, _x2, _y2;
		int wx, wy;
		tgui::determineAbsolutePosition(w, &wx, &wy);
		_x1 = wx;
		_x2 = wx + w->getWidth();
		_y1 = wy;
		_y2 = wy + w->getHeight();
		if (tgui::checkBoxCollision(x1, y1, x2, y2, _x1, _y1, _x2, _y2)) {
			v.push_back(w);
		}
	}
}

bool TGUI_Splitter::getAbsoluteChildPosition(tgui::TGUIWidget *widget, int *x, int *y)
{
	for (size_t i = 0; i < widgets.size(); i++) {
		if (widgets[i] == NULL) {
			continue;
		}
		if (widgets[i] == widget) {
			int own_x, own_y;
			tgui::determineAbsolutePosition(this, &own_x, &own_y);

			int xx = 0;
			int yy = 0;
			if (direction == TGUI_HORIZONTAL) {
				for (size_t j = 0; j < i; j++) {
					xx += sizes[j];
				}
			}
			else {
				for (size_t j = 0; j < i; j++) {
					yy += sizes[j];
				}
			}
			*x = own_x + xx + widget->getX();
			*y = own_y + yy + widget->getY();

			return true;
		}
	}
	
	for (size_t i = 0; i < widgets.size(); i++) {
		if (widgets[i] == NULL) {
			continue;
		}
		if (widgets[i]->getAbsoluteChildPosition(widget, x, y)) {
			return true;
		}
	}

	return false;
}

void TGUI_Splitter::setWidth(int w)
{
	width = w;
}

void TGUI_Splitter::setHeight(int h)
{
	height = h;
}

void TGUI_Splitter::keyDown(int keycode)
{
	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];
		if (widget) {
			widget->keyDown(keycode);
		}
	}
}

void TGUI_Splitter::keyUp(int keycode)
{
	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];
		if (widget) {
			widget->keyUp(keycode);
		}
	}
}

bool TGUI_Splitter::keyChar(int keycode, int unichar)
{
	bool used = false;
	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];
		if (widget) {
			used = used || widget->keyChar(keycode, unichar);
		}
	}
	return used;
}

void TGUI_Splitter::joyButtonDown(int button)
{
	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];
		if (widget) {
			widget->joyButtonDown(button);
		}
	}
}

void TGUI_Splitter::joyButtonDownRepeat(int button)
{
	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];
		if (widget) {
			widget->joyButtonDownRepeat(button);
		}
	}
}

void TGUI_Splitter::joyButtonUp(int button)
{
	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];
		if (widget) {
			widget->joyButtonUp(button);
		}
	}
}

void TGUI_Splitter::joyAxis(int stick, int axis, float value)
{
	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];
		if (widget) {
			widget->joyAxis(stick, axis, value);
		}
	}
}

bool TGUI_Splitter::joyAxisRepeat(int stick, int axis, float value)
{
	bool used = false;
	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];
		if (widget) {
			used = used || widget->joyAxisRepeat(stick, axis, value);
		}
	}
	return used;
}

void TGUI_Splitter::set_resizable(int split, bool value)
{
	assert(split >= 0 && (unsigned)split < widgets.size()-1);

	section_is_resizable[split] = value;
}

void TGUI_Splitter::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	int xx = abs_x;
	int yy = abs_y;

	float r, g, b, a;
	al_unmap_rgba_f(clear_color, &r, &g, &b, &a);
	if (a != 0) {
		int _x, _y, _w, _h;
		al_get_clipping_rectangle(&_x, &_y, &_w, &_h);
		tgui::setClip(xx, yy, width, height);
		al_clear_to_color(clear_color);
		al_set_clipping_rectangle(_x, _y, _w, _h);
	}

	if (drawLines) {
		al_draw_rectangle(xx+0.5, yy+0.5, xx+width-0.5, yy+height-0.5, al_map_rgb(0x00, 0x00, 0x00), 1);
	}

	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];

		int w, h, xinc, yinc;
		if (direction == TGUI_VERTICAL) {
			w = width;
			h = sizes[i];
			xinc = 0;
			yinc = h;
		}
		else {
			w = sizes[i];
			h = height;
			xinc = w;
			yinc = 0;
		}

		int _x, _y, _w, _h;
		al_get_clipping_rectangle(&_x, &_y, &_w, &_h);
		tgui::setClip(xx, yy, w, h);

		if (widget) {
			widget->draw(widget->getX()+xx, widget->getY()+yy);
		}

		if (drawLines) {
			if (direction == TGUI_VERTICAL) {
				al_draw_line(xx+0.5, yy+0.5, xx+w-0.5, yy+0.5, al_map_rgb(0x00, 0x00, 0x00), 1);
			}
			else {
				al_draw_line(xx+1+0.5, yy+0.5, xx+1+0.5, yy+h-0.5, al_map_rgb(0x00, 0x00, 0x00), 1);
			}
		}

		al_set_clipping_rectangle(_x, _y, _w, _h);

		xx += xinc;
		yy += yinc;
	}
}

void TGUI_Splitter::mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (resizing >= 0) {
		resizing = -1;
		return;
	}

	bool used;

	if (rel_x < 0 || rel_y < 0)
		used = true;
	else
		used = false;
	
	int xx = 0;
	int yy = 0;

	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];

		if (direction == TGUI_VERTICAL) {
			if (!used &&
					rel_x >= xx+hpadding &&
					rel_x <= xx+width-hpadding &&
					rel_y >= yy+vpadding &&
					rel_y <= yy+sizes[i]-vpadding &&
					widget
			) {
				if (((TGUI_Extended_Widget *)widget)->getTamperingEnabled() ||
					(rel_x >= xx+widget->getX() &&
					rel_x <= xx+widget->getX()+widget->getWidth() &&
					rel_y >= yy+widget->getY() &&
					rel_y <= yy+widget->getY()+widget->getHeight())) {
					widget->mouseUp(
						rel_x-xx,
						rel_y-yy,
						abs_x,
						abs_y,
						mb
					);
					used = true;
				}
			}
			else if (widget) {
				widget->mouseUp(
					-1, -1,
					abs_x, abs_y,
					mb
				);
			}
			yy += sizes[i];
		}
		else {
			if (!used &&
					rel_x >= xx+hpadding &&
					rel_x <= xx+sizes[i]-hpadding &&
					rel_y >= yy+vpadding &&
					rel_y <= yy+height-vpadding &&
					widget
			) {
				if (((TGUI_Extended_Widget *)widget)->getTamperingEnabled() ||
					(rel_x >= xx+widget->getX() &&
					rel_x <= xx+widget->getX()+widget->getWidth() &&
					rel_y >= yy+widget->getY() &&
					rel_y <= yy+widget->getY()+widget->getHeight())) {
					widget->mouseUp(
						rel_x-xx,
						rel_y-yy,
						abs_x,
						abs_y,
						mb
					);
					used = true;
				}
			}
			else if (widget) {
				widget->mouseUp(
					-1, -1,
					abs_x, abs_y,
					mb
				);
			}
			xx += sizes[i];
		}
	}
}

void TGUI_Splitter::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	bool used;

	if (rel_x < 0 || rel_y < 0)
		used = true;
	else
		used = false;
	
	int xx = 0;
	int yy = 0;

	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];

		if (direction == TGUI_VERTICAL) {
			if (!used &&
					rel_x >= xx &&
					rel_x <= xx+width &&
					rel_y >= yy+(sizes[i]-5) &&
					rel_y <= yy+sizes[i] &&
					i < (widgets.size()-1) &&
					section_is_resizable[i]
			) {
				resizing = i;
				last_resize_x = abs_x;
				last_resize_y = abs_y;
				used = true;
			}
			else if (!used &&
					rel_x >= xx+hpadding &&
					rel_x <= xx+width-hpadding &&
					rel_y >= yy+vpadding &&
					rel_y <= yy+sizes[i]-vpadding &&
					widget
			) {
				if (((TGUI_Extended_Widget *)widget)->getTamperingEnabled() ||
					(rel_x >= xx+widget->getX() &&
					rel_x <= xx+widget->getX()+widget->getWidth() &&
					rel_y >= yy+widget->getY() &&
					rel_y <= yy+widget->getY()+widget->getHeight())) {
					tgui::setFocus(widget);
					widget->mouseDown(
						rel_x-xx,
						rel_y-yy,
						abs_x,
						abs_y,
						mb
					);
					used = true;
				}
			}
			else if (widget) {
				widget->mouseDown(
					-1, -1,
					abs_x, abs_y,
					mb
				);
			}
			yy += sizes[i];
		}
		else {
			if (!used &&
					rel_x >= xx+(sizes[i]-5) &&
					rel_x <= xx+sizes[i] &&
					rel_y >= yy &&
					rel_y <= yy+height &&
					i < (widgets.size()-1) &&
					section_is_resizable[i]
			) {
				resizing = i;
				last_resize_x = abs_x;
				last_resize_y = abs_y;
				used = true;
			}
			else if (!used &&
					rel_x >= xx+hpadding &&
					rel_x <= xx+sizes[i]-hpadding &&
					rel_y >= yy+vpadding &&
					rel_y <= yy+height-vpadding &&
					widget
			) {
				if (((TGUI_Extended_Widget *)widget)->getTamperingEnabled() ||
					(rel_x >= xx+widget->getX() &&
					rel_x <= xx+widget->getX()+widget->getWidth() &&
					rel_y >= yy+widget->getY() &&
					rel_y <= yy+widget->getY()+widget->getHeight())) {
					tgui::setFocus(widget);
					widget->mouseDown(
						rel_x-xx,
						rel_y-yy,
						abs_x,
						abs_y,
						mb
					);
					used = true;
				}
			}
			else if (widget) {
				widget->mouseDown(
					-1, -1,
					abs_x, abs_y,
					mb
				);
			}
			xx += sizes[i];
		}
	}
}

void TGUI_Splitter::mouseDownAll(tgui::TGUIWidget *leftOut, int abs_x, int abs_y, int mb)
{
	int rel_x = abs_x - x;
	int rel_y = abs_y - y;

	int xx = x;
	int yy = y;

	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];
		bool is_left_out = false;

		if (direction == TGUI_VERTICAL) {
			if (
					rel_x >= xx &&
					rel_x <= xx+width &&
					rel_y >= yy &&
					rel_y <= yy+sizes[i]
			) {
				is_left_out = true;
			}
			yy += sizes[i];
		}
		else {
			if (
					rel_x >= xx &&
					rel_x <= xx+sizes[i] &&
					rel_y >= yy &&
					rel_y <= yy+height
			) {
				is_left_out = true;
			}
			xx += sizes[i];
		}
	
		if (widget) {
			widget->mouseDownAll(is_left_out ? widget : 0, abs_x, abs_y, mb);
		}
	}
}

void TGUI_Splitter::mouseMoveAll(tgui::TGUIWidget *leftOut, int abs_x, int abs_y)
{
	if (resizing >= 0) {
		int delta;
		if (direction == TGUI_VERTICAL) {
			delta = abs_y - last_resize_y;
		}
		else {
			delta = abs_x - last_resize_x;
		}
		if (sizes[resizing]+delta < 10)
			return;
		for (unsigned int i = 0; i < sizes.size(); i++) {
			if (i == (unsigned)resizing)
				continue;
			if (i < sizes.size()-1) {
				if (!((section_is_resizable[i]) ||
					(i > 0 && section_is_resizable[i-1]))) {
					continue;
				}
			}
			if (sizes[i]-delta < 10)
				return;
		}
		set_size(resizing, sizes[resizing]+delta);
		last_resize_x = abs_x;
		last_resize_y = abs_y;
	}
	else {
		for (unsigned int i = 0; i < widgets.size(); i++) {
			TGUIWidget *widget = widgets[i];
			if (widget) {
				widget->mouseMoveAll(leftOut, abs_x, abs_y);
			}
		}
	}
}

void TGUI_Splitter::mouseMove(int rel_x, int rel_y, int abs_x, int abs_y)
{
	bool used;

	if (rel_x < 0 || rel_y < 0)
		used = true;
	else
		used = false;

	int xx = 0;
	int yy = 0;
	
	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];

		if (direction == TGUI_VERTICAL) {
			if (!used &&
					rel_x >= xx+hpadding &&
					rel_x <= xx+width-hpadding &&
					rel_y >= yy+vpadding &&
					rel_y <= yy+sizes[i]-vpadding &&
					widget
			) {
				static ALLEGRO_SYSTEM_MOUSE_CURSOR cursor = ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT;
				ALLEGRO_SYSTEM_MOUSE_CURSOR current_cursor = cursor;
				if (
					i < widgets.size()-1 &&
					section_is_resizable[i] &&
					rel_y >= yy+(sizes[i]-5)
				) {
					current_cursor =
					ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_S;
				}
				else {
					current_cursor =
					ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT;
				}
				if (cursor != current_cursor) {
					al_set_system_mouse_cursor(tgui::getDisplay(), current_cursor);
					cursor = current_cursor;
				}
				if (((TGUI_Extended_Widget *)widget)->getTamperingEnabled() ||
					(rel_x >= xx+widget->getX() &&
					rel_x <= xx+widget->getX()+widget->getWidth() &&
					rel_y >= yy+widget->getY() &&
					rel_y <= yy+widget->getY()+widget->getHeight())) {
					widget->mouseMove(
						rel_x-xx,
						rel_y-yy,
						abs_x,
						abs_y
					);
					used = true;
				}
			}
			else if (widget) {
				widget->mouseMove(
					-1, -1,
					abs_x, abs_y
				);
			}
			yy += sizes[i];
		}
		else {
			if (!used &&
					rel_x >= xx+hpadding &&
					rel_x <= xx+sizes[i]-hpadding &&
					rel_y >= yy+vpadding &&
					rel_y <= yy+height-vpadding &&
					widget
			) {
				static ALLEGRO_SYSTEM_MOUSE_CURSOR cursor = ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT;
				ALLEGRO_SYSTEM_MOUSE_CURSOR current_cursor = cursor;
				if (
					i < widgets.size()-1 &&
					section_is_resizable[i] &&
					rel_x >= xx+(sizes[i]-5)
				) {
					current_cursor =
					ALLEGRO_SYSTEM_MOUSE_CURSOR_RESIZE_E;
				}
				else {
					current_cursor =
					ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT;
				}
				if (cursor != current_cursor) {
					al_set_system_mouse_cursor(tgui::getDisplay(), current_cursor);
					cursor = current_cursor;
				}
				al_set_system_mouse_cursor(tgui::getDisplay(), cursor);
				if (((TGUI_Extended_Widget *)widget)->getTamperingEnabled() ||
					(rel_x >= xx+widget->getX() &&
					rel_x <= xx+widget->getX()+widget->getWidth() &&
					rel_y >= yy+widget->getY() &&
					rel_y <= yy+widget->getY()+widget->getHeight())) {
					widget->mouseMove(
						rel_x-xx,
						rel_y-yy,
						abs_x,
						abs_y
					);
					used = true;
				}
			}
			else if (widget) {
				widget->mouseMove(
					-1, -1,
					abs_x, abs_y
				);
			}
			xx += sizes[i];
		}
	}
}

tgui::TGUIWidget *TGUI_Splitter::update()
{
	for (unsigned int i = 0; i < widgets.size(); i++) {
		if (!widgets[i])
			continue;
		TGUIWidget *widget = widgets[i]->update();
		if (widget)
			return widget;
	}
	return NULL;
}

int TGUI_Splitter::get_size(int index)
{
	assert(index >= 0 && (unsigned)index < widgets.size());

	return sizes[index];
}

void TGUI_Splitter::set_size(int index, int size)
{
	int difference = sizes[index] - size;
	sizes[index] = size;

	int total = 0;
	for (unsigned int i = 0; i < sizes.size(); i++) {
		total += sizes[i];
	}
	if (direction == TGUI_VERTICAL) {
		if (total == height)
			return;
	}
	else {
		if (total == width)
			return;
	}

	for (unsigned int i = 0; i < sizes.size(); i++) {
		if (i == (unsigned)index)
			continue;
		if (i < sizes.size()-1) {
			if (!((section_is_resizable[i]) ||
				(i > 0 && section_is_resizable[i-1]))) {
				continue;
			}
		}
		if (difference > 0) {
			sizes[i] += difference;
			break;
		}
		if (sizes[i] > 1) {
			int sub = MIN(abs(difference), sizes[i]-1);
			sizes[i] -= sub;
			difference += sub;
		}
	}
	
	layout();
}

void TGUI_Splitter::set_widget(int index, TGUIWidget *widget)
{
	widgets[index] = widget;
}

void TGUI_Splitter::setClearColor(ALLEGRO_COLOR c)
{
	clear_color = c;
}

void TGUI_Splitter::layout()
{
	int xx = x;
	int yy = y;

	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];

		if (widget) {
			if (direction == TGUI_VERTICAL) {
				widget->setWidth(width-(hpadding*2));
				widget->setHeight(sizes[i]-(vpadding*2));
			}
			else {
				widget->setWidth(sizes[i]-(hpadding*2));
				widget->setHeight(height-(vpadding*2));
			}

			TGUI_Splitter *s = dynamic_cast<TGUI_Splitter *>(widget);
			if (s) {
				s->layout();
			}
			else {
				dummies[i]->setX(xx);
				dummies[i]->setY(yy);
			}
		}

		if (direction == TGUI_VERTICAL) {
			yy += sizes[i];
		}
		else {
			xx += sizes[i];
		}
	}
}

void TGUI_Splitter::setPadding(int hpadding, int vpadding)
{
	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];

		if (widget) {
			widget->setX(widget->getX()-this->hpadding);
			widget->setY(widget->getY()-this->vpadding);
		}
	}

	this->hpadding = hpadding;
	this->vpadding = vpadding;
	
	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUIWidget *widget = widgets[i];

		if (widget) {
			widget->setX(widget->getX()+this->hpadding);
			widget->setY(widget->getY()+this->vpadding);
		}
	}

	layout();
}

std::vector<tgui::TGUIWidget *> &TGUI_Splitter::getWidgets()
{
	return widgets;
}

void TGUI_Splitter::setDrawLines(bool d)
{
	drawLines = d;
}

TGUI_Splitter::TGUI_Splitter(
	int x, int y,
	int w, int h,
	TGUI_Direction dir,
	bool can_resize,
	std::vector<TGUIWidget *> widgets
) :
	direction(dir),
	weighted_resize(false),
	widgets(widgets),
	resizing(-1),
	drawLines(true),
	hpadding(0),
	vpadding(0)
{
	this->x = x;
	this->y = y;
	width = w;
	height = h;

	resizable = can_resize;

	const int num_widgets = widgets.size();
	const int total_size = direction == TGUI_VERTICAL ? height : width;

	for (int i = 0; i < num_widgets; i++) {
		sizes.push_back(total_size / num_widgets);
		section_resize_weight.push_back(1.0f / num_widgets);
		section_is_resizable.push_back(can_resize);

			TGUI_DummyWidget *dummy = new TGUI_DummyWidget(0, 0);
			dummies.push_back(dummy);
		if (widgets[i]) {
			widgets[i]->setParent(dummy);
		}
	}

	clear_color = al_map_rgba_f(0, 0, 255, 255);

	layout();
}
/*
	TGUI_Direction direction;
	bool weighted_resize;

	std::vector<tgui::TGUIWidget *> widgets;
	std::vector<TGUI_DummyWidget *> dummies;
	std::vector<int> sizes;
	std::vector<float> section_resize_weight;
	std::vector<bool> section_is_resizable; // 1 less then widgets.size()
	ALLEGRO_COLOR clear_color;
	int resizing;
	int last_resize_x, last_resize_y;
	bool drawLines;
*/

TGUI_Splitter::~TGUI_Splitter()
{
	for (unsigned int i = 0; i < dummies.size(); i++) {
		delete dummies[i];
	}
}

// --

int TGUI_TextMenuItem::getShortcutKeycode()
{
	return shortcut_keycode;
}

void TGUI_TextMenuItem::setMenuBar(TGUI_MenuBar *mb)
{
	menuBar = mb;
}
	
void TGUI_TextMenuItem::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	ALLEGRO_COLOR fore;
	ALLEGRO_COLOR back;
	
	fore = al_map_rgb(0x00, 0x00, 0x00);

	if (hover) {
		back = al_map_rgb(0xff, 0xff, 0xff);
	}
	else {
		back = ::fore;
	}

	al_clear_to_color(back);
	al_draw_text(tgui::getFont(), fore, abs_x+HEIGHT, abs_y, 0,
		name.c_str());
	if (shortcut_keycode) {
		const char *str = al_keycode_to_name(shortcut_keycode);
		if (strlen(str) == 1) {
			al_draw_textf(tgui::getFont(), fore, abs_x+width-HEIGHT, abs_y, ALLEGRO_ALIGN_RIGHT, "%s",
				al_keycode_to_name(shortcut_keycode));
			al_draw_textf(tgui::getFont(), fore, abs_x+width-HEIGHT-al_get_font_line_height(tgui::getFont()), abs_y, ALLEGRO_ALIGN_RIGHT, "Ctrl-");
		}
		else {
			al_draw_textf(tgui::getFont(), fore, abs_x+width-HEIGHT, abs_y, ALLEGRO_ALIGN_RIGHT, "Ctrl-%s",
				al_keycode_to_name(shortcut_keycode));
		}
	}
	
	// distinction for sub menus
	al_draw_line(abs_x+1+0.5, abs_y+0.5, abs_x+1+0.5, abs_y+height-0.5, al_map_rgb(0xff, 0xff, 0xff), 1);
	al_draw_line(abs_x+2+0.5, abs_y+0.5, abs_x+2+0.5, abs_y+height-0.5, al_map_rgb(0xff, 0xff, 0xff), 1);
}

tgui::TGUIWidget *TGUI_TextMenuItem::update()
{
	if (clicked) {
		clicked = false;
		menuBar->close();
		return this;
	}
	
	return NULL;
}

void TGUI_TextMenuItem::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (rel_x >= 0 && rel_y >= 0) {
		clicked = true;
	}
}

void TGUI_TextMenuItem::mouseMove(int rel_x, int rel_y, int abs_x, int abs_y)
{
	if (rel_x < 0 || rel_y < 0)
		hover = false;
	else
		hover = true;
}

void TGUI_TextMenuItem::mouseMoveAll(tgui::TGUIWidget *leftOut, int abs_x, int abs_y)
{
	// do nothing except override default behaviour
}

TGUI_TextMenuItem::TGUI_TextMenuItem(std::string name, int shortcut_keycode) :
	name(name),
	shortcut_keycode(shortcut_keycode),
	clicked(false),
	hover(false)
{
}

// --
	
void TGUI_CheckMenuItem::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	TGUI_TextMenuItem::draw(abs_x, abs_y);

	ALLEGRO_COLOR fore;

	if (hover) {
		fore = al_map_rgb(0xff, 0xff, 0xff);
	}
	else {
		fore = al_map_rgb(0x00, 0x00, 0x00);
	}
	
	al_draw_rectangle(abs_x+3.5, abs_y+3.5, abs_x+HEIGHT-3.5, abs_y+HEIGHT-3.5, fore, 1);
	
	// Draw check
	if (checked) {
		for (int d = 3; d < 5; d++) {
			const int check_height = HEIGHT/2-1;
			int yy = abs_y + HEIGHT/2;
			for (int i = 0; i < check_height/2; i++) {
				al_draw_pixel(abs_x+i+d, yy+i, fore);
			}
			yy = abs_y + HEIGHT/2 + check_height/2;
			for (int i = 0; i < check_height; i++) {
				al_draw_pixel(abs_x+check_height/2+i+d, yy-i, fore);
			}
		}
	}
}

tgui::TGUIWidget *TGUI_CheckMenuItem::update()
{
	if (clicked) {
		checked = !checked;
	}

	return TGUI_TextMenuItem::update();
}

bool TGUI_CheckMenuItem::isChecked()
{
	return checked;
}

void TGUI_CheckMenuItem::setChecked(bool checked)
{
	this->checked = checked;
}
	
TGUI_CheckMenuItem::TGUI_CheckMenuItem(std::string name, int shortcut_keycode, bool checked) :
	TGUI_TextMenuItem(name, shortcut_keycode),
	checked(checked)
{
}

// --
void TGUI_RadioMenuItem::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	TGUI_TextMenuItem::draw(abs_x, abs_y);

	ALLEGRO_COLOR fore;

	if (hover) {
		fore = al_map_rgb(0xff, 0xff, 0xff);
	}
	else {
		fore = al_map_rgb(0x00, 0x00, 0x00);
	}

	// draw circle
	const int OFFSET = height/2+1;
	al_draw_circle(abs_x+OFFSET, abs_y+OFFSET, 6, fore, 1);
	if (group->selected == id) {
		al_draw_filled_circle(abs_x+OFFSET, abs_y+OFFSET, 4, fore);
	}
}

tgui::TGUIWidget *TGUI_RadioMenuItem::update()
{
	if (clicked) {
		group->selected = id;
		clicked = false;
		menuBar->close();
		return this;
	}

	return NULL;
}

bool TGUI_RadioMenuItem::isSelected()
{
	return group->selected == id;
}

void TGUI_RadioMenuItem::setSelected()
{
	group->selected = id;
}

TGUI_RadioMenuItem::TGUI_RadioMenuItem(std::string name, int shortcut_keycode, TGUI_RadioGroup *group, int id) :
	TGUI_TextMenuItem(name, 0),
	group(group),
	id(id)
{
}

// --

void TGUI_SubMenuItem::setParentSplitter(TGUI_Splitter *splitter)
{
	parentSplitter = splitter;
}

void TGUI_SubMenuItem::close()
{
	if (!is_open)
		return;
	
	std::vector<TGUI_SubMenuItem *> subMenus;
	getVisibleSubMenus(sub_menu, subMenus, 0, 0);
	for (unsigned int i = 0; i < subMenus.size(); i++) {
		subMenus[i]->close();
	}
	sub_menu->remove();
	
	is_open = false;
}


void TGUI_SubMenuItem::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	TGUI_TextMenuItem::draw(abs_x, abs_y);

	ALLEGRO_COLOR fore;

	if (hover) {
		fore = al_map_rgb(0xff, 0xff, 0xff);
	}
	else {
		fore = al_map_rgb(0x00, 0x00, 0x00);
	}

	al_draw_text(tgui::getFont(), fore, abs_x+x+width-al_get_text_width(tgui::getFont(), ">")-5, abs_y+y, 0, ">");
}

void TGUI_SubMenuItem::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
}

void TGUI_SubMenuItem::mouseMove(int rel_x, int rel_y, int abs_x, int abs_y)
{
	if (rel_x < 0 || rel_y < 0) {
		hover = false;
	}
	else {
		hover = true;
	}

	if (is_open && (rel_x < 0 || rel_y < 0)) {
		close();
	}
	else if (!is_open && (rel_x >= 0 && rel_y >= 0)) {
		sub_menu->setX((parent ? parent->getX() : 0)+x+width);
		sub_menu->setY((parent ? parent->getY() : 0)+y);
		tgui::addWidget(sub_menu);
		is_open = true;

		std::vector<tgui::TGUIWidget *> neighbors = parentSplitter->getWidgets();
		for (unsigned int i = 0; i < neighbors.size(); i++) {
			TGUI_SubMenuItem *item = dynamic_cast<TGUI_SubMenuItem *>(neighbors[i]);
			if (item && this != item) {
				item->close();
			}
		}
	}
}

bool TGUI_SubMenuItem::isOpen()
{
	return is_open;
}

TGUI_Splitter *TGUI_SubMenuItem::getSubMenu()
{
	return sub_menu;
}

void TGUI_SubMenuItem::setSubMenu(TGUI_Splitter *sub)
{
	sub_menu = sub;
}

TGUI_SubMenuItem::TGUI_SubMenuItem(std::string name, TGUI_Splitter *sub_menu) :
	TGUI_TextMenuItem(name, 0),
	is_open(false),
	sub_menu(sub_menu)
{
}

// --

void TGUI_MenuBar::checkKeys(int keycode, TGUI_Splitter *splitter)
{
	std::vector<tgui::TGUIWidget *> &w = splitter->getWidgets();
	for (unsigned int i = 0; i < w.size(); i++) {
		TGUI_TextMenuItem *item = dynamic_cast<TGUI_TextMenuItem *>(w[i]);
		if (item) {
			if (keycode == item->getShortcutKeycode()) {
				itemToReturn = item;
				return;
			}
		}
		else {
			TGUI_SubMenuItem *sub = dynamic_cast<TGUI_SubMenuItem *>(w[i]);
			if (sub) {
				checkKeys(keycode, sub->getSubMenu());
			}
		}
		if (itemToReturn)
			return;
	}
}

void TGUI_MenuBar::keyDown(int keycode)
{
	if (tgui::isKeyDown(ALLEGRO_KEY_LCTRL) == false &&
	    tgui::isKeyDown(ALLEGRO_KEY_RCTRL) == false)
	    return;

	for (unsigned int i = 0; i < menus.size(); i++) {
		checkKeys(keycode, menus[i]);
	}
}

void TGUI_MenuBar::setSubMenuSplitters(TGUI_Splitter *root)
{
	std::vector<tgui::TGUIWidget *> &w = root->getWidgets();
	for (unsigned int i = 0; i < w.size(); i++) {
		TGUI_SubMenuItem *item = dynamic_cast<TGUI_SubMenuItem *>(w[i]);
		if (item) {
			item->setParentSplitter(root);
			setSubMenuSplitters(item->getSubMenu());
		}
	}
}

void TGUI_MenuBar::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	al_clear_to_color(back);

	int xx = abs_x+PADDING;
	
	for (unsigned int i = 0; i < menu_names.size(); i++) {
		std::string name = menu_names[i];
		al_draw_text(tgui::getFont(), al_map_rgb(0xff, 0xff, 0xff), xx, abs_y, 0,
			name.c_str());
		int len = al_get_text_width(tgui::getFont(), name.c_str());
		xx += len + PADDING;
	}
}

void TGUI_MenuBar::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (rel_x < 0 || rel_y < 0)
		return;

	if (open_menu) {
		close();
		return;
	}
	
	int xx = x+PADDING;
	
	for (unsigned int i = 0; i < menu_names.size(); i++) {
		std::string name = menu_names[i];
		int len = al_get_text_width(tgui::getFont(), name.c_str());
		if (rel_x >= xx && rel_x <= xx+len) {
			open_menu = menus[i];
			open_menu->setX(xx);
			open_menu->setY(y+height);
			open_menu->layout();
			tgui::setNewWidgetParent(NULL);
			tgui::addWidget(open_menu);
			break;
		}
		xx += len + PADDING;
	}
}

void TGUI_MenuBar::close()
{
	// close submenus first
	std::vector<TGUI_SubMenuItem *> subMenus;
	getVisibleSubMenus(open_menu, subMenus, 0, 0);
	for (unsigned int i = 0; i < subMenus.size(); i++) {
		TGUI_SubMenuItem *sub = subMenus[i];
		sub->close();
	}
	close_menu = false;
	open_menu->remove();
	open_menu = NULL;
}

tgui::TGUIWidget *TGUI_MenuBar::update()
{
	if (itemToReturn) {
		tgui::TGUIWidget *tmp = itemToReturn;
		itemToReturn = NULL;
		return tmp;
	}

	return NULL;
}

TGUI_MenuBar::TGUI_MenuBar(
	int x, int y, int w, int h,
	std::vector<std::string> menu_names,
	std::vector<TGUI_Splitter *> menus
) :
	menu_names(menu_names),
	menus(menus),
	open_menu(NULL),
	close_menu(false),
	itemToReturn(NULL)
{
	this->x = x;
	this->y = y;
	this->width = w;
	this->height = h;

	for (unsigned int i = 0; i < menus.size(); i++) {
		std::vector<tgui::TGUIWidget *> w = menus[i]->getWidgets();
		for (unsigned int j = 0; j < w.size(); j++) {
			TGUI_TextMenuItem *item = dynamic_cast<TGUI_TextMenuItem *>(w[j]);
			item->setMenuBar(this);
		}
		setSubMenuSplitters(menus[i]);
	}
}

// --

void TGUI_ScrollPane::chainDraw()
{
	int abs_x, abs_y;
	determineAbsolutePosition(this, &abs_x, &abs_y);
	draw(abs_x, abs_y);
}


tgui::TGUIWidget *TGUI_ScrollPane::chainMouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (pointOnWidget(this, abs_x, abs_y)) {
		mouseDown(rel_x, rel_y, abs_x, abs_y, mb);
		return this;
	}
	return NULL;
}

void TGUI_ScrollPane::keyDown(int keycode)
{
	child->keyDown(keycode);
}

void TGUI_ScrollPane::keyUp(int keycode)
{
	child->keyUp(keycode);
}
	
bool TGUI_ScrollPane::keyChar(int keycode, int unichar)
{
	return child->keyChar(keycode, unichar);
}
	
void TGUI_ScrollPane::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	int offsx = (child->getWidth()-(width-SCROLLBAR_THICKNESS)) * ox;
	int offsy = (child->getHeight()-(height-SCROLLBAR_THICKNESS)) * oy;

	if (offsx < 0) offsx = 0;
	if (offsy < 0) offsy = 0;

	int _x, _y, _w, _h;
	al_get_clipping_rectangle(&_x, &_y, &_w, &_h);
	tgui::setClip(abs_x, abs_y, width, height);

	child->draw(abs_x-offsx, abs_y-offsy);

	al_set_clipping_rectangle(_x, _y, _w, _h);

	int x1, y1, x2, y2;
	get_vtab_details(&x1, &y1, &x2, &y2);

	// vertical scrollbar
	al_draw_filled_rectangle(
		abs_x+width-SCROLLBAR_THICKNESS,
		abs_y,
		abs_x+width,
		abs_y+height-SCROLLBAR_THICKNESS,
		back
	);
	al_draw_filled_rectangle(
		abs_x+x1,
		abs_y+y1,
		abs_x+x2,
		abs_y+y2,
		back_darker
	);
	
	get_htab_details(&x1, &y1, &x2, &y2);

	// horizontal scrollbar
	al_draw_filled_rectangle(
		abs_x,
		abs_y+height-SCROLLBAR_THICKNESS,
		abs_x+width-SCROLLBAR_THICKNESS,
		abs_y+height,
		back
	);
	al_draw_filled_rectangle(
		abs_x+x1,
		abs_y+y1,
		abs_x+x2,
		abs_y+y2,
		back_darker
	);
}

void TGUI_ScrollPane::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (rel_x < 0 || rel_y < 0)
		return;

	int x1, y1, x2, y2;

	get_vtab_details(&x1, &y1, &x2, &y2);
	if (rel_x >= x1 && rel_x <= x2 && rel_y >= y1 && rel_y <= y2) {
		down = DOWN_V;
		down_x = abs_x;
		down_y = abs_y;
		down_ox = ox;
		down_oy = oy;
		return;
	}

	get_htab_details(&x1, &y1, &x2, &y2);
	if (rel_x >= x1 && rel_x <= x2 && rel_y >= y1 && rel_y <= y2) {
		down = DOWN_H;
		down_x = abs_x;
		down_y = abs_y;
		down_ox = ox;
		down_oy = oy;
		return;
	}

	int xx, yy;
	get_pixel_offsets(&xx, &yy);

	child->mouseDown(rel_x+xx, rel_y+yy, abs_x, abs_y, mb);
}

void TGUI_ScrollPane::mouseMove(int rel_x, int rel_y, int abs_x, int abs_y)
{
	int xx, yy;
	get_pixel_offsets(&xx, &yy);
	child->mouseMove(xx+rel_x, yy+rel_y, abs_x, abs_y);
}

void TGUI_ScrollPane::mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	down = DOWN_NONE;

	int xx, yy;
	get_pixel_offsets(&xx, &yy);

	child->mouseUp(xx+rel_x, yy+rel_y, abs_x, abs_y, mb);
}

void TGUI_ScrollPane::mouseMoveAll(TGUIWidget *leftOut, int abs_x, int abs_y)
{
	if (down == DOWN_V) {
		int range = get_scrollbar_range(child->getHeight(), height-SCROLLBAR_THICKNESS);
		int pix_moved = abs_y - down_y;
		float percent_moved = (float)pix_moved / range;
		oy = down_oy + percent_moved;
		if (oy < 0) oy = 0;
		if (oy > 1) oy = 1;
	}
	else if (down == DOWN_H) {
		int range = get_scrollbar_range(child->getWidth(), width-SCROLLBAR_THICKNESS);
		int pix_moved = abs_x - down_x;
		float percent_moved = (float)pix_moved / range;
		ox = down_ox + percent_moved;
		if (ox < 0) ox = 0;
		if (ox > 1) ox = 1;
	}
	else {
		child->mouseMoveAll(leftOut, abs_x, abs_y);
	}
}

void TGUI_ScrollPane::get_values(float *ox, float *oy)
{
	*ox = this->ox;
	*oy = this->oy;
}

void TGUI_ScrollPane::setValues(float ox, float oy)
{
	this->ox = ox;
	this->oy = oy;
}

void TGUI_ScrollPane::get_pixel_offsets(int *xx, int *yy)
{
	int maxx = child->getWidth() - (width-SCROLLBAR_THICKNESS);
	if (maxx < 0) maxx = 0;

	int maxy = child->getHeight() - (height-SCROLLBAR_THICKNESS);
	if (maxy < 0) maxy = 0;

	*xx = ox * maxx;
	*yy = oy * maxy;
}

int TGUI_ScrollPane::get_scrollbar_size(int content_size, int scrollbar_size)
{
	if (content_size <= scrollbar_size)
		return scrollbar_size;
	
	int size = (int)((float)scrollbar_size / content_size * scrollbar_size);

	if (size < MIN_SCROLLBAR_SIZE)
		size = MIN_SCROLLBAR_SIZE;
	
	return size;
}

int TGUI_ScrollPane::get_scrollbar_range(int content_size, int scrollbar_size)
{
	if (content_size <= scrollbar_size)
		return 0;
	int size = get_scrollbar_size(content_size, scrollbar_size);
	return scrollbar_size - size;
}

void TGUI_ScrollPane::get_vtab_details(int *x1, int *y1, int *x2, int *y2)
{
	int scrollbar_range_y = get_scrollbar_range(child->getHeight(), height-SCROLLBAR_THICKNESS);
	int vsize = get_scrollbar_size(child->getHeight(), height-SCROLLBAR_THICKNESS);
	*x1 = width-SCROLLBAR_THICKNESS;
	*y1 = scrollbar_range_y*oy;
	*x2 = width;
	*y2 = scrollbar_range_y*oy+vsize;
}

void TGUI_ScrollPane::get_htab_details(int *x1, int *y1, int *x2, int *y2)
{
	int scrollbar_range_x = get_scrollbar_range(child->getWidth(), width-SCROLLBAR_THICKNESS);
	int hsize = get_scrollbar_size(child->getWidth(), width-SCROLLBAR_THICKNESS);
	*x1 = scrollbar_range_x*ox;
	*y1 = height-SCROLLBAR_THICKNESS;
	*x2 = scrollbar_range_x*ox+hsize;
	*y2 = height;
}

TGUI_ScrollPane::TGUI_ScrollPane(tgui::TGUIWidget *child) :
	ox(0),
	oy(0),
	down(DOWN_NONE)
{
	this->child = child;
	child->setParent(this);
}
	
// --

void TGUI_Slider::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	int x1, y1, w, h;
	int lx, ly, lw, lh;

	if (direction == TGUI_HORIZONTAL) {
		x1 = abs_x + pos*(size-TAB_SIZE);
		y1 = abs_y;
		w = TAB_SIZE;
		h = height;
		lx = TAB_SIZE/2;
		ly = height/2;
		lw = size-TAB_SIZE;
		lh = 0;
	}
	else {
		x1 = y1 = w = h = lx = ly = lw = lh = 0;
	}

	al_draw_line(abs_x+lx+0.5, abs_y+ly+0.5, abs_x+lx+0.5+lw, abs_y+ly+0.5+lh, fore, 1);
	al_draw_filled_rounded_rectangle(x1, y1, x1+w, y1+h, 3, 3, fore);
}

void TGUI_Slider::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (rel_x < 0 || rel_y < 0)
		return;
	
	rel_x -= TAB_SIZE/2;
	rel_y -= TAB_SIZE/2;

	if (direction == TGUI_HORIZONTAL) {
		if (rel_x < 0)
			pos = 0;
		else if (rel_x > (size-TAB_SIZE))
			pos = 1;
		else
			pos = rel_x / (float)(size - TAB_SIZE);
	}
	else {
		if (rel_y < 0)
			pos = 0;
		else if (rel_y > (size-TAB_SIZE))
			pos = 1;
		else
			pos = rel_y / (float)(size - TAB_SIZE);
	}

	if (pos > 1) pos = 1;

	dragging = true;
}

void TGUI_Slider::mouseMoveAll(tgui::TGUIWidget *leftOut, int abs_x, int abs_y)
{
	if (!dragging)
		return;

	int wx, wy;
	tgui::determineAbsolutePosition(this, &wx, &wy);

	int rel_x = abs_x - wx - TAB_SIZE/2;
	int rel_y = abs_y - wy - TAB_SIZE/2;

	if (direction == TGUI_HORIZONTAL) {
		if (rel_x < 0)
			pos = 0;
		else if (rel_x > (size-TAB_SIZE))
			pos = 1;
		else
			pos = rel_x / (float)(size - TAB_SIZE);
	}
	else {
		if (rel_y < 0)
			pos = 0;
		else if (rel_y > (size-TAB_SIZE))
			pos = 1;
		else
			pos = rel_y / (float)(size - TAB_SIZE);
	}
}

void TGUI_Slider::mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	dragging = false;

	if (callback) {
		callback(pos);
	}
}

float TGUI_Slider::getPosition()
{
	return pos;
}

void TGUI_Slider::setPosition(float pos)
{
	this->pos = pos;
}

void TGUI_Slider::setCallback(void (*callback)(float))
{
	this->callback = callback;
}

TGUI_Slider::TGUI_Slider(int x, int y, int size, TGUI_Direction direction) :
	size(size),
	direction(direction)
{
	this->x = x;
	this->y = y;

	width = direction == TGUI_VERTICAL ? 16 : size;
	height = direction == TGUI_VERTICAL ? size : 16;

	dragging = false;

	callback = NULL;
}

// --

bool TGUI_Button::acceptsFocus()
{
	return true;
}

void TGUI_Button::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	int x = abs_x;
	int y = abs_y;

	al_draw_filled_rectangle(x, y, x+width, y+height, back);
	al_draw_line(x+0.5, y+0.5, x+width-0.5, y+0.5, back_hilite, 1);
	al_draw_line(x+0.5, y+0.5, x+0.5, y+height-0.5, back_hilite, 1);
	al_draw_line(x+0.5, y+height-0.5, x+width+0.5, y+height-0.5, back_darker, 1); // little longer to cover pixel
	al_draw_line(x+width-0.5, y+0.5, x+width-0.5, y+height-0.5, back_darker, 1);

	al_draw_text(tgui::getFont(), al_map_rgb(0x00, 0x00, 0x00),
		x+(int)width/2-al_get_text_width(tgui::getFont(), text.c_str())/2,
		y+(int)height/2-al_get_font_line_height(tgui::getFont())/2,
		0, text.c_str());
}

TGUI_Button::TGUI_Button(std::string text, int x, int y, int w, int h) :
	TGUI_Icon(NULL, 0, 0, 0),
	text(text)
{
	this->x = x;
	this->y = y;
	width = w;
	height = h;
}

static void getVisibleSubMenus(TGUI_Splitter *root, std::vector<TGUI_SubMenuItem *> &subMenus, int depth, int maxDepth)
{
	if (depth > maxDepth)
		return;

	std::vector<tgui::TGUIWidget *> &widgets = root->getWidgets();

	for (unsigned int i = 0; i < widgets.size(); i++) {
		TGUI_SubMenuItem *sub = dynamic_cast<TGUI_SubMenuItem *>(widgets[i]);
		if (sub) {
			subMenus.push_back(sub);
			if (sub->isOpen()) {
				getVisibleSubMenus(sub->getSubMenu(), subMenus, depth+1, maxDepth);
			}
		}
	}
}

// --

bool TGUI_TextField::acceptsFocus()
{
	return true;
}

void TGUI_TextField::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	this->height = al_get_font_line_height(tgui::getFont()) + PADDING*2;

	ALLEGRO_COLOR bgcolor = al_map_rgb(0xff, 0xff, 0xff);

	al_draw_filled_rectangle(abs_x, abs_y, abs_x+width, abs_y+height, bgcolor);
	al_draw_rectangle(abs_x+0.5, abs_y+0.5, abs_x+width-0.5, abs_y+height-0.5, al_map_rgb(0x00, 0x00, 0x00), 1);
	if (this == tgui::getFocussedWidget()) {
		int len = cursorPos - offset;
		std::string before = str.substr(offset, len);
		int xx = abs_x+1+al_get_text_width(tgui::getFont(), before.c_str());
		int xx2;
		if (cursorPos >= (int)str.length()) {
			xx2 = xx + 5;
		}
		else {
			std::string at = str.substr(offset+len, 1);
			xx2 = xx+al_get_text_width(tgui::getFont(), at.c_str());
		}
		al_draw_filled_rectangle(xx, abs_y+1, xx2, abs_y+height-2,
			al_map_rgb(0, 255, 255));
	}
	int _x, _y, _w, _h;
	al_get_clipping_rectangle(&_x, &_y, &_w, &_h);
	tgui::setClip(abs_x+3, abs_y, width-4, height);
	al_draw_text(tgui::getFont(), al_map_rgb(0x00, 0x00, 0x00), abs_x+3, abs_y+PADDING, 0, str.substr(offset).c_str());
	al_set_clipping_rectangle(_x, _y, _w, _h);
}

void TGUI_TextField::findOffset()
{
	int len = cursorPos - offset;
	std::string tmp = str.substr(offset, len);
	if (al_get_text_width(tgui::getFont(), tmp.c_str()) >= width-15) {
		if (offset < (int)str.length()-1)
			offset++;
	}
}

bool TGUI_TextField::keyChar(int keycode, int unichar)
{
	bool used = false;

	if (this != tgui::getFocussedWidget())
		return false;

	if (keycode == ALLEGRO_KEY_BACKSPACE) {
		if (cursorPos > 0) {
			cursorPos--;
			str.erase(str.begin() + cursorPos);
			if (cursorPos < offset)
				offset--;
		}
	}
	else if (keycode == ALLEGRO_KEY_DELETE) {
		if (cursorPos < (int)str.length()) {
			str.erase(str.begin() + cursorPos);
		}
	}
	else if (keycode == ALLEGRO_KEY_LEFT) {
		used = true;
		if (cursorPos > 0) {
			cursorPos--;
			if (cursorPos < offset)
				offset--;
		}
	}
	else if (keycode == ALLEGRO_KEY_RIGHT) {
		used = true;
		if (cursorPos < (int)str.length()) {
			cursorPos++;
			findOffset();
		}
	}

	if (unichar <= 0) {
		return used;
	}

	std::string backup = str;
	if (cursorPos >= (int)str.length()) {
		str.push_back(unichar);
	}
	else {
		str.insert(str.begin() + cursorPos, unichar);
	}
	if (validate && validate(str)) {
		cursorPos++;
		findOffset();
	}
	else {
		str = backup;
	}

	return false;
}

void TGUI_TextField::setValidator(bool (*validate)(const std::string str))
{
	this->validate = validate;
}

bool TGUI_TextField::isValid()
{
	return validate(str);
}

std::string TGUI_TextField::getText()
{
	return str;
}

void TGUI_TextField::setText(std::string s)
{
	str = s;
	cursorPos = str.length();
	offset = 0;
	findOffset();
}

TGUI_TextField::TGUI_TextField(std::string startStr, int x, int y, int width) :
	str(startStr),
	cursorPos(startStr.length()),
	offset(0),
	validate(NULL)
{
	this->x = x;
	this->y = y;
	this->width = width;

	findOffset();
}

TGUI_TextField::~TGUI_TextField()
{
}

// --

int TGUI_Frame::barHeight()
{
	return al_get_font_line_height(tgui::getFont()) + TITLE_PADDING*2;
}

bool TGUI_Frame::getAbsoluteChildPosition(tgui::TGUIWidget *widget, int *x, int *y)
{
	if (child == widget) {
		*x = this->x + widget->getX();
		*y = this->y + widget->getY();
		return true;
	}
	return false;
}

void TGUI_Frame::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	int top = al_get_font_line_height(tgui::getFont()) + TITLE_PADDING*2;

	if (rel_x >= 0 && rel_y >= 0 && rel_x < width && rel_y < top) {
		dragging = true;
		drag_x = abs_x;
		drag_y = abs_y;
	}
}

void TGUI_Frame::mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	dragging = false;
}

void TGUI_Frame::mouseMove(int rel_x, int rel_y, int abs_x, int abs_y)
{
	if (dragging) {
		int dx = abs_x - drag_x;
		int dy = abs_y - drag_y;
		x += dx;
		y += dy;
		int scr_w, scr_h;
		tgui::getScreenSize(&scr_w, &scr_h);
		if (x < 0) x = 0;
		if (x > scr_w-width) x = scr_w-width;
		if (y < 0) y = 0;
		if (y > scr_h-height) y = scr_h-height;
		drag_x = abs_x;
		drag_y = abs_y;
	}
}

void TGUI_Frame::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	int top = barHeight();

	al_draw_filled_rectangle(abs_x, abs_y+top, abs_x+width,
		abs_y+height, back);

	al_draw_rectangle(abs_x+0.5, abs_y+0.5, abs_x-0.5+width,
		abs_y-0.5+height, fore, 1);
	al_draw_filled_rectangle(abs_x, abs_y, abs_x+width,
		abs_y+top, fore);

	al_draw_text(tgui::getFont(), al_map_rgb(0x00, 0x00, 0x00), abs_x+width/2,
		abs_y+TITLE_PADDING, ALLEGRO_ALIGN_CENTRE, title.c_str());
}

TGUI_Frame::TGUI_Frame(std::string title, int x, int y, int width, int height) :
	title(title),
	dragging(false)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
}

TGUI_Frame::~TGUI_Frame()
{
}

// --

void TGUI_Label::setText(std::string text)
{
	this->text = text;
}

void TGUI_Label::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	al_draw_text(tgui::getFont(), color, abs_x, abs_y, flags, text.c_str());
}

TGUI_Label::TGUI_Label(std::string text, ALLEGRO_COLOR color, int x, int y, int flags) :
	text(text),
	color(color),
	flags(flags)
{
	this->x = x;
	this->y = y;
	this->width = al_get_text_width(tgui::getFont(), text.c_str());
	this->height = al_get_font_line_height(tgui::getFont());
}

TGUI_Label::~TGUI_Label()
{
}

TGUI_List::TGUI_List(int x, int y, int width)
{
	this->x = x;
	this->y = y;
	this->width = width;
	height = 0;
	selected = 0;
}

TGUI_List::~TGUI_List()
{
}

void TGUI_List::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (rel_y >= 0) {
		int lh = al_get_font_line_height(tgui::getFont());
		int sel = rel_y / lh;
		if (sel < 0 || sel >= (int)labels.size()) {
			return;
		}
		selected = sel;
	}
}

const std::vector<std::string> &TGUI_List::getLabels()
{
	return labels;
}

void TGUI_List::setLabels(const std::vector<std::string> &labels)
{
	this->labels = labels;
	height = al_get_font_line_height(tgui::getFont()) * labels.size();
}

void TGUI_List::draw(int abs_x, int abs_y)
{
	setDefaultColors();

	int lh = al_get_font_line_height(tgui::getFont());

	for (size_t i = 0; i < labels.size(); i++) {
		ALLEGRO_COLOR fore;
		ALLEGRO_COLOR back;
		int yy = abs_y + lh*i;
		fore = al_map_rgb(0x00, 0x00, 0x00);
		if ((int)i == selected) {
			back = ::fore;
			al_draw_filled_rectangle(abs_x, yy, abs_x+width, yy+lh, back);
		}
		al_draw_text(tgui::getFont(), fore, abs_x+2, yy, 0, labels[i].c_str());
	}
}

void tguiWidgetsSetColors(ALLEGRO_COLOR f, ALLEGRO_COLOR b)
{
	fore = f;
	back = b;
	back_hilite = back;
	back_darker = back;
	back_hilite.r *= 1.35f;
	back_hilite.g *= 1.35f;
	back_hilite.b *= 1.35f;
	back_darker.r *= 0.35f;
	back_darker.g *= 0.35f;
	back_darker.b *= 0.35f;

	colors_set = true;
}

void tguiWidgetsGetColors(ALLEGRO_COLOR *f, ALLEGRO_COLOR *b)
{
	*f = fore;
	*b = back;
}

