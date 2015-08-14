// bigger better badder packtiles

#define TILE_SIZE 16

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <cstdio>
#include <string>
#include <cmath>

#define SHEET_SIZE 1024

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

int main(int argc, char **argv)
{
	al_init();
	al_init_image_addon();
	al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_32_WITH_ALPHA);
	al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	
	const char *EXT = argv[1];

	for (int set = 2; set < argc; set++) {
		const char *set_name = argv[set];
		int num_sheets = 0;
		for (int i = 0; i < 256; i++) {
			char buf[200];
			sprintf(buf, "%s/tiles%d.png", set_name, i);
			if (!al_filename_exists(buf))
				break;
			num_sheets++;
		}
		if (num_sheets < 1) {
			break;
		}
		int sz = 512/TILE_SIZE;
		const int tiles_per_sheet = sz*sz;
		bool **used = new bool*[num_sheets];
		for (int j = 0; j < num_sheets; j++) {
			used[j] = new bool[tiles_per_sheet];
		}
		for (int j = 0; j < num_sheets; j++) {
			for (int i = 0; i < tiles_per_sheet; i++) {
				used[j][i] = true;
			}
		}
		if (false) {
			ALLEGRO_FS_ENTRY *parent = al_create_fs_entry("..");
			al_open_directory(parent);
			ALLEGRO_FS_ENTRY *file;
			while ((file = al_read_directory(parent)) != NULL) {
				const char *name = al_get_fs_entry_name(file);
				if (strstr(name, ".area")) {
					char buf[1000];
					sprintf(buf, "%s/info", name);
					FILE *f = fopen(buf, "r");
					fgets(buf, 1000, f);
					fclose(f);
					while (buf[strlen(buf)-1] == '\n' || buf[strlen(buf)-1] == '\r')
						buf[strlen(buf)-1] = 0;
					if (!strcmp(buf, set_name)) {
						char buf2[1000];
						sprintf(buf2, "%s/area", name);
						f = fopen(buf2, "rb");
						int w = read32bits(f);
						int h = read32bits(f);
						int layers = read32bits(f);
						for (int l = 0; l < layers; l++) {
							for (int y = 0; y < h; y++) {
								for (int x = 0; x < w; x++) {
									int n = read32bits(f);
									int sheet = fgetc(f);
									// ignore solid
									fgetc(f);
									if (n >= 0 && sheet >= 0) {
										used[sheet][n] = true;
									}
								}
							}
						}
						fclose(f);
					}
				}
			}
		}
		if (false) {
			ALLEGRO_FS_ENTRY *parent = al_create_fs_entry("..");
			al_open_directory(parent);
			ALLEGRO_FS_ENTRY *file;
			while ((file = al_read_directory(parent)) != NULL) {
				const char *name = al_get_fs_entry_name(file);
				if (strstr(name, ".area")) {
					char buf[1000];
					sprintf(buf, "%s/info", name);
					FILE *f = fopen(buf, "r");
					fgets(buf, 1000, f);
					fclose(f);
					while (buf[strlen(buf)-1] == '\n' || buf[strlen(buf)-1] == '\r')
						buf[strlen(buf)-1] = 0;
					if (!strcmp(buf, set_name)) {
						char buf2[1000];
						sprintf(buf2, "%s/area", name);
						f = fopen(buf2, "rb");
						sprintf(buf2, "%s/area.tmp", name);
						FILE *out = fopen(buf2, "wb");
						int w = read32bits(f);
						int h = read32bits(f);
						int layers = read32bits(f);
						write32bits(out, w);
						write32bits(out, h);
						write32bits(out, layers);
						for (int l = 0; l < layers; l++) {
							for (int y = 0; y < h; y++) {
								for (int x = 0; x < w; x++) {
									int n = read32bits(f);
									int sheet = fgetc(f);
									int solid = fgetc(f);
									if (n >= 0 && sheet >= 0) {
								//		used[sheet][n] = true;
										int count = 0;
										for (int j = 0; j <= sheet; j++) {
											int end;
											if (j == sheet)
												end = n;
											else
												end = tiles_per_sheet;
											for (int i = 0; i < end; i++) {
												if (used[j][i]) {
													count++;
												}
											}
										}
										sheet = count / tiles_per_sheet;
										n = count % tiles_per_sheet;
									}
									write32bits(out, n);
									fputc(sheet, out);
									fputc(solid, out);
								}
							}
						}
						fclose(f);
						fclose(out);
					}
				}
			}
		}

		int drawn_tiles = 0;
		int curr_sheet = 0;
		ALLEGRO_BITMAP *out_bmp = NULL;

		for (int i = 0; i < num_sheets; i++) {
			char buf[1000];
			sprintf(buf, "%s/tiles%d.png", set_name, i);
			ALLEGRO_FILE *file = al_fopen(buf, "rb");
			ALLEGRO_BITMAP *in_bmp = al_load_bitmap_f(file, EXT);
			al_fclose(file);
			for (int j = 0; j < tiles_per_sheet; j++) {
				if (!used[i][j])
					continue;
				int x = j % (512/TILE_SIZE);
				int y = j / (512/TILE_SIZE);
				if (drawn_tiles == 0) {
					out_bmp = al_create_bitmap(SHEET_SIZE, SHEET_SIZE);
					al_set_target_bitmap(out_bmp);
					al_clear_to_color(al_map_rgba(0, 0, 0, 0));
					al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);
				}
				int cx = drawn_tiles % sz;
				int cy = drawn_tiles / sz;
				int sx = x*TILE_SIZE;
				int sy = y*TILE_SIZE;
				int dx = cx*TILE_SIZE+(cx*2)+1;
				int dy = cy*TILE_SIZE+(cy*2)+1;
				al_draw_bitmap_region(
					in_bmp,
					sx, sy,
					TILE_SIZE, TILE_SIZE,
					dx, dy,
					0
				);
				const int sides[4][6] = {
					{ 0, 0, 1, TILE_SIZE, -1, 0 },
					{ 0, 0, TILE_SIZE, 1, 0, -1 },
					{ 0, TILE_SIZE-1, TILE_SIZE, 1, 0, TILE_SIZE },
					{ TILE_SIZE-1, 0, 1, TILE_SIZE, TILE_SIZE, 0 }
				};
				for (int o = 0; o < 4; o++) {
					al_draw_bitmap_region(
						in_bmp,
						sx+sides[o][0], sy+sides[o][1],
						sides[o][2], sides[o][3],
						dx+sides[o][4],
						dy+sides[o][5],
						0
					);
				}

				al_put_pixel(
					dx-1, dy-1,
					al_get_pixel(
						in_bmp,
						sx, sy
					)
				);
				al_put_pixel(
					dx+TILE_SIZE, dy-1,
					al_get_pixel(
						in_bmp,
						sx+TILE_SIZE-1, sy
					)
				);
				al_put_pixel(
					dx-1, dy+TILE_SIZE,
					al_get_pixel(
						in_bmp,
						sx, sy+TILE_SIZE-1
					)
				);
				al_put_pixel(
					dx+TILE_SIZE, dy+TILE_SIZE,
					al_get_pixel(
						in_bmp,
						sx+TILE_SIZE-1, sy+TILE_SIZE-1
					)
				);

				drawn_tiles++;
				if (drawn_tiles >= tiles_per_sheet) {
					char buf2[1000];
					sprintf(buf2, "%s/tiles%d_new.png", set_name, curr_sheet++);
					ALLEGRO_FILE *file = al_fopen(buf2, "wb");
					al_save_bitmap_f(file, EXT, out_bmp);
					al_fclose(file);
					drawn_tiles = 0;
					al_destroy_bitmap(out_bmp);
					out_bmp = NULL;
				}
			}
			al_destroy_bitmap(in_bmp);
		}

		if (out_bmp && drawn_tiles > 0) {
			char buf2[1000];
			sprintf(buf2, "%s/tiles%d_new.png", set_name, curr_sheet++);
			ALLEGRO_FILE *file = al_fopen(buf2, "wb");
			al_save_bitmap_f(file, EXT, out_bmp);
			al_fclose(file);
			al_destroy_bitmap(out_bmp);
			out_bmp = NULL;
		}

		for (int j = 0; j < num_sheets; j++) {
			delete[] used[j];
		}
		delete[] used;
	}

	return 0;
}

