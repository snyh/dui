#include "config.h"
#include "page/Page.h"
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
    Page* page = new Page(clients);
}
