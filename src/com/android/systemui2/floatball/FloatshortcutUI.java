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

package com.android.systemui2.floatball;

import android.content.res.Configuration;
import android.util.Log;

import com.android.systemui2.R;
import com.android.systemui2.SystemUI;

import java.io.FileDescriptor;
import java.io.PrintWriter;

/*
 * #1. https://blog.csdn.net/HuangTieBing/article/details/62423970
 * #2. 
 */

public class FloatshortcutUI extends SystemUI {
    private static final String TAG = "FloatshortcutUI";

	private FloatshortcutUIButton mButton;
	private FloatshortcutUIController mController;

    @Override
    public void start() {
        Log.e(TAG, "on Start");	
		mController = new FloatshortcutUIController(mContext);
		mButton = new FloatshortcutUIButton(mContext, mController);
    }
	
	public interface FloatButtonImpl {
		void update();
		void show();
		void dismiss();
	}
}
