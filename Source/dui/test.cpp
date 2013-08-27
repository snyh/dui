#include <gtk/gtk.h>
void test_main(const char*);
void take_snapshot(cairo_t* cr);


gboolean draw_view(GtkWidget* w, cairo_t* cr)
{
    take_snapshot(cr);
    return true;
}

int main()
{
    gtk_init(NULL, NULL);
    test_main("/home/snyh/lib/dui/Source/dui/test.htm");
    GtkWidget* w = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(w, "destroy", gtk_main_quit, NULL);
    g_signal_connect(w, "draw", G_CALLBACK(draw_view), NULL);
    gtk_widget_show(w);
    gtk_main();
}
