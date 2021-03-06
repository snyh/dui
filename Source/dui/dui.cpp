#include "config.h"

#include "page/Page.h"
#include "page/Frame.h"
#include "page/FrameView.h"
#include "html/HTMLDocument.h"
#include "html/HTMLElement.h"
#include "dom/Document.h"


#include "loader/EmptyClients.h"
#include "page/EventHandler.h"

#include <wtf/dtoa/cached-powers.h>
#include <wtf/MainThread.h>

#include <glib.h>
#include <gtk/gtk.h>

#include "api/dui.h"

using namespace WebCore;

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
void d_main_quit()
{
    gtk_main_quit();
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

bool draw_frame(GtkWidget* widget, cairo_t* cr, DFrame* dframe)
{
    Frame* frame = (Frame*)dframe->core;
    frame->view()->updateLayoutAndStyleIfNeededRecursive();
    GdkWindow* w = gtk_widget_get_window(widget);
    int width = gdk_window_get_width(w);
    int height = gdk_window_get_height(w);
    GraphicsContext gc(cr);
    gc.clip(IntRect(0, 0, width, height));
    frame->view()->paint(&gc, IntRect(0, 0, width, height));
    return true;
}

gboolean let_we_draw(gpointer w)
{
    gdk_window_invalidate_rect(gtk_widget_get_window(GTK_WIDGET(w)), NULL, false);
    return true;
}


class DuiChromeClient : public EmptyChromeClient {
    WTF_MAKE_FAST_ALLOCATED;
    public:
    virtual void invalidateRootView(const IntRect& rect, bool immediate) OVERRIDE {
        invalidateContentsAndRootView(rect, immediate);
    }
    virtual void invalidateContentsAndRootView(const IntRect& rect, bool) OVERRIDE {
        //if (window) {
            //GdkRectangle _rect = rect;
            //gdk_window_invalidate_rect(window, &_rect, false);
        //}
    }
};


DFrame* d_frame_new(int width, int height)
{
    DFrame* dframe = g_new0(DFrame, 1);
    GtkWidget* win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_events(win, GDK_ALL_EVENTS_MASK);
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

void d_frame_add(DFrame* dframe, DElement* ele)
{
    Frame* frame = (Frame*)dframe->core;
    HTMLElement* body = frame->document()->body();
    if (body == 0) {
        frame->document()->setContent("");
        body = frame->document()->body();
    }
    //TODO: check ele's parent and refcount
    body->appendChild(adoptRef((Element*)ele));
}

DElement* d_frame_get_element(DFrame* dframe, const char* id)
{
    Frame* frame = (Frame*)dframe->core;
    return frame->document()->getElementById(id);
}
