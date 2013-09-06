#include <gtk/gtk.h>
#include "dui.h"

extern GdkWindow* window = NULL;
gboolean draw_view(GtkWidget* w, cairo_t* cr)
{
    take_snapshot(cr);
    return true;
}

gboolean update(gpointer)
{
    gdk_window_invalidate_rect(window, NULL, false);
    return true;
}

int main()
{
    gtk_init(NULL, NULL);
    test_main("/dev/shm/dui/Source/dui/test.htm");
    GtkWidget* w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(w, 300, 300);
    gtk_widget_realize(w);
    window = gtk_widget_get_window(w);
    g_timeout_add(500, update, NULL);
    g_signal_connect(w, "destroy", gtk_main_quit, NULL);
    g_signal_connect(w, "draw", G_CALLBACK(draw_view), NULL);
    gtk_widget_show(w);
    gtk_main();
}
