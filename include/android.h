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
void grab_input();
bool gamepadConnected();

#ifdef __cplusplus
}
#endif

#endif
