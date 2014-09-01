class Timeline : public TGUI_Extended_Widget
{
public:
	void draw(int abs_x, int abs_y);

	void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y);

	void setSkeleton(Skeleton::Skeleton *skeleton) { this->skeleton = skeleton; }

	Timeline(int x, int y, int width, int height, Skeleton::Skeleton *skeleton);
	virtual ~Timeline();

protected:

	Skeleton::Skeleton *skeleton;
	ALLEGRO_BITMAP *preview_work;
	float x_offs;
	bool down, scrolling;
	int down_x, down_y;
};

class Hierarchy : public TGUI_Extended_Widget
{
public:
	void draw(int abs_x, int abs_y);

	void mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb);
	void mouseMove(int rel_x, int rel_y, int abs_x, int abs_y);

	Hierarchy(int x, int y, int width, int height, Skeleton::Skeleton *skeleton);
	virtual ~Hierarchy();

protected:

	Skeleton::Skeleton *skeleton;

	General::Point<float> offset;
	bool down;
	int down_x, down_y;
};

