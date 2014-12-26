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
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.content.IntentFilter;

import com.amazon.inapp.purchasing.GetUserIdResponse;
import com.amazon.inapp.purchasing.GetUserIdResponse.GetUserIdRequestStatus;
import com.amazon.inapp.purchasing.Item;
import com.amazon.inapp.purchasing.ItemDataResponse.ItemDataRequestStatus;
import com.amazon.inapp.purchasing.PurchaseResponse.PurchaseRequestStatus;
import com.amazon.inapp.purchasing.PurchaseUpdatesResponse.PurchaseUpdatesRequestStatus;
import com.amazon.inapp.purchasing.PurchasingManager;
import com.nooskewl.crystalpicnic.AppPurchasingObserver.PurchaseData;
import com.nooskewl.crystalpicnic.AppPurchasingObserver.PurchaseDataStorage;

public class CPActivity extends AllegroActivity {

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

	public CPActivity() {
		super("libcrystalpicnic.so");
	}

	public void logString(String s) {
		Log.d("CrystalPicnic", s);
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
	
	public boolean gamepadAlwaysConnected()
	{
		if ((android.os.Build.MODEL.substring(0, 4).equals("AFTB") || android.os.Build.MODEL.substring(0, 4).equals("AFTM")) && android.os.Build.MANUFACTURER.equals("Amazon")) {
			return true;
		}
		return false;
	}
}

