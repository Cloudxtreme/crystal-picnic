#include "cpa.h"
#include <allegro5/allegro_memfile.h>
#ifdef ALLEGRO_ANDROID
#include <allegro5/allegro_android.h>
#include <allegro5/allegro_physfs.h>
#include <physfs.h>
#else
#include <zlib.h>
#endif
#include <cstdio>

ALLEGRO_FILE *CPA::load(std::string filename)
{
	if (!exists(filename)) {
		return NULL;
	}
	return al_open_memfile(bytes+info[filename].first, info[filename].second, "rb");
}

bool CPA::exists(std::string filename)
{
	return info.find(filename) != info.end();
}

CPA::CPA(std::string archive_name)
{
#if defined ALLEGRO_ANDROID
	ALLEGRO_PATH *apkname = al_get_standard_path(ALLEGRO_EXENAME_PATH);
	PHYSFS_init(al_path_cstr(apkname, '/'));
	PHYSFS_addToSearchPath(al_path_cstr(apkname, '/'), 1);
	al_destroy_path(apkname);
	al_set_physfs_file_interface();
	ALLEGRO_FILE *f = al_fopen(archive_name.c_str(), "rb");
	al_set_standard_file_interface();
	al_set_standard_fs_interface();
	int sz = al_fsize(f);
	bytes = new uint8_t[sz];
	int count = 0;
	while (count < sz) {
		count += al_fread(f, bytes+count, sz-count);
	}
	al_fclose(f);
#elif defined ALLEGRO_IPHONE
	ALLEGRO_FILE *f = al_fopen(archive_name.c_str(), "rb");
	int sz = al_fsize(f);
	bytes = new uint8_t[sz];
	int count = 0;
	while (count < sz) {
		count += al_fread(f, bytes+count, sz-count);
	}
	al_fclose(f);
#else
#ifdef __linux__
	ALLEGRO_PATH *path = al_get_standard_path(ALLEGRO_EXENAME_PATH);
	al_set_path_filename(path, archive_name.c_str());
	ALLEGRO_FILE *file = al_fopen(al_path_cstr(path, '/'), "rb");
#else
	ALLEGRO_FILE *file = al_fopen(archive_name.c_str(), "rb");
#endif
	al_fseek(file, -4, ALLEGRO_SEEK_END);
	int sz = al_fread32le(file);
	al_fclose(file);

#ifdef __linux__
	gzFile f = gzopen(al_path_cstr(path, '/'), "rb");
	al_destroy_path(path);
#else
	gzFile f = gzopen(archive_name.c_str(), "rb");
#endif
	bytes = new uint8_t[sz];
	gzread(f, bytes, sz);
	gzclose(f);
	int count;
#endif

	int header_sz = (strchr((const char *)bytes, '\n') - (char *)bytes) + 1;
	int data_sz = atoi((const char *)bytes);

	uint8_t *p = bytes + header_sz + data_sz;
	count = header_sz;

	char str[1000];

	while (p < bytes+sz) {
		const char *end = strchr((const char *)p, '\n');
		int len = end-(char *)p;
		memcpy(str, p, len);
		str[len] = 0;
		char size[1000];
		char name[1000];
		sscanf(str, "%s %s", size, name);
		std::pair<int, int> pair(count, atoi(size));
		info[name] = pair;
		count += atoi(size);
		p += len + 1;
	}
}

CPA::~CPA()
{
	delete[] bytes;
}

