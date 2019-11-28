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

import android.animation.ValueAnimator;
import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.annotation.NonNull;
import android.annotation.Nullable;
import android.view.View;
import android.util.AttributeSet;
import android.view.animation.DecelerateInterpolator;
import android.widget.FrameLayout;
import android.util.Log;

/**
 * slider animation
 */
public class SliderView extends View {
    private ISliderView slideView;
	private static final String TAG = "SliderView";
	
    private static final DecelerateInterpolator DECELERATE_INTERPOLATOR = new DecelerateInterpolator();
	
    private ValueAnimator mAnimator;
    private float mRate = 0;//曲线的控制点
    
	public SliderView(Context context, @NonNull ISliderView slideView) {
        this(context, slideView, null);
    }

    public SliderView(Context context, @NonNull ISliderView slideView, @Nullable AttributeSet attrs) {
        this(context, slideView, attrs, 0);
    }

    public SliderView(Context context, @NonNull ISliderView slideView, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
		this.slideView = slideView;
    }
	
    public ISliderView getSlideView() {
        return slideView;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);

		Log.e(TAG, "onDraw mRate:" + mRate);
        slideView.onDraw(canvas, mRate);
    }

    public void updateRate(float newRate, boolean hasAnim) {
        if (newRate > getWidth()) {
            newRate = getWidth();
        }
        if (mRate == newRate) {
            return;
        }
		
        cancelAnim();
        if (!hasAnim) {
            mRate = newRate;
            invalidate();
            if (mRate == 0) {
                setVisibility(GONE);
            }else{
                setVisibility(VISIBLE);
            }
        }

        mAnimator = ValueAnimator.ofFloat(mRate, newRate);
        mAnimator.setDuration(50);
        mAnimator.addUpdateListener(new ValueAnimator.AnimatorUpdateListener() {
			@Override
	        public void onAnimationUpdate(ValueAnimator animation) {
	            float mRate = (float) animation.getAnimatedValue();
	            
				postInvalidate();
				if (mRate == 0) {
					setVisibility(GONE);
				}else{
					setVisibility(VISIBLE);
				}
	        }
	    });
		
        mAnimator.setInterpolator(DECELERATE_INTERPOLATOR);
        mAnimator.start();
    }

    private void cancelAnim() {
        if (mAnimator != null && mAnimator.isRunning()) {
            mAnimator.cancel();
        }
    }

    @Override
    protected void onDetachedFromWindow() {
        cancelAnim();
        if (mRate != 0) {
            mRate = 0;
            invalidate();
        }
        super.onDetachedFromWindow();
    }
}
