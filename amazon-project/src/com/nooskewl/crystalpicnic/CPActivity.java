package com.nooskewl.crystalpicnic;

import org.liballeg.android.AllegroActivity;
import java.util.Map.Entry;
import android.net.Uri;
import android.content.Intent;
import android.text.ClipboardManager;
import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import java.io.File;
import android.util.Log;
import android.app.ActivityManager;
import android.os.Bundle;
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
import android.view.View.OnGenericMotionListener;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import org.liballeg.android.KeyListener;
import android.content.IntentFilter;
import android.view.InputDevice;

import com.amazon.inapp.purchasing.GetUserIdResponse;
import com.amazon.inapp.purchasing.GetUserIdResponse.GetUserIdRequestStatus;
import com.amazon.inapp.purchasing.Item;
import com.amazon.inapp.purchasing.ItemDataResponse.ItemDataRequestStatus;
import com.amazon.inapp.purchasing.PurchaseResponse.PurchaseRequestStatus;
import com.amazon.inapp.purchasing.PurchaseUpdatesResponse.PurchaseUpdatesRequestStatus;
import com.amazon.inapp.purchasing.PurchasingManager;
import com.nooskewl.crystalpicnic.AppPurchasingObserver.PurchaseData;
import com.nooskewl.crystalpicnic.AppPurchasingObserver.PurchaseDataStorage;

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

	public static int music;

	public CPActivity() {
		super("libcrystalpicnic.so");
	}

	public void logString(String s) {
		Log.d("CrystalPicnic", s);
	}

	public void setMusic(int music)
	{
		CPActivity.music = music;
	}

	public String getSDCardPrivateDir() {
		File f = getExternalFilesDir(null);
		if (f != null) {
			return f.getAbsolutePath();
		}
		else {
			return getFilesDir().getAbsolutePath();
		}
	}

	public boolean wifiConnected() {
		ConnectivityManager connManager = (ConnectivityManager)getSystemService(CONNECTIVITY_SERVICE);
		NetworkInfo mWifi = connManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);

		return mWifi.isConnected();
	}

	public void openURL(String url) {
		Intent intent = new Intent(Intent.ACTION_VIEW, Uri.parse("http://www.crystalpicnic.com"));
		startActivity(intent);
	}

	private boolean clip_thread_done = false;

	public void setClipData(String saveState) {
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

	public String getClipData() {
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
			/*
			int[] ids = InputDevice.getDeviceIds();
			for (int i = 0; i < ids.length; i++) {
				InputDevice inp = InputDevice.getDevice(ids[i]);
				int bits = inp.getSources();
				if (((bits & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) || ((bits & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)) {
					_gamepadConnected = true;
				}
			}
			*/
			// This is the id of Fire TV
			if (android.os.Build.MODEL.substring(0, 4).equals("AFTB") && android.os.Build.MANUFACTURER.equals("Amazon")) {
				_gamepadConnected = true;
			}
			gotGamepadConnected = true;
		}
		return _gamepadConnected;
	}
	
	private PurchaseDataStorage purchaseDataStorage;

	MyBroadcastReceiver bcr;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
    	
		purchaseDataStorage = new PurchaseDataStorage(this);
		AppPurchasingObserver purchasingObserver = new AppPurchasingObserver(this, purchaseDataStorage);
		PurchasingManager.registerObserver(purchasingObserver);	

		bcr = new MyBroadcastReceiver();
	}

	@Override
	public void onDestroy() {
		super.onDestroy();
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

	public void onPause() {
		super.onPause();

		unregisterReceiver(bcr);
	}
	
	public void onResume() {
		super.onResume();

		Log.i("CrystalPicnic", "onResume: call initiateGetUserIdRequest");
		PurchasingManager.initiateGetUserIdRequest();

		Log.i("CrystalPicnic", "onResume: call initiateItemDataRequest for skus: " + MySKU.getAll());
		PurchasingManager.initiateItemDataRequest(MySKU.getAll());

		registerReceiver(bcr, new IntentFilter("android.intent.action.DREAMING_STARTED"));
		registerReceiver(bcr, new IntentFilter("android.intent.action.DREAMING_STOPPED"));
	}

	static int purchased = -1;

	public void doIAP() {
		if (purchased == 1) {
			return;
		}
		purchased = -1;
		String requestId = PurchasingManager
				.initiatePurchaseRequest(MySKU.UNLOCK.getSku());
		PurchaseData purchaseData = purchaseDataStorage
				.newPurchaseData(requestId);
		Log.i("CrystalPicnic", "onBuyAccessToLevel2Click: requestId (" + requestId
				+ ") requestState (" + purchaseData.getRequestState() + ")");
	}

	public int isPurchased() {
		return purchased;
	}

	public void queryPurchased() {
	}

	public void setPurchased(int purchased) {
		if (this.purchased != -1) {
			return;
		}
		Log.i("CrystalPicnic", "setPurchased(" + purchased + ")");
		this.purchased = purchased;
	}
}

