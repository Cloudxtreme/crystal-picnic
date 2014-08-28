#include <allegro5/allegro.h>
#include <cstdio>
#include <cmath>

void calc_toss(
	float sx,
	float sy,
	float dx,
	float dy,
	float percent,
	float *outx,
	float *outy)
{
	float dist_x = dx - sx;
	float dist_y = dy - sy;
	float distance = sqrt(dist_x*dist_x + dist_y*dist_y);
	float diameter = distance;
	float start_angle = asin(0);
	float remain_angle = M_PI - start_angle;
	float want_angle = percent * remain_angle + start_angle;
	float angle = atan2(dist_y, dist_x);
	float real_start_x = cos(angle + M_PI) * (diameter - distance) + sx;
	float real_start_y = sin(angle + M_PI) * (diameter - distance) + sy;
	float center_x = (dx + real_start_x) / 2;
	float center_y = (dy + real_start_y) / 2;
	float in_x = real_start_x - center_x;
	float in_y = real_start_y - center_y;
	float in_z = 0;
	angle += M_PI / 2.0f;
	float vx = cos(angle);
	float vy = sin(angle);
	ALLEGRO_TRANSFORM t;
	al_identity_transform(&t);
	al_rotate_transform_3d(&t, vx, vy, 0, want_angle);
	float res_x = t.m[0][0] * in_x + t.m[1][0] * in_y + t.m[2][0] * in_z + t.m[3][0];
	float res_y = t.m[0][1] * in_x + t.m[1][1] * in_y + t.m[2][1] * in_z + t.m[3][1];
	float res_z = t.m[0][2] * in_x + t.m[1][2] * in_y + t.m[2][2] * in_z + t.m[3][2];
	*outx = res_x + center_x;
	*outy = res_y + center_y - res_z;
}

int main(int argc, char **argv)
{
	al_init();

	ALLEGRO_DISPLAY *d = al_create_display(1280, 720);

	float scale = 1280.0f / 285.0f;

	float sx = 285/2-30;
	float sy = 40;
	float dx = 0;
	float dy = 160;
	sx *= scale;
	sy *= scale;
	dx *= scale;
	dy *= scale;
	float x, y;

	for (float p = 0.0f; p < 1.0f; p += 0.001f) {
		calc_toss(sx, sy, dx, dy, p, &x, &y);
		al_put_pixel(x, y, al_map_rgb(255, 255, 255));
		al_flip_display();
		al_rest(0.001);
	}

	al_rest(5);
}

