#ifndef TGUI_H
#define TGUI_H

#include <vector>
#include <algorithm>

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#ifndef ALLEGRO_WINDOWS
#include <sys/time.h>
#endif

namespace tgui {

// forward declarations
void getScreenSize(int *w, int *h);

class TGUIWidget {
public:
	friend void drawRect(int x1, int y1, int x2, int y2);
	friend void handleEvent_pretransformed(void *allegro_event);
	friend void handleEvent(void *allegro_event);

	float getX() { return x; }
	float getY() { return y; }
	virtual void setX(float newX) { x = newX; }
	virtual void setY(float newY) { y = newY; }

	int getWidth() { return width; }
	int getHeight() { return height; }
	virtual void setWidth(int w) { width = w; }
	virtual void setHeight(int h) { height = h; }

	TGUIWidget *getParent() { return parent; }
	void setParent(TGUIWidget *p) { parent = p; }

	TGUIWidget *getChild() { return child; }
	void setChild(TGUIWidget *c) { child = c; }
	virtual bool getAbsoluteChildPosition(TGUIWidget *child, int *x, int *y) { return false; }

	virtual void draw(int abs_x, int abs_y) {}
	// -- only called if registered
	virtual void preDraw(int abs_x, int abs_y) {}
	virtual void postDraw(int abs_x, int abs_y) {}
	// --

	virtual TGUIWidget *update() {
		TGUIWidget *w;
		if (child) {
			w = child->update();
			if (w) return w;
		}
		return NULL;
	}
	virtual void resize() {
		resize_self();
		resize_child();
	}
	virtual void translate(int xx, int yy) {
		if (child) {
			child->translate(xx, yy);
		}
	}

	virtual void raise();
	virtual void lower();

	// give relative and absolute coordinates. rel_x/y can be -1 if not
	// over widget
	// keyChar and joyAxis should return true if a directional event was used
	// (ie left/right/up/down arrows/axis)
	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y) {}
	virtual void mouseScroll(int z, int w) {}
	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb) {}
	virtual void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int b) {}
	virtual void keyDown(int keycode) {}
	virtual void keyUp(int keycode) {}
	virtual bool keyChar(int keycode, int unichar) { return false; }
	virtual void joyButtonDown(int button) {}
	virtual void joyButtonDownRepeat(int button) {}
	virtual void joyButtonUp(int button) {}
	virtual void joyAxis(int stick, int axis, float value) {}
	virtual bool joyAxisRepeat(int stick, int axis, float value) { return false; }

	virtual void mouseMoveAll(TGUIWidget *leftOut, int abs_x, int abs_y)
	{
		if (this != leftOut) {
			mouseMove(-1, -1, abs_x, abs_y);
		}
		if (child) {
			child->mouseMoveAll(leftOut, abs_x, abs_y);
		}
	}
	virtual void mouseDownAll(TGUIWidget *leftOut, int abs_x, int abs_y, int mb)
	{
		if (this != leftOut) {
			mouseDown(-1, -1, abs_x, abs_y, mb);
		}
		if (child) {
			child->mouseDownAll(leftOut, abs_x, abs_y, mb);
		}
	}
	virtual void mouseUpAll(TGUIWidget *leftOut, int abs_x, int abs_y, int mb)
	{
		if (this != leftOut) {
			mouseUp(-1, -1, abs_x, abs_y, mb);
		}
		if (child) {
			child->mouseUpAll(leftOut, abs_x, abs_y, mb);
		}
	}

	virtual void remove();
	
	virtual bool acceptsFocus() { return false; }


	virtual TGUIWidget *chainMouseMove(int rel_x, int rel_y, int abs_x, int abs_y, int z, int w);
	virtual TGUIWidget *chainMouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	virtual TGUIWidget *chainMouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int b);
	virtual void chainKeyDown(int keycode);
	virtual void chainKeyUp(int keycode);
	virtual bool chainKeyChar(int keycode, int unichar);
	virtual void chainJoyButtonDown(int button);
	virtual void chainJoyButtonDownRepeat(int button);
	virtual void chainJoyButtonUp(int button);
	virtual void chainJoyAxis(int stick, int axis, float value);
	virtual bool chainJoyAxisRepeat(int stick, int axis, float value);
	virtual void chainDraw();

	virtual void losingFocus() {}
	virtual void gainingFocus() {}

	virtual void addCollidingChildrenToVector(std::vector<tgui::TGUIWidget *> &v, tgui::TGUIWidget *exception, int x1, int y1, int x2, int y2) {}

	void setFocusGroup(int focusGroup, int numberInFocusGroup) {
		this->focusGroup = focusGroup;
		this->numberInFocusGroup = numberInFocusGroup;
	}
	bool getDrawFocus() { return drawFocus; }
	void setDrawFocus(bool draw) { drawFocus = draw; }

	TGUIWidget() :
		parent(NULL),
		child(NULL),
		focusGroup(0),
		drawFocus(true)
	{
	}

	virtual ~TGUIWidget() {}

protected:

	void resize_self() {
		if (parent) {
			width = parent->getWidth();
			height = parent->getHeight();
		}
		else {
			int w, h;
			tgui::getScreenSize(&w, &h);
			width = w;
			height = h;
		}
	}

	void resize_child() {
		if (child)
			child->resize();
	}

	float x;
	float y;
	float width;
	float height;
	TGUIWidget *parent;
	TGUIWidget *child;

	int focusGroup;
	int numberInFocusGroup;
	bool drawFocus;
};

long currentTimeMillis();
void init(ALLEGRO_DISPLAY *display);
void shutdown();
void setFocus(TGUIWidget *widget);
TGUIWidget *getFocussedWidget();
void focusPrevious();
void focusNext();
void translateAll(int x, int y);
void addWidget(TGUIWidget *widget);
TGUIWidget *update();
std::vector<TGUIWidget *> updateAll();
void draw();
void drawRect(int x1, int y1, int x2, int y2);
void push();
bool pop();
void setNewWidgetParent(TGUIWidget *parent);
TGUIWidget *getNewWidgetParent();
void centerWidget(TGUIWidget *widget, int x, int y);
bool widgetIsChildOf(TGUIWidget *widget, TGUIWidget *parent);
void setScale(float x_scale, float y_scale);
void setOffset(float x_offset, float y_offset);
void ignore(int type);
void convertMousePosition(int *x, int *y);
void bufferToScreenPos(int *x, int *y, int bw, int bh);
void handleEvent_pretransformed(void *allegro_event);
void handleEvent(void *allegro_event);
TGUIWidget *getTopLevelParent(TGUIWidget *widget);
ALLEGRO_FONT *getFont();
void setFont(ALLEGRO_FONT *font);
void determineAbsolutePosition(TGUIWidget *widget, int *x, int *y);
TGUIWidget *determineTopLevelOwner(int x, int y);
bool pointOnWidget(TGUIWidget *widget, int x, int y);
void resize(TGUIWidget *parent);
void clearClip();
void setClippedClip(int x, int y, int width, int height);
void setClip(int x, int y, int width, int height);
bool isClipSet();
void getClip(int *x, int *y, int *w, int *h);
void raiseWidget(TGUIWidget *widget);
void lowerWidget(TGUIWidget *widget);
bool isDeepChild(TGUIWidget *parent, TGUIWidget *widget);
std::vector<TGUIWidget *> removeChildren(TGUIWidget *widget);
void addPreDrawWidget(TGUIWidget *widget);
void addPostDrawWidget(TGUIWidget *widget);
bool isKeyDown(int keycode);
void setScreenSize(int w, int h);
ALLEGRO_DISPLAY *getDisplay();
void drawFocusRectangle(int x, int y, int w, int h);
bool checkBoxCollision(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
void hide();
void unhide();
void releaseKeysAndButtons();

} // End namespace tgui

#endif
