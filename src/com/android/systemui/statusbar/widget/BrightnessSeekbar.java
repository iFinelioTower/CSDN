/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.systemui.statusbar.widget;

import android.content.Context;
import android.view.View;
import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.IPowerManager;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserHandle;
import android.provider.Settings;
import android.widget.ImageView;

import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;

import android.util.Log;

public class BrightnessSeekbar implements SeekBar.OnSeekBarChangeListener {
	private static final String TAG = "BrightnessSeekbar";
	
	private final int mMinimumBacklight;
    private final int mMaximumBacklight;

    private final Context mContext;
	private final boolean mAutomaticAvailable;
    private final IPowerManager mPower;
    private final Handler mHandler;
	
	private final SeekBar mControl;
	private final BrightnessObserver mBrightnessObserver;

	private boolean mTracking;
	private boolean mAutomatic;
    private boolean mListening;
    private boolean mExternalChange;
	
	public BrightnessSeekbar(Context context, SeekBar seekbar) {
		mHandler = new Handler();
		mBrightnessObserver = new BrightnessObserver(mHandler);

		mControl = seekbar;
		mContext = context;
		
		PowerManager pm = (PowerManager)context.getSystemService(Context.POWER_SERVICE);
		mMinimumBacklight = pm.getMinimumScreenBrightnessSetting();
		mMaximumBacklight = pm.getMaximumScreenBrightnessSetting();

		
        mAutomaticAvailable = context.getResources().getBoolean(
                com.android.internal.R.bool.config_automatic_brightness_available);
		mPower = IPowerManager.Stub.asInterface(ServiceManager.getService("power"));
	}

	public void registerCallbacks() {
        if (mListening) {
            return;
        }

        mBrightnessObserver.startObserving();
        // Update the slider and mode before attaching the listener so we don't
        updateSlider();
		
		mControl.setOnSeekBarChangeListener(this);
        mListening = true;
    }

    /** Unregister all call backs, both to and from the controller */
    public void unregisterCallbacks() {
        if (!mListening) {
            return;
        }

        mBrightnessObserver.stopObserving();
		mControl.setOnSeekBarChangeListener(null);
        mListening = false;
    }

	private void setBrightness(int brightness) {
        try {
            mPower.setTemporaryScreenBrightnessSettingOverride(brightness);
        } catch (RemoteException ex) {
        }
    }
	
    private void onChanged(boolean tracking, int value) {
        Log.e(TAG, "onChanged mExternalChange: " + mExternalChange + ", tracking: " + tracking + ", value: " + value);
        if (mExternalChange) return;

        if (!mAutomatic) {
            final int val = value + mMinimumBacklight;
            
            setBrightness(val);
            if (!tracking) {
                AsyncTask.execute(new Runnable() {
                    public void run() {
                        Settings.System.putIntForUser(mContext.getContentResolver(),
                                Settings.System.SCREEN_BRIGHTNESS, val,
                                UserHandle.USER_CURRENT);
                    }
                });
            }
        }
    }
	
	/** Fetch the brightness mode from the system settings and update the icon */
    private void updateMode() {
        if (mAutomaticAvailable) {
            int automatic;
            automatic = Settings.System.getIntForUser(mContext.getContentResolver(),
                    Settings.System.SCREEN_BRIGHTNESS_MODE,
                    Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL,
                    UserHandle.USER_CURRENT);
            mAutomatic = automatic != Settings.System.SCREEN_BRIGHTNESS_MODE_MANUAL;
        }
    }
		
	@Override
	public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
	    Log.e(TAG, "onProgressChanged: " + progress);
		onChanged(mTracking, progress);
	}

	@Override
	public void onStartTrackingTouch(SeekBar seekBar) {
	    Log.e(TAG, "onStartTrackingTouch");
		mTracking = true;
		onChanged(mTracking, seekBar.getProgress());
	}

	@Override
	public void onStopTrackingTouch(SeekBar seekBar) {
	    Log.e(TAG, "onStopTrackingTouch");
		mTracking = false;
		onChanged(mTracking, seekBar.getProgress());
	}

	/** ContentObserver to watch brightness **/
	private class BrightnessObserver extends ContentObserver {

	    private final Uri BRIGHTNESS_MODE_URI =
	            Settings.System.getUriFor(Settings.System.SCREEN_BRIGHTNESS_MODE);
	    private final Uri BRIGHTNESS_URI =
	            Settings.System.getUriFor(Settings.System.SCREEN_BRIGHTNESS);
	    private final Uri BRIGHTNESS_ADJ_URI =
	            Settings.System.getUriFor(Settings.System.SCREEN_AUTO_BRIGHTNESS_ADJ);

	    public BrightnessObserver(Handler handler) {
	        super(handler);
	    }

	    @Override
	    public void onChange(boolean selfChange) {
	        onChange(selfChange, null);
	    }

	    @Override
	    public void onChange(boolean selfChange, Uri uri) {
	        Log.e(TAG, "onChange: " + selfChange);
			
	        if (selfChange) return;
	        try {
                mExternalChange = true;
                if (BRIGHTNESS_MODE_URI.equals(uri)) {
                    updateMode();
                    updateSlider();
                } else if (BRIGHTNESS_URI.equals(uri) && !mAutomatic) {
                    updateSlider();
                } else if (BRIGHTNESS_ADJ_URI.equals(uri) && mAutomatic) {
                    updateSlider();
                } else {
                    updateMode();
                    updateSlider();
                }
            } finally {
                mExternalChange = false;
            }
	    }

	    public void startObserving() {
	        final ContentResolver cr = mContext.getContentResolver();
	        cr.unregisterContentObserver(this);
	        cr.registerContentObserver(
	                BRIGHTNESS_MODE_URI,
	                false, this, UserHandle.USER_ALL);
	        cr.registerContentObserver(
	                BRIGHTNESS_URI,
	                false, this, UserHandle.USER_ALL);
	        cr.registerContentObserver(
	                BRIGHTNESS_ADJ_URI,
	                false, this, UserHandle.USER_ALL);
	    }

	    public void stopObserving() {
	        final ContentResolver cr = mContext.getContentResolver();
	        cr.unregisterContentObserver(this);
	    }

	}

	/** Fetch the brightness from the system settings and update the slider */
	private void updateSlider() {	
		if (!mAutomatic) {
			int value = Settings.System.getIntForUser(mContext.getContentResolver(),
					Settings.System.SCREEN_BRIGHTNESS, mMaximumBacklight,
					UserHandle.USER_CURRENT);
			
			mControl.setMax(mMaximumBacklight - mMinimumBacklight);
			mControl.setProgress(value - mMinimumBacklight);
        }
	}
}
