#include <allegro5/allegro.h>
#include <jni.h>
#include <bass.h>

void _al_event_source_lock(ALLEGRO_EVENT_SOURCE *);
void _al_event_source_emit_event(ALLEGRO_EVENT_SOURCE *, ALLEGRO_EVENT *);
void _al_event_source_unlock(ALLEGRO_EVENT_SOURCE *);

JNIEXPORT void JNICALL Java_com_nooskewl_crystalpicnic_CPActivity_pushButtonEvent
  (JNIEnv *env, jobject obj, jint button, jboolean down)
{
	if (!al_is_joystick_installed()) return;
	ALLEGRO_EVENT event;
	event.type = down ? ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN : ALLEGRO_EVENT_JOYSTICK_BUTTON_UP;
	event.joystick.button = button;
	ALLEGRO_EVENT_SOURCE *es = al_get_joystick_event_source();
	_al_event_source_lock(es);
	_al_event_source_emit_event(es, &event);
	_al_event_source_unlock(es);
}

JNIEXPORT void JNICALL Java_com_nooskewl_crystalpicnic_CPActivity_pushAxisEvent
  (JNIEnv *env, jobject obj, jint axis, jfloat value)
{
	if (!al_is_joystick_installed()) return;
	ALLEGRO_EVENT event;
	event.type = ALLEGRO_EVENT_JOYSTICK_AXIS;
	event.joystick.stick = 0;
	event.joystick.axis = axis;
	event.joystick.pos = value;
	ALLEGRO_EVENT_SOURCE *es = al_get_joystick_event_source();
	_al_event_source_lock(es);
	_al_event_source_emit_event(es, &event);
	_al_event_source_unlock(es);
}

JNIEXPORT void JNICALL Java_com_nooskewl_crystalpicnic_MyBroadcastReceiver_pauseSound
  (JNIEnv *env, jobject obj)
{
	BASS_Stop();
}

JNIEXPORT void JNICALL Java_com_nooskewl_crystalpicnic_MyBroadcastReceiver_resumeSound
  (JNIEnv *env, jobject obj, jint music)
{
	BASS_Start();
	BASS_ChannelPlay(music, FALSE);
}

