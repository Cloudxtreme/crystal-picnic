package com.nooskewl.crystalpicnic;

import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;
import java.util.StringTokenizer;

import org.json.JSONException;
import org.json.JSONObject;

import android.app.Activity;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.AsyncTask;
import android.util.Log;

import com.amazon.inapp.purchasing.BasePurchasingObserver;
import com.amazon.inapp.purchasing.GetUserIdResponse;
import com.amazon.inapp.purchasing.GetUserIdResponse.GetUserIdRequestStatus;
import com.amazon.inapp.purchasing.ItemDataResponse;
import com.amazon.inapp.purchasing.ItemDataResponse.ItemDataRequestStatus;
import com.amazon.inapp.purchasing.Offset;
import com.amazon.inapp.purchasing.PurchaseResponse;
import com.amazon.inapp.purchasing.PurchaseResponse.PurchaseRequestStatus;
import com.amazon.inapp.purchasing.PurchaseUpdatesResponse;
import com.amazon.inapp.purchasing.PurchaseUpdatesResponse.PurchaseUpdatesRequestStatus;
import com.amazon.inapp.purchasing.PurchasingManager;
import com.amazon.inapp.purchasing.Receipt;

public class AppPurchasingObserver extends BasePurchasingObserver {

	private static final String TAG = "CrystalPicnic";

	private PurchaseDataStorage purchaseDataStorage;

	private CPActivity cp_activity;

	// Note: change below to a list if you want more than one listener
	public AppPurchasingObserver(Activity activity,
			PurchaseDataStorage purchaseDataStorage) {
		super(activity);
		this.cp_activity = (CPActivity)activity;
		this.purchaseDataStorage = purchaseDataStorage;
	}

	@Override
	public void onSdkAvailable(boolean isSandboxMode) {
		Log.i(TAG, "onSdkAvailable: isSandboxMode: " + isSandboxMode);
	}

	@Override
	public void onGetUserIdResponse(GetUserIdResponse response) {
		Log.i(TAG, "onGetUserIdResponse: requestId (" + response.getRequestId()
				+ ") userIdRequestStatus: " + response.getUserIdRequestStatus()
				+ ")");

		GetUserIdRequestStatus status = response.getUserIdRequestStatus();
		switch (status) {
		case SUCCESSFUL:
			String userId = response.getUserId();
			Log.i(TAG, "onGetUserIdResponse: save userId (" + userId
					+ ") as current user");
			boolean userChanged = saveCurrentUser(userId);

			Log.i(TAG,
					"onGetUserIdResponse: call onGetUserIdResponseSuccess for userId ("
							+ userId + ") userChanged (" + userChanged + ")");
			onGetUserIdResponseSuccessful(userId, userChanged);

			Offset offset = purchaseDataStorage.getPurchaseUpdatesOffset();

			Log.i(TAG,
					"onGetUserIdResponse: call initiatePurchaseUpdatesRequest from offset ("
							+ offset + ")");
			PurchasingManager.initiatePurchaseUpdatesRequest(offset);
			break;

		case FAILED:
			Log.i(TAG, "onGetUserIdResponse: FAILED");
			//listener.onGetUserIdResponseFailed(response.getRequestId());
			cp_activity.setPurchased(0);
			break;
		}
	}

	private boolean saveCurrentUser(String userId) {
		return purchaseDataStorage.saveCurrentUser(userId);
	}

	@Override
	public void onItemDataResponse(ItemDataResponse response) {
		final ItemDataRequestStatus status = response
				.getItemDataRequestStatus();
		Log.i(TAG, "onItemDataResponse: itemDataRequestStatus (" + status + ")");

		switch (status) {
		case SUCCESSFUL_WITH_UNAVAILABLE_SKUS:
			Set<String> unavailableSkus = response.getUnavailableSkus();
			Log.i(TAG, "onItemDataResponse: " + unavailableSkus.size()
					+ " unavailable skus");
			if (!unavailableSkus.isEmpty()) {
				Log.i(TAG,
						"onItemDataResponse: call onItemDataResponseUnavailableSkus");
				//listener.onItemDataResponseSuccessfulWithUnavailableSkus(unavailableSkus);
				cp_activity.setPurchased(0);
			}
		case SUCCESSFUL:
			Log.d(TAG,
					"onItemDataResponse: successful.  The item data map in this response includes the valid SKUs");
			//listener.onItemDataResponseSuccessful(response.getItemData());
			break;
		case FAILED:
			Log.d(TAG, "onItemDataResponse: failed, should retry request");
			//listener.onItemDataResponseFailed(response.getRequestId());
			break;
		}
	}

	@Override
	public void onPurchaseUpdatesResponse(PurchaseUpdatesResponse response) {
		final String userId = response.getUserId();
		final PurchaseUpdatesRequestStatus status = response
				.getPurchaseUpdatesRequestStatus();

		Log.i(TAG, "onPurchaseUpdatesResponse: userId (" + userId
				+ ") purchaseUpdatesRequestStatus (" + status + ")");
		if (!purchaseDataStorage.isSameAsCurrentUser(userId)) {
			// In most cases UserId in PurchaseUpdatesResponse should be the
			// same as UserId from GetUserIdResponse
			Log.i(TAG, "onPurchaseUpdatesResponse: userId (" + userId
					+ ") in response is NOT the same as current user!");
			cp_activity.setPurchased(0);
			return;
		}

		switch (status) {
		case SUCCESSFUL:
			// Update fulfillments for receipts
			// Handle receipts before revoked skus
			Set<Receipt> receipts = response.getReceipts();
			Set<String> revokedSkus = response.getRevokedSkus();
			Log.i(TAG, "onPurchaseUpdatesResponse: (" + receipts.size()
					+ ") receipts and (" + revokedSkus.size()
					+ ") revoked SKUs");
			if (!receipts.isEmpty() || !revokedSkus.isEmpty()) {
				PurchaseUpdatesData purchaseUpdatesResponseData = new PurchaseUpdatesData(
						response.getUserId(), receipts, revokedSkus);
				new PurchaseUpdatesAsyncTask()
						.execute(purchaseUpdatesResponseData);
			}
			else if (!response.isMore()) {
				cp_activity.setPurchased(0);
			}

			Offset offset = response.getOffset();
			// If more updates, send another request with current offset
			if (response.isMore()) {
				Log.i(TAG,
						"onPurchaseUpdatesResponse: more updates, call initiatePurchaseUpdatesRequest with offset: "
								+ offset);
				PurchasingManager.initiatePurchaseUpdatesRequest(offset);
			}
			purchaseDataStorage.savePurchaseUpdatesOffset(offset);
			break;
		case FAILED:
			Log.i(TAG, "onPurchaseUpdatesResponse: FAILED: response: "
					+ response);
			//listener.onPurchaseUpdatesResponseFailed(response.getRequestId());
			// May want to retry request
			cp_activity.setPurchased(0);
			break;
		}
	}

	@Override
	public void onPurchaseResponse(PurchaseResponse response) {
		String requestId = response.getRequestId();
		String userId = response.getUserId();
		PurchaseRequestStatus status = response.getPurchaseRequestStatus();
		Log.i(TAG, "onPurchaseResponse: requestId (" + requestId + ") userId ("
				+ userId + ") purchaseRequestStatus (" + status + ")");
		if (!purchaseDataStorage.isSameAsCurrentUser(userId)) {
			// In most cases UserId in PurchaseResponse should be the
			// same as UserId from GetUserIdResponse
			Log.i(TAG, "onPurchaseResponse: userId (" + userId
					+ ") in response is NOT the same as current user!");
			cp_activity.setPurchased(0);
			return;
		}

		PurchaseData purchaseDataForRequestId = null;
		String sku = null;
		
		switch (status) {
		case SUCCESSFUL:
			Receipt receipt = response.getReceipt();
			Log.i(TAG,
					"onPurchaseResponse: receipt itemType ("
							+ receipt.getItemType() + ") SKU ("
							+ receipt.getSku() + ") purchaseToken ("
							+ receipt.getPurchaseToken() + ")");

			Log.i(TAG,
					"onPurchaseResponse: call savePurchaseReceipt for requestId ("
							+ response.getRequestId() + ")");
			PurchaseData purchaseData = purchaseDataStorage
					.savePurchaseResponse(response);
			if (purchaseData == null) {
				Log.i(TAG,
						"onPurchaseResponse: could not save purchase receipt for requestId ("
								+ response.getRequestId()
								+ "), skipping fulfillment");
				break;
			}

			Log.i(TAG, "onPurchaseResponse: fulfill purchase with AsyncTask");
			new PurchaseResponseSuccessAsyncTask().execute(purchaseData);
			break;
		case ALREADY_ENTITLED:
			Log.i(TAG,
					"onPurchaseResponse: already entitled so remove purchase request from local storage");
			purchaseDataForRequestId = purchaseDataStorage.getPurchaseData(requestId);
			purchaseDataStorage.removePurchaseData(requestId);
		    if (purchaseDataForRequestId!=null) 
		        sku = purchaseDataForRequestId.getSKU();
			//listener.onPurchaseResponseAlreadyEntitled(userId, sku);
			cp_activity.setPurchased(1);
			break;
		case INVALID_SKU:
			Log.i(TAG,
					"onPurchaseResponse: invalid SKU! Should never get here, onItemDataResponse should have disabled buy button already.");
			// We should never get here because onItemDataResponse should have
			// taken care of invalid skus already and disabled the buy button
			purchaseDataForRequestId = purchaseDataStorage.getPurchaseData(requestId);
			purchaseDataStorage.removePurchaseData(requestId);
		    if (purchaseDataForRequestId!=null) 
		        sku = purchaseDataForRequestId.getSKU();
			//listener.onPurchaseResponseInvalidSKU(userId, sku);
			cp_activity.setPurchased(0);
			break;
		case FAILED:
			Log.i(TAG,
					"onPurchaseResponse: failed so remove purchase request from local storage");
			purchaseDataForRequestId = purchaseDataStorage.getPurchaseData(requestId);
			purchaseDataStorage.removePurchaseData(requestId);
		    if (purchaseDataForRequestId!=null) 
		        sku = purchaseDataForRequestId.getSKU();
			//listener.onPurchaseResponseFailed(requestId, sku);
			// May want to retry request
			cp_activity.setPurchased(0);
			break;
		}

	}

	private class PurchaseResponseSuccessAsyncTask extends
			AsyncTask<PurchaseData, Void, Boolean> {
		@Override
		protected Boolean doInBackground(PurchaseData... args) {
			PurchaseData purchaseData = args[0];

			String requestId = purchaseData.getRequestId();

			String userId = purchaseData.getUserId();
			String sku = purchaseData.getSKU();
			String purchaseToken = purchaseData.getPurchaseToken();

			Log.i(TAG,
					"PurchaseResponseSuccessAsyncTask.doInBackground: call listener's onPurchaseResponseSucccess for sku ("
							+ sku + ")");
			//listener.onPurchaseResponseSuccess(userId, sku, purchaseToken);
			cp_activity.setPurchased(1);

			Log.d(TAG,
					"PurchaseResponseSuccessAsyncTask.doInBackground: fulfilled SKU ("
							+ sku + ") purchaseToken (" + purchaseToken + ")");
			purchaseDataStorage.setPurchaseTokenFulfilled(purchaseToken);

			purchaseDataStorage.setRequestStateFulfilled(requestId);
			Log.d(TAG,
					"PurchaseResponseSuccessAsyncTask.doInBackground: completed for requestId ("
							+ requestId + ")");
			return true;
		}
	}

	private class PurchaseUpdatesAsyncTask extends
			AsyncTask<PurchaseUpdatesData, Void, Boolean> {
		@Override
		protected Boolean doInBackground(PurchaseUpdatesData... args) {
			PurchaseUpdatesData purchaseUpdatesData = args[0];
			String userId = purchaseUpdatesData.getUserId();
			Set<Receipt> receipts = purchaseUpdatesData.getReceipts();
			
			Set<String> revokedSkus = purchaseUpdatesData.getRevokedSkus();

			if (revokedSkus.size() > 0) {
				cp_activity.setPurchased(0);
			}
			else if (receipts.size() > 0) {
				cp_activity.setPurchased(1);
			}

			for (Receipt receipt : receipts) {
				Log.i(TAG,
						"PurchaseUpdatesAsyncTask.doInBackground: receipt itemType ("
								+ receipt.getItemType() + ") SKU ("
								+ receipt.getSku() + ") purchaseToken ("
								+ receipt.getPurchaseToken() + ")");
				String sku = receipt.getSku();

				Log.i(TAG,
						"PurchaseUpdatesAsyncTask.doInBackground: call onPurchaseUpdatesResponseSuccessSku for sku ("
								+ sku + ")");
				//listener.onPurchaseUpdatesResponseSuccess(userId, sku,
						//receipt.getPurchaseToken());

				Log.i(TAG,
						"PurchaseUpdatesAsyncTask.doInBackground: completed for receipt with purchaseToken ("
								+ receipt.getPurchaseToken() + ")");
			}

			for (String revokedSku : revokedSkus) {
				Log.i(TAG,
						"PurchaseUpdatesAsyncTask.doInBackground: call onPurchaseUpdatesResponseSuccessRevokedSku for revoked sku ("
								+ revokedSku + ")");
				//listener.onPurchaseUpdatesResponseSuccessRevokedSku(userId,
						//revokedSku);

				purchaseDataStorage.skuFulfilledCountDown(revokedSku);

			}
			return true;
		}
	}

	protected static class PurchaseDataStorage {

		private Activity activity;

		private String currentUser;

		private SharedPreferences savedUserRequestsAndPurchaseReceipts;

		public PurchaseDataStorage(Activity activity) {
			this.activity = activity;
		}

		public boolean saveCurrentUser(String userId) {
			boolean userChanged = ((this.currentUser == null) ? true
					: (!this.currentUser.equals(userId)));

			this.currentUser = userId;
			Log.d(TAG, "saveCurrentUser: " + userId);

			resetSavedUserRequestsAndPurchaseReceipts();
			return userChanged;
		}

		public boolean isSameAsCurrentUser(String userId) {
			return this.currentUser.equals(userId);
		}

		private void addRequestId(String requestId) {
			Set<String> requestIds = getStringSet("REQUEST_IDS");
			requestIds.add(requestId);
			putStringSet("REQUEST_IDS", requestIds);
		}

		private void removeRequestId(String requestId) {
			Set<String> requestIds = getStringSet("REQUEST_IDS");
			requestIds.remove(requestId);
			putStringSet("REQUEST_IDS", requestIds);
		}

		public Set<String> getAllRequestIds() {
			return getStringSet("REQUEST_IDS");
		}

		public PurchaseData savePurchaseResponse(PurchaseResponse response) {
			String requestId = response.getRequestId();
			String userId = response.getUserId();
			Receipt receipt = response.getReceipt();

			// RequestId should match a previously sent requestId
			if (!doesRequestIdMatchSentRequestId(requestId)) {
				Log.i(TAG, "savePurchaseReceipt: requestId (" + requestId
						+ ") does NOT match any requestId sent before!");
				return null;
			}

			String purchaseToken = receipt.getPurchaseToken();
			String sku = receipt.getSku();

			PurchaseData purchaseData = getPurchaseData(requestId);
			purchaseData.setUserId(userId);
			purchaseData.setRequestState(RequestState.RECEIVED);
			purchaseData.setPurchaseToken(purchaseToken);
			purchaseData.setSKU(sku);

			Log.d(TAG,
					"savePurchaseResponse: saving purchaseToken ("
							+ purchaseToken + ") sku (" + sku
							+ ") and request state as ("
							+ purchaseData.getRequestState() + ")");
			savePurchaseData(purchaseData);

			skuFulfilledCountUp(sku);

			return purchaseData;
		}

		private boolean doesRequestIdMatchSentRequestId(String requestId) {
			PurchaseData purchaseData = getPurchaseData(requestId);
			return purchaseData != null;
		}

		public void skuFulfilledCountUp(String sku) {
			SKUData skuData = getOrCreateSKUData(sku);
			skuData.fulfilledCountUp();
			Log.i(TAG,
					"skuFulfilledCountUp: fulfilledCountUp to ("
							+ skuData.getFulfilledCount() + ") for sku (" + sku
							+ "), save SKU data");
			saveSKUData(skuData);
		}

		protected void skuFulfilledCountDown(String revokedSku) {
			SKUData skuData = getSKUData(revokedSku);
			if (skuData == null)
				return;
			skuData.fulfilledCountDown();
			Log.i(TAG, "skuFulfilledCountDown: fulfilledCountDown to ("
					+ skuData.getFulfilledCount() + ") for revoked sku ("
					+ revokedSku + "), save SKU data");
			saveSKUData(skuData);
		}

		public boolean shouldFulfillSKU(String sku) {
			SKUData skuData = getSKUData(sku);
			if (skuData == null)
				return false;
			return skuData.getFulfilledCount() > 0;
		}

		public void setRequestStateFulfilled(String requestId) {
			PurchaseData purchaseData = getPurchaseData(requestId);
			purchaseData.setRequestState(RequestState.FULFILLED);
			savePurchaseData(purchaseData);
			Log.i(TAG,
					"setRequestStateFulfilled: requestId (" + requestId
							+ ") setting requestState to ("
							+ purchaseData.getRequestState() + ")");
		}
		
		public boolean isRequestStateSent(String requestId) {
			PurchaseData purchaseData = getPurchaseData(requestId);
			if (purchaseData==null)
				return false;
			return RequestState.SENT == purchaseData.getRequestState();
		}

		public void setPurchaseTokenFulfilled(String purchaseToken) {
			PurchaseData purchaseData = getPurchaseDataByPurchaseToken(purchaseToken);
			purchaseData.setPurchaseTokenFulfilled();
			Log.i(TAG, "setPurchaseTokenFulfilled: set purchaseToken ("
					+ purchaseToken + ") as fulfilled");
			savePurchaseData(purchaseData);
		}

		public boolean isPurchaseTokenFulfilled(String purchaseToken) {
			PurchaseData purchaseData = getPurchaseDataByPurchaseToken(purchaseToken);
			if (purchaseData == null)
				return false;
			return purchaseData.isPurchaseTokenFulfilled();
		}

		public PurchaseData newPurchaseData(String requestId) {
			addRequestId(requestId);

			PurchaseData purchaseData = new PurchaseData(requestId);
			purchaseData.setRequestState(RequestState.SENT);

			savePurchaseData(purchaseData);

			Log.d(TAG, "newPurchaseData: adding requestId (" + requestId
					+ ") to saved list and setting request state to ("
					+ purchaseData.getRequestState() + ")");
			return purchaseData;
		}

		public void savePurchaseData(PurchaseData purchaseData) {
			String json = PurchaseDataJSON.toJSON(purchaseData);

			Log.d(TAG, "savePurchaseData: saving for requestId ("
					+ purchaseData.getRequestId() + ") json: " + json);
			String requestId = purchaseData.getRequestId();
			putString(requestId, json);

			String purchaseToken = purchaseData.getPurchaseToken();
			if (purchaseToken != null) {
				Log.d(TAG, "savePurchaseData: saving for purchaseToken ("
						+ purchaseToken + ") json: " + json);
				putString(purchaseToken, json);
			}
		}

		public PurchaseData getPurchaseData(String requestId) {
			String json = getString(requestId);
			if (json == null)
				return null;
			return PurchaseDataJSON.fromJSON(json);
		}

		public PurchaseData getPurchaseDataByPurchaseToken(String purchaseToken) {
			String json = getString(purchaseToken);
			if (json == null)
				return null;
			return PurchaseDataJSON.fromJSON(json);
		}

		public SKUData newSKUData(String sku) {
			Log.d(TAG, "newSKUData: creating new SKUData for sku (" + sku + ")");
			return new SKUData(sku);
		}

		public void saveSKUData(SKUData skuData) {
			String json = SKUDataJSON.toJSON(skuData);
			Log.d(TAG, "saveSKUData: saving for sku (" + skuData.getSKU()
					+ ") json: " + json);
			putString(skuData.getSKU(), json);
		}

		public SKUData getSKUData(String sku) {
			String json = getString(sku);
			if (json == null)
				return null;
			return SKUDataJSON.fromJSON(json);
		}

		public SKUData getOrCreateSKUData(String sku) {
			SKUData skuData = getSKUData(sku);
			if (skuData == null) {
				skuData = newSKUData(sku);
			}
			return skuData;
		}

		public void savePurchaseUpdatesOffset(Offset offset) {
			putString("PURCHASE_UPDATES_OFFSET", offset.toString());
		}

		public Offset getPurchaseUpdatesOffset() {
			String offsetString = getString("PURCHASE_UPDATES_OFFSET");
			if (offsetString == null) {
				Log.i(TAG,
						"getPurchaseUpdatesOffset: no previous offset saved, use Offset.BEGINNING");
				return Offset.BEGINNING;
			}
			return Offset.fromString(offsetString);
		}

		public void removePurchaseData(String requestId) {
			remove(requestId);
			removeRequestId(requestId);
		}

		private void resetSavedUserRequestsAndPurchaseReceipts() {
			this.savedUserRequestsAndPurchaseReceipts = null;
		}

		private SharedPreferences getSavedUserRequestsAndPurchaseReceipts() {
			if (savedUserRequestsAndPurchaseReceipts != null)
				return savedUserRequestsAndPurchaseReceipts;
			savedUserRequestsAndPurchaseReceipts = activity
					.getSharedPreferences(currentUser, Activity.MODE_PRIVATE);
			return savedUserRequestsAndPurchaseReceipts;
		}

		protected Set<String> getStringSet(String key) {
			Set<String> emptySet = new HashSet<String>();
			return getStringSet(key, emptySet);
		}

		protected Set<String> getStringSet(String key, Set<String> defValues) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			// If you're only targeting devices with Android API Level 11 or above
			// you can just use the getStringSet method
			String pipeDelimitedValues = getString(key);
			return convertPipeDelimitedToList(pipeDelimitedValues);
		}
		
		private Set<String> convertPipeDelimitedToList(String pipeDelimitedValues) {
			Set<String> result = new HashSet<String>();
			if (pipeDelimitedValues==null || "".equals(pipeDelimitedValues))
				return result;
			
			StringTokenizer stk = new StringTokenizer(pipeDelimitedValues, "|");
			while (stk.hasMoreTokens()) {
				String token = stk.nextToken();
				result.add(token);
			}
			return result;
		}

		protected void putStringSet(String key, Set<String> valuesSet) {
			Editor editor = savedUserRequestsAndPurchaseReceipts.edit();
			// If you're only targeting devices with Android API Level 11 or above
			// you can just use the putStringSet method
			String pipeDelimitedValues = convertListToPipeDelimited(valuesSet);
			editor.putString(key, pipeDelimitedValues);
			editor.apply();
		}
		
		private String convertListToPipeDelimited(Set<String> values) {
			if (values==null || values.isEmpty())
				return "";
			StringBuilder result = new StringBuilder();
			for (Iterator<String> iter = values.iterator(); iter.hasNext();) {
				result.append(iter.next());
				if (iter.hasNext()) {
					result.append("|");
				}
			}
			return result.toString();
		}

		protected void putString(String key, String value) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			Editor editor = savedUserRequestsAndPurchaseReceipts.edit();
			editor.putString(key, value);
			editor.apply();
		}

		protected int getInt(String key, int defValue) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			return savedUserRequestsAndPurchaseReceipts.getInt(key, defValue);
		}

		protected void putInt(String key, int value) {
			Editor editor = savedUserRequestsAndPurchaseReceipts.edit();
			editor.putInt(key, value);
			editor.apply();
		}

		protected String getString(String key) {
			return getString(key, null);
		}

		protected String getString(String key, String defValue) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			return savedUserRequestsAndPurchaseReceipts
					.getString(key, defValue);
		}

		protected boolean getBoolean(String key) {
			return getBoolean(key, false);
		}

		protected boolean getBoolean(String key, boolean defValue) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			return savedUserRequestsAndPurchaseReceipts.getBoolean(key,
					defValue);
		}

		protected void putBoolean(String key, boolean value) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			Editor editor = savedUserRequestsAndPurchaseReceipts.edit();
			editor.putBoolean(key, value);
			editor.apply();
		}

		protected void remove(String key) {
			savedUserRequestsAndPurchaseReceipts = getSavedUserRequestsAndPurchaseReceipts();
			Editor editor = savedUserRequestsAndPurchaseReceipts.edit();
			editor.remove(key);
			editor.apply();
		}

	}

	protected static class PurchaseData {

		private String requestId;
		private String userId;
		private RequestState requestState;
		private String purchaseToken;
		private String sku;
		private boolean purchaseTokenFulfilled;

		public PurchaseData(String requestId) {
			this.requestId = requestId;
		}

		public void setUserId(String userId) {
			this.userId = userId;
		}

		public void setRequestState(RequestState requestState) {
			this.requestState = requestState;
		}

		public String getRequestId() {
			return requestId;
		}

		public String getUserId() {
			return userId;
		}

		public RequestState getRequestState() {
			return requestState;
		}

		public int getRequestStateAsInt() {
			return requestState.getState();
		}

		public void setPurchaseToken(String purchaseToken) {
			this.purchaseToken = purchaseToken;
		}

		public void setSKU(String sku) {
			this.sku = sku;
		}

		public String getPurchaseToken() {
			return purchaseToken;
		}

		public String getSKU() {
			return sku;
		}

		public void setPurchaseTokenFulfilled() {
			this.purchaseTokenFulfilled = true;
		}

		public boolean isPurchaseTokenFulfilled() {
			return purchaseTokenFulfilled;
		}

		@Override
		public String toString() {
			return "PurchaseData [requestId=" + requestId + ", userId="
					+ userId + ", requestState=" + requestState
					+ ", purchaseToken=" + purchaseToken + ", sku=" + sku
					+ ", purchaseTokenFulfilled=" + purchaseTokenFulfilled
					+ "]";
		}

	}

	protected static class PurchaseDataJSON {

		private static final String REQUEST_ID = "requestId";
		private static final String REQUEST_STATE = "requestState";
		private static final String PURCHASE_TOKEN = "purchaseToken";
		private static final String SKU = "sku";
		private static final String PURCHASE_TOKEN_FULFILLED = "purchaseTokenFulfilled";

		public static String toJSON(PurchaseData data) {
			JSONObject obj = new JSONObject();
			try {
				obj.put(REQUEST_ID, data.getRequestId());
				obj.put(REQUEST_STATE, data.getRequestStateAsInt());
				if (data.getPurchaseToken() != null)
					obj.put(PURCHASE_TOKEN, data.getPurchaseToken());
				if (data.getSKU() != null)
					obj.put(SKU, data.getSKU());
				if (data.isPurchaseTokenFulfilled())
					obj.put(PURCHASE_TOKEN_FULFILLED, true);
			} catch (JSONException e) {
				Log.e(TAG, "toJSON: ERROR serializing: " + data);
			}

			return obj.toString();
		}

		public static PurchaseData fromJSON(String json) {
			if (json == null)
				return null;
			JSONObject obj = null;
			try {
				obj = new JSONObject(json);
				String requestId = obj.getString(REQUEST_ID);
				int requestState = obj.getInt(REQUEST_STATE);
				String purchaseToken = obj.optString(PURCHASE_TOKEN);
				String sku = obj.optString(SKU);
				boolean purchaseTokenFulfilled = obj
						.optBoolean(PURCHASE_TOKEN_FULFILLED);

				PurchaseData result = new PurchaseData(requestId);
				result.setRequestState(RequestState.valueOf(requestState));
				result.setPurchaseToken(purchaseToken);
				result.setSKU(sku);
				if (purchaseTokenFulfilled) {
					result.setPurchaseTokenFulfilled();
				}
				return result;
			} catch (JSONException e) {
				e.printStackTrace();
			}
			return null;
		}

	}

	protected static class SKUData {
		private String sku;
		private int fulfilledCount;

		public SKUData(String sku) {
			this.sku = sku;
			this.fulfilledCount = 0;
		}

		public void fulfilledCountUp() {
			this.fulfilledCount++;
		}

		public void fulfilledCountDown() {
			this.fulfilledCount--;
		}

		public String getSKU() {
			return sku;
		}

		public int getFulfilledCount() {
			return fulfilledCount;
		}

		public void setFulfilledCount(int fulfilledCount) {
			this.fulfilledCount = fulfilledCount;
		}

		@Override
		public String toString() {
			return "SKUData [sku=" + sku + ", fulfilledCount=" + fulfilledCount
					+ "]";
		}

	}

	protected static class SKUDataJSON {

		private static final String SKU = "sku";
		private static final String FULFILLED_COUNT = "fulfilledCount";

		public static String toJSON(SKUData data) {
			if (data == null)
				return null;
			JSONObject obj = new JSONObject();
			try {
				obj.put(SKU, data.getSKU());
				obj.put(FULFILLED_COUNT, data.getFulfilledCount());
			} catch (JSONException e) {
				Log.e(TAG, "toJSON: ERROR serializing: " + data);
			}

			// Log.i(TAG, "toJSON: "+obj.toString());
			return obj.toString();
		}

		public static SKUData fromJSON(String json) {
			if (json == null)
				return null;
			JSONObject obj = null;
			try {
				obj = new JSONObject(json);
				String sku = obj.getString(SKU);
				int fulfilledCount = obj.getInt(FULFILLED_COUNT);
				SKUData result = new SKUData(sku);
				result.setFulfilledCount(fulfilledCount);
				// Log.i(TAG, "fromJSON: " + result);
				return result;
			} catch (JSONException e) {
				e.printStackTrace();
			}
			return null;
		}

	}

	protected static class PurchaseUpdatesData {
		private final String userId;
		private final Set<Receipt> receipts;
		private final Set<String> revokedSkus;

		public PurchaseUpdatesData(String userId, Set<Receipt> receipts,
				Set<String> revokedSkus) {
			this.userId = userId;
			this.receipts = receipts;
			this.revokedSkus = revokedSkus;
		}

		public Set<Receipt> getReceipts() {
			return receipts;
		}

		public Set<String> getRevokedSkus() {
			return revokedSkus;
		}

		public String getUserId() {
			return userId;
		}

		@Override
		public String toString() {
			return "PurchaseUpdatesData [userId=" + userId + ", receipts="
					+ receipts + ", revokedSkus=" + revokedSkus + "]";
		}

	}

	public static enum RequestState {

		NOT_AVAILABLE(0), //
		SENT(1), //
		RECEIVED(2), //
		FULFILLED(3);

		private int state;

		private RequestState(int state) {
			this.state = state;
		}

		public int getState() {
			return state;
		}

		public static RequestState valueOf(int state) {
			for (RequestState requestState : values()) {
				if (requestState.getState() == state) {
					return requestState;
				}
			}
			return null;
		}
	}

	public void onGetUserIdResponseSuccessful(String userId, boolean userChanged) {
		Log.i(TAG, "onGetUserIdResponseSuccessful: update display if userId ("
				+ userId + ") user changed from previous stored user ("
				+ userChanged + ")");

		if (!userChanged) {
			return;
		}
		
		// Reset to original setting where level2 is disabled
		//disableLevel2InView();

		Set<String> requestIds = purchaseDataStorage.getAllRequestIds();
		Log.i(TAG, "onGetUserIdResponseSuccessful: (" + requestIds.size()
				+ ") saved requestIds");
		for (String requestId : requestIds) {
			PurchaseData purchaseData = purchaseDataStorage
					.getPurchaseData(requestId);
			if (purchaseData == null) {
				Log.i(TAG,
						"onGetUserIdResponseSuccessful: could NOT find purchaseData for requestId ("
								+ requestId + "), skipping");
				continue;
			}
			if (purchaseDataStorage.isRequestStateSent(requestId)) {
				Log.i(TAG,
						"onGetUserIdResponseSuccessful: have not received purchase response for requestId still in SENT status: requestId ("
								+ requestId + "), skipping");
				continue;
			}

			Log.d(TAG, "onGetUserIdResponseSuccessful: requestId (" + requestId
					+ ") " + purchaseData);

			String purchaseToken = purchaseData.getPurchaseToken();
			String sku = purchaseData.getSKU();
			if (!purchaseData.isPurchaseTokenFulfilled()) {
				Log.i(TAG, "onGetUserIdResponseSuccess: requestId ("
						+ requestId + ") userId (" + userId + ") sku (" + sku
						+ ") purchaseToken (" + purchaseToken
						+ ") was NOT fulfilled, fulfilling purchase now");

				purchaseDataStorage.setPurchaseTokenFulfilled(purchaseToken);
				purchaseDataStorage.setRequestStateFulfilled(requestId);
				cp_activity.setPurchased(1);
			} else {
				boolean shouldFulfillSKU = purchaseDataStorage
						.shouldFulfillSKU(sku);
				if (shouldFulfillSKU) {
					Log.i(TAG, "onGetUserIdResponseSuccess: should fulfill sku ("
							+ sku + ") is true, so fulfilling purchasing now");
				}
				cp_activity.setPurchased(1);
			}
		}
	}
}
