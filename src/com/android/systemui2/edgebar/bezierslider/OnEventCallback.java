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

import com.android.systemui2.edgebar.EdgeEvent;

public interface OnEventCallback {
    void onEvent(EdgeEvent event);
}
