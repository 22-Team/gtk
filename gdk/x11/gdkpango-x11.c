/* GDK - The GIMP Drawing Kit
 * Copyright (C) 2000 Red Hat, Inc. 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <stdlib.h>

#include "gdkprivate-x11.h"
#include "gdkpango.h"
#include <pango/pangox.h>
#include "gdkscreen-x11.h"
#ifdef HAVE_XFT
#include <pango/pangoxft.h>
#endif

/**
 * gdk_pango_context_get_for_screen:
 * @screen: the #GdkScreen where the context is to be created.
 * 
 * Creates a #PangoContext for @display.
 *
 * The context must be freed when you're finished with it.
 * 
 * When using GTK+, normally you should use gtk_widget_get_pango_context()
 * instead of this function, to get the appropriate context for
 * the widget you intend to render text onto.
 * 
 * Return value: a new #PangoContext for @display
 **/
PangoContext *
gdk_pango_context_get_for_screen (GdkScreen *screen)
{
  PangoContext *context;
  g_return_val_if_fail (GDK_IS_SCREEN (screen), NULL);
  
#ifdef HAVE_XFT
  static gint use_xft = -1;
  if (use_xft == -1)
    {
      const char *val = g_getenv ("GDK_USE_XFT");

      use_xft = val && (atoi (val) != 0);
    }
  
  if (use_xft)
    context = pango_xft_get_context (GDK_SCREEN_XDISPLAY (screen),
				     GDK_SCREEN_IMPL_X11 (screen)->screen_num);
  else
#endif /* HAVE_XFT */
    context = pango_x_get_context (GDK_SCREEN_XDISPLAY (screen));
  
  g_object_set_data (G_OBJECT (context), "gdk-pango-screen", screen);
  
  return context;
}
#ifndef GDK_MULTIHEAD_SAFE
/**
 * gdk_pango_context_get:
 * 
 * Creates a #PangoContext for the default GDK display.
 *
 * The context must be freed when you're finished with it.
 * 
 * When using GTK+, normally you should use gtk_widget_get_pango_context()
 * instead of this function, to get the appropriate context for
 * the widget you intend to render text onto.
 * 
 * Return value: a new #PangoContext for the default display
 **/
PangoContext *
gdk_pango_context_get (void)
{
  return gdk_pango_context_get_for_screen (gdk_get_default_screen ());
}
#endif
