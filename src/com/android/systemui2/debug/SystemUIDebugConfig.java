/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.systemui2.debug;

class SystemUIDebugConfig {
    // Enable all debug log categories.
    static final boolean DEBUG_ALL = false;

    // Available log categories in the SystemUI2.
    static final boolean DEBUG_POWER = DEBUG_ALL || false;
    static final boolean DEBUG_MEDIA = DEBUG_ALL || false;
    static final boolean DEBUG_VOLUME = DEBUG_ALL || false;
    static final boolean DEBUG_FLOATSHORTCUT = DEBUG_ALL || false;
}