#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cairo.h>
 
void take_snapshot(cairo_t* cr);
void test_main(const char* url);

#define WIDTH 300
#define HEIGHT 300

int main(void) {
   Display *d;
   Window w;
   XEvent e;
   int s;

   d = XOpenDisplay(NULL);
   if (d == NULL) {
      fprintf(stderr, "Cannot open display\n");
      exit(1);
   }

   s = DefaultScreen(d);
   w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, WIDTH, HEIGHT, 1,
                           BlackPixel(d, s), WhitePixel(d, s));
   XSelectInput(d, w, ExposureMask | KeyPressMask);
   XMapWindow(d, w);

   cairo_surface_t* surface = cairo_xlib_surface_create(d, w, DefaultVisual(d, 0), WIDTH, HEIGHT);
   cairo_t *cr = cairo_create(surface);
   test_main("/dev/shm/dui/Source/dui/test.htm");
   while (1) {
      XNextEvent(d, &e);
      if (e.type == Expose) {
         take_snapshot(cr);
      }
      /*if (e.type == KeyPress)*/
         /*break;*/
   }
   cairo_surface_destroy(surface);
   cairo_destroy(cr);

   XCloseDisplay(d);
   return 0;
}
