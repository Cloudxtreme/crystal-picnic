#include <allegro5/allegro.h>

#include <jni.h>

#include "android.h"

extern "C" {
JNIEnv *_al_android_get_jnienv();
void __jni_checkException(JNIEnv *env, const char *file, const char *fname, int line);
jobject _al_android_activity_object();
}

#define _jni_checkException(env) __jni_checkException(env, __FILE__, __FUNCTION__, __LINE__)

#define _jni_call(env, rett, method, args...) ({ \
   rett ret = env->method(args); \
   _jni_checkException(env); \
   ret; \
})

#define _jni_callv(env, method, args...) ({ \
   env->method(args); \
   _jni_checkException(env); \
})

#define _jni_callVoidMethodV(env, obj, name, sig, args...) ({ \
   jclass class_id = _jni_call(env, jclass, GetObjectClass, obj); \
   \
   jmethodID method_id = _jni_call(env, jmethodID, GetMethodID, class_id, name, sig); \
   if(method_id == NULL) { \
   } else { \
      _jni_callv(env, CallVoidMethod, obj, method_id, ##args); \
   } \
   \
   _jni_callv(env, DeleteLocalRef, class_id); \
})

#define _jni_callBooleanMethodV(env, obj, name, sig, args...) ({ \
   jclass class_id = _jni_call(env, jclass, GetObjectClass, obj); \
   \
   jmethodID method_id = _jni_call(env, jmethodID, GetMethodID, class_id, name, sig); \
   \
   bool ret = false; \
   if(method_id == NULL) { \
   } \
   else { \
      ret = _jni_call(env, bool, CallBooleanMethod, obj, method_id, ##args); \
   } \
   \
   _jni_callv(env, DeleteLocalRef, class_id); \
   \
   ret; \
})

#define _jni_callIntMethodV(env, obj, name, sig, args...) ({ \
   jclass class_id = _jni_call(env, jclass, GetObjectClass, obj); \
   \
   jmethodID method_id = _jni_call(env, jmethodID, GetMethodID, class_id, name, sig); \
   \
   int ret = -1; \
   if(method_id == NULL) { \
   } \
   else { \
      ret = _jni_call(env, int, CallIntMethod, obj, method_id, ##args); \
   } \
   \
   _jni_callv(env, DeleteLocalRef, class_id); \
   \
   ret; \
})

#define _jni_callIntMethod(env, obj, name) _jni_callIntMethodV(env, obj, name, "()I");
#define _jni_callBooleanMethod(env, obj, name) _jni_callBooleanMethodV(env, obj, name, "()Z");

static jobject _jni_callObjectMethod(JNIEnv *env, jobject object, const char *name, const char *sig)
{
   jclass class_id = _jni_call(env, jclass, GetObjectClass, object);
   jmethodID method_id = _jni_call(env, jmethodID, GetMethodID, class_id, name, sig);
   jobject ret = _jni_call(env, jobject, CallObjectMethod, object, method_id);
   _jni_callv(env, DeleteLocalRef, class_id);

   return ret;
}

void logString(const char *s)
{
	JNIEnv *env = _al_android_get_jnienv();

	jstring S = env->NewStringUTF(s);

	_jni_callVoidMethodV(
		env,
		_al_android_activity_object(),
		"logString",
		"(Ljava/lang/String;)V",
		S
	);

	env->DeleteLocalRef(S);
}

int isPurchased()
{
#if defined OUYA && defined NO_DRM
	return true;
#else
	int ret = _jni_callIntMethod(
		_al_android_get_jnienv(),
		_al_android_activity_object(),
		"isPurchased"
	);

	return ret;
#endif
}

void queryPurchased()
{
	_jni_callVoidMethodV(
		_al_android_get_jnienv(),
		_al_android_activity_object(),
		"queryPurchased",
		"()V"
	);
}

void doIAP()
{
	_jni_callVoidMethodV(
		_al_android_get_jnienv(),
		_al_android_activity_object(),
		"doIAP",
		"()V"
	);
}

int checkPurchased()
{
	queryPurchased();

	int purchased = -1;

	do {
		purchased = isPurchased();
		if (purchased == -1) {
			al_rest(0.01);
		}
	} while (purchased == -1);

	return purchased;
}

const char * get_sdcarddir()
{
	static char buf[2000];

	jstring s =
		(jstring)_jni_callObjectMethod(
			_al_android_get_jnienv(),
			_al_android_activity_object(),
			"getSDCardPrivateDir",
			"()Ljava/lang/String;"
		);

	if (s == NULL)
		return "";

	const char *native = _al_android_get_jnienv()->GetStringUTFChars(s, 0);

	strcpy(buf, native);

	_al_android_get_jnienv()->ReleaseStringUTFChars(s, native);

	_al_android_get_jnienv()->DeleteLocalRef(s);

	return buf;
}

bool gamepadConnected()
{
	bool ret = _jni_callBooleanMethod(
		_al_android_get_jnienv(),
		_al_android_activity_object(),
		"gamepadAlwaysConnected"
	);

	if (ret) {
		return true;
	}

	if (!al_is_joystick_installed()) {
		return false;
	}

	int numjoy = al_get_num_joysticks();
	for (int i = 0; i < numjoy; i++) {
		ALLEGRO_JOYSTICK *joy = al_get_joystick(i);
		if (al_get_joystick_num_buttons(joy) != 0) {
			return true;
		}
	}

	return false;
}

