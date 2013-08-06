#include "config.h"
#include "page/Page.h"

using namespace WebCore;



int test()
{
    Page::PageClients clients;
    Page* page = new Page(clients);
}
