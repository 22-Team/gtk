/* GDK - The GIMP Drawing Kit
 *
 * gdkglcontext.h: GL context abstraction
 * 
 * Copyright © 2014  Emmanuele Bassi
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GDK_GL_CONTEXT_H__
#define __GDK_GL_CONTEXT_H__

#if !defined (__GDK_H_INSIDE__) && !defined (GDK_COMPILATION)
#error "Only <gdk/gdk.h> can be included directly."
#endif

#include <gdk/gdkversionmacros.h>
#include <gdk/gdktypes.h>

G_BEGIN_DECLS

#define GDK_TYPE_GL_CONTEXT             (gdk_gl_context_get_type ())
#define GDK_GL_CONTEXT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GDK_TYPE_GL_CONTEXT, GdkGLContext))
#define GDK_IS_GL_CONTEXT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GDK_TYPE_GL_CONTEXT))

#define GDK_GL_ERROR       (gdk_gl_error_quark ())

GDK_AVAILABLE_IN_3_16
GQuark gdk_gl_error_quark (void);

GDK_AVAILABLE_IN_3_16
GType gdk_gl_context_get_type (void) G_GNUC_CONST;

GDK_AVAILABLE_IN_3_16
GdkDisplay *            gdk_gl_context_get_display      (GdkGLContext *context);
GDK_AVAILABLE_IN_3_16
GdkWindow *             gdk_gl_context_get_window       (GdkGLContext *context);
GDK_AVAILABLE_IN_3_16
GdkGLProfile            gdk_gl_context_get_profile      (GdkGLContext *context);

GDK_AVAILABLE_IN_3_16
void                    gdk_gl_context_make_current     (GdkGLContext *context);
GDK_AVAILABLE_IN_3_16
GdkGLContext *          gdk_gl_context_get_current      (void);
GDK_AVAILABLE_IN_3_16
void                    gdk_gl_context_clear_current    (void);

/**
 * GdkGLFlags:
 * @GDK_GL_DISABLE: Disable creating new windows with GL rendering
 * @GDK_GL_ALWAYS: Make all newly created windows use GL rendering
 * @GDK_GL_SOFTWARE_DRAW_GL: Always use software fallback for drawing
 *     GL content to a cairo_t. This disables the fast paths that exist for
 *     drawing directly to a window and instead reads back the pixels into
 *     a cairo image surface.
 * @GDK_GL_SOFTWARE_DRAW_SURFACE: Always use software fallback for
 *     drawing cairo surfaces onto a GL-using window. This disables e.g.
 *     texture-from-pixmap on X11.
 * @GDK_GL_TEXTURE_RECTANGLE: Use the GL_ARB_texture_rectangle extension
 *
 * Flags that influence the OpenGL rendering used by GDK.
 */
typedef enum {
  GDK_GL_DISABLE                = 1 << 0,
  GDK_GL_ALWAYS                 = 1 << 1,
  GDK_GL_SOFTWARE_DRAW_GL       = 1 << 2,
  GDK_GL_SOFTWARE_DRAW_SURFACE  = 1 << 3,
  GDK_GL_TEXTURE_RECTANGLE      = 1 << 4
} GdkGLFlags;

GDK_AVAILABLE_IN_3_16
GdkGLFlags              gdk_gl_get_flags (void);
GDK_AVAILABLE_IN_3_16
void                    gdk_gl_set_flags (GdkGLFlags flags);

G_END_DECLS

#endif /* __GDK_GL_CONTEXT_H__ */
