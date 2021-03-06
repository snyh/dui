/*
 *  Copyright (C) 2010 Igalia S.L.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "platform/gtk/GOwnPtrGtk.h"

#include <gtk/gtk.h>

namespace WTF {

template <> void freeOwnedGPtr<GdkEvent>(GdkEvent* ptr)
{
    if (ptr)
        gdk_event_free(ptr);
}

template <> void freeOwnedGPtr<GtkIconInfo>(GtkIconInfo* info)
{
    if (info)
        gtk_icon_info_free(info);
}

}
