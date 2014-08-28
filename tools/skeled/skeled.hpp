extern std::string selected_part;

void draw(bool main_drawing_call = true, bool flip = true);
void interpolate();
void get_bitmap_icon_sizes(ALLEGRO_BITMAP *bmp, int *w, int *h, int *new_w, int *new_h);
void draw_blinking_rect(float x1, float y1, float x2, float y2);

