#ifndef _widgets_h
#define _widgets_h

#include <map>
#include <string>

#include <atlas.h>
#include <tgui2.hpp>
#include <tgui2_widgets.hpp>

class B_TileSelector : public TGUI_Extended_Widget
{
public:
	virtual void draw(int abs_x, int abs_y);
	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);

	ATLAS *getAtlas(void);
	int getSelected(void);

	std::string getName(int id);
	int getIndex(std::string name);
	float getScale(void);
	void set_draw(bool do_draw);

	B_TileSelector();
	virtual ~B_TileSelector();

protected:

	std::map<int, std::string> tile_names;
	ATLAS *atlas;
	int selected;
	int start_w, start_h;
	float last_scale;
	bool do_draw;
};

class B_BattleCanvas : public TGUI_Extended_Widget
{
public:
	enum Tool {
		TOOL_NONE = 0,
		TOOL_MOVE,
		TOOL_HAND,
		TOOL_ADD_POINT,
		TOOL_DELETE_POINT,
		TOOL_CONNECT_POINTS
	};
	
	struct Tile {
		int index;
		int x;
		int y;
	};
	struct Point {
		int x, y;
		Point *left, *right;
		Point(int x, int y) : x(x), y(y), left(NULL), right(NULL) {}
	};


	virtual void draw(int abs_x, int abs_y);
	virtual void keyDown(int keycode);
	virtual void keyUp(int keycode);
	virtual void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	virtual void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y);
	virtual void mouseMoveAll(tgui::TGUIWidget *leftOut, int abs_x, int abs_y);
	virtual void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	virtual tgui::TGUIWidget *update(void);

	float getScale(void);
	void setTool(Tool t);
	Tool getTool(void);
	void clearSelected(void);
	void clearSelectedPoint(void);
	void setLayer(int l);
	int getLayer(void);
	int getNumLayers(void);
	void addTile(int index, int x, int y);
	bool getVisible(int layer);
	void setVisible(int layer, bool value);
	void addLayer(int before); // -1 = at end
	void deleteLayer(int layer);
	std::vector<Tile> &getTiles(int layer);
	std::vector<Tile> copy(void);
	void _delete(void);
	void paste(std::vector<Tile> v);
	void selectAll(void);
	void invertSelection(void);
	void makeOutlines(void);
	void adjustSize(int top, int right, int bottom, int left);
	void select(Tile *t);
	void setParallaxDrawer(void (*drawParallax)(int abs_x, int abs_y));
	std::vector< std::vector<Point *> > getLines(void);
	void setLines(std::vector< std::vector<Point *> > l);
	void getStartSize(int *w, int *h);
	
	B_BattleCanvas &operator=(const B_BattleCanvas& rhs);

	B_BattleCanvas(void);
	B_BattleCanvas(ATLAS *atlas, int width, int height);
	virtual ~B_BattleCanvas(void);

protected:
	static const int START_MOVE_DELAY = (250/16); // 1/4 second @ 60hz

	void moveSelected(void);
	Tile *getClickedTile(int layer, int mx, int my);

	void findNearestPoint(int x, int y, int *line_out, int *pt_out);
	int findNearestUnownedPoint(int x, int y);
	void connectPoints(int x1, int y1, int x2, int y2);
	void addPoint(int x, int y);
	void deletePoint(int x, int y);
	void selectPoint(int x, int y);
	void nearestIsUnowned(
		int x, int y, int l, int p, int nearest, bool *niu,
		bool *bothNull);

	ATLAS *atlas;
	int movingX;
	int movingY;
	int layer;
	bool mouseIsDown;
	Tool tool;
	std::vector<Tile *> selected;
	std::vector<ALLEGRO_VERTEX *> selectedOutlines;
	std::vector<int> outlineSizes;
	std::vector< std::vector<Tile> > tiles; // layers
	std::vector<bool> visible;
	int moveDelay;
	int start_w, start_h;
	float last_scale;
	float brightness;
	float binc;
	bool controlDown;
	bool dragging;
	int lastMouseX;
	int lastMouseY;
	void (*drawParallax)(int abs_x, int abs_y);
	std::vector<Point *> unowned_points;
	std::vector< std::vector<Point *> > lines;

	bool isPointSelected;
	Point selectedPoint;

	int mouseX, mouseY;

	float dragdiffx, dragdiffy;
};
	
void undo(void);
void redo(void);
void push_undo(void);
void clearUndoRedo(void);

#endif
