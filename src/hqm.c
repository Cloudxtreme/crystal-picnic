#define _WIN32_WINNT 0x0501
#define _WINSOCKAPI_
#include <allegro5/allegro.h>

#ifdef ALLEGRO_ANDROID
#include <allegro5/allegro_android.h>
#include "android.h"
#endif

#ifdef ALLEGRO_WINDOWS
#define mkdir(a, b) mkdir(a)
#endif

#ifdef ALLEGRO_ANDROID
#define PERMS 0775
#else
#define PERMS 0755
#endif

#define LIST_FILENAME "list.txt"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CURL_STATICLIB
#include <curl/curl.h>

#include "snprintf.h"
#include "hqm.h"

#if defined ALLEGRO_MACOSX || defined ALLEGRO_IPHONE || defined __linux__
#include <sys/stat.h>
#endif

#define NUM_FILES 4
#define EXPECTED_LIST_SIZE (NUM_FILES * 81)

static char DOWNLOAD_PATH[1000];

static volatile bool stop = false;
static bool is_downloading = false;

static int download_file_curl(const char *filename)
{
	char outfilename[1000];
	char url[1000];
	sprintf(outfilename, "%s/%s", DOWNLOAD_PATH, filename);
	sprintf(url, "ftp://nooskewl.com/CrystalPicnic/%s", filename);

	return getfile(url, outfilename);
}

static void download_list(char **filenames, int *lengths)
{
	int i = 0;

	while (filenames[i]) {
		int len = download_file_curl(filenames[i]);
		if (stop) {
			stop = false;
			return;
		}
		if (len != lengths[i])
			continue;
		i++;
	}
}

static bool download_all(void)
{
	char fn[1000];
	FILE *f;
	size_t read;
	char buf[100];
	int sz;
	char **filenames = NULL;
	int *lengths = NULL;
	int count = 0;
	char fn2[1000];
	ALLEGRO_FILE *f2;

	sprintf(fn, "%s/%s", DOWNLOAD_PATH, LIST_FILENAME);
	f = fopen(fn, "r");
	while ((read = fread(buf, 1, 80, f)) == 80) {
		fgetc(f); // skip, fseek malfunctioning on windows oO
		buf[80] = 0;
		if (sscanf(buf, "%s %d", fn, &sz) != 2) {
			fclose(f);
			return false;
		}
		sprintf(fn2, "%s/%s", DOWNLOAD_PATH, fn);
		f2 = al_fopen(fn2, "rb");
		if (f2) {
			int sz2 = (int)al_fsize(f2);
			al_fclose(f2);
			if (sz2 == sz) {
				continue;
			}
		}
		count++;
		if (filenames == NULL) {
			filenames = malloc(2 * sizeof(char *));
			lengths = malloc(2 * sizeof(int));
		}
		else {
			filenames = realloc(filenames, (count+1) * sizeof(char *));
			lengths = realloc(lengths, (count+1) * sizeof(int));
		}
		filenames[count-1] = strdup(fn);
		lengths[count-1] = sz;
	}
	if (filenames != NULL) {
		filenames[count] = NULL;
		lengths[count] = 0;

		download_list(filenames, lengths);

		free(filenames);
		free(lengths);
	}

	fclose(f);

	return true;
}

static void *hqm_go_thread_curl(void *arg)
{
	int len;

	(void)arg;

	is_downloading = true;

	mkdir(DOWNLOAD_PATH, PERMS);

	char list_fn[1000];
	snprintf(list_fn, 1000, "%s", LIST_FILENAME);

	len = download_file_curl(list_fn);
	if (len != EXPECTED_LIST_SIZE) {
		is_downloading = false;
		return NULL;
	}

	hqm_get_status(NULL);

	download_all();

	is_downloading = false;

	return NULL;
}

bool hqm_is_downloading(void)
{
	return is_downloading;
}

void hqm_go(void)
{
	al_run_detached_thread(hqm_go_thread_curl, NULL);
}

void hqm_stop(void)
{
	stop = true;
}

const char *hqm_status_string(int status)
{
	const char *strs[] = {
		"Complete",
		"Partial",
		"Not started"
	};

	if (status == 2 && hqm_is_downloading()) {
		return "..";
	}

	return strs[status];
}

int hqm_get_status(float *percent)
{
	char fn[1000];
	ALLEGRO_FILE *f;
	size_t read;
	char buf[100];
	int sz;
	int count = 0;

	if (percent)
		*percent = 0.0f;

	sprintf(fn, "%s/%s", DOWNLOAD_PATH, LIST_FILENAME);

	f = al_fopen(fn, "r");

	if (f == NULL || al_fsize(f) != EXPECTED_LIST_SIZE) {
		if (f) al_fclose(f);
		goto nuthin;
	}

	while ((read = al_fread(f, buf, 80)) == 80) {
		char fn2[1000];
		ALLEGRO_FILE *f2;

		al_fgetc(f);
		buf[80] = 0;
		if (sscanf(buf, "%s %d", fn, &sz) != 2) {
			if (count == 0) {
				al_fclose(f);
				goto nuthin;
			}
			al_fclose(f);
			goto partial;
		}
		sprintf(fn2, "%s/%s", DOWNLOAD_PATH, fn);
		f2 = al_fopen(fn2, "rb");
		if (f2) {
			int sz2 = (int)al_fsize(f2);
			al_fclose(f2);
			if (sz2 != sz) {
				if (count == 0) {
					al_fclose(f);
					goto nuthin;
				}
				goto partial;
			}
		}
		else {
			if (count == 0) {
				al_fclose(f);
				goto nuthin;
			}
			goto partial;
		}
		count++;
	}

	if (count == NUM_FILES) {
		al_fclose(f);
		if (percent)
			*percent = 1.0f;
		return HQM_STATUS_COMPLETE;
	}

partial:
	al_fclose(f);
	if (percent)
		*percent = (float)count / NUM_FILES;
	return HQM_STATUS_PARTIAL;

nuthin:
	return HQM_STATUS_NOTSTARTED;
}

void hqm_set_download_path(const char *path)
{
	strcpy(DOWNLOAD_PATH, path);
	while (DOWNLOAD_PATH[strlen(DOWNLOAD_PATH)-1] == '/' ||
			DOWNLOAD_PATH[strlen(DOWNLOAD_PATH)-1] == '\\') {
		DOWNLOAD_PATH[strlen(DOWNLOAD_PATH)-1] = 0;
	}
}

static void hqm_delete_real()
{
	ALLEGRO_FS_ENTRY *file;

	ALLEGRO_FS_ENTRY *dir = al_create_fs_entry(DOWNLOAD_PATH);
	if (!dir) {
		printf("al_create_fs_entry failed for hqm dir\n");
		return;
	}
	if (!al_open_directory(dir)) {
		printf("al_open_directory failed for hqm dir\n");
		return;
	}

	while ((file = al_read_directory(dir)) != NULL) {
		al_remove_fs_entry(file);
		al_destroy_fs_entry(file);
	}

	al_close_directory(dir);
	al_remove_fs_entry(dir);
	al_destroy_fs_entry(dir);
}

void hqm_delete()
{
	hqm_delete_real();
}

struct FtpFile {
  const char *filename;
  FILE *stream;
  long written;
};

static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
  struct FtpFile *out=(struct FtpFile *)stream;
  if(out && !out->stream) {
    /* open file for writing */
    out->stream=fopen(out->filename, "wb");
    if(!out->stream)
      return -1; /* failure, can't open file to write */
  }
  size_t written = fwrite(buffer, size, nmemb, out->stream);
  out->written += written;
  return written;
}

int getfile(const char *url, const char *outfilename)
{
  CURL *curl;
  CURLcode res;
  struct FtpFile ftpfile={
    outfilename,
    NULL,
    0
  };

  curl_global_init(CURL_GLOBAL_DEFAULT);

  curl = curl_easy_init();
  if(curl) {
    /*
     * You better replace the URL with one that works!
     */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    /* Define our callback to get called when there's data to be written */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);
    /* Set a pointer to our struct to pass to the callback */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile);

    /* Switch on full protocol/debug output */
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);

    /* always cleanup */
    curl_easy_cleanup(curl);

    if(CURLE_OK != res) {
      /* we failed */
      return -1;
    }
  }

  if(ftpfile.stream)
    fclose(ftpfile.stream); /* close the local file */

  curl_global_cleanup();

  return (int)ftpfile.written;
}

