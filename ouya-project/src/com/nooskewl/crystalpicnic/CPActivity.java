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
import android.content.IntentFilter;

public class CPActivity extends AllegroActivity {

	/* load libs */
	static {
		System.loadLibrary("allegro_monolith");
		System.loadLibrary("bass");
		System.loadLibrary("bassmidi");
		System.loadLibrary("crystalpicnic");
	}

	MyBroadcastReceiver bcr;

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

	static int purchased = -1;

	public void requestPurchase(final Product product) {
		Purchasable purchasable = new Purchasable(product.getIdentifier());

		OuyaResponseListener<PurchaseResult> purchaseListener =	new OuyaResponseListener<PurchaseResult>() {
			@Override
			public void onCancel() {
				purchased = 0;
			}
			@Override
			public void onSuccess(PurchaseResult result) {
				if (result.getProductIdentifier().equals("CRYSTAL_PICNIC")) {
					Log.d("CrystalPicnic", "Congrats on your purchase");
					writeReceipt();
					purchased = 1;
				}
				else {
					purchased = 0;
					Log.e("CrystalPicnic", "Your purchase failed.");
				}
			}
			@Override
			public void onFailure(int errorCode, String errorMessage, Bundle errorBundle) {
				purchased = 0;
				Log.d("CrystalPicnic", errorMessage);
			}
		};

		OuyaFacade.getInstance().requestPurchase(this, purchasable, purchaseListener);
	}

	static void writeReceipt()
	{
		OuyaFacade.getInstance().putGameData("CrystalPicnic", "PURCHASED");
	}

	public void doIAP()
	{
		purchased = -1;

		if (OuyaFacade.getInstance().isRunningOnOUYASupportedHardware()) {
			// This is the set of product IDs which our app knows about
			List<Purchasable> PRODUCT_ID_LIST =
				Arrays.asList(new Purchasable("CRYSTAL_PICNIC"));

			OuyaResponseListener<List<Product>> productListListener =
				new OuyaResponseListener<List<Product>>() {
					@Override
					public void onCancel() {
						purchased = 0;
					}
					@Override
					public void onSuccess(List<Product> products) {
						if (products.size() == 0) {
							purchased = 0;
						}
						else {
							for (Product p : products) {
								Log.d("CrystalPicnic", p.getName() + " costs " + p.getPriceInCents());
								try {
									if (p.getIdentifier().equals("CRYSTAL_PICNIC")) {
										requestPurchase(p);
									}
								} catch (Exception e) {
									purchased = 0;
									Log.e("CrystalPicnic", "requestPurcase failure", e);
								}
							}
						}
					}

					@Override
					public void onFailure(int errorCode, String errorMessage, Bundle errorBundle) {
						purchased = 0;
						Log.d("CrystalPicnic", errorMessage);
					}
			};

			OuyaFacade.getInstance().requestProductList(this, PRODUCT_ID_LIST, productListListener);
		}
		else {
			purchased = 0;
		}
	}

	public int isPurchased()
	{
		return purchased;
	}

	public void queryPurchased()
	{
		purchased = -1;

		// The receipt listener now receives a collection of tv.ouya.console.api.Receipt objects.
		OuyaResponseListener<Collection<Receipt>> receiptListListener =
			new OuyaResponseListener<Collection<Receipt>>() {
				@Override
				public void onSuccess(Collection<Receipt> receipts) {
					for (Receipt r : receipts) {
						Log.d("CrystalPicnic", r.getIdentifier() + " purchased for " + r.getFormattedPrice());
						if (r.getIdentifier().equals("CRYSTAL_PICNIC")) {
							purchased = 1;
						}
					}
					if (purchased == -1) {
						purchased = 0;
					}
				}

				@Override
				public void onFailure(int errorCode, String errorMessage, Bundle errorBundle) {
					Log.d("CrystalPicnic", errorMessage);
					purchased = 0;
				}

				@Override
				public void onCancel() {
					Log.d("CrystalPicnic", "Cancelled checking receipts");
					purchased = 0;
				}
			};

		try {
			if (OuyaFacade.getInstance().getGameData("CrystalPicnic").equals("PURCHASED")) {
				purchased = 1;
			}
			else {
				purchased = 0;
			}
		}
		catch (Exception e) {
			if (OuyaFacade.getInstance().isRunningOnOUYASupportedHardware()) {
				OuyaFacade.getInstance().requestReceipts(this, receiptListListener);
			}
			else {
				purchased = 0;
			}
		}
	}

	public static final String DEVELOPER_ID = "";

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		Bundle developerInfo = new Bundle();

		// Your developer id can be found in the Developer Portal
		developerInfo.putString(OuyaFacade.OUYA_DEVELOPER_ID, DEVELOPER_ID);

		// There are a variety of ways to store and access your application key.
		// Two of them are demoed in the samples 'game-sample' and 'iap-sample-app'
		developerInfo.putByteArray(OuyaFacade.OUYA_DEVELOPER_PUBLIC_KEY, loadApplicationKey());

		OuyaFacade.getInstance().init(this, developerInfo);
		super.onCreate(savedInstanceState);

		bcr = new MyBroadcastReceiver();
	}

	@Override
	public void onDestroy() {
		OuyaFacade.getInstance().shutdown();
		super.onDestroy();
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

	public boolean gamepadAlwaysConnected() {
		return true;
	}

	byte[] loadApplicationKey() {
		// Create a PublicKey object from the key data downloaded from the developer portal.
		try {
			// Read in the key.der file (downloaded from the developer portal)
			InputStream inputStream = getResources().openRawResource(R.raw.key);
			byte[] applicationKey = new byte[inputStream.available()];
			inputStream.read(applicationKey);
			inputStream.close();
			return applicationKey;
		} catch (Exception e) {
			Log.e("CrystalPicnic", "Unable to load application key", e);
		}

		return null;
	}

	@Override
	public void onActivityResult(final int requestCode, final int resultCode, final Intent data) {
		super.onActivityResult(requestCode, resultCode, data);
		if (null != OuyaFacade.getInstance())
		{
			if (OuyaFacade.getInstance().processActivityResult(requestCode, resultCode, data)) {
				// handled activity result
			} else {
				// unhandled activity result
			}
		} else {
		// OuyaFacade not initialized
		}
	}
}
