#include <gtk/gtk.h>

extern "C" {
void test_main(const char*);
void take_snapshot(cairo_t* cr);
}
gboolean draw_view(GtkWidget* w, cairo_t* cr)
{
    take_snapshot(cr);
    return true;
}

int main()
{
    gtk_init(NULL, NULL);
    test_main("/dev/shm/dui/Source/dui/test.htm");
    GtkWidget* w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(w, "destroy", gtk_main_quit, NULL);
    g_signal_connect(w, "draw", G_CALLBACK(draw_view), NULL);
    gtk_widget_show(w);
    gtk_main();
}
