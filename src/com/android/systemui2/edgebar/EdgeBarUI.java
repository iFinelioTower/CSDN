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
 
package com.android.systemui2.edgebar;

import android.util.Log;

import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;

import com.android.systemui2.R;
import com.android.systemui2.SystemUI;

import android.os.Bundle;
import android.os.IBinder;
import android.content.Context;
import android.os.ServiceManager;
import android.os.RemoteException;
import com.android.systemui2.interfaces.CommandQueue;

import android.view.IWindowManager;
import android.view.WindowManager;
import android.view.WindowManagerGlobal;
import com.android.internal.statusbar.ISystemUI2BarService;
import com.android.internal.statusbar.StatusBarIcon;
import com.android.internal.statusbar.StatusBarIconList;

import com.android.systemui2.edgebar.bezierslider.EdgeBarWindowView;

public class EdgeBarUI extends SystemUI implements
        CommandQueue.Callbacks {
    private static final String TAG = "EdgeBarUI";
	public static final boolean DEBUG = true;

	protected CommandQueue mCommandQueue = null;
	protected ISystemUI2BarService mBarService = null;
	
	private EdgeBarWindowView mEdgeBarLeftView = null;
	private EdgeBarWindowView mEdgeBarRightView = null;
	private EdgeBarWindowManager mEdgeBarWindowManager = null;

	// tracking calls to View.setSystemUiVisibility()
    int mSystemUiVisibility = View.SYSTEM_UI_FLAG_VISIBLE;
	// last value sent to window manager
    private int mLastDispatchedSystemUiVisibility = ~View.SYSTEM_UI_FLAG_VISIBLE;
	private WindowManager mWindowManager;
	private IWindowManager mWindowManagerService;
	
    @Override
    public void start() {
        Log.e(TAG, "on Start");
		
		mWindowManager = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
		mWindowManagerService = WindowManagerGlobal.getWindowManagerService();
		
		mEdgeBarLeftView = (EdgeBarWindowView) View.inflate(mContext, R.layout.edge_bar, null);
		mEdgeBarLeftView.init(mWindowManager, EdgeEvent.EDGE_LEFT, 13);
		
		mEdgeBarRightView = (EdgeBarWindowView) View.inflate(mContext, R.layout.edge_bar, null);
		mEdgeBarRightView.init(mWindowManager, EdgeEvent.EDGE_RIGHT, 13);
		
		//mEdgeBarWindowManager = new EdgeBarWindowManager(mContext, mWindowManager);
		//mEdgeBarWindowManager.add(mEdgeBarLeftView);
		
		// CommandQueue 模块由 mEdgeBarWindowManager 管理.
		mBarService = ISystemUI2BarService.Stub.asInterface(
                ServiceManager.getService(Context.SYSTEMUI2_BAR_SERVICE));
		
		mCommandQueue = new CommandQueue(this, null);
        try {
			if (mBarService != null) {
				mBarService.registerStatusBar(mCommandQueue);
			} 
        } catch (RemoteException ex) {
            // If the system process isn't there we're doomed anyway.
        }
    }
	
	public void addIcon(String slot, int index, int viewIndex, StatusBarIcon icon){}
	public void updateIcon(String slot, int index, int viewIndex,
	        StatusBarIcon old, StatusBarIcon icon){}
	public void removeIcon(String slot, int index, int viewIndex){}
	public void disable(int state1, int state2, boolean animate){}
	public void animateExpandNotificationsPanel(){}
	public void animateCollapsePanels(int flags){}
	public void animateExpandSettingsPanel(){}

	public void setSystemUiVisibility(int vis, int mask) {
	    final int oldVal = mSystemUiVisibility;
        final int newVal = (oldVal&~mask) | (vis&mask);
        final int diff = newVal ^ oldVal;
        if (DEBUG) Log.d(TAG, String.format(
                "setSystemUiVisibility vis=%s mask=%s oldVal=%s newVal=%s diff=%s",
                Integer.toHexString(vis), Integer.toHexString(mask),
                Integer.toHexString(oldVal), Integer.toHexString(newVal),
                Integer.toHexString(diff)));

		if (diff != 0) {
			mSystemUiVisibility = newVal;
			
		    //if () {
				mEdgeBarWindowManager.setVisibility(View.VISIBLE);
			//}
			
			// send updated sysui visibility to window manager
            notifyUiVisibilityChanged(mSystemUiVisibility);
		}
	}

	private void notifyUiVisibilityChanged(int vis) {
        try {
            if (mLastDispatchedSystemUiVisibility != vis) {
                mWindowManagerService.statusBarVisibilityChanged(vis);
                mLastDispatchedSystemUiVisibility = vis;
            }
        } catch (RemoteException ex) {
        }
    }
	
	public void topAppWindowChanged(boolean visible){}
	public void setImeWindowStatus(IBinder token, int vis, int backDisposition,
	        boolean showImeSwitcher){}
	public void showRecentApps(boolean triggeredFromAltTab){}
	public void hideRecentApps(boolean triggeredFromAltTab, boolean triggeredFromHomeKey){}
	public void toggleRecentApps(){}
	public void preloadRecentApps(){}
	public void cancelPreloadRecentApps(){}
	public void setWindowState(int window, int state){}
	public void buzzBeepBlinked(){}
	public void notificationLightOff(){}
	public void notificationLightPulse(int argb, int onMillis, int offMillis){}
	public void showScreenPinningRequest(){}
	public void appTransitionPending(){}
	public void appTransitionCancelled(){}
	public void appTransitionStarting(long startTime, long duration){}
	public void showAssistDisclosure(){}
	public void startAssist(Bundle args){}
	public void onCameraLaunchGestureDetected(int source){}
}
