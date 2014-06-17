/*
 * Copyright © 2014 Canonical Ltd
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include "gdkinternals.h"
#include "gdkdisplayprivate.h"
#include "gdkmir.h"
#include "gdkmir-private.h"

struct _GdkMirWindowReference {
  GdkMirEventSource *source;
  GdkWindow         *window;
  gint               ref_count;
};

typedef struct {
  GdkMirWindowReference *window_ref;
  MirEvent               event;
} GdkMirQueuedEvent;

struct _GdkMirEventSource
{
  GSource parent_instance;

  GMutex mir_event_lock;
  GQueue mir_events;

  GdkDisplay *display;
};

static void
print_modifiers (unsigned int modifiers)
{
  g_printerr (" Modifiers");
  if ((modifiers & mir_key_modifier_alt) != 0)
    g_printerr (" alt");
  if ((modifiers & mir_key_modifier_alt_left) != 0)
    g_printerr (" alt-left");
  if ((modifiers & mir_key_modifier_alt_right) != 0)
    g_printerr (" alt-right");
  if ((modifiers & mir_key_modifier_shift) != 0)
    g_printerr (" shift");
  if ((modifiers & mir_key_modifier_shift_left) != 0)
    g_printerr (" shift-left");
  if ((modifiers & mir_key_modifier_shift_right) != 0)
    g_printerr (" shift-right");
  if ((modifiers & mir_key_modifier_sym) != 0)
    g_printerr (" sym");
  if ((modifiers & mir_key_modifier_function) != 0)
    g_printerr (" function");
  if ((modifiers & mir_key_modifier_ctrl) != 0)
    g_printerr (" ctrl");
  if ((modifiers & mir_key_modifier_ctrl_left) != 0)
    g_printerr (" ctrl-left");
  if ((modifiers & mir_key_modifier_ctrl_right) != 0)
    g_printerr (" ctrl-right");
  if ((modifiers & mir_key_modifier_meta) != 0)
    g_printerr (" meta");
  if ((modifiers & mir_key_modifier_meta_left) != 0)
    g_printerr (" meta-left");
  if ((modifiers & mir_key_modifier_meta_right) != 0)
    g_printerr (" meta-right");
  if ((modifiers & mir_key_modifier_caps_lock) != 0)
    g_printerr (" caps-lock");
  if ((modifiers & mir_key_modifier_num_lock) != 0)
    g_printerr (" num-lock");
  if ((modifiers & mir_key_modifier_scroll_lock) != 0)
    g_printerr (" scroll-lock");
  g_printerr ("\n");
}

static void
print_key_event (const MirKeyEvent *event)
{
  g_printerr ("KEY\n");
  g_printerr (" Device %i\n", event->device_id);
  g_printerr (" Source %i\n", event->source_id);
  g_printerr (" Action ");
  switch (event->action)
    {
    case mir_key_action_down:
      g_printerr ("down");
      break;
    case mir_key_action_up:
      g_printerr ("up");
      break;
    case mir_key_action_multiple:
      g_printerr ("multiple");
      break;
    default:
      g_printerr ("%u", event->action);
      break;
    }
  g_printerr ("\n");
  g_printerr (" Flags");
  if ((event->flags & mir_key_flag_woke_here) != 0)
    g_printerr (" woke-here");
  if ((event->flags & mir_key_flag_soft_keyboard) != 0)
    g_printerr (" soft-keyboard");
  if ((event->flags & mir_key_flag_keep_touch_mode) != 0)
    g_printerr (" keep-touch-mode");
  if ((event->flags & mir_key_flag_from_system) != 0)
    g_printerr (" from-system");
  if ((event->flags & mir_key_flag_editor_action) != 0)
    g_printerr (" editor-action");
  if ((event->flags & mir_key_flag_canceled) != 0)
    g_printerr (" canceled");
  if ((event->flags & mir_key_flag_virtual_hard_key) != 0)
    g_printerr (" virtual-hard-key");
  if ((event->flags & mir_key_flag_long_press) != 0)
    g_printerr (" long-press");
  if ((event->flags & mir_key_flag_canceled_long_press) != 0)
    g_printerr (" canceled-long-press");
  if ((event->flags & mir_key_flag_tracking) != 0)
    g_printerr (" tracking");
  if ((event->flags & mir_key_flag_fallback) != 0)
    g_printerr (" fallback");
  g_printerr ("\n");
  print_modifiers (event->modifiers);
  g_printerr (" Key Code %i\n", event->key_code);
  g_printerr (" Scan Code %i\n", event->scan_code);
  g_printerr (" Repeat Count %i\n", event->repeat_count);
  g_printerr (" Down Time %lli\n", (long long int) event->down_time);
  g_printerr (" Event Time %lli\n", (long long int) event->event_time);
  g_printerr (" Is System Key %s\n", event->is_system_key ? "true" : "false");
}

static void
print_motion_event (const MirMotionEvent *event)
{
  size_t i;

  g_printerr ("MOTION\n");
  g_printerr (" Device %i\n", event->device_id);
  g_printerr (" Source %i\n", event->source_id);
  g_printerr (" Action ");
  switch (event->action)
    {
    case mir_motion_action_down:
      g_printerr ("down");
      break;
    case mir_motion_action_up:
      g_printerr ("up");
      break;
    case mir_motion_action_move:
      g_printerr ("move");
      break;
    case mir_motion_action_cancel:
      g_printerr ("cancel");
      break;
    case mir_motion_action_outside:
      g_printerr ("outside");
      break;
    case mir_motion_action_pointer_down:
      g_printerr ("pointer-down");
      break;
    case mir_motion_action_pointer_up:
      g_printerr ("pointer-up");
      break;
    case mir_motion_action_hover_move:
      g_printerr ("hover-move");
      break;
    case mir_motion_action_scroll:
      g_printerr ("scroll");
      break;
    case mir_motion_action_hover_enter:
      g_printerr ("hover-enter");
      break;
    case mir_motion_action_hover_exit:
      g_printerr ("hover-exit");
      break;
    default:
      g_printerr ("%u", event->action);
    }
  g_printerr ("\n");
  g_printerr (" Flags");
  switch (event->flags)
    {
    case mir_motion_flag_window_is_obscured:
      g_printerr (" window-is-obscured");
      break;
    }
  g_printerr ("\n");
  print_modifiers (event->modifiers);
  g_printerr (" Edge Flags %i\n", event->edge_flags);
  g_printerr (" Button State");
  switch (event->button_state)
    {
    case mir_motion_button_primary:
      g_printerr (" primary");
      break;
    case mir_motion_button_secondary:
      g_printerr (" secondary");
      break;
    case mir_motion_button_tertiary:
      g_printerr (" tertiary");
      break;
    case mir_motion_button_back:
      g_printerr (" back");
      break;
    case mir_motion_button_forward:
      g_printerr (" forward");
      break;
    }
  g_printerr ("\n");
  g_printerr (" Offset (%f, %f)\n", event->x_offset, event->y_offset);
  g_printerr (" Precision (%f, %f)\n", event->x_precision, event->y_precision);
  g_printerr (" Down Time %lli\n", (long long int) event->down_time);
  g_printerr (" Event Time %lli\n", (long long int) event->event_time);
  g_printerr (" Pointer Coordinates\n");
  for (i = 0; i < event->pointer_count; i++)
    {
      g_printerr ("  ID=%i location=(%f, %f) raw=(%f, %f) touch=(%f, %f) size=%f pressure=%f orientation=%f scroll=(%f, %f) tool=",
                  event->pointer_coordinates[i].id,
                  event->pointer_coordinates[i].x, event->pointer_coordinates[i].y,
                  event->pointer_coordinates[i].raw_x, event->pointer_coordinates[i].raw_y,
                  event->pointer_coordinates[i].touch_major, event->pointer_coordinates[i].touch_minor,
                  event->pointer_coordinates[i].size,
                  event->pointer_coordinates[i].pressure,
                  event->pointer_coordinates[i].orientation,
                  event->pointer_coordinates[i].hscroll, event->pointer_coordinates[i].vscroll);
      switch (event->pointer_coordinates[i].tool_type)
        {
        case mir_motion_tool_type_unknown:
          g_printerr ("unknown");
          break;
        case mir_motion_tool_type_finger:
          g_printerr ("finger");
          break;
        case mir_motion_tool_type_stylus:
          g_printerr ("stylus");
          break;
        case mir_motion_tool_type_mouse:
          g_printerr ("mouse");
          break;
        case mir_motion_tool_type_eraser:
          g_printerr ("eraser");
          break;
        default:
          g_printerr ("%u", event->pointer_coordinates[i].tool_type);
          break;
        }
      g_printerr ("\n");
    }
}

static void
print_surface_event (const MirSurfaceEvent *event)
{
  g_printerr ("SURFACE\n");
  g_printerr (" Surface %i\n", event->id);
  g_printerr (" Attribute ");
  switch (event->attrib)
    {
    case mir_surface_attrib_type:
      g_printerr ("type");
      break;
    case mir_surface_attrib_state:
      g_printerr ("state");
      break;
    case mir_surface_attrib_swapinterval:
      g_printerr ("swapinterval");
      break;
    case mir_surface_attrib_focus:
      g_printerr ("focus");
      break;
    default:
      g_printerr ("%u", event->attrib);
      break;
    }
  g_printerr ("\n");
  g_printerr (" Value %i\n", event->value);
}

static void
print_resize_event (const MirResizeEvent *event)
{
  g_printerr ("RESIZE\n");
  g_printerr (" Surface %i\n", event->surface_id);
  g_printerr (" Size (%i, %i)\n", event->width, event->height);
}

static void
print_event (const MirEvent *event)
{
  switch (event->type)
    {
    case mir_event_type_key:
      print_key_event (&event->key);
      break;
    case mir_event_type_motion:
      print_motion_event (&event->motion);
      break;
    case mir_event_type_surface:
      print_surface_event (&event->surface);
      break;
    case mir_event_type_resize:
      print_resize_event (&event->resize);
      break;
    default:
      g_printerr ("EVENT %u\n", event->type);
      break;
    }
}

static void
send_event (GdkWindow *window, GdkDevice *device, GdkEvent *event)
{
  GdkDisplay *display;
  GList *node;

  gdk_event_set_device (event, device);
  gdk_event_set_screen (event, gdk_display_get_screen (gdk_window_get_display (window), 0));
  event->any.window = g_object_ref (window);

  display = gdk_window_get_display (window);
  node = _gdk_event_queue_append (display, event);
  _gdk_windowing_got_event (display, node, event, _gdk_display_get_next_serial (display));
}

static void
generate_key_event (GdkWindow *window, GdkEventType type, guint state, guint keyval, guint16 keycode, gboolean is_modifier)
{
  GdkEvent *event;

  event = gdk_event_new (type);
  event->key.state = state;
  event->key.keyval = keyval;
  event->key.length = 0;
  event->key.string = NULL; // FIXME: Is this still needed?
  event->key.hardware_keycode = keycode;
  event->key.group = 0; // FIXME
  event->key.is_modifier = is_modifier;

  send_event (window, _gdk_mir_device_manager_get_keyboard (gdk_display_get_device_manager (gdk_window_get_display (window))), event);
}

static GdkDevice *
get_pointer (GdkWindow *window)
{
  return gdk_device_manager_get_client_pointer (gdk_display_get_device_manager (gdk_window_get_display (window)));
}

static void
generate_button_event (GdkWindow *window, GdkEventType type, gdouble x, gdouble y, guint button, guint state)
{
  GdkEvent *event;

  event = gdk_event_new (type);
  event->button.x = x;
  event->button.y = y;
  event->button.state = state;
  event->button.button = button;

  send_event (window, get_pointer (window), event);
}

static void
generate_scroll_event (GdkWindow *window, gdouble x, gdouble y, gdouble delta_x, gdouble delta_y, guint state)
{
  GdkEvent *event;

  event = gdk_event_new (GDK_SCROLL);
  event->scroll.x = x;
  event->scroll.y = y;
  event->scroll.state = state;
  event->scroll.direction = GDK_SCROLL_SMOOTH;
  event->scroll.delta_x = delta_x;
  event->scroll.delta_y = delta_y;

  send_event (window, get_pointer (window), event);
}

static void
generate_motion_event (GdkWindow *window, gdouble x, gdouble y, guint state)
{
  GdkEvent *event;

  event = gdk_event_new (GDK_MOTION_NOTIFY);
  event->motion.x = x;
  event->motion.y = y;
  event->motion.state = state;
  event->motion.is_hint = FALSE;

  send_event (window, get_pointer (window), event);
}

static void
generate_crossing_event (GdkWindow *window, GdkEventType type, gdouble x, gdouble y)
{
  GdkEvent *event;

  event = gdk_event_new (type);
  event->crossing.x = x;
  event->crossing.y = y;
  event->crossing.mode = GDK_CROSSING_NORMAL;
  event->crossing.detail = GDK_NOTIFY_ANCESTOR;
  event->crossing.focus = TRUE;

  send_event (window, get_pointer (window), event);
}

static guint
get_modifier_state (unsigned int modifiers, unsigned int button_state)
{
  guint modifier_state = 0;

  if ((modifiers & mir_key_modifier_alt) != 0)
    modifier_state |= GDK_MOD1_MASK;
  if ((modifiers & mir_key_modifier_shift) != 0)
    modifier_state |= GDK_SHIFT_MASK;
  if ((modifiers & mir_key_modifier_ctrl) != 0)
    modifier_state |= GDK_CONTROL_MASK;
  if ((modifiers & mir_key_modifier_meta) != 0)
    modifier_state |= GDK_SUPER_MASK;
  if ((modifiers & mir_key_modifier_caps_lock) != 0)
    modifier_state |= GDK_LOCK_MASK;
  if ((button_state & mir_motion_button_primary) != 0)
    modifier_state |= GDK_BUTTON1_MASK;
  if ((button_state & mir_motion_button_secondary) != 0)
    modifier_state |= GDK_BUTTON3_MASK;
  if ((button_state & mir_motion_button_tertiary) != 0)
    modifier_state |= GDK_BUTTON2_MASK;

  return modifier_state;
}

/*
  GdkMirWindowImpl *impl = GDK_MIR_WINDOW_IMPL (event_data->window->impl);
  MirMotionButton changed_button_state;
  GdkEventType event_type;
  gdouble x, y;
  guint modifier_state;
  gboolean is_modifier = FALSE;
*/

static void
handle_key_event (GdkWindow *window, const MirKeyEvent *event)
{
  guint modifier_state;
  gboolean is_modifier = FALSE;

  modifier_state = get_modifier_state (event->modifiers, 0); // FIXME: Need to track button state

  switch (event->action)
    {
    case mir_key_action_down:
    case mir_key_action_up:
      // FIXME: Convert keycode
      // FIXME: is_modifier
      generate_key_event (window,
                          event->action == mir_key_action_down ? GDK_KEY_PRESS : GDK_KEY_RELEASE,
                          modifier_state,
                          event->key_code,
                          event->scan_code,
                          is_modifier);
      break;
    default:
    //case mir_key_action_multiple:
      // FIXME
      break;
    }
}

static void
handle_motion_event (GdkWindow *window, const MirMotionEvent *event)
{
  GdkMirWindowImpl *impl = GDK_MIR_WINDOW_IMPL (window->impl);
  guint modifier_state;
  GdkEventType event_type;
  MirMotionButton changed_button_state;

  if (event->pointer_count > 0)
    {
      impl->x = event->pointer_coordinates[0].x;
      impl->y = event->pointer_coordinates[0].y;
    }
  modifier_state = get_modifier_state (event->modifiers, event->button_state);

  /* The Mir events generate hover-exits even while inside the window so
     counteract this by always generating an enter notify on all other events */
  if (!impl->cursor_inside && event->action != mir_motion_action_hover_exit)
    {
      impl->cursor_inside = TRUE;
      generate_crossing_event (window, GDK_ENTER_NOTIFY, impl->x, impl->y);
    }

  /* Update which window has focus */
  _gdk_mir_pointer_set_location (get_pointer (window), impl->x, impl->y, window, modifier_state);
  switch (event->action)
    {
    case mir_motion_action_down:
    case mir_motion_action_up:
      event_type = event->action == mir_motion_action_down ? GDK_BUTTON_PRESS : GDK_BUTTON_RELEASE;
      changed_button_state = impl->button_state ^ event->button_state;
      if ((changed_button_state & mir_motion_button_primary) != 0)
        generate_button_event (window, event_type, impl->x, impl->y, GDK_BUTTON_PRIMARY, modifier_state);
      if ((changed_button_state & mir_motion_button_secondary) != 0)
        generate_button_event (window, event_type, impl->x, impl->y, GDK_BUTTON_SECONDARY, modifier_state);
      if ((changed_button_state & mir_motion_button_tertiary) != 0)
        generate_button_event (window, event_type, impl->x, impl->y, GDK_BUTTON_MIDDLE, modifier_state);
      impl->button_state = event->button_state;
      break;
    case mir_motion_action_scroll:
      generate_scroll_event (window, impl->x, impl->y, event->pointer_coordinates[0].hscroll, event->pointer_coordinates[0].vscroll, modifier_state);
      break;
    case mir_motion_action_move: // move with button
    case mir_motion_action_hover_move: // move without button
      generate_motion_event (window, impl->x, impl->y, modifier_state);
      break;
    case mir_motion_action_hover_exit:
      impl->cursor_inside = FALSE;
      generate_crossing_event (window, GDK_LEAVE_NOTIFY, impl->x, impl->y);
      break;
    }
}

static void
handle_surface_event (GdkWindow *window, const MirSurfaceEvent *event)
{
  GdkMirWindowImpl *impl = GDK_MIR_WINDOW_IMPL (window->impl);

  switch (event->attrib)
    {
    case mir_surface_attrib_type:
      break;
    case mir_surface_attrib_state:
      impl->surface_state = event->value;
      // FIXME: notify
      break;
    case mir_surface_attrib_swapinterval:
      break;
    case mir_surface_attrib_focus:
      if (event->value)
        gdk_synthesize_window_state (window, 0, GDK_WINDOW_STATE_FOCUSED);
      else
        gdk_synthesize_window_state (window, GDK_WINDOW_STATE_FOCUSED, 0);
      break;
    default:
      break;
    }
}

typedef struct
{
  GdkWindow *window;
  MirEvent event;
} EventData;

static void
gdk_mir_event_source_queue_event (GdkDisplay     *display,
                                  GdkWindow      *window,
                                  const MirEvent *event)
{
  if (g_getenv ("GDK_MIR_LOG_EVENTS"))
    print_event (event);

  // FIXME: Only generate events if the window wanted them?
  switch (event->type)
    {
    case mir_event_type_key:
      handle_key_event (window, &event->key);
      break;
    case mir_event_type_motion:
      handle_motion_event (window, &event->motion);
      break;
    case mir_event_type_surface:
      handle_surface_event (window, &event->surface);
      break;
    case mir_event_type_resize:
      // FIXME: Generate configure event
      break;
    default:
      // FIXME?
      break;
    }
}

static GdkMirQueuedEvent *
gdk_mir_event_source_take_queued_event (GdkMirEventSource *source)
{
  GdkMirQueuedEvent *queued_event;

  g_mutex_lock (&source->mir_event_lock);
  queued_event = g_queue_pop_head (&source->mir_events);
  g_mutex_unlock (&source->mir_event_lock);

  return queued_event;
}

static void
gdk_mir_queued_event_free (GdkMirQueuedEvent *event)
{
  _gdk_mir_window_reference_unref (event->window_ref);
  g_slice_free (GdkMirQueuedEvent, event);
}

static void
gdk_mir_event_source_convert_events (GdkMirEventSource *source)
{
  GdkMirQueuedEvent *event;

  while ((event = gdk_mir_event_source_take_queued_event (source)))
    {
      GdkWindow *window = event->window_ref->window;

      /* The window may have been destroyed in the main thread while the
       * event was being dispatched...
       */
      if (window != NULL)
        gdk_mir_event_source_queue_event (source->display, window, &event->event);

      gdk_mir_queued_event_free (event);
    }
}

static gboolean
gdk_mir_event_source_prepare (GSource *g_source,
                              gint    *timeout)
{
  GdkMirEventSource *source = (GdkMirEventSource *) g_source;
  gboolean mir_events_in_queue;

  if (_gdk_event_queue_find_first (source->display))
   return TRUE;

  g_mutex_lock (&source->mir_event_lock);
  mir_events_in_queue = g_queue_get_length (&source->mir_events) > 0;
  g_mutex_unlock (&source->mir_event_lock);

  return mir_events_in_queue;
}

static gboolean
gdk_mir_event_source_check (GSource *g_source)
{
  return gdk_mir_event_source_prepare (g_source, NULL);
}

static gboolean
gdk_mir_event_source_dispatch (GSource     *g_source,
                               GSourceFunc  callback,
                               gpointer     user_data)
{
  GdkMirEventSource *source = (GdkMirEventSource *) g_source;
  GdkEvent *event;

  /* First, run the queue of events from the thread */
  gdk_mir_event_source_convert_events (source);

  /* Next, dispatch one single event from the display's queue.
   *
   * If there is more than one event then we will soon find ourselves
   * back here again.
   */

  gdk_threads_enter ();

  event = gdk_display_get_event (source->display);

  if (event)
    {
      _gdk_event_emit (event);

      gdk_event_free (event);
    }

  gdk_threads_leave ();

  return TRUE;
}

static void
gdk_mir_event_source_finalize (GSource *g_source)
{
  GdkMirEventSource *source = (GdkMirEventSource *) g_source;
  GdkMirQueuedEvent *event;

  while ((event = gdk_mir_event_source_take_queued_event (source)))
    gdk_mir_queued_event_free (event);

  g_mutex_clear (&source->mir_event_lock);
}

static GSourceFuncs gdk_mir_event_source_funcs = {
  gdk_mir_event_source_prepare,
  gdk_mir_event_source_check,
  gdk_mir_event_source_dispatch,
  gdk_mir_event_source_finalize
};

GdkMirEventSource *
_gdk_mir_event_source_new (GdkDisplay *display)
{
  GdkMirEventSource *source;
  GSource *g_source;

  g_source = g_source_new (&gdk_mir_event_source_funcs, sizeof (GdkMirEventSource));
  g_source_attach (g_source, NULL);

  source = (GdkMirEventSource *) g_source;
  g_mutex_init (&source->mir_event_lock);
  source->display = display;

  return source;
}

GdkMirWindowReference *
_gdk_mir_event_source_get_window_reference (GdkWindow *window)
{
  static GQuark win_ref_quark;
  GdkMirWindowReference *ref;

  if G_UNLIKELY (!win_ref_quark)
    win_ref_quark = g_quark_from_string ("GdkMirEventSource window reference");

  ref = g_object_get_qdata (G_OBJECT (window), win_ref_quark);

  if (!ref)
    {
      GdkMirEventSource *source;

      source = _gdk_mir_display_get_event_source (gdk_window_get_display (window));
      g_source_ref ((GSource *) source);

      ref = g_slice_new (GdkMirWindowReference);
      ref->window = window;
      ref->source = source;
      ref->ref_count = 0;
      g_object_add_weak_pointer (G_OBJECT (window), (gpointer *) &ref->window);

      g_object_set_qdata_full (G_OBJECT (window), win_ref_quark,
                               ref, (GDestroyNotify) _gdk_mir_window_reference_unref);
    }

  g_atomic_int_inc (&ref->ref_count);

  return ref;
}

void
_gdk_mir_window_reference_unref (GdkMirWindowReference *ref)
{
  if (g_atomic_int_dec_and_test (&ref->ref_count))
    {
      if (ref->window)
        g_object_remove_weak_pointer (G_OBJECT (ref->window), (gpointer *) &ref->window);

      g_source_unref ((GSource *) ref->source);

      g_slice_free (GdkMirWindowReference, ref);
    }
}

void
_gdk_mir_event_source_queue (GdkMirWindowReference *window_ref,
                             const MirEvent        *event)
{
  GdkMirEventSource *source = window_ref->source;
  GdkMirQueuedEvent *queued_event;

  /* We are in the wrong thread right now.  We absolutely cannot touch
   * the window.
   *
   * We can do pretty much anything we want with the source, though...
   */

  queued_event = g_slice_new (GdkMirQueuedEvent);
  g_atomic_int_inc (&window_ref->ref_count);
  queued_event->window_ref = window_ref;
  queued_event->event = *event;

  g_mutex_lock (&source->mir_event_lock);
  g_queue_push_tail (&source->mir_events, queued_event);
  g_mutex_unlock (&source->mir_event_lock);

  g_main_context_wakeup (NULL);
}
