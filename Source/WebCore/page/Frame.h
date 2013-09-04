/*
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999-2001 Lars Knoll <knoll@kde.org>
 *                     1999-2001 Antti Koivisto <koivisto@kde.org>
 *                     2000-2001 Simon Hausmann <hausmann@kde.org>
 *                     2000-2001 Dirk Mueller <mueller@kde.org>
 *                     2000 Stefan Schimanski <1Stein@gmx.de>
 * Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009, 2010 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
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

#ifndef Frame_h
#define Frame_h

#include "page/AdjustViewSizeOrNot.h"
#include "platform/DragImage.h"
#include "loader/FrameLoader.h"
#include "page/FrameTree.h"
#include "platform/graphics/IntRect.h"
#include "platform/ScrollTypes.h"
#include "page/UserScriptTypes.h"
#include <wtf/RefCounted.h>

#if PLATFORM(WIN)
#include "FrameWin.h"
#endif

#if USE(TILED_BACKING_STORE)
#include "TiledBackingStoreClient.h"
#endif

#if PLATFORM(WIN)
typedef struct HBITMAP__* HBITMAP;
#endif

namespace WebCore {

    class AnimationController;
    class Color;
    class Document;
    class Editor;
    class Element;
    class EventHandler;
    class FrameDestructionObserver;
    class FrameSelection;
    class FrameView;
    class HTMLTableCellElement;
    class IntRect;
    class Node;
    class RegularExpression;
    class RenderPart;
    class RenderView;
    class Settings;
    class TiledBackingStore;
    class TreeScope;
    class VisiblePosition;

#if !USE(TILED_BACKING_STORE)
    class TiledBackingStoreClient { };
#endif


    enum {
        LayerTreeFlagsIncludeDebugInfo = 1 << 0,
        LayerTreeFlagsIncludeVisibleRects = 1 << 1,
        LayerTreeFlagsIncludeTileCaches = 1 << 2,
        LayerTreeFlagsIncludeRepaintRects = 1 << 3,
        LayerTreeFlagsIncludePaintingPhases = 1 << 4
    };
    typedef unsigned LayerTreeFlags;

    class Frame : public RefCounted<Frame>, public TiledBackingStoreClient {
    public:
        static PassRefPtr<Frame> create(Page*, HTMLFrameOwnerElement*, FrameLoaderClient*);

        void init();
        void setView(PassRefPtr<FrameView>);
        void createView(const IntSize&, const Color&, bool,
            const IntSize& fixedLayoutSize = IntSize(), const IntRect& fixedVisibleContentRect = IntRect(),
            bool useFixedLayout = false, ScrollbarMode = ScrollbarAuto, bool horizontalLock = false,
            ScrollbarMode = ScrollbarAuto, bool verticalLock = false);

        ~Frame();

        void addDestructionObserver(FrameDestructionObserver*);
        void removeDestructionObserver(FrameDestructionObserver*);

        void willDetachPage();
        void detachFromPage();
        void disconnectOwnerElement();

        Page* page() const;
        HTMLFrameOwnerElement* ownerElement() const;

        Document* document() const;
        FrameView* view() const;

        Editor& editor() const;
        EventHandler* eventHandler() const;
        FrameLoader* loader() const;
        FrameSelection* selection() const;
        FrameTree* tree() const;
        AnimationController* animation() const;
        
        RenderView* contentRenderer() const; // Root of the render tree for the document contained in this frame.
        RenderPart* ownerRenderer() const; // Renderer for the element that contains this frame.

    // ======== All public functions below this point are candidates to move out of Frame into another class. ========

        bool inScope(TreeScope*) const;

        String layerTreeAsText(LayerTreeFlags = 0) const;
        String trackedRepaintRectsAsText() const;

        static Frame* frameForWidget(const Widget*);

        Settings* settings() const; // can be NULL

        void setPrinting(bool printing, const FloatSize& pageSize, const FloatSize& originalPageSize, float maximumShrinkRatio, AdjustViewSizeOrNot);
        bool shouldUsePrintingLayout() const;
        FloatSize resizePageRectsKeepingRatio(const FloatSize& originalSize, const FloatSize& expectedSize);

        void setDocument(PassRefPtr<Document>);

        void setPageZoomFactor(float factor);
        float pageZoomFactor() const { return m_pageZoomFactor; }
        void setTextZoomFactor(float factor);
        float textZoomFactor() const { return m_textZoomFactor; }
        void setPageAndTextZoomFactors(float pageZoomFactor, float textZoomFactor);

        // Scale factor of this frame with respect to the container.
        float frameScaleFactor() const;

#if USE(ACCELERATED_COMPOSITING)
        void deviceOrPageScaleFactorChanged();
#endif

        void clearTimers();
        static void clearTimers(FrameView*, Document*);

        String documentTypeString() const;

        String displayStringModifiedByEncoding(const String&) const;

        DragImageRef nodeImage(Node*);
        DragImageRef dragImageForSelection();

        VisiblePosition visiblePositionForPoint(const IntPoint& framePoint);
        Document* documentAtPoint(const IntPoint& windowPoint);
        PassRefPtr<Range> rangeForPoint(const IntPoint& framePoint);

        String searchForLabelsAboveCell(RegularExpression*, HTMLTableCellElement*, size_t* resultDistanceFromStartOfCell);
        String searchForLabelsBeforeElement(const Vector<String>& labels, Element*, size_t* resultDistance, bool* resultIsInCellAbove);
        String matchLabelsAgainstElement(const Vector<String>& labels, Element*);

        void suspendActiveDOMObjectsAndAnimations();
        void resumeActiveDOMObjectsAndAnimations();
        bool activeDOMObjectsAndAnimationsSuspended() const { return m_activeDOMObjectsAndAnimationsSuspendedCount > 0; }

        // Should only be called on the main frame of a page.
        void notifyChromeClientWheelEventHandlerCountChanged() const;

        bool isURLAllowed(const KURL&) const;

    // ========

    private:
        Frame(Page*, HTMLFrameOwnerElement*, FrameLoaderClient*);

        HashSet<FrameDestructionObserver*> m_destructionObservers;

        Page* m_page;
        mutable FrameTree m_treeNode;
        mutable FrameLoader m_loader;

        HTMLFrameOwnerElement* m_ownerElement;
        RefPtr<FrameView> m_view;
        RefPtr<Document> m_doc;

        const OwnPtr<Editor> m_editor;
        OwnPtr<FrameSelection> m_selection;
        OwnPtr<EventHandler> m_eventHandler;
        OwnPtr<AnimationController> m_animationController;

        float m_pageZoomFactor;
        float m_textZoomFactor;

#if USE(TILED_BACKING_STORE)
    // FIXME: The tiled backing store belongs in FrameView, not Frame.

    public:
        TiledBackingStore* tiledBackingStore() const { return m_tiledBackingStore.get(); }
        void setTiledBackingStoreEnabled(bool);

    private:
        // TiledBackingStoreClient interface
        virtual void tiledBackingStorePaintBegin();
        virtual void tiledBackingStorePaint(GraphicsContext*, const IntRect&);
        virtual void tiledBackingStorePaintEnd(const Vector<IntRect>& paintedArea);
        virtual IntRect tiledBackingStoreContentsRect();
        virtual IntRect tiledBackingStoreVisibleRect();
        virtual Color tiledBackingStoreBackgroundColor() const;

        OwnPtr<TiledBackingStore> m_tiledBackingStore;
#endif

        int m_activeDOMObjectsAndAnimationsSuspendedCount;
    };

    inline void Frame::init()
    {
        m_loader.init();
    }

    inline FrameLoader* Frame::loader() const
    {
        return &m_loader;
    }

    inline FrameView* Frame::view() const
    {
        return m_view.get();
    }

    inline Document* Frame::document() const
    {
        return m_doc.get();
    }

    inline FrameSelection* Frame::selection() const
    {
        return m_selection.get();
    }

    inline Editor& Frame::editor() const
    {
        return *m_editor;
    }

    inline AnimationController* Frame::animation() const
    {
        return m_animationController.get();
    }

    inline HTMLFrameOwnerElement* Frame::ownerElement() const
    {
        return m_ownerElement;
    }

    inline FrameTree* Frame::tree() const
    {
        return &m_treeNode;
    }

    inline Page* Frame::page() const
    {
        return m_page;
    }

    inline void Frame::detachFromPage()
    {
        m_page = 0;
    }

    inline EventHandler* Frame::eventHandler() const
    {
        return m_eventHandler.get();
    }

} // namespace WebCore

#endif // Frame_h
