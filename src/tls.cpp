#include <allegro5/allegro.h>
#include "tls.h"

#if defined ALLEGRO_IPHONE || defined ALLEGRO_MACOSX
#include <pthread.h>
#endif

namespace TLS
{
#if defined ALLEGRO_IPHONE || defined ALLEGRO_MACOSX

static pthread_key_t tls_key = 0;

static void pthreads_thread_destroy(void* ptr)
{
   delete (Thread_Local_State *)ptr;
}

void init(void)
{
   pthread_key_create(&tls_key, pthreads_thread_destroy);
}

//static Thread_Local_State _tls;

static Thread_Local_State *pthreads_thread_init(void)
{
   Thread_Local_State *ptr = new Thread_Local_State;
   pthread_setspecific(tls_key, ptr);
   return ptr;
}

Thread_Local_State *get_state(void)
{
   Thread_Local_State *ptr =
   	(Thread_Local_State *)pthread_getspecific(tls_key);
   if (ptr == NULL)
   {
      ptr = pthreads_thread_init();
      //initialize_tls_values(ptr);
   }
   return ptr;
}
#else
#ifdef _MSC_VER
static __declspec(thread) Thread_Local_State tls;
#else
static __thread Thread_Local_State tls;
#endif

void init(void)
{
}

Thread_Local_State *get_state(void)
{
	return &tls;
}
#endif

} // end namespace TLS
