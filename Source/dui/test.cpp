#include "config.h"
#include "Page.h"

using namespace WebCore;

int main()
{
    Page::PageClients clients;
    Page* page = new Page(clients);
}
