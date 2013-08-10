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
#include "rendering/style/RenderStyle.h"
#include "rendering/style/StyleInheritedData.h"
#include <wtf/MainThread.h>
#include <wtf/dtoa/cached-powers.h>
#include <wtf/text/CString.h>
#include <glib.h>
#include "loader/EmptyClients.h"

using namespace WebCore;
void take_snapshot(RefPtr<Frame> frame)
{
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 300, 300);
    RefPtr<cairo_t> cr = adoptRef(cairo_create(surface));
    GraphicsContext gc(cr.get());
    gc.clip(IntRect(0, 0, 300, 300));
    frame->view()->paint(&gc, IntRect(0, 0, 300, 300));
    printf("1render: %p\n", frame->document()->renderView());
    cairo_surface_write_to_png(surface, "/dev/shm/t.png");
}

void load_content(RefPtr<Document> document, const char* path)
{
    char* content = NULL;
    g_file_get_contents(path, &content, NULL, NULL);
    document->setContent(content);
    g_free(content);
}

const char my_interp[] __attribute__((section(".interp"))) = "/lib/ld-linux.so.2";
void test_main(int argc, char* argv[])
{
    WTF::double_conversion::initialize();
    WTF::initializeThreading();
    WTF::initializeMainThread();

    Page::PageClients clients;
    clients.editorClient = new EmptyEditorClient();
    clients.chromeClient = new EmptyChromeClient();
    clients.dragClient = new EmptyDragClient();
    clients.contextMenuClient = new EmptyContextMenuClient();
    Page* page = new Page(clients);
    //page->setDeviceScaleFactor(1000000);

    RefPtr<Frame> frame = Frame::create(page, 0, new EmptyFrameLoaderClient);
    frame->loader()->init();
    frame->createView(IntSize(300, 300), WebCore::Color::white, false);

    KURL url;
    RefPtr<Document> document = HTMLDocument::create(frame.get(), url);
    document->createDOMWindow();
    frame->setDocument(document);
    load_content(document, "/dev/shm/t.htm");
    document->updateLayout();
    document->dumpStatistics();
    Element* _snyh = document->getElementById("snyh");
    HTMLStyleElement* style = toHTMLStyleElement(document->getElementById("s"));
    printf("scope: %p\n", style->scopingElement());


    DOMWindow* window = document->defaultView();
    take_snapshot(frame);
}
