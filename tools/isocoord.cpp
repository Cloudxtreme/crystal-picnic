#include <stdlib.h>
#include <stdio.h>

void iso_project(int *x, int *y, float offsx, float offsy) {
	double xinc = -(*y) / 2.0;
	double yinc = (*x) / 4.0;
	
	*x = (int)(((float)(*x)/2) + xinc);
	*y = (int)(((float)(*y)/4) + yinc);

	*x += offsx;
	*y += offsy;
}

void reverse_iso_project(int *x, int *y, float offsx, float offsy) {
	*x -= offsx;
	*y -= offsy;

	double xinc = -2*(*y);
	double yinc = (*x) / 2;

	*x = (int)((*x) - xinc);
	*y = (int)(2 * ((*y) - yinc));
}

int main(int argc, char **argv)
{
	FILE *f = fopen(argv[1], "rb");
	int width = (fgetc(f)) | (fgetc(f) << 8) | (fgetc(f) << 16) | (fgetc(f) << 24);
	int height = (fgetc(f)) | (fgetc(f) << 8) | (fgetc(f) << 16) | (fgetc(f) << 24);
	fclose(f);

	float offsx = 0; //(width > height ? width : height) * 64 / 2;
	float offsy = 0;

	int startx = atoi(argv[2]);
	int starty = atoi(argv[3]);
	iso_project(&startx, &starty, offsx, offsy);

	printf("%d, %d\n", startx, starty);
}
