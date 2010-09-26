/* GDK - The GIMP Drawing Kit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 * Copyright (C) 1998-2004 Tor Lillqvist
 * Copyright (C) 2001-2005 Hans Breuer
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#include "config.h"
#include <math.h>
#include <stdio.h>
#include <glib.h>

#include <pango/pangowin32.h>
#include <cairo-win32.h>

#include "gdkscreen.h" /* gdk_screen_get_default() */
#include "gdkprivate-win32.h"

#define ROP3_D 0x00AA0029
#define ROP3_DSna 0x00220326
#define ROP3_DSPDxax 0x00E20746

#define LINE_ATTRIBUTES (GDK_GC_LINE_WIDTH|GDK_GC_LINE_STYLE| \
			 GDK_GC_CAP_STYLE|GDK_GC_JOIN_STYLE)

#define MUST_RENDER_DASHES_MANUALLY(gcwin32)			\
  (gcwin32->line_style == GDK_LINE_DOUBLE_DASH ||		\
   (gcwin32->line_style == GDK_LINE_ON_OFF_DASH && gcwin32->pen_dash_offset))

static cairo_surface_t *gdk_win32_ref_cairo_surface (GdkDrawable *drawable);
static cairo_surface_t *gdk_win32_create_cairo_surface (GdkDrawable *drawable,
                                                        int          width,
                                                        int          height);
     
static GdkColormap* gdk_win32_get_colormap   (GdkDrawable    *drawable);

static void gdk_drawable_impl_win32_finalize   (GObject *object);

static const cairo_user_data_key_t gdk_win32_cairo_key;

G_DEFINE_TYPE (GdkDrawableImplWin32,  _gdk_drawable_impl_win32, GDK_TYPE_DRAWABLE)


static void
_gdk_drawable_impl_win32_class_init (GdkDrawableImplWin32Class *klass)
{
  GdkDrawableClass *drawable_class = GDK_DRAWABLE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gdk_drawable_impl_win32_finalize;

  drawable_class->ref_cairo_surface = gdk_win32_ref_cairo_surface;
  drawable_class->create_cairo_surface = gdk_win32_create_cairo_surface;
  
  drawable_class->get_colormap = gdk_win32_get_colormap;
}

static void
_gdk_drawable_impl_win32_init (GdkDrawableImplWin32 *impl)
{
}

static void
gdk_drawable_impl_win32_finalize (GObject *object)
{
  G_OBJECT_CLASS (_gdk_drawable_impl_win32_parent_class)->finalize (object);
}

/*****************************************************
 * Win32 specific implementations of generic functions *
 *****************************************************/

static GdkColormap*
gdk_win32_get_colormap (GdkDrawable *drawable)
{
  return GDK_DRAWABLE_IMPL_WIN32 (drawable)->colormap;
}

/* Drawing
 */

/**
 * _gdk_win32_drawable_acquire_dc
 * @drawable: a Win32 #GdkDrawable implementation
 * 
 * Gets a DC with the given drawable selected into
 * it.
 *
 * Return value: The DC, on success. Otherwise
 *  %NULL. If this function succeeded
 *  _gdk_win32_drawable_release_dc()  must be called
 *  release the DC when you are done using it.
 **/
HDC 
_gdk_win32_drawable_acquire_dc (GdkDrawable *drawable)
{
  GdkDrawableImplWin32 *impl = GDK_DRAWABLE_IMPL_WIN32 (drawable);
  
  if (GDK_IS_WINDOW_IMPL_WIN32 (drawable) &&
      GDK_WINDOW_DESTROYED (impl->wrapper))
    return NULL;

  if (!impl->hdc)
    {
      if (GDK_IS_PIXMAP_IMPL_WIN32 (impl))
	{
	  impl->hdc = CreateCompatibleDC (NULL);
	  if (!impl->hdc)
	    WIN32_GDI_FAILED ("CreateCompatibleDC");
	  
	  if (impl->hdc)
	    {
	      impl->saved_dc_bitmap = SelectObject (impl->hdc,
						    impl->handle);
	      if (!impl->saved_dc_bitmap)
		{
		  WIN32_GDI_FAILED ("CreateCompatibleDC");
		  DeleteDC (impl->hdc);
		  impl->hdc = NULL;
		}
	    }
	}
      else
	{
	  impl->hdc = GetDC (impl->handle);
	  if (!impl->hdc)
	    WIN32_GDI_FAILED ("GetDC");
	}
    }

  if (impl->hdc)
    {
      impl->hdc_count++;
      return impl->hdc;
    }
  else
    {
      return NULL;
    }
}

/**
 * _gdk_win32_drawable_release_dc
 * @drawable: a Win32 #GdkDrawable implementation
 * 
 * Releases the reference count for the DC
 * from _gdk_win32_drawable_acquire_dc()
 **/
void
_gdk_win32_drawable_release_dc (GdkDrawable *drawable)
{
  GdkDrawableImplWin32 *impl = GDK_DRAWABLE_IMPL_WIN32 (drawable);
  
  g_return_if_fail (impl->hdc_count > 0);

  impl->hdc_count--;
  if (impl->hdc_count == 0)
    {
      if (impl->saved_dc_bitmap)
	{
	  GDI_CALL (SelectObject, (impl->hdc, impl->saved_dc_bitmap));
	  impl->saved_dc_bitmap = NULL;
	}
      
      if (impl->hdc)
	{
	  if (GDK_IS_PIXMAP_IMPL_WIN32 (impl))
	    GDI_CALL (DeleteDC, (impl->hdc));
	  else
	    GDI_CALL (ReleaseDC, (impl->handle, impl->hdc));
	  impl->hdc = NULL;
	}
    }
}

static cairo_surface_t *
gdk_win32_create_cairo_surface (GdkDrawable *drawable,
				gint width,
				gint height)
{
  /* width and height are determined from the DC */
  return gdk_win32_ref_cairo_surface (drawable);
}

static void
gdk_win32_cairo_surface_destroy (void *data)
{
  GdkDrawableImplWin32 *impl = data;

  _gdk_win32_drawable_release_dc (GDK_DRAWABLE (impl));
  impl->cairo_surface = NULL;
}

static cairo_surface_t *
gdk_win32_ref_cairo_surface (GdkDrawable *drawable)
{
  GdkDrawableImplWin32 *impl = GDK_DRAWABLE_IMPL_WIN32 (drawable);

  if (GDK_IS_WINDOW_IMPL_WIN32 (drawable) &&
      GDK_WINDOW_DESTROYED (impl->wrapper))
    return NULL;

  if (!impl->cairo_surface)
    {
      HDC hdc = _gdk_win32_drawable_acquire_dc (drawable);
      if (!hdc)
	return NULL;

      impl->cairo_surface = cairo_win32_surface_create (hdc);

      cairo_surface_set_user_data (impl->cairo_surface, &gdk_win32_cairo_key,
				   drawable, gdk_win32_cairo_surface_destroy);
    }
  else
    cairo_surface_reference (impl->cairo_surface);

  return impl->cairo_surface;
}

gboolean
_gdk_windowing_set_cairo_surface_size (cairo_surface_t *surface,
				       gint width,
				       gint height)
{
  // Do nothing.  The surface size is determined by the DC
  return FALSE;
}

HGDIOBJ
gdk_win32_drawable_get_handle (GdkDrawable *drawable)
{
  return GDK_DRAWABLE_HANDLE (drawable);
}

/**
 * _gdk_win32_drawable_finish
 * @drawable: a Win32 #GdkDrawable implementation
 * 
 * Releases any resources allocated internally for the drawable.
 * This is called when the drawable becomes unusable, i.e.
 * gdk_window_destroy() is called.
 **/
void
_gdk_win32_drawable_finish (GdkDrawable *drawable)
{
  GdkDrawableImplWin32 *impl = GDK_DRAWABLE_IMPL_WIN32 (drawable);

  if (impl->cairo_surface)
    {
      cairo_surface_finish (impl->cairo_surface);
      cairo_surface_set_user_data (impl->cairo_surface, &gdk_win32_cairo_key, NULL, NULL);
    }

  g_assert (impl->hdc_count == 0);
}

