#ifndef _WIDGETS_H
#define _WIDGETS_H

#include "tgui2.hpp"

#include <string>
#include <vector>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

enum TGUI_Direction
{
	TGUI_HORIZONTAL = 0,
	TGUI_VERTICAL
};

class TGUI_Extended_Widget : public tgui::TGUIWidget
{
public:
	void setTamperingEnabled(bool tampering) {
		this->tampering = tampering;
	}
	bool getTamperingEnabled() {
		return tampering;
	}
	void setX(float x) {
		if (tampering) {
			this->x = x;
		}
	}
	void setY(float y) {
		if (tampering) {
			this->y = y;
		}
	}
	void setWidth(int width) {
		if (tampering) {
			this->width = width;
		}
	}
	void setHeight(int height) {
		if (tampering) {
			this->height = height;
		}
	}

	TGUI_Extended_Widget();
	virtual ~TGUI_Extended_Widget() {}

protected:
	bool resizable;

	bool tampering;
};

class TGUI_Checkbox : public TGUI_Extended_Widget
{
public:
	void draw(int abs_x, int abs_y);
	void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);

	bool getChecked();
	void setChecked(bool checked);

	TGUI_Checkbox(int x, int y, int w, int h, bool checked);
	virtual ~TGUI_Checkbox();

protected:
	bool checked;
};

class TGUI_Icon : public TGUI_Extended_Widget
{
public:
	bool acceptsFocus();
	void draw(int abs_x, int abs_y);
	void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	tgui::TGUIWidget *update();
	void keyDown(int keycode);
	void joyButtonDown(int button);

	void setClearColor(ALLEGRO_COLOR c);

	// image gets destroyed in destructor
	TGUI_Icon(ALLEGRO_BITMAP *image, int x, int y, int flags);
	virtual ~TGUI_Icon();

protected:
	int flags;
	ALLEGRO_BITMAP *image;
	bool clicked;
	ALLEGRO_COLOR clear_color;
};

class TGUI_IconButton : public TGUI_Icon
{
public:
	void draw(int abs_x, int abs_y);

	// image gets destroyed in destructor
	TGUI_IconButton(ALLEGRO_BITMAP *image, int x, int y, 
		int width, int height, int ico_ofs_x, int ico_ofs_y, int flags);
	virtual ~TGUI_IconButton();

protected:
	int ico_ofs_x, ico_ofs_y;
};

class TGUI_Splitter : public TGUI_Extended_Widget
{
public:
	void draw(int abs_x, int abs_y);
	void keyDown(int keycode);
	void keyUp(int keycode);
	bool keyChar(int keycode, int unichar);
	void joyButtonDown(int button);
	void joyButtonDownRepeat(int button);
	void joyButtonUp(int button);
	void joyAxis(int stick, int axis, float value);
	bool joyAxisRepeat(int stick, int axis, float value);
	void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void mouseDownAll(TGUIWidget *leftOut, int abs_x, int abs_y, int mb);
	void mouseMoveAll(tgui::TGUIWidget *leftOut, int abs_x, int abs_y);
	void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y);
	tgui::TGUIWidget *update();
	bool getAbsoluteChildPosition(tgui::TGUIWidget *child, int *x, int *y);
	void addCollidingChildrenToVector(std::vector<tgui::TGUIWidget *> &v, tgui::TGUIWidget *exception, int x1, int y1, int x2, int y2);

	void set_resizable(int split, bool value);
	int get_size(int index);
	void set_size(int index, int size);
	void set_widget(int index, TGUIWidget *widget);
	void setClearColor(ALLEGRO_COLOR c);
	void layout();
	std::vector<tgui::TGUIWidget *> &getWidgets();
	void setDrawLines(bool drawLines);
	void setWidth(int w);
	void setHeight(int h);
	void setPadding(int hpadding, int vpadding);

	TGUI_Splitter(
		int x, int y,
		int w, int h,
		TGUI_Direction dir,
		bool can_resize,
		std::vector<tgui::TGUIWidget *> widgets
	);

	virtual ~TGUI_Splitter();

protected:
	class TGUI_DummyWidget : public TGUI_Extended_Widget
	{
	public:
		TGUI_DummyWidget(int x, int y) {
			this->x = x;
			this->y = y;
		}
		virtual ~TGUI_DummyWidget() {}
	};

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
	int hpadding;
	int vpadding;
};

class TGUI_MenuBar;

class TGUI_TextMenuItem : public TGUI_Extended_Widget
{
public:
	static const int HEIGHT = 16;

	virtual void close() {};
	void setMenuBar(TGUI_MenuBar *menuBar);
	int getShortcutKeycode();

	virtual void draw(int abs_x, int abs_y);
	virtual tgui::TGUIWidget *update();
	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y);
	virtual void mouseMoveAll(tgui::TGUIWidget *leftOut, int abs_x, int abs_y);

	TGUI_TextMenuItem(std::string name, int shortcut_keycode);
	virtual ~TGUI_TextMenuItem() {}

protected:
	std::string name;
	int shortcut_keycode;
	bool clicked;
	bool hover;
	TGUI_MenuBar *menuBar;
};

class TGUI_CheckMenuItem : public TGUI_TextMenuItem
{
public:
	void draw(int abs_x, int abs_y);
	tgui::TGUIWidget *update();

	bool isChecked();
	void setChecked(bool checked);
	
	TGUI_CheckMenuItem(std::string name, int shortcut_keycode, bool checked);
	virtual ~TGUI_CheckMenuItem() {}

protected:
	bool checked;
};

struct TGUI_RadioGroup {
	int selected;
};

class TGUI_RadioMenuItem : public TGUI_TextMenuItem
{
public:
	void draw(int abs_x, int abs_y);
	tgui::TGUIWidget *update();

	bool isSelected();
	void setSelected();

	TGUI_RadioMenuItem(std::string name, int shortcut_keycode, TGUI_RadioGroup *group, int id);
	virtual ~TGUI_RadioMenuItem() {}

protected:
	TGUI_RadioGroup *group;
	int id;
};

class TGUI_SubMenuItem : public TGUI_TextMenuItem
{
public:
	void close();

	void draw(int abs_x, int abs_y);
	void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y);

	bool isOpen();
	TGUI_Splitter *getSubMenu();
	void setSubMenu(TGUI_Splitter *sub);
	void setParentSplitter(TGUI_Splitter *splitter);

	TGUI_SubMenuItem(std::string name, TGUI_Splitter *sub_menu);
	virtual ~TGUI_SubMenuItem() {}

protected:
	bool is_open;
	TGUI_Splitter *sub_menu;
	TGUI_Splitter *parentSplitter;
};

class TGUI_MenuBar : public TGUI_Extended_Widget
{
public:
	static const int PADDING = 10;
	static const int HEIGHT = 16;
	
	void close();

	void draw(int abs_x, int abs_y);
	void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void keyDown(int keycode);
	tgui::TGUIWidget *update();

	TGUI_MenuBar(
		int x, int y, int w, int h,
		std::vector<std::string> menu_names,
		std::vector<TGUI_Splitter *> menus
	);
protected:
	void setSubMenuSplitters(TGUI_Splitter *root);
	void checkKeys(int keycode, TGUI_Splitter *splitter);

	std::vector<std::string> menu_names;
	std::vector<TGUI_Splitter *> menus;
	TGUI_Splitter *open_menu;
	bool close_menu;
	tgui::TGUIWidget *itemToReturn;
};

class TGUI_ScrollPane : public TGUI_Extended_Widget
{
public:
	static const int SCROLLBAR_THICKNESS = 16;
	static const int MIN_SCROLLBAR_SIZE = 16;
	
	void draw(int abs_x, int abs_y);
	void keyDown(int keycode);
	void keyUp(int keycode);
	bool keyChar(int keycode, int unichar);
	void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y);
	void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void mouseMoveAll(TGUIWidget *leftOut, int abs_x, int abs_y);
	void chainDraw();
	TGUIWidget *chainMouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);

	void get_values(float *ox, float *oy);
	void setValues(float ox, float oy);
	void get_pixel_offsets(int *xx, int *yy);

	TGUI_ScrollPane(tgui::TGUIWidget *child);

protected:
	enum Down_Type {
		DOWN_V,
		DOWN_H,
		DOWN_NONE
	};

	int get_scrollbar_size(int content_size, int scrollbar_size);
	int get_scrollbar_range(int content_size, int scrollbar_size);
	void get_vtab_details(int *x1, int *y1, int *x2, int *y2);
	void get_htab_details(int *x1, int *y1, int *x2, int *y2);

	float ox, oy;
	
	Down_Type down;
	int down_x, down_y;
	float down_ox, down_oy;
};

class TGUI_Slider : public TGUI_Extended_Widget
{
public:
	static const int TAB_SIZE = 8;

	virtual void draw(int abs_x, int abs_y);
	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	virtual void mouseMoveAll(tgui::TGUIWidget *leftOut, int abs_x, int abs_y);
	virtual void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb);

	float getPosition();
	void setPosition(float pos);
	void setCallback(void (*callback)(float pos));

	TGUI_Slider(int x, int y, int size, TGUI_Direction direction);

protected:
	int size;
	TGUI_Direction direction;
	float pos;
	bool dragging;
	void (*callback)(float);
};

class TGUI_Button : public TGUI_Icon
{
public:
	bool acceptsFocus();
	void draw(int abs_x, int abs_y);

	TGUI_Button(std::string text, int x, int y, int w, int h);

protected:
	std::string text;
};
	
class TGUI_TextField : public TGUI_Extended_Widget
{
public:
	static const int PADDING = 3;

	bool acceptsFocus();
	void draw(int abs_x, int abs_y);
	bool keyChar(int keycode, int unichar);

	void setValidator(bool (*validate)(const std::string str));
	bool isValid();
	std::string getText();
	void setText(std::string s);

	TGUI_TextField(std::string startStr, int x, int y, int width);
	virtual ~TGUI_TextField();

protected:
	void findOffset();

	std::string str;
	int cursorPos;
	int offset;
	bool (*validate)(const std::string str);
};

// only need modal frame right now so this one won't be draggable (yet)
class TGUI_Frame : public TGUI_Extended_Widget
{
public:
	static const int TITLE_PADDING = 3;

	void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y);
	void draw(int abs_x, int abs_y);
	bool getAbsoluteChildPosition(tgui::TGUIWidget *widget, int *x, int *y);

	int barHeight();

	TGUI_Frame(std::string title, int x, int y, int width, int height);
	virtual ~TGUI_Frame();

protected:

	std::string title;
	bool dragging;
	int drag_x, drag_y;
};

class TGUI_Label : public TGUI_Extended_Widget
{
public:
	void draw(int abs_x, int abs_y);

	void setText(std::string text);

	TGUI_Label(std::string text, ALLEGRO_COLOR color, int x, int y, int flags);
	virtual ~TGUI_Label();

protected:

	std::string text;
	ALLEGRO_COLOR color;
	int flags;
};

class TGUI_List : public TGUI_Extended_Widget
{
public:
	void draw(int abs_x, int abs_y);

	const std::vector<std::string> &getLabels();
	void setLabels(const std::vector<std::string> &labels);
	void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);

	int getSelected() { return selected; }
	void setSelected(int selected) { this->selected = selected; }

	TGUI_List(int x, int y, int width);
	virtual ~TGUI_List();

protected:

	std::vector<std::string> labels;
	int selected;
};

void tguiWidgetsSetColors(ALLEGRO_COLOR fore, ALLEGRO_COLOR back);
void tguiWidgetsGetColors(ALLEGRO_COLOR *fore, ALLEGRO_COLOR *back);

#endif // _WIDGETS_H
