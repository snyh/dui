/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RuntimeEnabledFeatures_h
#define RuntimeEnabledFeatures_h

#include "platform/PlatformExportMacros.h"

namespace WebCore {

// A class that stores static enablers for all experimental features. Note that
// the method names must line up with the JavaScript method they enable for code
// generation to work properly.

class RuntimeEnabledFeatures {
public:
    static void setLocalStorageEnabled(bool isEnabled) { isLocalStorageEnabled = isEnabled; }
    static bool localStorageEnabled() { return isLocalStorageEnabled; }

    static void setSessionStorageEnabled(bool isEnabled) { isSessionStorageEnabled = isEnabled; }
    static bool sessionStorageEnabled() { return isSessionStorageEnabled; }

    static void setWebkitNotificationsEnabled(bool isEnabled) { isWebkitNotificationsEnabled = isEnabled; }
    static bool webkitNotificationsEnabled() { return isWebkitNotificationsEnabled; }

    static void setApplicationCacheEnabled(bool isEnabled) { isApplicationCacheEnabled = isEnabled; }
    static bool applicationCacheEnabled() { return isApplicationCacheEnabled; }

    static void setDataTransferItemsEnabled(bool isEnabled) { isDataTransferItemsEnabled = isEnabled; }
    static bool dataTransferItemsEnabled() { return isDataTransferItemsEnabled; }

    static void setWebkitIndexedDBEnabled(bool isEnabled) { isIndexedDBEnabled = isEnabled; }
    static bool webkitIndexedDBEnabled() { return isIndexedDBEnabled; }
    static bool indexedDBEnabled() { return isIndexedDBEnabled; }

#if ENABLE(CANVAS_PATH)
    static void setCanvasPathEnabled(bool isEnabled) { isCanvasPathEnabled = isEnabled; }
    static bool canvasPathEnabled() { return isCanvasPathEnabled; }
#else
    static void setCanvasPathEnabled(bool) { }
    static bool canvasPathEnabled() { return false; }
#endif

#if ENABLE(CSS_EXCLUSIONS)
    static void setCSSExclusionsEnabled(bool isEnabled) { isCSSExclusionsEnabled = isEnabled; }
    static bool cssExclusionsEnabled() { return isCSSExclusionsEnabled; }
#else
    static void setCSSExclusionsEnabled(bool) { }
    static bool cssExclusionsEnabled() { return false; }
#endif

#if ENABLE(CSS_SHAPES)
    static void setCSSShapesEnabled(bool isEnabled) { isCSSShapesEnabled = isEnabled; }
    static bool cssShapesEnabled() { return isCSSShapesEnabled; }
#else
    static void setCSSShapesEnabled(bool) { }
    static bool cssShapesEnabled() { return false; }
#endif

#if ENABLE(CSS_REGIONS)
    static void setCSSRegionsEnabled(bool isEnabled) { isCSSRegionsEnabled = isEnabled; }
    static bool cssRegionsEnabled() { return isCSSRegionsEnabled; }
#else
    static void setCSSRegionsEnabled(bool) { }
    static bool cssRegionsEnabled() { return false; }
#endif

    static void setCSSCompositingEnabled(bool isEnabled) { isCSSCompositingEnabled = isEnabled; }
    static bool cssCompositingEnabled() { return isCSSCompositingEnabled; }

#if ENABLE(FONT_LOAD_EVENTS)
    static void setFontLoadEventsEnabled(bool isEnabled) { isFontLoadEventsEnabled = isEnabled; }
    static bool fontLoadEventsEnabled() { return isFontLoadEventsEnabled; }
#else
    static void setFontLoadEventsEnabled(bool) { }
    static bool fontLoadEventsEnabled() { return false; }
#endif

#if ENABLE(TOUCH_EVENTS)
    static bool touchEnabled() { return isTouchEnabled; }
    static void setTouchEnabled(bool isEnabled) { isTouchEnabled = isEnabled; }
#endif

    static void setDeviceMotionEnabled(bool isEnabled) { isDeviceMotionEnabled = isEnabled; }
    static bool deviceMotionEnabled() { return isDeviceMotionEnabled; }
    static bool deviceMotionEventEnabled() { return isDeviceMotionEnabled; }
    static bool ondevicemotionEnabled() { return isDeviceMotionEnabled; }

    static void setDeviceOrientationEnabled(bool isEnabled) { isDeviceOrientationEnabled = isEnabled; }
    static bool deviceOrientationEnabled() { return isDeviceOrientationEnabled; }
    static bool deviceOrientationEventEnabled() { return isDeviceOrientationEnabled; }
    static bool ondeviceorientationEnabled() { return isDeviceOrientationEnabled; }

#if ENABLE(CUSTOM_ELEMENTS)
    static bool customDOMElementsEnabled() { return isCustomDOMElementsEnabled; }
    static void setCustomDOMElements(bool isEnabled) { isCustomDOMElementsEnabled = isEnabled; }
#endif

#if ENABLE(INPUT_TYPE_DATE)
    static bool inputTypeDateEnabled() { return isInputTypeDateEnabled; }
    static void setInputTypeDateEnabled(bool isEnabled) { isInputTypeDateEnabled = isEnabled; }
#endif

#if ENABLE(INPUT_TYPE_DATETIME_INCOMPLETE)
    static bool inputTypeDateTimeEnabled() { return isInputTypeDateTimeEnabled; }
    static void setInputTypeDateTimeEnabled(bool isEnabled) { isInputTypeDateTimeEnabled = isEnabled; }
#endif

#if ENABLE(INPUT_TYPE_DATETIMELOCAL)
    static bool inputTypeDateTimeLocalEnabled() { return isInputTypeDateTimeLocalEnabled; }
    static void setInputTypeDateTimeLocalEnabled(bool isEnabled) { isInputTypeDateTimeLocalEnabled = isEnabled; }
#endif

#if ENABLE(INPUT_TYPE_MONTH)
    static bool inputTypeMonthEnabled() { return isInputTypeMonthEnabled; }
    static void setInputTypeMonthEnabled(bool isEnabled) { isInputTypeMonthEnabled = isEnabled; }
#endif

#if ENABLE(INPUT_TYPE_TIME)
    static bool inputTypeTimeEnabled() { return isInputTypeTimeEnabled; }
    static void setInputTypeTimeEnabled(bool isEnabled) { isInputTypeTimeEnabled = isEnabled; }
#endif

#if ENABLE(INPUT_TYPE_WEEK)
    static bool inputTypeWeekEnabled() { return isInputTypeWeekEnabled; }
    static void setInputTypeWeekEnabled(bool isEnabled) { isInputTypeWeekEnabled = isEnabled; }
#endif

#if ENABLE(DIALOG_ELEMENT)
    static bool dialogElementEnabled() { return isDialogElementEnabled; }
    static void setDialogElementEnabled(bool isEnabled) { isDialogElementEnabled = isEnabled; }
#endif

    static bool langAttributeAwareFormControlUIEnabled() { return isLangAttributeAwareFormControlUIEnabled; }
    // The lang attribute support is incomplete and should only be turned on for tests.
    static void setLangAttributeAwareFormControlUIEnabled(bool isEnabled) { isLangAttributeAwareFormControlUIEnabled = isEnabled; }

private:
    // Never instantiate.
    RuntimeEnabledFeatures() { }

    static bool isLocalStorageEnabled;
    static bool isSessionStorageEnabled;
    static bool isWebkitNotificationsEnabled;
    static bool isApplicationCacheEnabled;
    static bool isDataTransferItemsEnabled;
    static bool isIndexedDBEnabled;
    static bool isTouchEnabled;
    static bool isDeviceMotionEnabled;
    static bool isDeviceOrientationEnabled;
    static bool isCanvasPathEnabled;
    static bool isCSSExclusionsEnabled;
    static bool isCSSShapesEnabled;
    static bool isCSSRegionsEnabled;
    static bool isCSSCompositingEnabled;
    WEBCORE_TESTING static bool isLangAttributeAwareFormControlUIEnabled;
#if ENABLE(CUSTOM_ELEMENTS)
    static bool isCustomDOMElementsEnabled;
#endif

#if ENABLE(INPUT_TYPE_DATE)
    static bool isInputTypeDateEnabled;
#endif

#if ENABLE(INPUT_TYPE_DATETIME_INCOMPLETE)
    static bool isInputTypeDateTimeEnabled;
#endif

#if ENABLE(INPUT_TYPE_DATETIMELOCAL)
    static bool isInputTypeDateTimeLocalEnabled;
#endif

#if ENABLE(INPUT_TYPE_MONTH)
    static bool isInputTypeMonthEnabled;
#endif

#if ENABLE(INPUT_TYPE_TIME)
    static bool isInputTypeTimeEnabled;
#endif

#if ENABLE(INPUT_TYPE_WEEK)
    static bool isInputTypeWeekEnabled;
#endif

#if ENABLE(DIALOG_ELEMENT)
    static bool isDialogElementEnabled;
#endif

#if ENABLE(FONT_LOAD_EVENTS)
    static bool isFontLoadEventsEnabled;
#endif

};

} // namespace WebCore

#endif // RuntimeEnabledFeatures_h
