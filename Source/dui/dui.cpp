#include "config.h"
#include "page/Page.h"
#include "page/Frame.h"
#include "dom/Document.h"
#include "page/DOMWindow.h"
#include <wtf/MainThread.h>
#include <wtf/dtoa/cached-powers.h>
#include "loader/EmptyClients.h"

using namespace WebCore;

int test()
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

    RefPtr<Frame> frame = Frame::create(page, 0, 0);
    printf("document: %p\n", frame->document());
    //frame->init();
    KURL url;
    RefPtr<Document> document = Document::create(frame.get(), url);
    document->createDOMWindow();
    frame->setDocument(document);
    printf("document: %p\n", frame->document());

    DOMWindow* window = document->defaultView();
    printf("window: %p\n", window);
    printf("allow popup: %d\n", window->allowPopUp());

}
