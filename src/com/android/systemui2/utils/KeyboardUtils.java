package com.android.systemui2.utils;

import android.util.Log;

import android.app.Instrumentation;

public class KeyboardUtils {
	private static final boolean DEBUG = true;
	private static final String TAG = "KeyboardUtils";
	
	private static KeyboardUtils _this = null;
	public static KeyboardUtils getInstance() {
		if(null == _this) {
			synchronized(KeyboardUtils.class) {
				try {
					if(null == _this) {
						_this = new KeyboardUtils();
					}	
				} catch (Exception ex) {
					Log.e(TAG, "KeyboardUtils getInstance error.", ex);
				}
			}
		}
		
		return _this;
	}
	
	public void sendKey(final int keyCode) {
		new Thread() {
			@Override
			public void run() {
				super.run();
				Instrumentation inst = new Instrumentation();  
				inst.sendKeyDownUpSync(keyCode); 
			}
		}.start();
	}
}
