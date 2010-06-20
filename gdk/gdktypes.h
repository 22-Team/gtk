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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
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

#if !defined (__GDK_H_INSIDE__) && !defined (GDK_COMPILATION)
#error "Only <gdk/gdk.h> can be included directly."
#endif

#ifndef __GDK_TYPES_H__
#define __GDK_TYPES_H__

/* GDK uses "glib". (And so does GTK).
 */
#include <glib.h>
#include <pango/pango.h>
#include <glib-object.h>

#ifdef G_OS_WIN32
#  ifdef GDK_COMPILATION
#    define GDKVAR __declspec(dllexport)
#  else
#    define GDKVAR extern __declspec(dllimport)
#  endif
#else
#  define GDKVAR extern
#endif

/* The system specific file gdkconfig.h contains such configuration
 * settings that are needed not only when compiling GDK (or GTK)
 * itself, but also occasionally when compiling programs that use GDK
 * (or GTK). One such setting is what windowing API backend is in use.
 */
#include <gdkconfig.h>

/* some common magic values */
#define GDK_CURRENT_TIME     0L

/**
 * GDK_PARENT_RELATIVE:
 *
 * A special value for #GdkPixmap variables, indicating that the background
 * pixmap for a window should be inherited from the parent window.
 */
#define GDK_PARENT_RELATIVE  1L



G_BEGIN_DECLS


/* Type definitions for the basic structures.
 */
typedef struct _GdkPoint	      GdkPoint;
typedef struct _GdkRectangle	      GdkRectangle;
typedef struct _GdkSegment	      GdkSegment;
typedef struct _GdkSpan	              GdkSpan;

typedef struct _GdkAtom            *GdkAtom;

#define GDK_ATOM_TO_POINTER(atom) (atom)
#define GDK_POINTER_TO_ATOM(ptr)  ((GdkAtom)(ptr))

#ifdef GDK_NATIVE_WINDOW_POINTER
#define GDK_GPOINTER_TO_NATIVE_WINDOW(p) ((GdkNativeWindow) (p))
#else
#define GDK_GPOINTER_TO_NATIVE_WINDOW(p) GPOINTER_TO_UINT(p)
#endif

#define _GDK_MAKE_ATOM(val) ((GdkAtom)GUINT_TO_POINTER(val))
#define GDK_NONE            _GDK_MAKE_ATOM (0)

#ifdef GDK_NATIVE_WINDOW_POINTER
typedef gpointer GdkNativeWindow;
#else
typedef guint32 GdkNativeWindow;
#endif
 
/* Forward declarations of commonly used types
 */
typedef struct _GdkColor	      GdkColor;
typedef struct _GdkColormap	      GdkColormap;
typedef struct _GdkCursor	      GdkCursor;
typedef struct _GdkFont		      GdkFont;
typedef struct _GdkGC                 GdkGC;
typedef struct _GdkImage              GdkImage;
typedef struct _GdkRegion             GdkRegion;
typedef struct _GdkVisual             GdkVisual;

typedef struct _GdkDrawable           GdkDrawable;
typedef struct _GdkDrawable           GdkBitmap;
typedef struct _GdkDrawable           GdkPixmap;

/**
 * GdkWindow:
 *
 * An opaque structure representing an onscreen drawable. Pointers to structures
 * of type #GdkPixmap, #GdkBitmap, and #GdkWindow can often be used
 * interchangeably. The type #GdkDrawable refers generically to any of these
 * types.
 */
typedef struct _GdkDrawable           GdkWindow;
typedef struct _GdkDisplay	      GdkDisplay;
typedef struct _GdkScreen	      GdkScreen;

typedef enum
{
  GDK_LSB_FIRST,
  GDK_MSB_FIRST
} GdkByteOrder;

/* Types of modifiers.
 */
/**
 * GdkModifierType:
 * @GDK_SHIFT_MASK: the Shift key.
 * @GDK_LOCK_MASK: a Lock key (depending on the modifier mapping of the
 *  X server this may either be CapsLock or ShiftLock).
 * @GDK_CONTROL_MASK: the Control key.
 * @GDK_MOD1_MASK: the fourth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier, but
 *  normally it is the Alt key).
 * @GDK_MOD2_MASK: the fifth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @GDK_MOD3_MASK: the sixth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @GDK_MOD4_MASK: the seventh modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @GDK_MOD5_MASK: the eighth modifier key (it depends on the modifier
 *  mapping of the X server which key is interpreted as this modifier).
 * @GDK_BUTTON1_MASK: the first mouse button.
 * @GDK_BUTTON2_MASK: the second mouse button.
 * @GDK_BUTTON3_MASK: the third mouse button.
 * @GDK_BUTTON4_MASK: the fourth mouse button.
 * @GDK_BUTTON5_MASK: the fifth mouse button.
 * @GDK_SUPER_MASK: the Super modifier. Since 2.10
 * @GDK_HYPER_MASK: the Hyper modifier. Since 2.10
 * @GDK_META_MASK: the Meta modifier. Since 2.10
 * @GDK_RELEASE_MASK: not used in GDK itself. GTK+ uses it to differentiate
 *  between (keyval, modifiers) pairs from key press and release events.
 * @GDK_MODIFIER_MASK: a mask covering all modifier types.
 *
 * A set of bit-flags to indicate the state of modifier keys and mouse buttons
 * in various event types. Typical modifier keys are Shift, Control, Meta,
 * Super, Hyper, Alt, Compose, Apple, CapsLock or ShiftLock.
 *
 * Like the X Window System, GDK supports 8 modifier keys and 5 mouse buttons.
 *
 * Since 2.10, GDK recognizes which of the Meta, Super or Hyper keys are mapped
 * to Mod2 - Mod5, and indicates this by setting %GDK_SUPER_MASK,
 * %GDK_HYPER_MASK or %GDK_META_MASK in the state field of key events.
 */
typedef enum
{
  GDK_SHIFT_MASK    = 1 << 0,
  GDK_LOCK_MASK	    = 1 << 1,
  GDK_CONTROL_MASK  = 1 << 2,
  GDK_MOD1_MASK	    = 1 << 3,
  GDK_MOD2_MASK	    = 1 << 4,
  GDK_MOD3_MASK	    = 1 << 5,
  GDK_MOD4_MASK	    = 1 << 6,
  GDK_MOD5_MASK	    = 1 << 7,
  GDK_BUTTON1_MASK  = 1 << 8,
  GDK_BUTTON2_MASK  = 1 << 9,
  GDK_BUTTON3_MASK  = 1 << 10,
  GDK_BUTTON4_MASK  = 1 << 11,
  GDK_BUTTON5_MASK  = 1 << 12,

  /* The next few modifiers are used by XKB, so we skip to the end.
   * Bits 15 - 25 are currently unused. Bit 29 is used internally.
   */
  
  GDK_SUPER_MASK    = 1 << 26,
  GDK_HYPER_MASK    = 1 << 27,
  GDK_META_MASK     = 1 << 28,
  
  GDK_RELEASE_MASK  = 1 << 30,

  GDK_MODIFIER_MASK = 0x5c001fff
} GdkModifierType;

typedef enum
{
  GDK_OK	  = 0,
  GDK_ERROR	  = -1,
  GDK_ERROR_PARAM = -2,
  GDK_ERROR_FILE  = -3,
  GDK_ERROR_MEM	  = -4
} GdkStatus;

/* We define specific numeric values for these constants,
 * since old application code may depend on them matching the X values
 * We don't actually depend on the matchup ourselves.
 */
typedef enum
{
  GDK_GRAB_SUCCESS         = 0,
  GDK_GRAB_ALREADY_GRABBED = 1,
  GDK_GRAB_INVALID_TIME    = 2,
  GDK_GRAB_NOT_VIEWABLE    = 3,
  GDK_GRAB_FROZEN          = 4
} GdkGrabStatus;

/**
 * GdkGrabOwnership:
 * @GDK_OWNERSHIP_NONE: All other devices' events are allowed.
 * @GDK_OWNERSHIP_WINDOW: Other devices' events are blocked for the grab window.
 * @GDK_OWNERSHIP_APPLICATION: Other devices' events are blocked for the whole application.
 *
 * Defines how device grabs interact with other devices.
 */
typedef enum
{
  GDK_OWNERSHIP_NONE,
  GDK_OWNERSHIP_WINDOW,
  GDK_OWNERSHIP_APPLICATION
} GdkGrabOwnership;

/* Event masks. (Used to select what types of events a window
 *  *  will receive).
 *   */
typedef enum
{
  GDK_EXPOSURE_MASK             = 1 << 1,
  GDK_POINTER_MOTION_MASK       = 1 << 2,
  GDK_POINTER_MOTION_HINT_MASK  = 1 << 3,
  GDK_BUTTON_MOTION_MASK        = 1 << 4,
  GDK_BUTTON1_MOTION_MASK       = 1 << 5,
  GDK_BUTTON2_MOTION_MASK       = 1 << 6,
  GDK_BUTTON3_MOTION_MASK       = 1 << 7,
  GDK_BUTTON_PRESS_MASK         = 1 << 8,
  GDK_BUTTON_RELEASE_MASK       = 1 << 9,
  GDK_KEY_PRESS_MASK            = 1 << 10,
  GDK_KEY_RELEASE_MASK          = 1 << 11,
  GDK_ENTER_NOTIFY_MASK         = 1 << 12,
  GDK_LEAVE_NOTIFY_MASK         = 1 << 13,
  GDK_FOCUS_CHANGE_MASK         = 1 << 14,
  GDK_STRUCTURE_MASK            = 1 << 15,
  GDK_PROPERTY_CHANGE_MASK      = 1 << 16,
  GDK_VISIBILITY_NOTIFY_MASK    = 1 << 17,
  GDK_PROXIMITY_IN_MASK         = 1 << 18,
  GDK_PROXIMITY_OUT_MASK        = 1 << 19,
  GDK_SUBSTRUCTURE_MASK         = 1 << 20,
  GDK_SCROLL_MASK               = 1 << 21,
  GDK_ALL_EVENTS_MASK           = 0x3FFFFE
} GdkEventMask;

struct _GdkPoint
{
  gint x;
  gint y;
};

struct _GdkRectangle
{
  gint x;
  gint y;
  gint width;
  gint height;
};

struct _GdkSegment
{
  gint x1;
  gint y1;
  gint x2;
  gint y2;
};

struct _GdkSpan
{
  gint x;
  gint y;
  gint width;
};

G_END_DECLS


#endif /* __GDK_TYPES_H__ */
