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

import android.graphics.Canvas;

public interface ISliderView {
        /**
         * 是否可以垂直滑动
         *
         * @return
         */
        boolean scrollVertical();

        /**
         * 宽度
         *
         * @return
         */
        int getWidth();

        /**
         * 高度
         *
         * @return
         */
        int getHeight();

        /**
         * 绘制
         *
         * @param canvas
         * @param currentWidth 根据手指滑动得出的当前宽度（最大值为getWidth())
         */
        void onDraw(Canvas canvas, float currentWidth);
    }
