#include <allegro5/allegro.h>

// FIXME: new MinGW needs this
#ifdef ALLEGRO_WINDOWS
#include <windows.h>
#endif

#include "bass_fileprocs.h"

namespace BASS_FileProcs
{

void CALLBACK close(void *user)
{
	ALLEGRO_FILE *f = (ALLEGRO_FILE *)user;
	al_fclose(f);
}
QWORD CALLBACK len(void *user)
{
	ALLEGRO_FILE *f = (ALLEGRO_FILE *)user;
	QWORD sz = al_fsize(f);
	return sz;
}
DWORD CALLBACK read(void *buf, DWORD length, void *user)
{
	ALLEGRO_FILE *f = (ALLEGRO_FILE *)user;
	DWORD r = al_fread(f, buf, length);
	return r;
}
BOOL CALLBACK seek(QWORD offset, void *user)
{
	ALLEGRO_FILE *f = (ALLEGRO_FILE *)user;
	BOOL b = al_fseek(f, offset, ALLEGRO_SEEK_SET);
	return b;
}

} // end namespace
