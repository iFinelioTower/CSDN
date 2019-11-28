/*
 * Copyright (C) 2019 The iFinelio Tower <308662170@qq.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * CSDN blog:
 *       
 *       https://blog.csdn.net/qq_33443989
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


package com.android.systemui2.edgebar.bezierslider;

import android.util.Log;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.PixelFormat;
import android.media.session.MediaSessionLegacyHelper;

import android.os.IBinder;
import android.util.AttributeSet;
import android.util.DisplayMetrics;

import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewRootImpl;
import android.view.WindowManager;
import android.view.WindowManagerGlobal;
import android.view.Gravity;
import android.widget.FrameLayout;

import com.android.systemui2.utils.WindowUtils;
import com.android.systemui2.edgebar.EdgeEvent;

public class EdgeBarWindowView extends FrameLayout {
    public static final String TAG = "EdgeBarWindowView";
    public static final boolean DEBUG = true;

	private float downX;
    private float moveX;
	private boolean startDrag = false;	
	
	private boolean mEnable = true;	
	private OnEventCallback mEventCallback = null;
    private float mTriggerEventWidth = -1;
	private SliderView mSliderView = null;

	private EdgeEvent mEvent = EdgeEvent.EDGE_UNKNOWN;
	private WindowManager mWindowManager;

	private int mLayoutWidth;
	private WindowManager.LayoutParams mLp;
    private WindowManager.LayoutParams mLpChanged;
	private final State mCurrentState = new State();
	
    public EdgeBarWindowView(Context context, AttributeSet attrs) {
        super(context, attrs);
		setMotionEventSplittingEnabled(false);
    }

	public void init(WindowManager wm, EdgeEvent event, int width) {
		mWindowManager = wm;
		mEvent = event;
		mLayoutWidth = width;
		
		initBezierSliderView();
		
        // Now that the status bar window encompasses the sliding panel and its
        // translucent backdrop, the entire thing is made TRANSLUCENT and is
        // hardware-accelerated.        
        mLp = new WindowManager.LayoutParams(width, 
                ViewGroup.LayoutParams.MATCH_PARENT,
                WindowManager.LayoutParams.TYPE_PHONE,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                        | WindowManager.LayoutParams.FLAG_TOUCHABLE_WHEN_WAKING
                        | WindowManager.LayoutParams.FLAG_SPLIT_TOUCH
                        | WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH
                        | WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS,
                PixelFormat.TRANSLUCENT);
        mLp.flags |= WindowManager.LayoutParams.FLAG_HARDWARE_ACCELERATED;
        mLp.gravity = getEventGravity(event);
        mLp.softInputMode = WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE;
		
        mLp.setTitle("EdgeBar");
		mLp.packageName = mContext.getPackageName();
        wm.addView(this, mLp);

		mLpChanged = new WindowManager.LayoutParams();
        mLpChanged.copyFrom(mLp);
	}
	
	public void setEdgeEventCallback(OnEventCallback callback) {
	    this.mEventCallback = callback;
	}

	public boolean getEdgeEventEnable() {
	    return this.mEnable;
	}
	
	public void setEdgeEventEnable(boolean enable) {
	    this.mEnable = enable;
	}

	private boolean isEventMatch(EdgeEvent event) {
	    if (event == mEvent) {
			return true;
		}

		return false;
	}
	
	private int getEventGravity(EdgeEvent event) {
	    if (isEventMatch(EdgeEvent.EDGE_LEFT)) {
			return Gravity.LEFT;
		}

		if (isEventMatch(EdgeEvent.EDGE_RIGHT)) {
			return Gravity.RIGHT;
		}

		return Gravity.CENTER;
	}


	private float screenWidth; // 屏幕宽
	private void initBezierSliderView() {
	    ISliderView sliderView = new DefaultSliderView(mContext);
		
		// 获取屏幕信息，初始化控件设置
		DisplayMetrics dm = mContext.getResources().getDisplayMetrics();
		screenWidth = dm.widthPixels;
		mSliderView = new SliderView(mContext, sliderView);
		
		if (isEventMatch(EdgeEvent.EDGE_LEFT)) {
			this.mTriggerEventWidth = WindowUtils.d2p(mContext, 18);
		} else {
			this.mTriggerEventWidth = screenWidth - WindowUtils.d2p(mContext, 18);
			// 右侧侧滑 需要旋转180°
			mSliderView.setRotationY(180);
		}
		
		addView(mSliderView);
	}
	
    @Override
    protected boolean fitSystemWindows(Rect insets) {
        if (getFitsSystemWindows()) {
			// do some thing
        } else {
			// do some thing
        }
		
        return false;
    }
		
	private void onEventCallback(EdgeEvent event) {
        if (mEventCallback == null) {
			// Default support Action.
        } else {
            mEventCallback.onEvent(event);
        }
    }

	/**
     * 给SlideBackIconView设置topMargin，起到定位效果
     *
     * @param view     SlideBackIconView
     * @param position 触点位置
     */
    private void setSlideBackPosition(SliderView view, int position) {
        // 触点位置减去 SlideBackIconView 一半高度即为 topMargin
        int topMargin = (int) (position - (view.getSlideView().getHeight() / 2));
        FrameLayout.LayoutParams layoutParams = new FrameLayout.LayoutParams(view.getLayoutParams());
        layoutParams.topMargin = topMargin;
        view.setLayoutParams(layoutParams);
    }
	
    @Override
    public boolean dispatchKeyEvent(KeyEvent event) {
        return super.dispatchKeyEvent(event);
    }

    @Override
    public boolean dispatchTouchEvent(MotionEvent ev) {
        return super.dispatchTouchEvent(ev);
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent motionEvent) {
        if (!mEnable) {
            return false;
        }

        switch (motionEvent.getAction()) {
            case MotionEvent.ACTION_DOWN:
				if (motionEvent.getRawX() <= mTriggerEventWidth) {
                    return true;
				}
        }
		
        return super.onInterceptTouchEvent(motionEvent);
    }
	
	@Override
	public boolean onTouchEvent(MotionEvent motionEvent) {
		if (!mEnable) {
			return super.onTouchEvent(motionEvent);
		}

        float currentX = motionEvent.getRawX();
		
		switch (motionEvent.getAction()) {
			case MotionEvent.ACTION_DOWN:
				setPanelVisible(true);
		
				float currentY = motionEvent.getRawY();
			    boolean isTrigger = (Math.abs(currentX - mTriggerEventWidth) > 0);	
				if (currentY > WindowUtils.d2p(mContext, 100) && isTrigger) {
					downX = currentX;
					startDrag = true;
					mSliderView.updateRate(0, false);
					setSlideBackPosition(mSliderView, (int) (motionEvent.getRawY()));
				}
				break;

			case MotionEvent.ACTION_MOVE:
				if (startDrag) {
					moveX = currentX - downX;
					if (Math.abs(moveX) <= mSliderView.getSlideView().getWidth() * 2) {
						mSliderView.updateRate(Math.abs(moveX) / 2, false);
					} else {
						mSliderView.updateRate(mSliderView.getSlideView().getWidth(), false);
					}
					setSlideBackPosition(mSliderView, (int) (motionEvent.getRawY()));
				}
				break;

			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_CANCEL:
			case MotionEvent.ACTION_OUTSIDE:
				setPanelVisible(false);
			
				if (startDrag && moveX >= mSliderView.getSlideView().getWidth() * 2) {
					onEventCallback(mEvent);
					mSliderView.updateRate(0, false);
				} else {
					mSliderView.updateRate(0, startDrag);
				}
				
				moveX = 0;
				startDrag = false;
				break;
		}

		return startDrag || super.onTouchEvent(motionEvent);
	}

    @Override
    public void onDraw(Canvas canvas) {
        super.onDraw(canvas);
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();		
    }
	
    @Override
    protected void onAttachedToWindow () {
        super.onAttachedToWindow();
    }

	public void setPanelExpanded(boolean isExpanded) {
		mCurrentState.panelExpanded = isExpanded;
		apply(mCurrentState);
	}
	
	public void setPanelVisible(boolean visible) {
	    mCurrentState.panelVisible = visible;
		apply(mCurrentState);
    }
	
	private boolean isExpanded(State state) {
        return state.panelVisible;
    }
	
	private void applywidth(State state) {
        boolean expanded = isExpanded(state);
        if (expanded) {
            mLpChanged.width = ViewGroup.LayoutParams.MATCH_PARENT;
        } else {
            mLpChanged.width = mLayoutWidth;
        }
    }

	private void apply(State state) {
        applywidth(state);
        if (mLp.copyFrom(mLpChanged) != 0) {
            mWindowManager.updateViewLayout(this, mLp);
        }
    }
	
	private static class State {
		boolean panelVisible;
		boolean panelExpanded;
    }
}

