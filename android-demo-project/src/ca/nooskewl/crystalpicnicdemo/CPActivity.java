package ca.nooskewl.crystalpicnicdemo;

import org.liballeg.android.AllegroActivity;
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

public class CPActivity extends AllegroActivity {

	/* load libs */
	static {
		System.loadLibrary("allegro_monolith");
		System.loadLibrary("bass");
		System.loadLibrary("bassmidi");
		System.loadLibrary("crystalpicnic");
	}

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
	
	public boolean gamepadAlwaysConnected()
	{
		return false;
	}
}

