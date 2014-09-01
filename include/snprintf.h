#ifdef _MSC_VER

#define snprintf c99_snprintf

#ifdef __cplusplus
extern "C" {
#endif

int c99_vsnprintf(char* str, int size, const char* format, va_list ap);
int c99_snprintf(char* str, int size, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif // _MSC_VER

