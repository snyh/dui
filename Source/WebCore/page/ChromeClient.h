/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012 Apple, Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2012 Samsung Electronics. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef ChromeClient_h
#define ChromeClient_h

#include "accessibility/AXObjectCache.h"
#include "platform/Cursor.h"
#include "page/FocusDirection.h"
#include "loader/FrameLoader.h"
#include "platform/graphics/GraphicsContext.h"
#include "platform/HostWindow.h"
#include "platform/PopupMenu.h"
#include "platform/PopupMenuClient.h"
#include "platform/ScrollTypes.h"
#include "platform/SearchPopupMenu.h"
#include "page/WebCoreKeyboardUIMode.h"
#include <wtf/Forward.h>
#include <wtf/PassOwnPtr.h>
#include <wtf/Vector.h>

OBJC_CLASS NSResponder;

namespace WebCore {

class AccessibilityObject;
class ColorChooser;
class ColorChooserClient;
class DateTimeChooser;
class DateTimeChooserClient;
class Element;
class FileChooser;
class FloatRect;
class Frame;
class GraphicsContext3D;
class GraphicsLayer;
class GraphicsLayerFactory;
class HitTestResult;
class HTMLInputElement;
class IntRect;
class NavigationAction;
class Node;
class Page;
class PopupMenuClient;
class Widget;

struct DateTimeChooserParameters;
struct FrameLoadRequest;
struct GraphicsDeviceAdapter;
struct ViewportArguments;
struct WindowFeatures;

class ChromeClient {
public:
    virtual void chromeDestroyed() = 0;

    virtual void setWindowRect(const FloatRect&) = 0;
    virtual FloatRect windowRect() = 0;

    virtual FloatRect pageRect() = 0;

    virtual void focus() = 0;
    virtual void unfocus() = 0;

    virtual bool canTakeFocus(FocusDirection) = 0;
    virtual void takeFocus(FocusDirection) = 0;

    virtual void focusedNodeChanged(Node*) = 0;
    virtual void focusedFrameChanged(Frame*) = 0;

    virtual void show() = 0;

    virtual void closeWindowSoon() = 0;

    virtual KeyboardUIMode keyboardUIMode() = 0;

    virtual IntRect windowResizerRect() const = 0;

    // Methods used by HostWindow.
    virtual bool supportsImmediateInvalidation() { return false; }
    virtual void invalidateRootView(const IntRect&, bool immediate) = 0;
    virtual void invalidateContentsAndRootView(const IntRect&, bool immediate) = 0;
    virtual void invalidateContentsForSlowScroll(const IntRect&, bool immediate) = 0;
    virtual void scroll(const IntSize&, const IntRect&, const IntRect&) = 0;
#if USE(TILED_BACKING_STORE)
    virtual void delegatedScrollRequested(const IntPoint&) = 0;
#endif
    virtual IntPoint screenToRootView(const IntPoint&) const = 0;
    virtual IntRect rootViewToScreen(const IntRect&) const = 0;
    virtual PlatformPageClient platformPageClient() const = 0;
    virtual void scrollbarsModeDidChange() const = 0;
    virtual void setCursor(const Cursor&) = 0;
    virtual void setCursorHiddenUntilMouseMoves(bool) = 0;
#if ENABLE(REQUEST_ANIMATION_FRAME) && !USE(REQUEST_ANIMATION_FRAME_TIMER)
    virtual void scheduleAnimation() = 0;
#endif
    // End methods used by HostWindow.

    virtual void dispatchViewportPropertiesDidChange(const ViewportArguments&) const { }

    virtual void contentsSizeChanged(Frame*, const IntSize&) const = 0;
    virtual void layoutUpdated(Frame*) const { }
    virtual void scrollRectIntoView(const IntRect&) const { }; // Currently only Mac has a non empty implementation.

    virtual void mouseDidMoveOverElement(const HitTestResult&, unsigned modifierFlags) = 0;

    virtual void setToolTip(const String&, TextDirection) = 0;

    virtual void print(Frame*) = 0;
    virtual bool shouldRubberBandInDirection(ScrollDirection) const = 0;

    virtual Color underlayColor() const { return Color(); }

#if ENABLE(DRAGGABLE_REGION)
    virtual void annotatedRegionsChanged();
#endif

    virtual void populateVisitedLinks();

    virtual FloatRect customHighlightRect(Node*, const AtomicString& type, const FloatRect& lineRect);
    virtual void paintCustomHighlight(Node*, const AtomicString& type, const FloatRect& boxRect, const FloatRect& lineRect, bool behindText, bool entireLine);
            
    virtual bool shouldReplaceWithGeneratedFileForUpload(const String& path, String& generatedFilename);
    virtual String generateReplacementFile(const String& path);

    virtual bool paintCustomOverhangArea(GraphicsContext*, const IntRect&, const IntRect&, const IntRect&);

#if ENABLE(INPUT_TYPE_COLOR)
    virtual PassOwnPtr<ColorChooser> createColorChooser(ColorChooserClient*, const Color&) = 0;
#endif

#if ENABLE(DATE_AND_TIME_INPUT_TYPES)
    virtual PassRefPtr<DateTimeChooser> openDateTimeChooser(DateTimeChooserClient*, const DateTimeChooserParameters&) = 0;
#endif

    virtual void runOpenPanel(Frame*, PassRefPtr<FileChooser>) = 0;

#if ENABLE(DIRECTORY_UPLOAD)
    // Asychronous request to enumerate all files in a directory chosen by the user.
    virtual void enumerateChosenDirectory(FileChooser*) = 0;
#endif

    // Notification that the given form element has changed. This function
    // will be called frequently, so handling should be very fast.
    virtual void formStateDidChange(const Node*) = 0;
        
    virtual void elementDidFocus(const Node*) { };
    virtual void elementDidBlur(const Node*) { };
    
    virtual bool shouldPaintEntireContents() const { return false; }

#if USE(ACCELERATED_COMPOSITING)
    // Allows ports to customize the type of graphics layers created by this page.
    virtual GraphicsLayerFactory* graphicsLayerFactory() const { return 0; }

    // Pass 0 as the GraphicsLayer to detatch the root layer.
    virtual void attachRootGraphicsLayer(Frame*, GraphicsLayer*) = 0;
    // Sets a flag to specify that the next time content is drawn to the window,
    // the changes appear on the screen in synchrony with updates to GraphicsLayers.
    virtual void setNeedsOneShotDrawingSynchronization() = 0;
    // Sets a flag to specify that the view needs to be updated, so we need
    // to do an eager layout before the drawing.
    virtual void scheduleCompositingLayerFlush() = 0;
    // Returns whether or not the client can render the composited layer,
    // regardless of the settings.
    virtual bool allowsAcceleratedCompositing() const { return true; }

    enum CompositingTrigger {
        ThreeDTransformTrigger = 1 << 0,
        VideoTrigger = 1 << 1,
        PluginTrigger = 1 << 2,
        CanvasTrigger = 1 << 3,
        AnimationTrigger = 1 << 4,
        FilterTrigger = 1 << 5,
        ScrollableInnerFrameTrigger = 1 << 6,
        AnimatedOpacityTrigger = 1 << 7,
        AllTriggers = 0xFFFFFFFF
    };
    typedef unsigned CompositingTriggerFlags;

    // Returns a bitfield indicating conditions that can trigger the compositor.
    virtual CompositingTriggerFlags allowedCompositingTriggers() const { return static_cast<CompositingTriggerFlags>(AllTriggers); }
    
    // Returns true if layer tree updates are disabled.
    virtual bool layerTreeStateIsFrozen() const { return false; }
#endif

#if PLATFORM(WIN) && USE(AVFOUNDATION)
    virtual GraphicsDeviceAdapter* graphicsDeviceAdapter() const { return 0; }
#endif

    virtual bool supportsFullscreenForNode(const Node*) { return false; }
    virtual void enterFullscreenForNode(Node*) { }
    virtual void exitFullscreenForNode(Node*) { }
    virtual bool requiresFullscreenForVideoPlayback() { return false; } 

#if ENABLE(FULLSCREEN_API)
    virtual bool supportsFullScreenForElement(const Element*, bool) { return false; }
    virtual void enterFullScreenForElement(Element*) { }
    virtual void exitFullScreenForElement(Element*) { }
    virtual void fullScreenRendererChanged(RenderBox*) { }
    virtual void setRootFullScreenLayer(GraphicsLayer*) { }
#endif

#if USE(TILED_BACKING_STORE)
    virtual IntRect visibleRectForTiledBackingStore() const { return IntRect(); }
#endif

#if PLATFORM(MAC)
    virtual NSResponder *firstResponder() { return 0; }
    virtual void makeFirstResponder(NSResponder *) { }
    // Focuses on the containing view associated with this page.
    virtual void makeFirstResponder() { }
#endif

    virtual void enableSuddenTermination() { }
    virtual void disableSuddenTermination() { }

#if PLATFORM(WIN)
    virtual void setLastSetCursorToCurrentCursor() = 0;
    virtual void AXStartFrameLoad() = 0;
    virtual void AXFinishFrameLoad() = 0;
#endif

#if ENABLE(TOUCH_EVENTS)
    virtual void needTouchEvents(bool) = 0;
#endif

    virtual bool selectItemWritingDirectionIsNatural() = 0;
    virtual bool selectItemAlignmentFollowsMenuWritingDirection() = 0;
    // Checks if there is an opened popup, called by RenderMenuList::showPopup().
    virtual bool hasOpenedPopup() const = 0;
    virtual PassRefPtr<PopupMenu> createPopupMenu(PopupMenuClient*) const = 0;
    virtual PassRefPtr<SearchPopupMenu> createSearchPopupMenu(PopupMenuClient*) const = 0;

    virtual void postAccessibilityNotification(AccessibilityObject*, AXObjectCache::AXNotification) { }

    virtual void notifyScrollerThumbIsVisibleInRect(const IntRect&) { }
    virtual void recommendedScrollbarStyleDidChange(int /*newStyle*/) { }

    enum DialogType {
        AlertDialog = 0,
        ConfirmDialog = 1,
        PromptDialog = 2,
        HTMLDialog = 3
    };
    virtual bool shouldRunModalDialogDuringPageDismissal(const DialogType&, const String& dialogMessage, FrameLoader::PageDismissalType) const { UNUSED_PARAM(dialogMessage); return true; }

    virtual void numWheelEventHandlersChanged(unsigned) = 0;
        
    virtual bool isSVGImageChromeClient() const { return false; }

#if ENABLE(POINTER_LOCK)
    virtual bool requestPointerLock() { return false; }
    virtual void requestPointerUnlock() { }
    virtual bool isPointerLocked() { return false; }
#endif

    virtual void logDiagnosticMessage(const String& message, const String& description, const String& status) { UNUSED_PARAM(message); UNUSED_PARAM(description); UNUSED_PARAM(status); }

    virtual FloatSize minimumWindowSize() const { return FloatSize(100, 100); };

    virtual bool isEmptyChromeClient() const { return false; }

    virtual String plugInStartLabelTitle(const String& mimeType) const { UNUSED_PARAM(mimeType); return String(); }
    virtual String plugInStartLabelSubtitle(const String& mimeType) const { UNUSED_PARAM(mimeType); return String(); }
    virtual String plugInExtraStyleSheet() const { return String(); }
    virtual String plugInExtraScript() const { return String(); }

    // FIXME: Port should return true using heuristic based on scrollable(RenderBox).
    virtual bool shouldAutoscrollForDragAndDrop(RenderBox*) const { return false; }

    virtual void didAssociateFormControls(const Vector<RefPtr<Element> >&) { };
    virtual bool shouldNotifyOnFormChanges() { return false; };

    virtual void didAddHeaderLayer(GraphicsLayer*) { }
    virtual void didAddFooterLayer(GraphicsLayer*) { }

    // These methods are used to report pages that are performing
    // some task that we consider to be "active", and so the user
    // would likely want the page to remain running uninterrupted.
    virtual void incrementActivePageCount() { }
    virtual void decrementActivePageCount() { }

protected:
    virtual ~ChromeClient() { }
};

}
#endif // ChromeClient_h
