#ifndef ANDROID_H
#define ANDROID_H

#ifdef __cplusplus
extern "C" {
#endif

void logString(const char *s);
const char * get_sdcarddir();

int isPurchased();
void queryPurchased();
void doIAP();
int checkPurchased();

#ifdef FIRETV
void grab_input();
void setMusic(int music);
#endif

#ifdef __cplusplus
}
#endif

#endif
