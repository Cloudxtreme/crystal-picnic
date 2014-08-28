#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <cstdio>
#include <string>

bool save_cpi_f(ALLEGRO_FILE *f, ALLEGRO_BITMAP *b)
{
	ALLEGRO_LOCKED_REGION *lr = al_lock_bitmap(b, ALLEGRO_PIXEL_FORMAT_ABGR_8888_LE, ALLEGRO_LOCK_READONLY);
	if (!lr) return false;
	int w = al_get_bitmap_width(b);
	int h = al_get_bitmap_height(b);

	al_fwrite32le(f, w);
	al_fwrite32le(f, h);

	for (int y = 0; y < h; y++) {
		uint8_t *p = (uint8_t *)lr->data + y * lr->pitch;
		al_fwrite(f, p, w*4);
	}

	al_unlock_bitmap(b);

	return true;
}

bool save_cpi(const char *filename, ALLEGRO_BITMAP *b)
{
	ALLEGRO_FILE *f = al_fopen(filename, "wb");
	if (!f) return false;
	bool ret = save_cpi_f(f, b);
	al_fclose(f);
	return ret;
}

void convert(std::string path)
{
	ALLEGRO_FS_ENTRY *dir = al_create_fs_entry(path.c_str());
	if (!al_open_directory(dir)) return;
	ALLEGRO_FS_ENTRY *file;
	while ((file = al_read_directory(dir))) {
		const char *name = al_get_fs_entry_name(file);
		if (strstr(name, ".png")) {
			printf("%s\n", name);
			char new_name[1000];
			strcpy(new_name, name);
			strcpy(new_name+strlen(new_name)-3, "cpi");
			ALLEGRO_BITMAP *b = al_load_bitmap(name);
			al_save_bitmap(new_name, b);
			al_destroy_bitmap(b);
			al_remove_fs_entry(file);
		}
		else {
			convert(name);
		}
		al_destroy_fs_entry(file);
	}
	al_close_directory(dir);
}

int main(int argc, char **argv)
{
	al_init();
	al_init_image_addon();

	al_register_bitmap_saver(".cpi", save_cpi);
	al_register_bitmap_saver_f(".cpi", save_cpi_f);

	convert(".");

	return 0;
}
