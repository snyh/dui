#include "config.h"
#include "page/Page.h"
#include "page/Frame.h"
#include "page/FrameView.h"
#include "page/DOMWindow.h"
#include "html/HTMLDocument.h"
#include "html/HTMLStyleElement.h"
#include "html/HTMLElement.h"
#include "dom/Document.h"
#include "dom/DocumentParser.h"
#include "dom/Element.h"
#include "rendering/RenderView.h"
#include "rendering/style/RenderStyle.h"
#include "rendering/style/StyleInheritedData.h"
#include <wtf/MainThread.h>
#include <wtf/dtoa/cached-powers.h>
#include <wtf/text/CString.h>
#include <glib.h>
#include "loader/EmptyClients.h"
#include <gtk/gtk.h>
#include "platform/network/soup/ResourceRequest.h"
#include "loader/SubstituteData.h"
#include "loader/DocumentLoader.h"
#include "loader/FrameLoadRequest.h"
#include "css/StyleResolver.h"

#include "loader/cache/MemoryCache.h"
#include "dui.h"

GdkWindow* window = NULL;

struct _DWindow {
};

#define WIDTH 300
#define HEIGHT 300

using namespace WebCore;
RefPtr<Frame> frame = 0;
void take_snapshot(cairo_t* cr)
{
    GraphicsContext gc(cr);
    gc.clip(IntRect(0, 0, WIDTH, HEIGHT));
    frame->view()->paint(&gc, IntRect(0, 0, WIDTH, HEIGHT));
}

void load_content(RefPtr<Document> document, const char* path)
{
    char* content = NULL;
    g_file_get_contents(path, &content, NULL, NULL);
    document->setContent(content);
    g_free(content);
}

void dumpRuleSet(RuleSet* set)
{
}

class DuiChromeClient : public EmptyChromeClient {
    WTF_MAKE_FAST_ALLOCATED;
    public:
    virtual void invalidateRootView(const IntRect& rect, bool immediate) OVERRIDE {
        invalidateContentsAndRootView(rect, immediate);
    }
    virtual void invalidateContentsAndRootView(const IntRect& rect, bool) OVERRIDE {
        if (window) {
            GdkRectangle _rect = rect;
            gdk_window_invalidate_rect(window, &_rect, false);
        }
    }
};

const char my_interp[] __attribute__((section(".interp"))) = "/lib/ld-linux.so.2";
void test_main(const char* url)
{
    WTF::double_conversion::initialize();
    WTF::initializeThreading();
    WTF::initializeMainThread();

    Page::PageClients clients;
    clients.editorClient = new EmptyEditorClient();
    clients.chromeClient = new DuiChromeClient();
    clients.dragClient = new EmptyDragClient();
    Page* page = new Page(clients);

    frame = Frame::create(page, 0, new EmptyFrameLoaderClient);
    //frame->loader()->init();

    frame->createView(IntSize(WIDTH, HEIGHT), WebCore::Color::white, false);

    KURL _url;
    RefPtr<Document> document = HTMLDocument::create(frame.get(), _url);
    document->createDOMWindow();
    frame->setDocument(document);
    load_content(document, url);

    document->updateLayout();
    //document->dumpStatistics();
    Element* snyh = document->getElementById("snyh");
    dumpRuleSet(document->ensureStyleResolver()->ruleSets().authorStyle());
    dumpRuleSet(document->ensureStyleResolver()->ruleSets().userStyle());

    //memoryCache()->dumpStats();

    DOMWindow* window = document->defaultView();
}
