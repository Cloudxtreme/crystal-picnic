package com.nooskewl.crystalpicnic;

import org.liballeg.android.AllegroActivity;
import android.net.Uri;
import android.content.Intent;
import android.text.ClipboardManager;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import java.io.File;
import android.view.KeyEvent;
import android.util.Log;
import android.app.ActivityManager;
import android.os.Bundle;
import tv.ouya.console.api.*;
import org.json.*;
import java.security.*;
import java.io.*;
import javax.crypto.*;
import javax.crypto.spec.*;
import android.util.*;
import java.util.*;
import java.security.spec.*;
import android.app.Activity;
import android.view.View;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.view.View.OnGenericMotionListener;
import android.view.MotionEvent;
import org.liballeg.android.KeyListener;
import android.view.InputDevice;
import android.content.IntentFilter;

public class CPActivity extends AllegroActivity implements OnGenericMotionListener {

	/* load libs */
	static {
		System.loadLibrary("allegro");
		System.loadLibrary("allegro_memfile");
		System.loadLibrary("allegro_primitives");
		System.loadLibrary("allegro_image");
		System.loadLibrary("allegro_font");
		System.loadLibrary("allegro_ttf");
		System.loadLibrary("allegro_color");
		System.loadLibrary("allegro_physfs");
		System.loadLibrary("bass");
		System.loadLibrary("bassmidi");
		System.loadLibrary("android_extras");
	}

	native void pushButtonEvent(int button, boolean down);
	native void pushAxisEvent(int axis, float value);

	public CPActivity()
	{
		super("libcrystalpicnic.so");
	}

	public void logString(String s)
	{
		Log.d("CrystalPicnic", s);
	}

	public String getSDCardPrivateDir()
	{
		File f = getExternalFilesDir(null);
		if (f != null) {
			return f.getAbsolutePath();
		}
		else {
			return getFilesDir().getAbsolutePath();
		}
	}

	public boolean wifiConnected()
	{
		ConnectivityManager connManager = (ConnectivityManager)getSystemService(CONNECTIVITY_SERVICE);
		NetworkInfo mWifi = connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);

		return mWifi.isConnected();
	}

	public void openURL(String url)
	{
		Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse("http://www.crystalpicnic.com"));
		startActivity(intent);
	}

	private boolean clip_thread_done = false;

	public void setClipData(String saveState)
	{
		final String ss = saveState;

		Runnable runnable = new Runnable() {
			public void run() {
				ClipboardManager m = 
					(ClipboardManager)getSystemService(Context.CLIPBOARD_SERVICE);

				m.setText(ss);

				clip_thread_done = true;
				}
			};
		runOnUiThread(runnable);

		while (!clip_thread_done)
			;
		clip_thread_done = false;
	}

	private String clipdata;

	public String getClipData()
	{
		Runnable runnable = new Runnable() {
			public void run() {
				ClipboardManager m = 
					(ClipboardManager)getSystemService(Context.CLIPBOARD_SERVICE);

				clipdata = m.getText().toString();

				clip_thread_done = true;
			}
		};
		runOnUiThread(runnable);

		while (!clip_thread_done)
			;
		clip_thread_done = false;

		return clipdata;
	}
	
	boolean gotGamepadConnected = false;
	boolean _gamepadConnected = false;

	public boolean gamepadConnected()
	{
		if (!gotGamepadConnected) {
			int[] ids = InputDevice.getDeviceIds();
			for (int i = 0; i < ids.length; i++) {
				InputDevice inp = InputDevice.getDevice(ids[i]);
				int bits = inp.getSources();
				if (((bits & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) || ((bits & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)) {
					_gamepadConnected = true;
				}
			}
			gotGamepadConnected = true;
		}
		return _gamepadConnected;
	}

	public void grabInput() {
		surface.setOnKeyListener(new KeyListener(this) {
			@Override
			public boolean onKey(View v, int keyCode, KeyEvent event) {
				int code = getCode(keyCode);
				if (code == -1) {
					return surface.key_listener.onKey(v, keyCode, event);
				}
				if (event.getAction() == KeyEvent.ACTION_DOWN) {
					if (event.getRepeatCount() == 0) {
						pushButtonEvent(code, true);
					}
				}
				else {
					pushButtonEvent(code, false);
				}
				return true;
			}
		});

		surface.setOnGenericMotionListener(this);
	}

	float axis_x = 0.0f;
	float axis_y = 0.0f;
	float axis_hat_x = 0.0f;
	float axis_hat_y = 0.0f;

	@Override
	public boolean onGenericMotion(View v, MotionEvent event) {
		int bits = event.getSource();
		if ((bits & InputDevice.SOURCE_GAMEPAD) != 0 || (bits & InputDevice.SOURCE_JOYSTICK) != 0) {
			float ax = event.getAxisValue(MotionEvent.AXIS_X, 0);
			float ay = event.getAxisValue(MotionEvent.AXIS_Y, 0);
			float ahx = event.getAxisValue(MotionEvent.AXIS_HAT_X, 0);
			float ahy = event.getAxisValue(MotionEvent.AXIS_HAT_Y, 0);
			if (ax != axis_x || ay != axis_y) {
				pushAxisEvent(0, ax);
				pushAxisEvent(1, ay);
				axis_x = ax;
				axis_y = ay;
			}
			else if (ahx != axis_hat_x || ahy != axis_hat_y) {
				pushAxisEvent(0, ahx);
				pushAxisEvent(1, ahy);
				axis_hat_x = ahx;
				axis_hat_y = ahy;
			}
			return true;
		}
		return false;
	}

	static final int joy_ability0 = 0;
	static final int joy_ability1 = 1;
	static final int joy_ability2 = 2;
	static final int joy_ability3 = 3;
	static final int joy_menu = 4;
	static final int joy_switch = 5;
	static final int joy_arrange_up = 6;

	static int getCode(int keyCode) {
		int code = -1;
		if (keyCode == KeyEvent.KEYCODE_BUTTON_Y) {
			code = joy_ability0;
		}
		else if (keyCode == KeyEvent.KEYCODE_BUTTON_X) {
			code = joy_ability1;
		}
		else if (keyCode == KeyEvent.KEYCODE_BUTTON_B) {
			code = joy_ability2;
		}
		else if (keyCode == KeyEvent.KEYCODE_BUTTON_A) {
			code = joy_ability3;
		}
		else if (keyCode == KeyEvent.KEYCODE_MENU) {
			code = joy_menu;
		}
		else if (keyCode == KeyEvent.KEYCODE_BUTTON_L1) {
			code = joy_arrange_up;
		}
		else if (keyCode == KeyEvent.KEYCODE_BUTTON_R1) {
			code = joy_switch;
		}
		return code;
	}
	
	MyBroadcastReceiver bcr;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
    	
		bcr = new MyBroadcastReceiver();
	}
	
	public void onResume() {
		super.onResume();

		registerReceiver(bcr, new IntentFilter("android.intent.action.DREAMING_STARTED"));
		registerReceiver(bcr, new IntentFilter("android.intent.action.DREAMING_STOPPED"));
	}

	public void onPause() {
		super.onPause();

		unregisterReceiver(bcr);
	}
}

