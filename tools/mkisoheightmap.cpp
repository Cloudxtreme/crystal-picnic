#include <allegro5/allegro.h>
//#include <zlib.h>
#include <string>
#include <vector>

int width, height;
int pixel_w, pixel_h;
int pitch;
unsigned char *heightmap;

struct Point
{
	float x, y;
};

Point iso_offset;

void reverse_iso_project(int *x, int *y, Point iso_offset) {
	*x -= iso_offset.x;
	*y -= iso_offset.y;

	double xinc = -2*(*y);
	double yinc = (*x) / 2;

	*x = (int)((*x) - xinc);
	*y = (int)(2 * ((*y) - yinc));
}

void iso_project(int *x, int *y, Point iso_offset) {
	double xinc = -(*y) / 2.0;
	double yinc = (*x) / 4.0;
	
	*x = (int)(((float)(*x)/2) + xinc);
	*y = (int)(((float)(*y)/4) + yinc);

	*x += iso_offset.x;
	*y += iso_offset.y;
}

unsigned char *load_from_zip(std::string filename)
{
	ALLEGRO_FILE *f = al_fopen(filename.c_str(), "rb");
	long size = al_fsize(f);
	unsigned char *bytes;
	if (size < 0) {
		std::vector<char> v;
		int c;
		while ((c = al_fgetc(f)) != EOF) {
			v.push_back(c);
		}
		bytes = new unsigned char[v.size()+1];
		for (unsigned int i = 0; i < v.size(); i++) {
			bytes[i] = v[i];
		}
	}
	else {
		bytes = new unsigned char[size+1];
		al_fread(f, bytes, size);
	}
	al_fclose(f);
	bytes[size] = 0;
	return bytes;
}

// create a pixel for pixel heightmap from a tile based one
void load_isometric_heightmap(std::string filename)
{
	pitch = pixel_w / 2 + pixel_w % 2;
	heightmap = new unsigned char [pitch*pixel_h];
	memset(heightmap, 8, pitch*pixel_h);

	int tile_pitch = width / 2 + width % 2;
	unsigned char *tile_heightmap = load_from_zip(filename.c_str());

	for (int y = 0; y < pixel_h; y++) {
		for (int x = 0; x < pixel_w; x++) {
			int tmp_x = x;
			int tmp_y = y;
			reverse_iso_project(&tmp_x, &tmp_y, iso_offset);
			if (tmp_x < 0 || tmp_y < 0 || tmp_x >= width*64 || tmp_y >= height*64)
				continue;
			tmp_x /= 64;
			bool tile_use_low = tmp_x % 2 == 0;
			tmp_x /= 2;
			tmp_y /= 64;
			int tile_byte = tile_heightmap[tmp_y*tile_pitch+tmp_x];
			int hm_offset = y * pitch + x / 2;
			int hm_curr = heightmap[hm_offset];
			bool hm_use_low = x % 2 == 0;
			int height;
			if (tile_use_low) {
				printf("lo");
				height = tile_byte & 0xf;
			}
			else {
				printf("hi");
				height = (tile_byte >> 4) & 0xf;
			}
			if (hm_use_low) {
				hm_curr = (hm_curr & 0xf0) | height;
			}
			else {
				hm_curr = (hm_curr & 0xf) | (height << 4);
			}
			heightmap[hm_offset] = hm_curr;
		}
	}

	delete[] tile_heightmap;
}
	
int readInt(FILE *f) {
	unsigned char b1 = fgetc(f);
	unsigned char b2 = fgetc(f);
	unsigned char b3 = fgetc(f);
	unsigned char b4 = fgetc(f);
	return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("Usage: %s <level> <tile_heightmap> <output>", argv[0]);
	}	
	al_init();
	FILE *f = fopen(argv[1], "rb");
	width = readInt(f);
	height = readInt(f);
	fclose(f);

	pixel_w = width*64;
	pixel_h = height*64;
	iso_offset.x = (width > height ? width : height) * 64 / 2;
	iso_offset.y = 0;
	int startx = pixel_w;
	int starty = 0;
	iso_project(&pixel_w, &starty, iso_offset);
	iso_project(&startx, &pixel_h, iso_offset);

	load_isometric_heightmap(std::string(argv[2]));

	/*
	gzFile out = gzopen(argv[3], "wb");
	for (int i = 0; i < pitch*pixel_h; i++) {
		gzputc(out, heightmap[i]);
	}
	gzclose(out);
	*/
	FILE *out = fopen(argv[3], "wb");
	for (int i = 0; i < pitch*pixel_h; i++) {
		fputc(heightmap[i], out);
	}
	fclose(out);
}


