#include <cstdio>

long igetl(FILE *f)
{
	int b1 = fgetc(f);
	int b2 = fgetc(f);
	int b3 = fgetc(f);
	int b4 = fgetc(f);
	return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}

void iputl(long l, FILE *f)
{
	fputc((l >>  0) & 0xff, f);
	fputc((l >>  8) & 0xff, f);
	fputc((l >> 16) & 0xff, f);
	fputc((l >> 24) & 0xff, f);
}

struct Tile {
	int tile_num;
	char sheet_num;
	char solid;
};

void read_row(FILE *f, int w, Tile *row)
{
	for (int i = 0; i < w; i++) {
		row[i].tile_num = igetl(f);
		row[i].sheet_num = fgetc(f);
		row[i].solid = fgetc(f);
	}
}

void write_row(FILE *f, int w, Tile *row)
{
	for (int i = w-1; i >= 0; i--) {
		iputl(row[i].tile_num, f);
		fputc(row[i].sheet_num, f);
		fputc(row[i].solid, f);
	}
}

int main(int argc, char **argv)
{
	FILE *f = fopen(argv[1], "rb");
	FILE *out = fopen(argv[2], "wb");

	int w = igetl(f);
	int h = igetl(f);
	int num_layers = igetl(f);

	printf("w=%d h=%d layers=%d\n", w, h, num_layers);

	iputl(w, out);
	iputl(h, out);
	iputl(num_layers, out);

	Tile row[w];

	for (int l = 0; l < num_layers; l++) {
		for (int i = 0; i < h; i++) {
			read_row(f, w, row);
			write_row(out, w, row);
		}
	}

	fclose(f);
	fclose(out);
}
