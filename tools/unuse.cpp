#include <cstdio>
#include <stdint.h>
#include <cstdlib>
#include <cstring>

uint32_t read32bits(FILE *f)
{
	int b1 = fgetc(f);
	int b2 = fgetc(f);
	int b3 = fgetc(f);
	int b4 = fgetc(f);
	return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}

void write32bits(FILE *f, int i) {
	fputc((i >> 0) & 0xff, f);
	fputc((i >> 8) & 0xff, f);
	fputc((i >> 16) & 0xff, f);
	fputc((i >> 24) & 0xff, f);
}

void list(const char *area_name, int num_sheets)
{
	bool used[num_sheets];
	memset(used, 0, num_sheets*sizeof(bool));

	FILE *f = fopen(area_name, "rb");

	int w = read32bits(f);
	int h = read32bits(f);
	int layers = read32bits(f);

	for (int l = 0; l < layers; l++) {
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				int n = read32bits(f);
				int sheet = fgetc(f);
				int solid = fgetc(f);
				if (sheet >= 0)
					used[sheet] = true;
			}
		}
	}
	
	fclose(f);
	
	for (int i = 0; i < num_sheets; i++) {
		if (!used[i])
			printf("%d unused\n", i);
	}
}

void unuse(const char *area_name, int layer_to_delete)
{
	FILE *f = fopen(area_name, "rb");

	int w = read32bits(f);
	int h = read32bits(f);
	int layers = read32bits(f);

	write32bits(stdout, w);
	write32bits(stdout, h);
	write32bits(stdout, layers);

	for (int l = 0; l < layers; l++) {
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				int n = read32bits(f);
				int sheet = fgetc(f);
				int solid = fgetc(f);

				write32bits(stdout, n);

				if (sheet > layer_to_delete) {
					sheet--;
				}
				
				fputc(sheet, stdout);
				fputc(solid, stdout);
			}
		}
	}
	
	fclose(f);
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("Usage: unuse [-list] <area> <num>\n");
		return 0;
	}

	const char *arg1 = argv[1];
	const char *arg2 = argv[2];

	if (!strcmp(arg1, "-list")) {
		list(arg2, atoi(argv[3]));
	}
	else {
		unuse(arg1, atoi(arg2));
	}
	
	return 0;
}

