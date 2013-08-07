# Configuration flags that are used throughout WebKitGTK+.
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext catalog name])

if test "$enable_debug" = "yes"; then
    AC_DEFINE([GDK_PIXBUF_DISABLE_DEPRECATED], [1], [ ])
    AC_DEFINE([GDK_DISABLE_DEPRECATED], [1], [ ])
    AC_DEFINE([GTK_DISABLE_DEPRECATED], [1], [ ])
    AC_DEFINE([PANGO_DISABLE_DEPRECATED], [1], [ ])
else
    AC_DEFINE([NDEBUG], [1], [Define to disable debugging])
fi

if test "$os_win32" = "no"; then
AC_CHECK_HEADERS([pthread.h],
    AC_DEFINE([HAVE_PTHREAD_H],[1],[Define if pthread exists]),
    AC_MSG_ERROR([pthread support is required to build WebKit]))
AC_CHECK_LIB(pthread, pthread_rwlock_init,
    AC_DEFINE([HAVE_PTHREAD_RWLOCK],[1],[Define if pthread rwlock is present]),
    AC_MSG_WARN([pthread rwlock support is not available]))
fi

if test "$GTK_API_VERSION" = "2.0"; then
    AC_DEFINE([GTK_API_VERSION_2], [1], [ ])
fi

if test "$enable_fast_malloc" = "no"; then
    AC_DEFINE([WTF_SYSTEM_MALLOC], [1], [ ])
fi

if test "$enable_opcode_stats" = "yes"; then
    AC_DEFINE([ENABLE_OPCODE_STATS], [1], [Define to enable Opcode statistics])
fi

if test "$enable_video" = "yes" || test "$enable_web_audio" = "yes"; then
    AC_DEFINE([WTF_USE_GSTREAMER], [1], [ ])
    if test "$enable_debug" = "yes"; then
        AC_DEFINE([GST_DISABLE_DEPRECATED], [1], [ ])
    fi
fi

if test "$enable_web_audio" = "yes"; then
    AC_DEFINE([WTF_USE_WEBAUDIO_GSTREAMER], [1], [1])
fi

if test "$enable_accelerated_compositing" = "yes"; then
    AC_DEFINE([WTF_USE_ACCELERATED_COMPOSITING], [1], [ ])
    AC_DEFINE([WTF_USE_TEXTURE_MAPPER], [1], [ ])
    AC_DEFINE([WTF_USE_TEXTURE_MAPPER_GL], [1], [ ])
fi

if test "$found_opengl" = "yes"; then
    AC_DEFINE([WTF_USE_OPENGL], [1], [ ])
fi

if test "$enable_glx" = "yes"; then
    AC_DEFINE([WTF_USE_GLX], [1], [ ])
fi

if test "$enable_egl" = "yes"; then
    AC_DEFINE([WTF_USE_EGL], [1], [ ])
fi

if test "$enable_gles2" = "yes"; then
    AC_DEFINE([WTF_USE_OPENGL_ES_2], [1], [ ])
fi

AC_DEFINE([WTF_USE_3D_GRAPHICS], [1], [ ])
AC_DEFINE([WTF_USE_CAIRO], [1], [ ])
AC_DEFINE([WTF_PLATFORM_GTK], [1], [Define if target is GTK])
AC_DEFINE([WTF_PLATFORM_X11], [1], [Define if target is X11])
