#ifndef THREADS_H
#define THREADS_H

#ifdef ALLEGRO_IPHONE
#define THREAD_BEGIN NSAutoreleasePool *_p = [[NSAutoreleasePool alloc] init];
#define THREAD_END [_p release];
#else
#define THREAD_BEGIN
#define THREAD_END
#endif

#endif // THREADS_H
