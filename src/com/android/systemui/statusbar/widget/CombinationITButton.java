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
import android.view.MotionEvent;
import android.view.LayoutInflater;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.ImageView;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;

import android.util.AttributeSet;
import android.content.res.TypedArray;
import android.graphics.drawable.Drawable;

import android.util.Log;

import com.android.systemui.R;
import com.android.systemui.qs.QSTile;
import com.android.systemui.qs.QSTile.State;
import com.android.systemui.qs.QSTile.SignalState;
import com.android.systemui.qs.QSTile.BooleanState;

public class CombinationITButton extends LinearLayout implements QSTile.Callback, 
	    View.OnClickListener, View.OnLongClickListener {
	    
	private static final String TAG = "CombinationITButton";
	private static final boolean DEBUG = true;
	
	private final H mHandler = new H();

    private ImageView iv;
    private TextView tv;

	private int ce = -1;
	private int cd = -1;
	
    public CombinationITButton(Context context) {
        super(context);
    }
	
    public CombinationITButton(Context context, AttributeSet attrs) {
        super(context, attrs);
        LayoutInflater.from(context).inflate(R.layout.combination_itbutton, this, true);
        iv = (ImageView) findViewById(R.id.iv);
        tv = (TextView) findViewById(R.id.tv);
		
		obtainStyleAttrs(attrs);
    }

	private void obtainStyleAttrs(AttributeSet attrs) {
        TypedArray ta = getContext().obtainStyledAttributes(attrs, R.styleable.CombinationITButton);
		setDefaultImageDrawable(ta.getDrawable(R.styleable.CombinationITButton_imageSrc));
		setDefaultTextViewText(ta.getString(R.styleable.CombinationITButton_textDesc));
		
		ce = ta.getColor(R.styleable.CombinationITButton_colorEnable, 0);
		cd = ta.getColor(R.styleable.CombinationITButton_colorDisable, 0);
		
        ta.recycle();
    }
	
    private void setDefaultImageResource(int resId) {
        iv.setImageResource(resId);
    }

	private void setDefaultImageDrawable(Drawable drawable) {
	    iv.setImageDrawable(drawable);
	}

    private void setDefaultTextViewText(String text) {
        tv.setText(text);
    }

    /**
     * @param resId
     */
    public void setImageResource(int resId) {
        iv.setImageResource(resId);
    }

	/**
     * @param color
     */
    public void setColorFilter(int color) {
        iv.setColorFilter(color);
    }
	
    /**
     * @param text
     */
    public void setTextViewText(String text) {
        tv.setText(text);
    }

    /**
     * @param color
     */
    public void setTextColor(int color) {
        tv.setTextColor(color);
    }
	
	private OnClickCallbcak callback = null;
	public void setOnClickCallBack(OnClickCallbcak cb) {
	    callback = cb;
		iv.setOnClickListener(this);
		iv.setOnLongClickListener(this);
	}
	
	@Override
    public void onClick(View v) {
		if (callback != null) 
			callback.onClick();
    }

	@Override
	public boolean onLongClick(View v) {
        if (callback != null) 
			callback.onLongClick();

		return true;
    }
	
	@Override
	public void onStateChanged(QSTile.State state) {
	    if (DEBUG) Log.e(TAG, "state: " + state);
		mHandler.obtainMessage(H.STATE_CHANGED, state).sendToTarget();
	}

	@Override
    public void onShowDetail(boolean show) {

	}

	@Override
    public void onToggleStateChanged(boolean state) {

	}

	@Override
	public void onScanStateChanged(boolean state) {

	}
	
	@Override
    public void onAnnouncementRequested(CharSequence announcement) {

	}
	
	protected void handleStateChanged(QSTile.State state) {
	    if (state instanceof BooleanState)	{
			final BooleanState s = (BooleanState) state;
			setColorFilter(s.value ? ce : cd);
		} else if (state instanceof SignalState){
			Log.e(TAG, "wait handleStateChanged state:" + state);
			final SignalState s = (SignalState) state;
			setColorFilter(s.enabled ? ce : cd);
			setTextViewText(s.label);
		}
    }
	
	private class H extends Handler {
        private static final int STATE_CHANGED = 1;
        public H() {
            super(Looper.getMainLooper());
        }
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == STATE_CHANGED) {
                handleStateChanged((State) msg.obj);
            }
        }
    }

	public interface OnClickCallbcak {
	    void onClick();
		void onLongClick();
	}
}

