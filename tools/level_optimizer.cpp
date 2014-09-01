#include <cstdio>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <vector>

int readInt(FILE *f) {
	unsigned char b1 = fgetc(f);
	unsigned char b2 = fgetc(f);
	unsigned char b3 = fgetc(f);
	unsigned char b4 = fgetc(f);
	return b1 | (b2 << 8) | (b3 << 16) | (b4 << 24);
}

void writeInt(FILE *f, int i) {
	fputc((i >> 0) & 0xff, f);
	fputc((i >> 8) & 0xff, f);
	fputc((i >> 16) & 0xff, f);
	fputc((i >> 24) & 0xff, f);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("Usage: %s <level dir.zip>\n", argv[0]);
		return 0;
	}

	al_init();
	al_init_image_addon();
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);

	std::vector<ALLEGRO_BITMAP *> sheets;
	std::vector< std::vector<bool> > has_alpha;
	bool is_iso;

	char isometric_filename[1000];
	char area_filename[1000];
	char info_filename[1000];

	sprintf(isometric_filename, "%s/isometric", argv[1]);
	sprintf(area_filename, "%s/area", argv[1]);
	sprintf(info_filename, "%s/info", argv[1]);

	FILE *f = fopen(isometric_filename, "r");
	if (f) { is_iso = true; fclose(f); }
	f = fopen(area_filename, "rb");
	if (!f) { return 0; }
	fclose(f);

	char tile_dir[1000];

	f = fopen(info_filename, "r");
	fgets(tile_dir, 1000, f);
	fclose(f);
	if (tile_dir[strlen(tile_dir)-1] == '\n')
		tile_dir[strlen(tile_dir)-1] = 0;
	
	for (int i = 0; i < 256; i++) {
		char filename[1000];
		sprintf(filename, "tiles/%s/tiles%d.png", tile_dir, i);
		ALLEGRO_BITMAP *b = al_load_bitmap(filename);
		if (!b) break;
		sheets.push_back(b);
	}

	for (unsigned int i = 0; i < sheets.size(); i++) {
		ALLEGRO_BITMAP *b = sheets[i];
		int w = al_get_bitmap_width(b) / 64;
		int h = al_get_bitmap_height(b) / 64;
		al_lock_bitmap(b, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_READONLY);
		std::vector<bool> v;
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				int ox = x*64;
				int oy = y*64;
				for (int py = 0; py < 64; py++) {
					for (int px = 0; px < 64; px++) {
						ALLEGRO_COLOR c = al_get_pixel(b, ox+px, oy+py);
						if (c.a < 1.0) {
							v.push_back(true);
							goto done;
						}
					}
				}
				v.push_back(false);
done:;
			}
		}
		al_unlock_bitmap(b);
		has_alpha.push_back(v);
	}

	f = fopen(area_filename, "rb");
	FILE *out = fopen("__tmp_area__", "wb");

	int w = readInt(f);
	int h = readInt(f);
	int num_layers = readInt(f);

	writeInt(out, w);
	writeInt(out, h);
	writeInt(out, num_layers);

	struct Tile {
		int num;
		int sheet;
		bool solid;
	};

	Tile level[num_layers][w][h];

	for (int l = 0; l < num_layers; l++) {
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				Tile t;
				t.num = readInt(f);
				t.sheet = fgetc(f);
				t.solid = fgetc(f);
				level[l][x][y] = t;
			}
		}
	}

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			for (int l = num_layers-1; l > 0; l--) {
				int num = level[l][x][y].num;
				int sheet = level[l][x][y].sheet;
				if (sheet < 0 || num < 0) continue;
				if (!has_alpha[sheet][num]) {
					for (int ll = l-1; l >= 0; l--) {
						level[ll][x][y].num = -1;
						level[ll][x][y].sheet = -1;
					}
					break;
				}
			}
		}
	}

	fclose(f);

	for (int l = 0; l < num_layers; l++) {
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				writeInt(out, level[l][x][y].num);
				fputc(level[l][x][y].sheet, out);
				fputc(level[l][x][y].solid, out);
			}
		}
	}

	fclose(out);

	return 0;
}
