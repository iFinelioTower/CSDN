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

import android.content.Context;

import android.util.Log;
import android.util.DisplayMetrics;

import android.content.res.Configuration;

import android.view.View;
import android.view.Window;
import android.view.Gravity;
import android.view.WindowManager;
import android.media.AudioManager;

import android.view.LayoutInflater;
import android.view.WindowManager.LayoutParams;

import android.view.KeyEvent;
import android.view.MotionEvent;

import android.widget.ImageView;

import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.drawable.ColorDrawable;

import android.animation.ValueAnimator;
import android.animation.ObjectAnimator;
import android.view.animation.Animation;
import android.view.animation.AnimationSet;
import android.view.animation.AlphaAnimation;
import android.view.animation.TranslateAnimation;

import android.os.Looper;
import android.os.Handler;
import android.os.Message;
import android.widget.ImageView;
import android.widget.TextView;

import android.animation.Animator;
import android.view.ViewAnimationUtils;
import android.animation.AnimatorListenerAdapter;

import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.os.BatteryManager;

import android.os.SystemProperties;

import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import com.android.systemui2.R;
import com.android.systemui2.utils.KeyboardUtils;

enum DisplayState {
	DISPLAY_DIM,
	DISPLAY_FULL,
	DISPLAY_MISS,
}

public class FloatshortcutUIButton implements FloatshortcutUI.FloatButtonImpl {
    private static final String TAG = "FloatshortcutUIButton";
	private Context mContext;

	private int mScreenWidth;
	private boolean mWeltWhat = false;
	
	private View mFloatView;
	private ImageView mImageView;
	private WindowManager mWindowManager;
    private LayoutParams mLayoutParams;
    
	private final H mHandler = new H();	
	private ValueAnimator mWeltValueAnimator;
	private DisplayState mDispalyState = DisplayState.DISPLAY_MISS;
	
	private LayoutParams backHomeParams;
	
	private LayoutParams createLayout() {
		LayoutParams layoutParams = new LayoutParams (
								LayoutParams.WRAP_CONTENT,
								LayoutParams.WRAP_CONTENT,
								LayoutParams.TYPE_PHONE,
								LayoutParams.FLAG_NOT_FOCUSABLE,
								PixelFormat.TRANSLUCENT);
		layoutParams.gravity = Gravity.CENTER | Gravity.LEFT;
		
		return layoutParams;
	}

	private View.OnTouchListener touchListener = new View.OnTouchListener() {
		@Override
		public boolean onTouch(View v, MotionEvent event){
			switch (event.getAction()) {
	        case MotionEvent.ACTION_DOWN:
				v.setScaleX(1.2f);
				v.setScaleY(1.2f);
				break;
			case MotionEvent.ACTION_UP:
				v.setScaleX(1f);
				v.setScaleY(1f);
				break;
			}
				return false;
		}

	};

    public FloatshortcutUIButton(Context context, FloatshortcutUIController controller) {
        mContext = context;
		mWindowManager = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
		
		mLayoutParams  = (LayoutParams) createLayout();
        mFloatView = LayoutInflater.from(context).inflate(R.layout.float_ball, null);
		mScreenWidth  = mContext.getResources().getDisplayMetrics().widthPixels;
		
        // init UI.
        mFloatView.setScaleX(0.9f);
	    mFloatView.setScaleY(0.9f);
		mHandler.sendMessage(mHandler.obtainMessage(H.MSG_DIM_HIDE)); 
		mFloatView.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
			    if (mDispalyState == DisplayState.DISPLAY_FULL)
					KeyboardUtils.getInstance().sendKey(KeyEvent.KEYCODE_BACK);
			}
		});

		mFloatView.setOnTouchListener(new View.OnTouchListener() {
		    int xDelta,yDelta;
			private long mEndTime = 0;
			private long mStartTime = 0;
			private boolean isClick = false;
			
			@Override
			public boolean onTouch(View view, MotionEvent event) {
				final int mStartX = (int) event.getRawX();
		        final int mStartY = (int) event.getRawY();
		        switch (event.getAction()) {
		        case MotionEvent.ACTION_DOWN :
					isClick = false;
					xDelta = (int)event.getX();
					yDelta =(int)event.getY();
					
					mStartTime = System.currentTimeMillis();
		
					view.setScaleX(1.0f);
					view.setScaleY(1.0f);
					mHandler.removeMessages(H.MSG_DIM_HIDE);
					mHandler.removeMessages(H.MSG_FULL_ONTOUCH);
					mHandler.sendMessage(mHandler.obtainMessage(H.MSG_FULL_TOUCHING));
					break;
				
				case MotionEvent.ACTION_MOVE :
					mLayoutParams.gravity = Gravity.LEFT | Gravity.TOP;
					isClick = true;
			
					mLayoutParams.x = mStartX - xDelta;
					mLayoutParams.y = mStartY - yDelta;
	
					mWindowManager.updateViewLayout(mFloatView, mLayoutParams);
					
					break;
				
		        case MotionEvent.ACTION_UP :
					mEndTime = System.currentTimeMillis();
					if ((mEndTime - mStartTime) > (0.2 * 1000L)) {
		                isClick = true;	
					} 
					view.setScaleX(0.9f);
				    view.setScaleY(0.9f);
					mWeltWhat = ((int) event.getRawX() > (mScreenWidth / 2));
					startWeltAnimation(mLayoutParams.x);
					mHandler.sendMessageDelayed(mHandler.obtainMessage(H.MSG_FULL_ONTOUCH), 1000 * 2);
					break;
					
				default:
					break;
				}
				
				return isClick;
			};
		});
		
		mImageView = (ImageView) mFloatView.findViewById(R.id.floatButton);	
		show();
	}

	private void startWeltAnimation(int startX) {
		int dest = 0;
	    if (mWeltWhat) {
			dest = mScreenWidth + mFloatView.getWidth();
		}
		
		mWeltValueAnimator = ValueAnimator.ofInt(startX, dest);
		if (mWeltValueAnimator != null) {
			mWeltValueAnimator.setDuration(500);
			mWeltValueAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
				@Override
				public void onAnimationUpdate(ValueAnimator animation) {
					Integer value = (Integer)animation.getAnimatedValue();
					mLayoutParams.x = value;
					mWindowManager.updateViewLayout(mFloatView, mLayoutParams);
				};
			});
			mWeltValueAnimator.start();
		}
	}
	
	private final class H extends Handler {
        private static final int MSG_DIM_HIDE       = 1;
        private static final int MSG_FULL_ONTOUCH   = 2;
        private static final int MSG_FULL_TOUCHING  = 3;
		
        public H() {
            super(Looper.getMainLooper());
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_DIM_HIDE:
				mDispalyState = DisplayState.DISPLAY_DIM;
                if (mWeltWhat) {
                    mImageView.setBackgroundResource(R.mipmap.ic_public_float_dim_right);
                } else {
					mImageView.setBackgroundResource(R.mipmap.ic_public_float_dim_left);
                }
                break;

            case MSG_FULL_ONTOUCH:
                mImageView.setBackgroundResource(R.mipmap.ic_public_float_notouch);
                mHandler.sendMessageDelayed(mHandler.obtainMessage(H.MSG_DIM_HIDE), 1200);
                break;

            case MSG_FULL_TOUCHING:
				mDispalyState = DisplayState.DISPLAY_FULL;
                mImageView.setBackgroundResource(R.mipmap.ic_public_float_touch);
				break;
				
			default:
				break;
            }
        }
    }

	@Override
	public void show() {
	    if (mDispalyState != DisplayState.DISPLAY_DIM) {
			mDispalyState = DisplayState.DISPLAY_DIM;
			mWindowManager.addView(mFloatView, mLayoutParams);
		}
	}
	
	public void showFull() {
	    if (mDispalyState != DisplayState.DISPLAY_FULL) {
			mDispalyState = DisplayState.DISPLAY_FULL;
			mWindowManager.addView(mFloatView, mLayoutParams);
		}
	}

	@Override
	public void dismiss() {
	    if (mDispalyState != DisplayState.DISPLAY_MISS) {
			mDispalyState = DisplayState.DISPLAY_MISS;
	        mWindowManager.removeView(mFloatView);
	    }
	}
	
	@Override
	public void update() {
	
	}
}
