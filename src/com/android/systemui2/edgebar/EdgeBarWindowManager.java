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
 * limitations under the License
 */

package com.android.systemui2.edgebar;

import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.Resources;
import android.graphics.PixelFormat;
import android.os.SystemProperties;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.lang.reflect.Field;

/**
 * Encapsulates all logic for the status bar window state management.
 */
public class EdgeBarWindowManager {
    private final Context mContext;
    private WindowManager mWindowManager;
    private View mEdgeBarView;
    private WindowManager.LayoutParams mLp;

	public EdgeBarWindowManager(Context context) {
        mContext = context;
        mWindowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
	}
	 
    public EdgeBarWindowManager(Context context, WindowManager windowManager) {
        mContext = context;
        mWindowManager = windowManager;
	}
	
    /**
     * Adds the status bar view to the window manager.
     *
     * @param statusBarView The view to add.
     * @param barHeight The height of the status bar in collapsed state.
     */
    public void add(View EdgeBarView, int gravity, int width) {
        //mWindowManager.addView(mEdgeBarView, mLp);
    }
	
	public void setVisibility(int vis) {
	    mEdgeBarView.setVisibility(vis);
	}
	
    public void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        pw.println("EdgeBarWindowManager state:");
    }
}
