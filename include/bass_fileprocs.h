#ifndef BASS_FILEPROCS_H
#define BASS_FILEPROCS_H

#include <bass.h>

namespace BASS_FileProcs
{

void CALLBACK close(void *user);
QWORD CALLBACK len(void *user);
DWORD CALLBACK read(void *buf, DWORD length, void *user);
BOOL CALLBACK seek(QWORD offset, void *user);

} // end namespace

#endif
