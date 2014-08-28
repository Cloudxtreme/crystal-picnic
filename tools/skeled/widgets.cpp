void Timeline::draw(int abs_x, int abs_y)
{
	al_draw_filled_rectangle(abs_x, abs_y, abs_x+width, abs_y+height, al_color_name("gray"));

	if (skeleton->get_animations().size() == 0) {
		return;
	}

	Skeleton::Animation *anim = skeleton->get_animations()[skeleton->get_curr_anim()];
	int num_frames = anim->frames.size();

	if (num_frames == 0) {
		return;
	}

	ALLEGRO_COLOR fore, back;
	tguiWidgetsGetColors(&fore, &back);

	float xx = x_offs;

	int old_time = anim->curr_time;
	int old_frame = anim->curr_frame;
	anim->curr_time = 0;

	for (int i = 0; i < num_frames; i++) {
		anim->curr_frame = i;
		interpolate();
		al_draw_filled_rectangle(xx+abs_x, abs_y, xx+abs_x+100, abs_y+100, old_frame == i ? al_color_name("white") : back);
		al_draw_line(xx+abs_x+99.5, abs_y, xx+abs_x+99.5, abs_y+100.5, al_color_name("black"), 1);
		ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
		al_set_target_bitmap(preview_work);
		::draw(false, false);
		al_set_target_bitmap(old_target);
		al_draw_scaled_bitmap(preview_work, 200, 200, 400, 300, xx+abs_x+10, abs_y+10, 80, 60, 0);
		al_draw_textf(tgui::getFont(), al_color_name("black"), xx+abs_x+3, abs_y+100-20, 0, "%d", i);
		al_draw_textf(tgui::getFont(), al_color_name("black"), xx+abs_x+96, abs_y+100-20, ALLEGRO_ALIGN_RIGHT, "%dms", anim->delays[i]);
		xx += 100;
	}
	
	anim->curr_time = old_time;
	anim->curr_frame = old_frame;
	interpolate();
}

void Timeline::mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (down) {
		down = false;

		Skeleton::Animation *anim = skeleton->get_animations()[skeleton->get_curr_anim()];

		if (!scrolling) {
			int xx = rel_x + -x_offs;
			xx /= 100;
			if (xx < (int)anim->frames.size()) {
				anim->curr_frame = xx;
				interpolate();
			}
		}
	}
}

void Timeline::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (skeleton->get_animations().size() == 0) {
		return;
	}
	
	Skeleton::Animation *anim = skeleton->get_animations()[skeleton->get_curr_anim()];
	int num_frames = anim->frames.size();

	if (num_frames == 0) {
		return;
	}

	if (rel_x >= 0 && rel_y >= 0) {
		down = true;
		scrolling = false;
		down_x = abs_x;
		down_y = abs_y;
	}
}

void Timeline::mouseMove(int rel_x, int rel_y, int abs_x, int abs_y)
{
	if (down) {
		Skeleton::Animation *anim = skeleton->get_animations()[skeleton->get_curr_anim()];

		int dx = abs_x - down_x;
		int dy = abs_y - down_y;

		if (!scrolling) {
			if (abs(dx) > 5 || abs(dy) > 5) {
				scrolling = true;
			}
		}
		if (scrolling) {
			x_offs += dx;
			if (x_offs > 0) x_offs = 0;
			int min = (anim->frames.size() <= 8) ? 0 : -(anim->frames.size()*100 - 800);
			if (x_offs < min) x_offs = min;
			down_x = abs_x;
			down_y = abs_y;
		}
	}
}

Timeline::Timeline(int x, int y, int width, int height, Skeleton::Skeleton *skeleton) :
	skeleton(skeleton),
	x_offs(0.0f)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	preview_work = al_create_bitmap(800, 600);
	down = false;
}

Timeline::~Timeline()
{
	al_destroy_bitmap(preview_work);
}

static void real_hierarchy_draw(
	Skeleton::Link *l,
	int parent_x,
	int parent_y,
	int this_x,
	int this_y)
{
	int num_children = l->num_children;

	int xx = this_x - (num_children*100+(num_children-1)*10)/2;
	int yy = this_y - 50;

	for (int i = 0; i < num_children; i++) {
		Skeleton::Part *p = l->children[i]->part;
		ALLEGRO_BITMAP *bmp;
		bmp = p->get_bitmaps()[p->get_curr_bitmap()]->bitmap;

		int w, h, new_w, new_h;

		get_bitmap_icon_sizes(bmp, &w, &h, &new_w, &new_h);

		int nc = l->children[i]->num_children;

		float a = (M_PI/4 + M_PI/2) - M_PI/2/nc*i;

		int cx = this_x + cos(a) * 300;
		int cy = this_y + sin(a) * 300;
		
		real_hierarchy_draw(
			l->children[i],
			xx+50, yy+50,
			cx, cy);

		al_draw_scaled_bitmap(
			bmp,
			0, 0, w, h,
			xx+(100-new_w)/2, yy+(100-new_h)/2, new_w, new_h,
			0
		);

		al_draw_line(xx+50, yy+50, parent_x, parent_y, al_color_name("white"), 1);

		if (l->children[i]->part->get_name() == selected_part) {
			draw_blinking_rect(
				xx, yy, xx+100, yy+100
			);
		}

		ALLEGRO_MOUSE_STATE state;
		al_get_mouse_state(&state);
		if (state.buttons) {
			if (state.x >= xx && state.y >= yy && state.x < xx+100 && state.y < yy+100) {
				selected_part = l->children[i]->part->get_name();
			}
		}

		if ((i+1) % 5 == 0) {
			xx = this_x - (num_children*100+(num_children-1)*10)/2;
			yy += 110;
		}
		else {
			xx += 110;
		}
	}
}

void Hierarchy::draw(int abs_x, int abs_y)
{
	if (skeleton->get_animations().size() == 0) {
		return;
	}
	
	Skeleton::Animation *anim = skeleton->get_animations()[skeleton->get_curr_anim()];
	int num_frames = anim->frames.size();

	if (num_frames == 0) {
		return;
	}

	ALLEGRO_COLOR fore, back;
	tguiWidgetsGetColors(&fore, &back);

	al_draw_filled_rectangle(
		x, y,
		x+width, y+height,
		back
	);

	if (anim->frames[anim->curr_frame]->num_children > 0) {
		real_hierarchy_draw(
			anim->frames[anim->curr_frame],
			abs_x+400+offset.x, abs_y+60+offset.y,
			abs_x+400+offset.x, abs_y+260+offset.y
		);
	}

	Skeleton::Part *p = anim->frames[anim->curr_frame]->part;
	ALLEGRO_BITMAP *bmp = p->get_bitmaps()[p->get_curr_bitmap()]->bitmap;

	int w, h, new_w, new_h;
	get_bitmap_icon_sizes(bmp, &w, &h, &new_w, &new_h);

	al_draw_scaled_bitmap(
		bmp,
		0, 0, w, h,
		abs_x+350+offset.x+(100-new_w)/2,
		abs_y+10+offset.y+(100-new_h)/2,
		new_w, new_h,
		0
	);

	int xx = abs_x+350+offset.x;
	int yy = abs_y+10+offset.y;

	if (anim->work->part->get_name() == selected_part) {
		draw_blinking_rect(
			xx, yy,
			xx+100, yy+100
		);
	}

	ALLEGRO_MOUSE_STATE state;
	al_get_mouse_state(&state);
	if (state.buttons) {
		if (state.x >= xx && state.y >= yy && state.x < xx+100 && state.y < yy+100) {
			selected_part = anim->work->part->get_name();
		}
	}
}

void Hierarchy::mouseUp(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (down) {
		down = false;
	}
}

void Hierarchy::mouseDown(int rel_x, int rel_y, int abs_x, int abs_y, int mb)
{
	if (rel_x >= 0 && rel_y >= 0) {
		down = true;
		down_x = abs_x;
		down_y = abs_y;
	}
}

void Hierarchy::mouseMove(int rel_x, int rel_y, int abs_x, int abs_y)
{
	if (down) {
		int dx = abs_x - down_x;
		int dy = abs_y - down_y;

		offset.x += dx;
		offset.y += dy;

		down_x = abs_x;
		down_y = abs_y;
	}
}

Hierarchy::Hierarchy(int x, int y, int width, int height, Skeleton::Skeleton *skeleton) :
	skeleton(skeleton),
	offset(0, 0)
{
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
	down = false;
}

Hierarchy::~Hierarchy()
{
}

