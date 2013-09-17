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
#include "page/EventHandler.h"

#include "loader/cache/MemoryCache.h"
#include "dui.h"

GdkWindow* window = NULL;

#define WIDTH 300
#define HEIGHT 300

using namespace WebCore;

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

void d_init()
{
    WTF::double_conversion::initialize();
    WTF::initializeThreading();
    WTF::initializeMainThread();

    gtk_init(NULL, NULL);
}
void d_main()
{
    gtk_main();
}

void d_frame_load_content(DFrame* frame, const char* content)
{
    ((Frame*)(frame->core))->document()->setContent(content);
}

bool translate_event(GtkWidget* w, GdkEvent* event, DFrame* dframe)
{
    EventHandler* handler = ((Frame*)(dframe->core))->eventHandler();
    switch(event->type) {
        case GDK_KEY_PRESS:
        case GDK_KEY_RELEASE:
            return false;
        case GDK_MOTION_NOTIFY:
            handler->mouseMoved(PlatformMouseEvent((GdkEventMotion*)event));
            return true;
        case GDK_BUTTON_PRESS:
            {
                PlatformMouseEvent platformEvent((GdkEventButton*)event);
                return handler->handleMousePressEvent(platformEvent);
            }
        case GDK_BUTTON_RELEASE:
            {
                PlatformMouseEvent platformEvent((GdkEventButton*)event);
                handler->handleMouseReleaseEvent(platformEvent);
                return false;
            }
        case GDK_DELETE:
            gtk_main_quit();
            return true;
    }
    return false;
}

void draw_frame(GtkWidget* widget, cairo_t* cr, DFrame* dframe)
{
    GraphicsContext gc(cr);
    gc.clip(IntRect(0, 0, WIDTH, HEIGHT));
    ((Frame*)(dframe->core))->view()->paint(&gc, IntRect(0, 0, WIDTH, HEIGHT));
}

gboolean let_we_draw(gpointer w)
{
    gdk_window_invalidate_rect(gtk_widget_get_window(GTK_WIDGET(w)), NULL, false);
    return true;
}

DFrame* d_frame_new(int width, int height)
{
    DFrame* dframe = g_new0(DFrame, 1);
    GtkWidget* win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(win, width, height);
    gtk_widget_realize(win);
    g_signal_connect(win, "event", G_CALLBACK(translate_event), dframe);
    g_signal_connect(win, "draw", G_CALLBACK(draw_frame), dframe);
    g_timeout_add(50, let_we_draw, (gpointer)win);

    Page::PageClients clients;
    clients.editorClient = new EmptyEditorClient();
    clients.chromeClient = new DuiChromeClient();
    clients.dragClient = new EmptyDragClient();
    Page* page = new Page(clients);
    RefPtr<Frame> frame = Frame::create(page, 0, new EmptyFrameLoaderClient);
    frame->createView(IntSize(width, height), WebCore::Color::white, false);
    KURL _url;
    RefPtr<Document> document = HTMLDocument::create(frame.get(), _url);
    document->createDOMWindow();
    frame->setDocument(document);

    gtk_widget_show(win);
    dframe->core = frame.get();
    dframe->native = win;
    return dframe;
}
