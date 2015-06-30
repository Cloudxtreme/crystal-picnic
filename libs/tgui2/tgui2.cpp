#define ALLEGRO_STATICLINK

#include "tgui2.hpp"

#include <allegro5/allegro_primitives.h>

#include <cstdio>
#include <cmath>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

namespace tgui {

	struct TGUI {
		std::vector<TGUIWidget*> widgets;
		bool hidden;
	};

}

static void drawRect(tgui::TGUI *gui, int x1, int y1, int x2, int y2);

namespace tgui {

static TGUIWidget *getWidgetInDirection(TGUIWidget *widget, int xdir, int ydir);

static ALLEGRO_DISPLAY *display;

static std::vector<TGUI*> stack;
static std::vector<TGUIWidget *> stackFocus;
static double lastUpdate;
static TGUIWidget* currentParent = 0;

static int screenWidth = 0;
static int screenHeight = 0;

static float x_scale = 1;
static float y_scale = 1;
static float x_offset = 0;
static float y_offset = 0;

static std::vector<TGUIWidget *> focusOrderList;
static bool focusWrap = false;
static TGUIWidget *focussedWidget;

static ALLEGRO_FONT *font;

static bool clipSet = false;

static std::vector<TGUIWidget *> preDrawWidgets;
static std::vector<TGUIWidget *> postDrawWidgets;

static bool keyState[ALLEGRO_KEY_MAX] = { 0, };

static int screenSizeOverrideX = -1;
static int screenSizeOverrideY = -1;

static bool joyButtonDown = false;
static int joyButtonDownCount = 0;
static int joyButtonDownNum;
static double joyButtonDownTime;
static bool joyAxisDown = false;
static int joyAxisStick;
static int joyAxisAxis;
static int joyAxisDownCount = 0;
static int joyAxisDownXdir;
static int joyAxisDownYdir;
static double joyAxisDownTime;

bool checkBoxCollision(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
	if ((y2 < y3) || (y1 > y4) || (x2 < x3) || (x1 > x4))
		return false;
	return true;
}

void setFont(ALLEGRO_FONT *f)
{
	font = f;
}

ALLEGRO_FONT *getFont()
{
	return font;
}

long currentTimeMillis()
{
#ifndef ALLEGRO4
	return (long)(al_get_time() * 1000);
#else
#ifndef ALLEGRO_WINDOWS
	struct timeval tv;
	gettimeofday(&tv, 0);
	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#else
	return timeGetTime();
#endif
#endif
}

static void deleteGUI(TGUI *gui)
{
	for (size_t i = 0; i < gui->widgets.size(); i++) {
		delete gui->widgets[i];
	}
	delete gui;
}
	
static void deletestack()
{
	while (stack.size() > 0) {
		deleteGUI(stack[0]);
		stack.erase(stack.begin());
	}
	stackFocus.clear();
}

void init(ALLEGRO_DISPLAY *d)
{
	display = d;

	deletestack();

	TGUI *gui = new TGUI;
	gui->hidden = false;

	stack.push_back(gui);
	stackFocus.push_back(NULL);

	lastUpdate = currentTimeMillis();

	getScreenSize(&screenWidth, &screenHeight);
}

void shutdown()
{
	deletestack();
	preDrawWidgets.clear();
	postDrawWidgets.clear();
}

void setFocus(TGUIWidget *widget)
{
	if (widget == NULL || widget->acceptsFocus()) {
		focussedWidget = widget;
	}
}

void setFocusLosingGaining(TGUIWidget *widget)
{
	if (focussedWidget) {
		focussedWidget->losingFocus();
	}
	if (widget) {
		widget->gainingFocus();
	}
	setFocus(widget);
}

TGUIWidget *getFocussedWidget()
{
	return focussedWidget;
}

void setFocusOrder(std::vector<TGUIWidget *> list)
{
	focusOrderList = list;
}

void focusPrevious()
{
	TGUIWidget *focussed = getFocussedWidget();

	if (focussed == NULL)
		return;

	for (size_t i = 0; i < focusOrderList.size(); i++) {
		if (focusOrderList[i] == focussed) {
			if (i > 0) {
				setFocusLosingGaining(focusOrderList[i-1]);
			}
			else {
				return;
			}
		}
	}
}

void focusNext()
{
	TGUIWidget *focussed = getFocussedWidget();

	if (focussed == NULL)
		return;

	for (size_t i = 0; i < focusOrderList.size(); i++) {
		if (focusOrderList[i] == focussed) {
			if (i < focusOrderList.size()-1) {
				setFocusLosingGaining(focusOrderList[i+1]);
			}
			else {
				return;
			}
		}
	}
}

void translateAll(int x, int y)
{
	TGUI *gui = stack[0];

	for (size_t i = 0; i < gui->widgets.size(); i++) {
		TGUIWidget *w = gui->widgets[i];
		w->translate(x, y);
	}
}

void addWidget(TGUIWidget* widget)
{
	if (!widget->getParent())
		widget->setParent(currentParent);
	if (currentParent) {
		currentParent->setChild(widget);
	}
	stack[0]->widgets.push_back(widget);
}

static void handleJoyAxisRepeat(int stick, int axis, float value)
{
	bool used = false;
	for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
		if (stack[0]->widgets[i]->getParent() == NULL) {
			used = used || stack[0]->widgets[i]->chainJoyAxisRepeat(stick, axis, value);
		}
	}
	if (!used && focussedWidget) {
		if (axis == 0) {
			if (value <= -0.5f) {
				TGUIWidget *w = getWidgetInDirection(focussedWidget, -1, 0);
				if (w) {
					setFocusLosingGaining(w);
				}
			}
			else if (value >= 0.5f) {
				TGUIWidget *w = getWidgetInDirection(focussedWidget, 1, 0);
				if (w) {
					setFocusLosingGaining(w);
				}
			}
		}
		else {
			if (value <= -0.5f) {
				TGUIWidget *w = getWidgetInDirection(focussedWidget, 0, -1);
				if (w) {
					setFocusLosingGaining(w);
				}
			}
			else if (value >= 0.5f) {
				TGUIWidget *w = getWidgetInDirection(focussedWidget, 0, 1);
				if (w) {
					setFocusLosingGaining(w);
				}
			}
		}
	}
}

TGUIWidget *update()
{
	long currTime = currentTimeMillis();
	long elapsed = currTime - lastUpdate;
	if (elapsed > 50) {
		elapsed = 50;
	}
	lastUpdate = currTime;

	if (joyButtonDown) {
		double delay;
		if (joyButtonDownCount == 0) {
			delay = 0.3;
		}
		else {
			delay = 0.15;
		}
		if (al_get_time()-delay > joyButtonDownTime) {
			joyButtonDownCount++;
			joyButtonDownTime = al_get_time();
			for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
				if (stack[0]->widgets[i]->getParent() == NULL) {
					stack[0]->widgets[i]->chainJoyButtonDownRepeat(joyButtonDownNum);
				}
			}
		}
	}
	if (joyAxisDown) {
		double delay;
		if (joyAxisDownCount == 0) {
			delay = 0.3;
		}
		else {
			delay = 0.15;
		}
		if (al_get_time()-delay > joyAxisDownTime) {
			joyAxisDownCount++;
			joyAxisDownTime = al_get_time();
			int stick = 0; // FIXME
			int axis = joyAxisDownXdir ? 0 : 1;
			float value = joyAxisDownXdir ? joyAxisDownXdir : joyAxisDownYdir;
			handleJoyAxisRepeat(stick, axis, value);
		}
	}

	for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
		TGUIWidget *widget = stack[0]->widgets[i];
		TGUIWidget *retVal = widget->update();
		if (retVal) {
			return retVal;
		}
	}

	return NULL;
}

std::vector<TGUIWidget *> updateAll()
{
	std::vector<TGUIWidget *> retVect;
	long currTime = currentTimeMillis();
	long elapsed = currTime - lastUpdate;

	if (elapsed > 50) {
		elapsed = 50;
	}

	lastUpdate = currTime;

	for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
		TGUIWidget *widget = stack[0]->widgets[i];
		TGUIWidget *retVal = widget->update();
		if (retVal) {
			retVect.push_back(retVal);
		}
	}

	return retVect;
}

void draw()
{
	int abs_x, abs_y;

	for (size_t i = 0; i < preDrawWidgets.size(); i++) {
		determineAbsolutePosition(preDrawWidgets[i], &abs_x, &abs_y);
		preDrawWidgets[i]->preDraw(abs_x, abs_y);
	}

	int sw, sh;
	getScreenSize(&sw, &sh);
	drawRect(0, 0, sw, sh);

	// Draw focus
	if (focussedWidget && focussedWidget->getDrawFocus()) {
		int x, y;
		determineAbsolutePosition(focussedWidget, &x, &y);
		int w = focussedWidget->getWidth();
		int h = focussedWidget->getHeight();
		drawFocusRectangle(x, y, w, h);
	}
	
	for (size_t i = 0; i < postDrawWidgets.size(); i++) {
		determineAbsolutePosition(postDrawWidgets[i], &abs_x, &abs_y);
		postDrawWidgets[i]->postDraw(abs_x, abs_y);
	}
}

void drawRect(int x1, int y1, int x2, int y2)
{
	for (int i = stack.size()-1; i >= 0; i--) {
		if (!stack[i]->hidden) {
			::drawRect(stack[i], x1, y1, x2, y2);
		}
	}
}

void push()
{
	TGUI *gui = new TGUI;
	gui->hidden = false;

	stack.insert(stack.begin(), gui);
	stackFocus.insert(stackFocus.begin(), getFocussedWidget());

	setFocus(NULL);
}

bool pop()
{
	if (stack.size() <= 0)
		return false;

	deleteGUI(stack[0]);
	stack.erase(stack.begin());

	setFocus(stackFocus[0]);
	stackFocus.erase(stackFocus.begin());

	return true;
}

void setNewWidgetParent(TGUIWidget* parent)
{
	currentParent = parent;
}

TGUIWidget *getNewWidgetParent()
{
	return currentParent;
}

void centerWidget(TGUIWidget* widget, int x, int y)
{
	if (x >= 0) {
		widget->setX(x - (int)(widget->getWidth() / 2));
	}
	if (y >= 0) {
		widget->setY(y - (int)(widget->getHeight() / 2));
	}
}

void setScale(float xscale, float yscale)
{
	x_scale = xscale;
	y_scale = yscale;
}

void setOffset(float xoffset, float yoffset)
{
	x_offset = xoffset;
	y_offset = yoffset;
}

void convertMousePosition(int *x, int *y)
{
	*x = *x / x_scale;
	*y = *y / y_scale;

	*x -= x_offset;
	*y -= y_offset;
}

void bufferToScreenPos(int *x, int *y, int bw, int bh)
{
	*x = *x * x_scale + x_offset;
	*y = *y * y_scale + y_offset;
}

void setFocusWrap(bool wrap)
{
	focusWrap = wrap;
}

void maybe_make_mouse_event(ALLEGRO_EVENT *event)
{
	if (event->type == ALLEGRO_EVENT_TOUCH_BEGIN ||
			event->type == ALLEGRO_EVENT_TOUCH_END ||
			event->type == ALLEGRO_EVENT_TOUCH_MOVE) {
		if (event->type == ALLEGRO_EVENT_TOUCH_BEGIN) {
			event->type = ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
		}
		else if (event->type == ALLEGRO_EVENT_TOUCH_END) {
			event->type = ALLEGRO_EVENT_MOUSE_BUTTON_UP;
		}
		else if (event->type == ALLEGRO_EVENT_TOUCH_MOVE) {
			event->type = ALLEGRO_EVENT_MOUSE_AXES;
		}
		event->mouse.x = event->touch.x;
		event->mouse.y = event->touch.y;
		event->mouse.z = 0;
		event->mouse.w = 0;
		event->mouse.button = event->touch.id;
	}
}

void handleEvent_pretransformed(void *allegro_event)
{
	ALLEGRO_EVENT *event = (ALLEGRO_EVENT *)allegro_event;

	if (event->type == ALLEGRO_EVENT_JOYSTICK_AXIS && event->joystick.id && al_get_joystick_num_buttons((ALLEGRO_JOYSTICK *)event->joystick.id) == 0) {
		return;
	}
	
#if defined ALLEGRO_ANDROID || defined ALLEGRO_IPHONE
	maybe_make_mouse_event(event);
#endif

	switch (event->type) {
		case ALLEGRO_EVENT_TIMER:
			break;
		case ALLEGRO_EVENT_MOUSE_AXES: {
			int mx = event->mouse.x;
			int my = event->mouse.y;
			int mz = event->mouse.z;
			int mw = event->mouse.w;
			TGUIWidget *w = determineTopLevelOwner(mx, my);
			if (w) {
				int rel_x;
				int rel_y;
				int abs_x, abs_y;
				determineAbsolutePosition(w, &abs_x, &abs_y);
				rel_x = mx - abs_x;
				rel_y = my - abs_y;
				TGUIWidget *leftOut = w->chainMouseMove(rel_x, rel_y, mx, my, mz, mw);
				for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
					TGUIWidget *w2 = stack[0]->widgets[i];
					if (w2->getParent() == NULL) {
						w2->mouseMoveAll(leftOut, mx, my);
					}
				}
			}
			else {
				for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
					stack[0]->widgets[i]->mouseMove(-1, -1, mx, my);
				}
			}
			break;
		}
		case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		case ALLEGRO_EVENT_MOUSE_BUTTON_UP: {
			bool down = event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN;
			int mx = event->mouse.x;
			int my = event->mouse.y;
			TGUIWidget *w = determineTopLevelOwner(mx, my);
			if (w) {
				int rel_x;
				int rel_y;
				int abs_x, abs_y;
				determineAbsolutePosition(w, &abs_x, &abs_y);
				rel_x = mx - abs_x;
				rel_y = my - abs_y;
				if (down) {
					TGUIWidget *leftOut = w->chainMouseDown(rel_x, rel_y, mx, my, event->mouse.button);
					setFocus(leftOut);
					for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
						TGUIWidget *w2 = stack[0]->widgets[i];
						if (w2->getParent() == NULL) {
							w2->mouseDownAll(leftOut, mx, my, event->mouse.button);
						}
					}
				}
				else {
					TGUIWidget *leftOut = w->chainMouseUp(rel_x, rel_y, mx, my, event->mouse.button);
					for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
						TGUIWidget *w2 = stack[0]->widgets[i];
						if (w2->getParent() == NULL) {
							w2->mouseUpAll(leftOut, mx, my, event->mouse.button);
						}
					}
				}
			}
			else {
				for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
					if (down)
						stack[0]->widgets[i]->mouseDown(-1, -1, mx, my, event->mouse.button);
					else
						stack[0]->widgets[i]->mouseUp(-1, -1, mx, my, event->mouse.button);
				}
			}
			break;
		}
		case ALLEGRO_EVENT_KEY_DOWN: {
			keyState[event->keyboard.keycode] = true;
			for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
				if (stack[0]->widgets[i]->getParent() == NULL) {
					stack[0]->widgets[i]->chainKeyDown(event->keyboard.keycode);
				}
			}
			break;
		}
		case ALLEGRO_EVENT_KEY_UP: {
			keyState[event->keyboard.keycode] = false;
			for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
				if (stack[0]->widgets[i]->getParent() == NULL) {
					stack[0]->widgets[i]->chainKeyUp(event->keyboard.keycode);
				}
			}
			break;
		}
		case ALLEGRO_EVENT_KEY_CHAR: {
			bool used = false;
			for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
				if (stack[0]->widgets[i]->getParent() == NULL) {
					used = used || stack[0]->widgets[i]->chainKeyChar(event->keyboard.keycode, event->keyboard.unichar);
				}
			}
			if (!used) {
				if (focussedWidget && event->keyboard.keycode == ALLEGRO_KEY_LEFT) {
					TGUIWidget *w = getWidgetInDirection(focussedWidget, -1, 0);
					if (w) {
						setFocusLosingGaining(w);
					}
				}
				else if (focussedWidget && event->keyboard.keycode == ALLEGRO_KEY_RIGHT) {
					TGUIWidget *w = getWidgetInDirection(focussedWidget, 1, 0);
					if (w) {
						setFocusLosingGaining(w);
					}
				}
				else if (focussedWidget && event->keyboard.keycode == ALLEGRO_KEY_UP) {
					TGUIWidget *w = getWidgetInDirection(focussedWidget, 0, -1);
					if (w) {
						setFocusLosingGaining(w);
					}
				}
				else if (focussedWidget && event->keyboard.keycode == ALLEGRO_KEY_DOWN) {
					TGUIWidget *w = getWidgetInDirection(focussedWidget, 0, 1);
					if (w) {
						setFocusLosingGaining(w);
					}
				}
			}
			break;
		}
		case ALLEGRO_EVENT_JOYSTICK_AXIS: {
			int stick = event->joystick.stick;
			int axis = event->joystick.axis;
			float value = event->joystick.pos;

			if (stick == 0 && joyAxisDown && fabs(value) <= 0.25f && ((axis == 0 && joyAxisDownXdir) || (axis == 1 && joyAxisDownYdir))) {
				joyAxisDown = false;
			}
			else if (!joyAxisDown && fabs(value) >= 0.5f && stick == 0) {
				int xdir, ydir;
				if (axis == 0) {
					ydir = 0;
					if (value <= -0.5f) {
						xdir = -1;
					}
					else {
						xdir = 1;
					}
				}
				else {
					xdir = 0;
					if (value <= -0.5f) {
						ydir = -1;
					}
					else {
						ydir = 1;
					}
				}
				joyAxisDown = true;
				joyAxisStick = stick;
				joyAxisAxis = axis;
				joyAxisDownCount = 0;
				joyAxisDownXdir = xdir;
				joyAxisDownYdir = ydir;
				joyAxisDownTime = al_get_time();
				handleJoyAxisRepeat(stick, axis, value);
			}

			for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
				if (stack[0]->widgets[i]->getParent() == NULL) {
					stack[0]->widgets[i]->chainJoyAxis(stick, axis, value);
				}
			}
			break;
		}
		case ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN: {
			if (!joyButtonDown) {
				joyButtonDown = true;
				joyButtonDownCount = 0;
				joyButtonDownNum = event->joystick.button;
				joyButtonDownTime = al_get_time();
			}

			for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
				if (stack[0]->widgets[i]->getParent() == NULL) {
					stack[0]->widgets[i]->chainJoyButtonDown(event->joystick.button);
				}
			}
			for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
				if (stack[0]->widgets[i]->getParent() == NULL) {
					stack[0]->widgets[i]->chainJoyButtonDownRepeat(event->joystick.button);
				}
			}
			break;
		}
		case ALLEGRO_EVENT_JOYSTICK_BUTTON_UP: {
			if (joyButtonDown && event->joystick.button == joyButtonDownNum) {
				joyButtonDown = false;
			}

			for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
				if (stack[0]->widgets[i]->getParent() == NULL) {
					stack[0]->widgets[i]->chainJoyButtonUp(event->joystick.button);
				}
			}
			break;
		}
	}
}

void handleEvent(void *allegro_event)
{
	ALLEGRO_EVENT *ev = (ALLEGRO_EVENT *)allegro_event;
	ALLEGRO_EVENT event = *ev;

#if defined ALLEGRO_ANDROID || defined ALLEGRO_IPHONE
	maybe_make_mouse_event(&event);
#endif

	if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP ||
	    event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
	    event.type == ALLEGRO_EVENT_MOUSE_AXES) {
		ALLEGRO_EVENT ev;
		memcpy(&ev, &event, sizeof(ALLEGRO_EVENT));
		convertMousePosition(&ev.mouse.x, &ev.mouse.y);
		handleEvent_pretransformed(&ev);
	}
	else {
		handleEvent_pretransformed(&event);
	}
}

TGUIWidget *getTopLevelParent(TGUIWidget *widget)
{
	TGUIWidget *p = widget;
	while (p->getParent()) {
		p = p->getParent();
		if (p->getParent() == NULL) {
			return p;
		}
	}

	return NULL;
}

TGUIWidget *determineTopLevelOwner(int x, int y)
{
	if (stack[0]->widgets.size() <= 0)
		return NULL;

	for (int i = (int)stack[0]->widgets.size()-1; i >= 0; i--) {
		TGUIWidget *widget = stack[0]->widgets[i];
		if (pointOnWidget(widget, x, y)) {
			TGUIWidget *p = widget;
			while (p->getParent()) {
				p = p->getParent();
			}
			return p;
		}
	}
	return NULL;
}

void determineAbsolutePosition(TGUIWidget *widget, int *x, int *y)
{
	// Check each widget in case widget is a child of one
	for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
		if (stack[0]->widgets[i] == widget) {
			continue;
		}
		if (stack[0]->widgets[i]->getAbsoluteChildPosition(widget, x, y)) {
			return;
		}
	}
	*x = widget->getX();
	*y = widget->getY();
}

bool pointOnWidget(TGUIWidget *widget, int x, int y)
{
	int wx, wy;
	determineAbsolutePosition(widget, &wx, &wy);
	if (x >= wx && y >= wy && x < wx+widget->getWidth() && y < wy+widget->getHeight()) {
		return true;
	}
	return false;
}

TGUIWidget *TGUIWidget::chainMouseMove(int rel_x, int rel_y, int abs_x, int abs_y, int z, int w)
{
	bool used = false;
	TGUIWidget *ret = NULL;

	if (child) {
		// pass it on to the child
		if (pointOnWidget(child, abs_x, abs_y)) {
			int wx, wy;
			determineAbsolutePosition(child, &wx, &wy);
			rel_x = abs_x - wx;
			rel_y = abs_y - wy;
			ret = child->chainMouseMove(
				rel_x,
				rel_y,
				abs_x,
				abs_y,
				z,
				w
			);
			used = true;
		}
	}

	if (!used) {
		// handle it within ourself
		if (pointOnWidget(this, abs_x, abs_y)) {
			mouseMove(rel_x, rel_y, abs_x, abs_y);
			mouseScroll(z, w);
			ret = this;
		}
	}

	return ret;
}

TGUIWidget *TGUIWidget::chainMouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	bool used = false;
	TGUIWidget *ret = NULL;

	if (child) {
		// pass it on to the child
		if (pointOnWidget(child, abs_x, abs_y)) {
			int wx, wy;
			determineAbsolutePosition(child, &wx, &wy);
			rel_x = abs_x - wx;
			rel_y = abs_y - wy;
			ret = child->chainMouseDown(
				rel_x,
				rel_y,
				abs_x,
				abs_y,
				mb
			);
			used = true;
		}
	}

	if (!used) {
		// handle it within ourself
		if (pointOnWidget(this, abs_x, abs_y)) {
			mouseDown(rel_x, rel_y, abs_x, abs_y, mb);
			ret = this;
		}
	}

	return ret;
}

TGUIWidget *TGUIWidget::chainMouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	bool used = false;
	TGUIWidget *ret = NULL;

	if (child) {
		// pass it on to the child
		if (pointOnWidget(child, abs_x, abs_y)) {
			int wx, wy;
			determineAbsolutePosition(child, &wx, &wy);
			rel_x = abs_x - wx;
			rel_y = abs_y - wy;
			ret = child->chainMouseUp(
				rel_x,
				rel_y,
				abs_x,
				abs_y,
				mb
			);
			used = true;
		}
	}

	if (!used) {
		// handle it within ourself
		if (pointOnWidget(this, abs_x, abs_y)) {
			mouseUp(rel_x, rel_y, abs_x, abs_y, mb);
			ret = this;
		}
	}

	return ret;
}

void TGUIWidget::chainKeyDown(int keycode)
{
	// handle it within ourself
	keyDown(keycode);

	// pass it on to the child
	if (child) {
		child->chainKeyDown(
			keycode
		);
	}
}

void TGUIWidget::chainKeyUp(int keycode)
{
	// handle it within ourself
	keyUp(keycode);

	// pass it on to the child
	if (child) {
		child->chainKeyUp(
			keycode
		);
	}
}

bool TGUIWidget::chainKeyChar(int keycode, int unichar)
{
	// handle it within ourself
	bool used = keyChar(keycode, unichar);

	// pass it on to the child
	if (child) {
		used = used || child->chainKeyChar(keycode, unichar);
	}

	return used;
}

void TGUIWidget::chainJoyButtonDown(int button)
{
	// handle it within ourself
	joyButtonDown(button);

	// pass it on to the child
	if (child) {
		child->chainJoyButtonDown(
			button
		);
	}
}

void TGUIWidget::chainJoyButtonDownRepeat(int button)
{
	// handle it within ourself
	joyButtonDownRepeat(button);

	// pass it on to the child
	if (child) {
		child->chainJoyButtonDownRepeat(
			button
		);
	}
}

void TGUIWidget::chainJoyButtonUp(int button)
{
	// handle it within ourself
	joyButtonUp(button);

	// pass it on to the child
	if (child) {
		child->chainJoyButtonUp(
			button
		);
	}
}

void TGUIWidget::chainJoyAxis(int stick, int axis, float value)
{
	// handle it within ourself
	joyAxis(stick, axis, value);

	// pass it on to the child
	if (child) {
		child->chainJoyAxis(stick, axis, value);
	}
}

bool TGUIWidget::chainJoyAxisRepeat(int stick, int axis, float value)
{
	// handle it within ourself
	bool used = joyAxisRepeat(stick, axis, value);

	// pass it on to the child
	if (child) {
		used = used || child->chainJoyAxisRepeat(stick, axis, value);
	}

	return used;
}

void TGUIWidget::chainDraw()
{
	int abs_x, abs_y;
	determineAbsolutePosition(this, &abs_x, &abs_y);
	draw(abs_x, abs_y);

	if (child) {
		child->chainDraw();
	}
}

void setScreenSize(int w, int h)
{
	screenSizeOverrideX = w;
	screenSizeOverrideY = h;
}

void getScreenSize(int *w, int *h)
{
	if (screenSizeOverrideX > 0) {
		*w = screenSizeOverrideX;
		*h = screenSizeOverrideY;
		return;
	}

	if (al_get_current_display()) {
		ALLEGRO_BITMAP *bb = al_get_backbuffer(al_get_current_display());

		screenWidth = al_get_bitmap_width(bb);
		screenHeight = al_get_bitmap_height(bb);
	}
	else {
		screenWidth = -1;
		screenHeight = -1;
	}
	if (w) {
		*w = screenWidth;
	}
	if (h) {
		*h = screenHeight;
	}
}

void resize(TGUIWidget *parent)
{
	clearClip();

	if (parent) {
		parent->resize();
	}
	else {
		for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
			if (parent == stack[0]->widgets[i]->getParent()) {
				stack[0]->widgets[i]->resize();
			}
		}
	}
}

bool isClipSet()
{
	return clipSet;
}

void setClip(int x, int y, int width, int height)
{
	const ALLEGRO_TRANSFORM *t = al_get_current_transform();
	float tx = t->m[3][0];
	float ty = t->m[3][1];

	x = x * x_scale;
	y = y * y_scale;

	x += tx;
	y += ty;

	al_set_clipping_rectangle(x, y, ceil(width*x_scale), ceil(height*y_scale));
	clipSet = true;
}

void setClippedClip(int x, int y, int width, int height)
{
	int curr_x, curr_y, curr_w, curr_h;
	al_get_clipping_rectangle(&curr_x, &curr_y, &curr_w, &curr_h);
	if (
		x >= curr_x+curr_w || x+width <= curr_x ||
		y >= curr_y+curr_h || y+height <= curr_y
	) {
		x = curr_x;
		y = curr_y;
		width = curr_w;
		height = curr_h;
	}
	else {
		if (curr_x > x && curr_x < x+width) {
			int d = curr_x-x;
			x += d;
			width -= d;
		}
		if (curr_y > y && curr_y < y+height) {
			int d = curr_y-y;
			y += d;
			height -= d;
		}
		if (curr_x+curr_w > x && curr_x+curr_w < x+width) {
			width -= (x+width)-(curr_x+curr_w);
		}
		if (curr_y+curr_h > y && curr_y+curr_h < y+height) {
			height -= (y+height)-(curr_y+curr_h);
		}
	}
	setClip(x, y, width, height);
}

void clearClip()
{
	clipSet = false;
	int sw, sh;
	getScreenSize(&sw, &sh);
	al_set_clipping_rectangle(0, 0, sw, sh);
}

void getClip(int *x, int *y, int *w, int *h)
{
	al_get_clipping_rectangle(x, y, w, h);
}

bool isDeepChild(TGUIWidget *child, TGUIWidget *parent)
{
	TGUIWidget *p = parent;
	while (p->getChild()) {
		if (p->getChild() == child)
			return true;
		p = p->getChild();
	}
	return false;
}

std::vector<TGUIWidget *> findChildren(TGUIWidget *widget) {
	std::vector<TGUIWidget *> found;
	std::vector<TGUIWidget *>::iterator it;
	for (it = stack[0]->widgets.begin(); it != stack[0]->widgets.end(); it++) {
		if (isDeepChild(*it, widget)) {
			found.push_back(*it);
		}
	}

	return found;
}

void TGUIWidget::raise() {
	// Remove and place parent at top
	std::vector<TGUIWidget *>::iterator it = std::find(stack[0]->widgets.begin(), stack[0]->widgets.end(), this);
	if (it != stack[0]->widgets.end()) {
		stack[0]->widgets.erase(it);
		stack[0]->widgets.push_back(this);
	}

	// Do the same with all the children of this widget
	std::vector<TGUIWidget *> toRaise = findChildren(this);
	for (size_t i = 0; i < toRaise.size(); i++) {
		toRaise[i]->raise();
	}
}

void TGUIWidget::lower() {
	// Remove and place parent at top
	std::vector<TGUIWidget *>::iterator it = std::find(stack[0]->widgets.begin(), stack[0]->widgets.end(), this);
	if (it != stack[0]->widgets.end()) {
		stack[0]->widgets.erase(it);
		stack[0]->widgets.insert(stack[0]->widgets.begin(), this);
	}

	// Do the same with all the children of this widget
	std::vector<TGUIWidget *> toLower = findChildren(this);
	for (size_t i = 0; i < toLower.size(); i++) {
		toLower[i]->lower();
	}
}

void addPreDrawWidget(TGUIWidget *widget)
{
	preDrawWidgets.push_back(widget);
}

void addPostDrawWidget(TGUIWidget *widget)
{
	postDrawWidgets.push_back(widget);
}

void TGUIWidget::remove() {
	std::vector<TGUIWidget *>::iterator it = std::find(stack[0]->widgets.begin(), stack[0]->widgets.end(), this);
	if (it != stack[0]->widgets.end()) {
		stack[0]->widgets.erase(it);
	}
	it = std::find(preDrawWidgets.begin(), preDrawWidgets.end(), this);
	if (it != preDrawWidgets.end()) {
		preDrawWidgets.erase(it);
	}
	it = std::find(postDrawWidgets.begin(), postDrawWidgets.end(), this);
	if (it != postDrawWidgets.end()) {
		postDrawWidgets.erase(it);
	}
	
	if (child) {
		child->remove();
	}

	if (this == focussedWidget) {
		setFocus(NULL);
	}
}

bool isKeyDown(int keycode) {
	return keyState[keycode];
}

ALLEGRO_DISPLAY *getDisplay()
{
	return display;
}

static TGUIWidget *getWidgetInDirection(TGUIWidget *widget, int xdir, int ydir)
{
	int x1, y1, x2, y2;
	int measuring_point;

	int wx1, wy1;
	determineAbsolutePosition(widget, &wx1, &wy1);

	int sw, sh;
	getScreenSize(&sw, &sh);

	if (xdir < 0) {
		x1 = 0;
		x2 = wx1;
		y1 = wy1;
		y2 = wy1 + widget->getHeight();
		measuring_point = wx1;
	}
	else if (xdir > 0) {
		x1 = wx1 + widget->getWidth();
		y1 = wy1;
		x2 = sw;
		y2 = wy1 + widget->getHeight();
		measuring_point = wx1 + widget->getWidth();
	}
	else if (ydir < 0) {
		x1 = wx1;
		x2 = wx1 + widget->getWidth();
		y1 = 0;
		y2 = wy1;
		measuring_point = wy1;
	}
	else { // ydir > 0
		x1 = wx1;
		x2 = wx1 + widget->getWidth();
		y1 = wy1 + widget->getHeight();
		y2 = sh;
		measuring_point = wy1 + widget->getHeight();
	}

	std::vector<TGUIWidget *> colliding;

	for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
		TGUIWidget* w = stack[0]->widgets[i];
		w->addCollidingChildrenToVector(colliding, widget, x1, y1, x2, y2);
		if (w == widget || !w->acceptsFocus()) {
			continue;
		}
		int _x1, _y1, _x2, _y2;
		int wx2, wy2;
		determineAbsolutePosition(w, &wx2, &wy2);
		_x1 = wx2;
		_x2 = wx2 + w->getWidth();
		_y1 = wy2;
		_y2 = wy2 + w->getHeight();
		if (checkBoxCollision(x1, y1, x2, y2, _x1, _y1, _x2, _y2)) {
			colliding.push_back(w);
		}
	}

	if (colliding.size() == 0) {
		// Find any in that direction regardless of widget dimensions
		if (xdir < 0) {
			x1 = 0;
			x2 = wx1 - 1;
			y1 = 0;
			y2 = sh;
		}
		else if (xdir > 0) {
			x1 = wx1 + widget->getWidth() + 1;
			x2 = sw;
			y1 = 0;
			y2 = sh;
		}
		else if (ydir < 0) {
			x1 = 0;
			x2 = sw;
			y1 = 0;
			y2 = wy1 - 1;
		}
		else { // ydir > 0
			x1 = 0;
			x2 = sw;
			y1 = wy1 + widget->getHeight() + 1;
			y2 = sh;
		}
		for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
			TGUIWidget* w = stack[0]->widgets[i];
			w->addCollidingChildrenToVector(colliding, widget, x1, y1, x2, y2);
			if (w == widget || !w->acceptsFocus()) {
				continue;
			}
			int _x1, _y1, _x2, _y2;
			int wx2, wy2;
			determineAbsolutePosition(w, &wx2, &wy2);
			_x1 = wx2;
			_x2 = wx2 + w->getWidth();
			_y1 = wy2;
			_y2 = wy2 + w->getHeight();
			if (checkBoxCollision(x1, y1, x2, y2, _x1, _y1, _x2, _y2)) {
				colliding.push_back(w);
			}
		}
		if (colliding.size() == 0) {
			return NULL;
		}
		if (xdir < 0 || xdir > 0) {
			// find closest xs to center
			int closest = INT_MAX;
			for (size_t i = 0; i < colliding.size(); i++) {
				int dx = abs((widget->getX() + widget->getWidth()/2) - (colliding[i]->getX() + colliding[i]->getWidth()/2));
				if (dx < closest) {
					closest = dx;
				}
			}
			// drop everything not == closest distance
			std::vector<TGUIWidget *>::iterator it;
			for (it = colliding.begin(); it != colliding.end();) {
				TGUIWidget *w = *it;
				int dx = abs((widget->getX() + widget->getWidth()/2) - (w->getX() + w->getWidth()/2));
				if (dx != closest) {
					it = colliding.erase(it);
				}
				else {
					it++;
				}
			}
			// Now return the one with the closest x (all xs are the same now or there's only 1)
			closest = INT_MAX;
			TGUIWidget *closest_w = NULL;
			for (size_t i = 0; i < colliding.size(); i++) {
				int dy = abs((widget->getY() + widget->getHeight()/2) - (colliding[i]->getY() + colliding[i]->getHeight()/2));
				if (dy < closest) {
					closest = dy;
					closest_w = colliding[i];
				}
			}
			return closest_w;
		}
		else { // ydir < 0 || ydir > 0
			// find closest ys to center
			int closest = INT_MAX;
			for (size_t i = 0; i < colliding.size(); i++) {
				int dy = abs((widget->getY() + widget->getHeight()/2) - (colliding[i]->getY() + colliding[i]->getHeight()/2));
				if (dy < closest) {
					closest = dy;
				}
			}
			// drop everything not == closest distance
			std::vector<TGUIWidget *>::iterator it;
			for (it = colliding.begin(); it != colliding.end();) {
				TGUIWidget *w = *it;
				int dy = abs((widget->getY() + widget->getHeight()/2) - (w->getY() + w->getHeight()/2));
				if (dy != closest) {
					it = colliding.erase(it);
				}
				else {
					it++;
				}
			}
			// Now return the one with the closest y (all ys are the same now or there's only 1)
			closest = INT_MAX;
			TGUIWidget *closest_w = NULL;
			for (size_t i = 0; i < colliding.size(); i++) {
				int dx = abs((widget->getX() + widget->getWidth()/2) - (colliding[i]->getX() + colliding[i]->getWidth()/2));
				if (dx < closest) {
					closest = dx;
					closest_w = colliding[i];
				}
			}
			return closest_w;
		}
	}

	int closest = INT_MAX;
	TGUIWidget *closest_widget = NULL;

	for (size_t i = 0; i < colliding.size(); i++) {
		TGUIWidget *w = colliding[i];
		int wx2, wy2;
		determineAbsolutePosition(w, &wx2, &wy2);
		int measuring_point2;
		if (xdir < 0) {
			measuring_point2 = wx2 + w->getWidth();
		}
		else if (xdir > 0) {
			measuring_point2 = wx2;
		}
		else if (ydir < 0) {
			measuring_point2 = wy2 + w->getHeight();
		}
		else { // ydir > 0
			measuring_point2 = wy2;
		}
		int dist = abs(measuring_point-measuring_point2);
		if (dist < closest) {
			closest = dist;
			closest_widget = w;
		}
	}

	return closest_widget;
}

void drawFocusRectangle(int x, int y, int w, int h)
{
	float f = fmod(al_get_time(), 2);
	if (f > 1) f = 2 - f;
	al_draw_rectangle(
		x+0.5f,
		y+0.5f,
		x+w-0.5f,
		y+h-0.5f,
		al_map_rgb_f(f, f, 0),
		1
	);
}

// Must be called when al_flush_event_queue is called in the game
void flush()
{
	joyButtonDown = false;
	joyAxisDown = false;
}

void hide()
{
	stack[0]->hidden = true;
}

void unhide()
{
	stack[0]->hidden = false;
}

void releaseKeysAndButtons()
{
	for (int j = 0; j < ALLEGRO_KEY_MAX; j++) {
		if (keyState[j]) {
			keyState[j] = false;
			for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
				if (stack[0]->widgets[i]->getParent() == NULL) {
					stack[0]->widgets[i]->chainKeyUp(j);
				}
			}
		}
	}

	if (joyAxisDown) {
		joyAxisDown = false;
		for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
			if (stack[0]->widgets[i]->getParent() == NULL) {
				stack[0]->widgets[i]->chainJoyAxis(joyAxisStick, joyAxisAxis, 0.0);
			}
		}
	}

	if (joyButtonDown) {
		joyButtonDown = false;
		for (size_t i = 0; i < stack[0]->widgets.size(); i++) {
			if (stack[0]->widgets[i]->getParent() == NULL) {
				stack[0]->widgets[i]->chainJoyButtonUp(joyButtonDownNum);
			}
		}
	}
}

} // end namespace tgui

static void drawRect(tgui::TGUI *gui, int x1, int y1, int x2, int y2)
{
	for (size_t i = 0; i < gui->widgets.size(); i++) {
		tgui::TGUIWidget* widget = gui->widgets[i];
		if (widget->getParent() == NULL) {
			widget->chainDraw();
		}
	}
}

