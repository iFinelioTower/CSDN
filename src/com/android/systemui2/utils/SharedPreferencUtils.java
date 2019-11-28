package com.android.systemui2.utils;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

@SuppressLint("NewApi")
public class SharedPreferencUtils {
	private static Context mContext;

	public static SharedPreferences getSharePreferences(Context context) {
		mContext=context;
		return mContext.getSharedPreferences("isFirstBoot", 0);
	}

	public static SharedPreferences getClickTipTimes(Context context) {
		mContext=context;
		return mContext.getSharedPreferences("isCheckBox", 0);
	}
	

	public static long getLong(String key, long def) {
		SharedPreferences preferences = getSharePreferences(mContext);
		if (preferences != null)
			return preferences.getLong(key, def);
		else
			return 0L;
	}

	public static void setLong(String key, long value) {
		SharedPreferences preferences = getSharePreferences(mContext);
		if (preferences != null) {
			Editor editor = preferences.edit();
			editor.putLong(key, value);
			editor.commit();
		}
	}

	public static int getInt(String key, int def) {
		SharedPreferences preferences = getSharePreferences(mContext);
		if (preferences != null)
			return preferences.getInt(key, def);
		else
			return 0;
	}

	public static void setInt(String key, int value) {
		SharedPreferences preferences = getSharePreferences(mContext);
		if (preferences != null) {
			Editor editor = preferences.edit();
			editor.putInt(key, value);
			editor.commit();
		}
	}

	public static int getClickTipTimesInt(String key, int def) {
		SharedPreferences preferences = getClickTipTimes(mContext);
		if (preferences != null)
			return preferences.getInt(key, def);
		else
			return 0;
	}

	public static void setClickTipTimesInt(String key, int value) {
		SharedPreferences preferences = getClickTipTimes(mContext);
		if (preferences != null) {
			Editor editor = preferences.edit();
			editor.putInt(key, value);
			editor.commit();
		}
	}
}
