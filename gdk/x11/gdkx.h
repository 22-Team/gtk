/* GDK - The GIMP Drawing Kit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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

#ifndef __GDK_X_H__
#define __GDK_X_H__

#include <gdk/gdkprivate.h>
#include <gdk/gdkcursor.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

G_BEGIN_DECLS

Display *gdk_x11_drawable_get_xdisplay    (GdkDrawable *drawable);
XID      gdk_x11_drawable_get_xid         (GdkDrawable *drawable);
Display *gdk_x11_image_get_xdisplay       (GdkImage    *image);
XImage  *gdk_x11_image_get_ximage         (GdkImage    *image);
Display *gdk_x11_colormap_get_xdisplay    (GdkColormap *colormap);
Colormap gdk_x11_colormap_get_xcolormap   (GdkColormap *colormap);
Display *gdk_x11_cursor_get_xdisplay      (GdkCursor   *cursor);
Cursor   gdk_x11_cursor_get_xcursor       (GdkCursor   *cursor);
Visual * gdk_x11_visual_get_xvisual       (GdkVisual   *visual);
Display *gdk_x11_gc_get_xdisplay          (GdkGC       *gc);
GC       gdk_x11_gc_get_xgc               (GdkGC       *gc);
Window   gdk_x11_screen_get_root_xwindow  (GdkScreen   *screen);
#ifndef GDK_MULTIHEAD_SAFE
Window   gdk_x11_get_default_root_xwindow (void);
Display *gdk_x11_get_default_xdisplay     (void);
gint     gdk_x11_get_default_screen       (void);
#endif

#define GDK_COLORMAP_XDISPLAY(cmap)   (gdk_x11_colormap_get_xdisplay (cmap))
#define GDK_COLORMAP_XCOLORMAP(cmap)  (gdk_x11_colormap_get_xcolormap (cmap))
#define GDK_CURSOR_XDISPLAY(cursor)   (gdk_x11_cursor_get_xdisplay (cursor))
#define GDK_CURSOR_XCURSOR(cursor)    (gdk_x11_cursor_get_xcursor (cursor))
#define GDK_IMAGE_XDISPLAY(image)     (gdk_x11_image_get_xdisplay (image))
#define GDK_IMAGE_XIMAGE(image)       (gdk_x11_image_get_ximage (image))

#ifdef INSIDE_GDK_X11

#include "gdkprivate-x11.h"
#define GDK_WINDOW_DISPLAY(win)       (GDK_SCREEN_IMPL_X11(GDK_DRAWABLE_IMPL_X11(((GdkWindowObject *)win)->impl)->screen)->display)
#define GDK_WINDOW_SCREEN(win)	      (GDK_DRAWABLE_IMPL_X11(((GdkWindowObject *)win)->impl)->screen)
#define GDK_WINDOW_XDISPLAY(win)      (GDK_SCREEN_IMPL_X11(GDK_DRAWABLE_IMPL_X11(((GdkWindowObject *)win)->impl)->screen)->xdisplay)
#define GDK_WINDOW_XROOTWIN(win)      (GDK_SCREEN_IMPL_X11(GDK_DRAWABLE_IMPL_X11(((GdkWindowObject *)win)->impl)->screen)->xroot_window)
#define GDK_WINDOW_XID(win)           (GDK_DRAWABLE_IMPL_X11(((GdkWindowObject *)win)->impl)->xid)

#define GDK_PIXMAP_DISPLAY(win)       (GDK_SCREEN_IMPL_X11(GDK_DRAWABLE_IMPL_X11(((GdkPixmapObject *)win)->impl)->screen)->display)
#define GDK_PIXMAP_SCREEN(win)	      (GDK_DRAWABLE_IMPL_X11(((GdkPixmapObject *)win)->impl)->screen)
#define GDK_PIXMAP_XDISPLAY(win)      (GDK_SCREEN_IMPL_X11(GDK_DRAWABLE_IMPL_X11(((GdkPixmapObject *)win)->impl)->screen)->xdisplay)
#define GDK_PIXMAP_XROOTWIN(win)      (GDK_SCREEN_IMPL_X11(GDK_DRAWABLE_IMPL_X11(((GdkPixmapObject *)win)->impl)->screen)->xroot_window)


#define GDK_WINDOW_XID(win)           (GDK_DRAWABLE_IMPL_X11(((GdkWindowObject *)win)->impl)->xid)
#define GDK_PIXMAP_XID(win)           (GDK_DRAWABLE_IMPL_X11(((GdkPixmapObject *)win)->impl)->xid)

#define GDK_DRAWABLE_DISPLAY(win)     (GDK_IS_WINDOW (win) ? GDK_WINDOW_DISPLAY (win) : GDK_PIXMAP_DISPLAY (win))
#define GDK_DRAWABLE_SCREEN(win)      (GDK_IS_WINDOW (win) ? GDK_WINDOW_SCREEN (win) : GDK_PIXMAP_SCREEN (win))
#define GDK_DRAWABLE_XDISPLAY(win)    (GDK_IS_WINDOW (win) ? GDK_WINDOW_XDISPLAY (win) : GDK_PIXMAP_XDISPLAY (win))
#define GDK_DRAWABLE_XROOTWIN(win)    (GDK_IS_WINDOW (win) ? GDK_WINDOW_XROOTWIN (win) : GDK_PIXMAP_XROOTWIN (win))
#define GDK_DRAWABLE_XID(win)         (GDK_IS_WINDOW (win) ? GDK_WINDOW_XID (win) : GDK_PIXMAP_XID (win))

#define GDK_IMAGE_PRIVATE_DATA(image) ((GdkImagePrivateX11 *) GDK_IMAGE (image)->windowing_data)
#define GDK_GC_XDISPLAY(gc)           (GDK_SCREEN_XDISPLAY(GDK_GC_X11(gc)->screen))
#define GDK_GC_DISPLAY(gc)            (GDK_SCREEN_DISPLAY(GDK_GC_X11(gc)->screen))
#define GDK_GC_XGC(gc)		      (GDK_GC_X11(gc)->xgc)
#define GDK_GC_GET_XGC(gc)	      (GDK_GC_X11(gc)->dirty_mask ? _gdk_x11_gc_flush (gc) : ((GdkGCX11 *)(gc))->xgc)
#define GDK_WINDOW_XWINDOW	      GDK_DRAWABLE_XID

#else /* INSIDE_GDK_X11 */

GdkVisual* gdkx_visual_get_for_screen (GdkScreen *screen,
				       VisualID   xvisualid);
#ifndef GDK_MULTIHEAD_SAFE
GdkVisual* gdkx_visual_get            (VisualID   xvisualid);
#endif
#define GDK_WINDOW_XDISPLAY(win)      (gdk_x11_drawable_get_xdisplay (((GdkWindowObject *)win)->impl))
#define GDK_WINDOW_XID(win)           (gdk_x11_drawable_get_xid (win))
#define GDK_WINDOW_XWINDOW(win)       (gdk_x11_drawable_get_xid (win))
#define GDK_WINDOW_XROOTWIN(win)      (gdk_x11_screen_get_root_xwindow (gdk_drawable_get_screen (((GdkWindowObject *)win)->impl)))
#define GDK_PIXMAP_XDISPLAY(win)      (gdk_x11_drawable_get_xdisplay (((GdkPixmapObject *)win)->impl))
#define GDK_PIXMAP_XID(win)           (gdk_x11_drawable_get_xid (win))
#define GDK_DRAWABLE_XDISPLAY(win)    (gdk_x11_drawable_get_xdisplay (win))
#define GDK_DRAWABLE_XID(win)         (gdk_x11_drawable_get_xid (win))
#define GDK_VISUAL_XVISUAL(visual)    (gdk_x11_visual_get_xvisual (visual))
#define GDK_GC_XDISPLAY(gc)           (gdk_x11_gc_get_xdisplay (gc))
#define GDK_GC_XGC(gc)                (gdk_x11_gc_get_xgc (gc))
#endif /* INSIDE_GDK_X11 */

/* XXX: Do not use this function until it is fixed. An X Colormap
 *      is useless unless we also have the visual. */
GdkColormap* gdkx_colormap_get (Colormap xcolormap);
/* Utility function in gdk.c - not sure where it belongs, but it's
   needed in more than one place, so make it public */
Window        gdk_get_client_window      (Display  *display,
                                          Window    win);

/* Return the Gdk* for a particular XID */
gpointer      gdk_xid_table_lookup_for_display (GdkDisplay *display,
						XID         xid);
guint32       gdk_x11_get_server_time  (GdkWindow       *window);

/* returns TRUE if we support the given WM spec feature */
gboolean gdk_net_wm_supports_for_screen (GdkScreen *screen,
					 GdkAtom    property);

#ifndef GDK_MULTIHEAD_SAFE
gpointer      gdk_xid_table_lookup   (XID              xid);
gboolean      gdk_net_wm_supports    (GdkAtom    property);
void          gdk_x11_grab_server    ();
void          gdk_x11_ungrab_server  ();

#endif

GC _gdk_x11_gc_flush (GdkGC *gc);
GList *gdk_list_visuals_for_screen (GdkScreen *screen);


/* Functions to get the X Atom equivalent to the GdkAtom */
Atom	              gdk_x11_atom_to_xatom_for_display (GdkDisplay  *display,
							 GdkAtom      virtual_atom);
GdkAtom		      gdk_x11_xatom_to_atom_for_display (GdkDisplay  *display,
							 Atom	      xatom);
Atom		      gdk_x11_get_xatom_by_name_for_display (GdkDisplay  *display,
							     const gchar *atom_name);
G_CONST_RETURN gchar *gdk_x11_get_xatom_name_for_display (GdkDisplay  *display,
							  Atom         xatom);
#ifndef GDK_MULTIHEAD_SAFE
Atom                  gdk_x11_atom_to_xatom     (GdkAtom      atom);
GdkAtom               gdk_x11_xatom_to_atom     (Atom         xatom);
Atom                  gdk_x11_get_xatom_by_name (const gchar *atom_name);
G_CONST_RETURN gchar *gdk_x11_get_xatom_name    (Atom         xatom);
#endif

#ifndef GDK_DISABLE_DEPRECATED

Display *            gdk_x11_font_get_xdisplay (GdkFont *font);
gpointer             gdk_x11_font_get_xfont    (GdkFont *font);
G_CONST_RETURN char *gdk_x11_font_get_name     (GdkFont *font);

#define GDK_FONT_XDISPLAY(font)       (gdk_x11_font_get_xdisplay (font))
#define GDK_FONT_XFONT(font)          (gdk_x11_font_get_xfont (font))

#ifndef GDK_MULTIHEAD_SAFE

#define gdk_font_lookup(xid)	   ((GdkFont*) gdk_xid_table_lookup (xid))

#endif /* GDK_MULTIHEAD_SAFE */
#define gdk_font_lookup_for_display(display, xid) ((GdkFont*) gdk_xid_table_lookup_for_display (display, xid))

#endif /* GDK_DISABLE_DEPRECATED */

G_END_DECLS

#endif /* __GDK_X_H__ */
