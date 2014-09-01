#include <cstdio>
#include <cstdlib>

int main(int argc, char **argv)
{
	FILE *in = fopen(argv[1], "rb");
	FILE *out = fopen(argv[2], "wb");
	int w = atoi(argv[3]);
	int h = atoi(argv[4]);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x+=2) {
			int byte1 = fgetc(in);
			int byte2;
			if (x+1 < w)
				byte2 = fgetc(in);
			else
				byte2 = 8;
			int result = (byte1 >> 4) | (byte2 & 0xf0);
			fputc(result, out);
		}
	}

	fclose(in);
	fclose(out);
}

