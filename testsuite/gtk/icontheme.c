#include <gtk/gtk.h>

#include <string.h>

#define SCALABLE_IMAGE_SIZE (128)

static GtkIconTheme *
get_test_icontheme (void)
{
  static GtkIconTheme *icon_theme = NULL;
  const char *current_dir;

  if (icon_theme)
    return icon_theme;

  icon_theme = gtk_icon_theme_new ();
  gtk_icon_theme_set_custom_theme (icon_theme, "icons");
  current_dir = g_test_get_dir (G_TEST_DIST);
  gtk_icon_theme_set_search_path (icon_theme, &current_dir, 1);

  return icon_theme;
}

static char *
lookup_flags_to_string (GtkIconLookupFlags flags)
{
  GValue flags_value = { 0, }, string_value = { 0, };
  char *result;

  g_value_init (&flags_value, GTK_TYPE_ICON_LOOKUP_FLAGS);
  g_value_init (&string_value, G_TYPE_STRING);

  g_value_set_flags (&flags_value, flags);
  if (!g_value_transform (&flags_value, &string_value))
    {
      g_assert_not_reached ();
    }

  result = g_value_dup_string (&string_value);

  g_value_unset (&flags_value);
  g_value_unset (&string_value);

  return result;
}

static void
assert_icon_lookup_size (const char         *icon_name,
                         gint                size,
                         GtkIconLookupFlags  flags,
                         const char         *filename,
                         gint                pixbuf_size)
{
  GtkIconInfo *info;

  info = gtk_icon_theme_lookup_icon (get_test_icontheme (), icon_name, size, flags);
  if (info == NULL)
    {
      g_error ("Could not look up an icon for \"%s\" with flags %s at size %d",
               icon_name, lookup_flags_to_string (flags), size);
      return;
    }

  if (!g_str_has_suffix (gtk_icon_info_get_filename (info), filename))
    {
      g_error ("Icon for \"%s\" with flags %s at size %d should be \"...%s\" but is \"...%s\"",
               icon_name, lookup_flags_to_string (flags), size,
               filename, gtk_icon_info_get_filename (info) + strlen (g_get_current_dir ()));
      return;
    }

  if (pixbuf_size > 0)
    {
      GdkPixbuf *pixbuf;
      GError *error = NULL;

      pixbuf = gtk_icon_info_load_icon (info, &error);
      g_assert_no_error (error);
      g_assert_cmpint (gdk_pixbuf_get_width (pixbuf), ==, pixbuf_size);
      g_object_unref (pixbuf);
    }

  g_object_unref (info);
}

static void
assert_icon_lookup (const char         *icon_name,
                    gint                size,
                    GtkIconLookupFlags  flags,
                    const char         *filename)
{
  assert_icon_lookup_size (icon_name, size, flags, filename, -1);
}

static void
assert_icon_lookup_fails (const char         *icon_name,
                          gint                size,
                          GtkIconLookupFlags  flags)
{
  GtkIconInfo *info;

  info = gtk_icon_theme_lookup_icon (get_test_icontheme (), icon_name, size, flags);

  if (info != NULL)
    {
      g_error ("Should not find an icon for \"%s\" with flags %s at size %d, but found \"%s\"",
               icon_name, lookup_flags_to_string (flags), size, gtk_icon_info_get_filename (info) + strlen (g_get_current_dir ()));
      g_object_unref (info);
      return;
    }
}

static GList *lookups = NULL;

static void
print_func (const gchar *string)
{
  if (g_str_has_prefix (string, "\tlookup name: "))
    {
      gchar *s;
      s = g_strchomp (g_strdup (string + strlen ("\tlookup name: ")));
      lookups = g_list_append (lookups, s);
    }
}

static void
assert_lookup_order (const char         *icon_name,
                     gint                size,
                     GtkIconLookupFlags  flags,
                     const char         *first,
                     ...)
{
  guint debug_flags;
  GPrintFunc old_print_func;
  va_list args;
  const gchar *s;
  GtkIconInfo *info;
  GList *l;

  debug_flags = gtk_get_debug_flags ();
  gtk_set_debug_flags (debug_flags | GTK_DEBUG_ICONTHEME);
  old_print_func = g_set_print_handler (print_func);

  g_assert (lookups == NULL);
  
  info = gtk_icon_theme_lookup_icon (get_test_icontheme (), icon_name, size, flags);
  if (info)
    g_object_unref (info);
  
  va_start (args, first);
  s = first;
  l = lookups;
  while (s != NULL)
    {
      g_assert (l != NULL);
      g_assert_cmpstr (s, ==, l->data);
      s = va_arg (args, gchar*);
      l = l->next;
    }
  g_assert (l == NULL);
  va_end (args);

  g_list_free_full (lookups, g_free);
  lookups = NULL;

  g_set_print_handler (old_print_func);
  gtk_set_debug_flags (debug_flags);
}

static void
test_basics (void)
{
  /* just a basic boring lookup so we know everything works */
  assert_icon_lookup ("simple", 16, 0, "/icons/16x16/simple.png");

  /* The first time an icon is looked up that doesn't exist, GTK spews a 
   * warning.
   * We make that happen right here, so we can get rid of the warning 
   * and do failing lookups in other tests.
   */
  g_test_expect_message ("Gtk", G_LOG_LEVEL_WARNING, "Could not find the icon*");
  assert_icon_lookup_fails ("this-icon-totally-does-not-exist", 16, 0);
  g_test_assert_expected_messages ();
}

static void
test_lookup_order (void)
{
  assert_lookup_order ("foo-bar-baz", 16, GTK_ICON_LOOKUP_GENERIC_FALLBACK,
                       "foo-bar-baz",
                       "foo-bar",
                       "foo",
                       NULL);
  assert_lookup_order ("foo-bar-baz-symbolic", 16, GTK_ICON_LOOKUP_GENERIC_FALLBACK,
                       "foo-bar-baz-symbolic",
                       "foo-bar-symbolic",
                       "foo-symbolic",
                       "foo-bar-baz",
                       "foo-bar",
                       "foo",
                       NULL);

  assert_lookup_order ("bla-bla", 16, GTK_ICON_LOOKUP_GENERIC_FALLBACK|GTK_ICON_LOOKUP_FORCE_SYMBOLIC,
                       "bla-bla-symbolic",
                       "bla-symbolic",
                       "bla-bla",
                       "bla",
                       NULL);
  assert_lookup_order ("bla-bla-symbolic", 16, GTK_ICON_LOOKUP_GENERIC_FALLBACK|GTK_ICON_LOOKUP_FORCE_SYMBOLIC,
                       "bla-bla-symbolic",
                       "bla-symbolic",
                       "bla-bla-symbolic", /* awkward */
                       "bla-symbolic", /* awkward */
                       "bla-bla",
                       "bla",
                       NULL);

  assert_lookup_order ("bar-baz", 16, GTK_ICON_LOOKUP_FORCE_SYMBOLIC|GTK_ICON_LOOKUP_GENERIC_FALLBACK|GTK_ICON_LOOKUP_DIR_RTL,
                       "bar-baz-symbolic-rtl",
                       "bar-baz-symbolic",
                       "bar-symbolic-rtl",
                       "bar-symbolic",
                       "bar-baz-rtl",
                       "bar-baz",
                       "bar-rtl",
                       "bar",
                       NULL);
  assert_lookup_order ("bar-baz-symbolic", 16, GTK_ICON_LOOKUP_FORCE_SYMBOLIC|GTK_ICON_LOOKUP_GENERIC_FALLBACK|GTK_ICON_LOOKUP_DIR_RTL,
                       "bar-baz-symbolic-rtl",
                       "bar-baz-symbolic",
                       "bar-symbolic-rtl",
                       "bar-symbolic",
                       "bar-baz-symbolic-rtl", /* awkward */
                       "bar-baz-symbolic", /* awkward */
                       "bar-symbolic-rtl", /* awkward */
                       "bar-symbolic", /* awkward */
                       "bar-baz-rtl",
                       "bar-baz",
                       "bar-rtl",
                       "bar",
                       NULL);

  assert_lookup_order ("bar-baz", 16, GTK_ICON_LOOKUP_FORCE_SYMBOLIC|GTK_ICON_LOOKUP_GENERIC_FALLBACK|GTK_ICON_LOOKUP_DIR_LTR,
                       "bar-baz-symbolic-ltr",
                       "bar-baz-symbolic",
                       "bar-symbolic-ltr",
                       "bar-symbolic",
                       "bar-baz-ltr",
                       "bar-baz",
                       "bar-ltr",
                       "bar",
                       NULL);
  assert_lookup_order ("bar-baz-symbolic", 16, GTK_ICON_LOOKUP_FORCE_SYMBOLIC|GTK_ICON_LOOKUP_GENERIC_FALLBACK|GTK_ICON_LOOKUP_DIR_LTR,
                       "bar-baz-symbolic-ltr",
                       "bar-baz-symbolic",
                       "bar-symbolic-ltr",
                       "bar-symbolic",
                       "bar-baz-symbolic-ltr", /* awkward */
                       "bar-baz-symbolic", /* awkward */
                       "bar-symbolic-ltr", /* awkward */
                       "bar-symbolic", /* awkward */
                       "bar-baz-ltr",
                       "bar-baz",
                       "bar-ltr",
                       "bar",
                       NULL);
}

static void
test_generic_fallback (void)
{
  /* simple test for generic fallback */
  assert_icon_lookup ("simple-foo-bar",
                      16,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK,
                      "/icons/16x16/simple.png");

  /* Check generic fallback also works for symbolics falling back to regular items */
  assert_icon_lookup ("simple-foo-bar-symbolic",
                      16,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK,
                      "/icons/16x16/simple.png");

  /* Check we fall back to more generic symbolic icons before falling back to
   * non-symbolics */
  assert_icon_lookup ("everything-justregular-symbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK,
                      "/icons/scalable/everything-symbolic.svg");
}

static void
test_force_symbolic (void)
{
  /* check forcing symbolic works */
  assert_icon_lookup ("everything",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_FORCE_SYMBOLIC,
                      "/icons/scalable/everything-symbolic.svg");
  /* check forcing symbolic also works for symbolic icons (d'oh) */
  assert_icon_lookup ("everything-symbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_FORCE_SYMBOLIC,
                      "/icons/scalable/everything-symbolic.svg");

  /* check all the combos for fallbacks on an icon that only exists as symbolic */
  assert_icon_lookup ("everything-justsymbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_FORCE_SYMBOLIC,
                      "/icons/scalable/everything-justsymbolic-symbolic.svg");
  assert_icon_lookup ("everything-justsymbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK | GTK_ICON_LOOKUP_FORCE_SYMBOLIC,
                      "/icons/scalable/everything-justsymbolic-symbolic.svg");
  assert_icon_lookup ("everything-justsymbolic-symbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_FORCE_SYMBOLIC,
                      "/icons/scalable/everything-justsymbolic-symbolic.svg");
  assert_icon_lookup ("everything-justsymbolic-symbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK | GTK_ICON_LOOKUP_FORCE_SYMBOLIC,
                      "/icons/scalable/everything-justsymbolic-symbolic.svg");

  /* check all the combos for fallbacks, this time for an icon that only exists as regular */
  assert_icon_lookup ("everything-justregular",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_FORCE_SYMBOLIC,
                      "/icons/scalable/everything-justregular.svg");
  assert_icon_lookup ("everything-justregular",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK | GTK_ICON_LOOKUP_FORCE_SYMBOLIC,
                      "/icons/scalable/everything-symbolic.svg");
  assert_icon_lookup_fails ("everything-justregular-symbolic",
                            SCALABLE_IMAGE_SIZE,
                            GTK_ICON_LOOKUP_FORCE_SYMBOLIC);
  assert_icon_lookup ("everything-justregular-symbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK | GTK_ICON_LOOKUP_FORCE_SYMBOLIC,
                      "/icons/scalable/everything-symbolic.svg");
}

static void
test_force_regular (void)
{
  /* check forcing regular works (d'oh) */
  assert_icon_lookup ("everything",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_FORCE_REGULAR,
                      "/icons/scalable/everything.svg");
  /* check forcing regular also works for symbolic icons ) */
  assert_icon_lookup ("everything-symbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_FORCE_REGULAR,
                      "/icons/scalable/everything.svg");

  /* check all the combos for fallbacks on an icon that only exists as regular */
  assert_icon_lookup ("everything-justregular",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_FORCE_REGULAR,
                      "/icons/scalable/everything-justregular.svg");
  assert_icon_lookup ("everything-justregular",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK | GTK_ICON_LOOKUP_FORCE_REGULAR,
                      "/icons/scalable/everything-justregular.svg");
  assert_icon_lookup ("everything-justregular-symbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_FORCE_REGULAR,
                      "/icons/scalable/everything-justregular.svg");
  assert_icon_lookup ("everything-justregular-symbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK | GTK_ICON_LOOKUP_FORCE_REGULAR,
                      "/icons/scalable/everything-justregular.svg");

  /* check all the combos for fallbacks, this time for an icon that only exists as symbolic */
  assert_icon_lookup_fails ("everything-justsymbolic",
                            SCALABLE_IMAGE_SIZE,
                            GTK_ICON_LOOKUP_FORCE_REGULAR);
  assert_icon_lookup ("everything-justsymbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK | GTK_ICON_LOOKUP_FORCE_REGULAR,
                      "/icons/scalable/everything.svg");
  assert_icon_lookup ("everything-justsymbolic-symbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_FORCE_REGULAR,
                      "/icons/scalable/everything-justsymbolic-symbolic.svg");
  assert_icon_lookup ("everything-justsymbolic-symbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK | GTK_ICON_LOOKUP_FORCE_REGULAR,
                      "/icons/scalable/everything.svg");
}

static void
test_rtl (void)
{
  assert_icon_lookup ("everything",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_DIR_RTL,
                      "/icons/scalable/everything-rtl.svg");
  assert_icon_lookup ("everything-symbolic",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_DIR_RTL,
                      "/icons/scalable/everything-symbolic-rtl.svg");

  assert_icon_lookup_fails ("everything-justrtl",
                            SCALABLE_IMAGE_SIZE,
                            0);
  assert_icon_lookup_fails ("everything-justrtl",
                            SCALABLE_IMAGE_SIZE,
                            GTK_ICON_LOOKUP_DIR_LTR);
  assert_icon_lookup ("everything-justrtl",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_DIR_RTL,
                      "/icons/scalable/everything-justrtl-rtl.svg");

  assert_icon_lookup ("everything-justrtl",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK,
                      "/icons/scalable/everything.svg");
  assert_icon_lookup ("everything-justrtl",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK | GTK_ICON_LOOKUP_DIR_LTR,
                      "/icons/scalable/everything.svg");
  assert_icon_lookup ("everything-justrtl",
                      SCALABLE_IMAGE_SIZE,
                      GTK_ICON_LOOKUP_GENERIC_FALLBACK | GTK_ICON_LOOKUP_DIR_RTL,
                      "/icons/scalable/everything-justrtl-rtl.svg");
}

static void
test_symbolic_single_size (void)
{
  /* Check we properly load a symbolic icon from a sized directory */
  assert_icon_lookup ("only32-symbolic",
                      32,
                      0,
                      "/icons/32x32/only32-symbolic.svg");
  /* Check that we still properly load it even if a different size is requested */
  assert_icon_lookup ("only32-symbolic",
                      16,
                      0,
                      "/icons/32x32/only32-symbolic.svg");
  assert_icon_lookup ("only32-symbolic",
                      128,
                      0,
                      "/icons/32x32/only32-symbolic.svg");
}

static void
test_svg_size (void)
{
   /* To understand these results, keep in mind that we never allow upscaling,
   * and don't respect min/max size for scaling (though we do take it into
   * account for choosing).
   */
  /* Check we properly load a svg icon from a sized directory */
  assert_icon_lookup_size ("twosize-fixed", 48, 0, "/icons/32x32/twosize-fixed.svg", 32);
  assert_icon_lookup_size ("twosize-fixed", 32, 0, "/icons/32x32/twosize-fixed.svg", 32);
  assert_icon_lookup_size ("twosize-fixed", 20, 0, "/icons/32x32/twosize-fixed.svg", 32);
  assert_icon_lookup_size ("twosize-fixed", 16, 0, "/icons/16x16/twosize-fixed.svg", 16);

  /* Check that we still properly load it even if a different size is requested */
  assert_icon_lookup_size ("twosize", 64, 0, "/icons/32x32s/twosize.svg", 64);
  assert_icon_lookup_size ("twosize", 48, 0, "/icons/32x32s/twosize.svg", 48);
  assert_icon_lookup_size ("twosize", 32, 0, "/icons/32x32s/twosize.svg", 32);
  assert_icon_lookup_size ("twosize", 24, 0, "/icons/32x32s/twosize.svg", 24);
  assert_icon_lookup_size ("twosize", 16, 0, "/icons/16x16s/twosize.svg", 16);
  assert_icon_lookup_size ("twosize", 12, 0, "/icons/16x16s/twosize.svg", 12);
  assert_icon_lookup_size ("twosize",  8, 0, "/icons/16x16s/twosize.svg",  8);
}

int
main (int argc, char *argv[])
{
  gtk_test_init (&argc, &argv);

  g_test_add_func ("/icontheme/basics", test_basics);
  g_test_add_func ("/icontheme/lookup-order", test_lookup_order);
  g_test_add_func ("/icontheme/generic-fallback", test_generic_fallback);
  g_test_add_func ("/icontheme/force-symbolic", test_force_symbolic);
  g_test_add_func ("/icontheme/force-regular", test_force_regular);
  g_test_add_func ("/icontheme/rtl", test_rtl);
  g_test_add_func ("/icontheme/symbolic-single-size", test_symbolic_single_size);
  g_test_add_func ("/icontheme/svg-size", test_svg_size);

  return g_test_run();
}
