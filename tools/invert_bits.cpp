#include <cstdio>

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("Usage: invert_bits <in> <out>\n");
		return 0;
	}

	FILE *in = fopen(argv[1], "rb");
	FILE *out = fopen(argv[2], "wb");

	int b;

	while ((b = fgetc(in)) != EOF) {
		b = ~b;
		fputc(b, out);
	}

	fclose(in);
	fclose(out);
}

