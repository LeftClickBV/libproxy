/* config-leftclick-test.c
 *
 * Copyright 2024 LeftClick Web Services B.V.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "px-manager.h"

#include "px-manager-helper.h"

typedef struct {
  const char *url;
  const char *proxy;
  gboolean success;
} ConfigLeftClickTest;

static const ConfigLeftClickTest config_leftclick_test_disabled_set[] = {
  { "https://www.example.com", "http://127.0.0.1:8080", FALSE},
};

static const ConfigLeftClickTest config_leftclick_test_invalid_set[] = {
  { "https://www.example.com", "http://127.0.0.1:8080", FALSE},
};

static const ConfigLeftClickTest config_leftclick_test_manual_set[] = {
  { "https://www.example.com", "http://127.0.0.1:8080", TRUE},
  { "http://www.example.com", "http://127.0.0.1:8080", TRUE},
  { "http://cm.leftclick.cloud", "http://127.0.0.1:8080", FALSE},
  { "ftp://www.example.com", "http://127.0.0.1:8080", FALSE},
  { "tcp://localhost:1234", "http://127.0.0.1:8080", FALSE},
};

static const ConfigLeftClickTest config_leftclick_test_pac_set[] = {
  { "https://www.example.com", "http://127.0.0.1:8080",TRUE},
};

static const ConfigLeftClickTest config_leftclick_test_wpad_set[] = {
  { "https://www.example.com", "http://127.0.0.1:8080", TRUE},
};

static void
test_config_leftclick_disabled (void)
{
  int idx;

  for (idx = 0; idx < G_N_ELEMENTS (config_leftclick_test_disabled_set); idx++) {
    g_autoptr (PxManager) manager = NULL;
    g_autoptr (GError) error = NULL;
    g_autoptr (GUri) uri = NULL;
    g_auto (GStrv) config = NULL;
    ConfigLeftClickTest test = config_leftclick_test_disabled_set[idx];
    g_autofree char *path = g_test_build_filename (G_TEST_DIST, "data", "sample-leftclick-config-disabled", NULL);

    manager = px_test_manager_new ("config-leftclick", path);
    g_clear_error (&error);

    uri = g_uri_parse (test.url, G_URI_FLAGS_NONE, &error);
    if (!uri) {
      g_warning ("Could not parse url '%s': %s", test.url, error ? error->message : "");
      g_assert_not_reached ();
    }

    config = px_manager_get_configuration (manager, uri);
    if (test.success)
      g_assert_cmpstr (config[0], ==, test.proxy);
    else
      g_assert_cmpstr (config[0], !=, test.proxy);

    g_clear_object (&manager);
  }
}

static void
test_config_leftclick_invalid (void)
{
  int idx;

  for (idx = 0; idx < G_N_ELEMENTS (config_leftclick_test_invalid_set); idx++) {
    g_autoptr (PxManager) manager = NULL;
    g_autoptr (GError) error = NULL;
    g_autoptr (GUri) uri = NULL;
    g_auto (GStrv) config = NULL;
    ConfigLeftClickTest test = config_leftclick_test_invalid_set[idx];
    g_autofree char *path = g_test_build_filename (G_TEST_DIST, "data", "sample-leftclick-config-invalid", NULL);

    manager = px_test_manager_new ("config-leftclick", path);
    g_clear_error (&error);

    uri = g_uri_parse (test.url, G_URI_FLAGS_NONE, &error);
    if (!uri) {
      g_warning ("Could not parse url '%s': %s", test.url, error ? error->message : "");
      g_assert_not_reached ();
    }

    config = px_manager_get_configuration (manager, uri);
    if (test.success)
      g_assert_cmpstr (config[0], ==, test.proxy);
    else
      g_assert_cmpstr (config[0], !=, test.proxy);

    g_clear_object (&manager);
  }
}

static void
test_config_leftclick_manual (void)
{
  int idx;

  for (idx = 0; idx < G_N_ELEMENTS (config_leftclick_test_manual_set); idx++) {
    g_autoptr (PxManager) manager = NULL;
    g_autoptr (GError) error = NULL;
    g_autoptr (GUri) uri = NULL;
    g_auto (GStrv) config = NULL;
    ConfigLeftClickTest test = config_leftclick_test_manual_set[idx];
    g_autofree char *path = g_test_build_filename (G_TEST_DIST, "data", "sample-leftclick-config-manual", NULL);

    manager = px_test_manager_new ("config-leftclick", path);
    g_clear_error (&error);

    uri = g_uri_parse (test.url, G_URI_FLAGS_NONE, &error);
    if (!uri) {
      g_warning ("Could not parse url '%s': %s", test.url, error ? error->message : "");
      g_assert_not_reached ();
    }

    config = px_manager_get_configuration (manager, uri);
    if (test.success)
      g_assert_cmpstr (config[0], ==, test.proxy);
    else
      g_assert_cmpstr (config[0], !=, test.proxy);

    g_clear_object (&manager);
  }
}

static void
test_config_leftclick_pac (void)
{
  int idx;

  for (idx = 0; idx < G_N_ELEMENTS (config_leftclick_test_pac_set); idx++) {
    g_autoptr (PxManager) manager = NULL;
    g_autoptr (GError) error = NULL;
    g_autoptr (GUri) uri = NULL;
    g_auto (GStrv) config = NULL;
    ConfigLeftClickTest test = config_leftclick_test_pac_set[idx];
    g_autofree char *path = g_test_build_filename (G_TEST_DIST, "data", "sample-leftclick-config-pac", NULL);

    manager = px_test_manager_new ("config-leftclick", path);
    g_clear_error (&error);

    uri = g_uri_parse (test.url, G_URI_FLAGS_NONE, &error);
    if (!uri) {
      g_warning ("Could not parse url '%s': %s", test.url, error ? error->message : "");
      g_assert_not_reached ();
    }

    config = px_manager_get_configuration (manager, uri);
    if (test.success)
      g_assert_cmpstr (config[0], ==, "pac+http://127.0.0.1/px-manager-sample.pac");
    else
      g_assert_cmpstr (config[0], !=, "pac+http://127.0.0.1/px-manager-sample.pac");

    g_clear_object (&manager);
  }
}


static void
test_config_leftclick_wpad (void)
{
  int idx;

  for (idx = 0; idx < G_N_ELEMENTS (config_leftclick_test_wpad_set); idx++) {
    g_autoptr (PxManager) manager = NULL;
    g_autoptr (GError) error = NULL;
    g_autoptr (GUri) uri = NULL;
    g_auto (GStrv) config = NULL;
    ConfigLeftClickTest test = config_leftclick_test_wpad_set[idx];
    g_autofree char *path = g_test_build_filename (G_TEST_DIST, "data", "sample-leftclick-config-wpad", NULL);

    manager = px_test_manager_new ("config-leftclick", path);
    g_clear_error (&error);

    uri = g_uri_parse (test.url, G_URI_FLAGS_NONE, &error);
    if (!uri) {
      g_warning ("Could not parse url '%s': %s", test.url, error ? error->message : "");
      g_assert_not_reached ();
    }

    config = px_manager_get_configuration (manager, uri);
    if (test.success)
      g_assert_cmpstr (config[0], ==, "wpad://");
    else
      g_assert_cmpstr (config[0], !=, "wpad://");

    g_clear_object (&manager);
  }
}


int
main (int    argc,
      char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/config/leftclick/disabled", test_config_leftclick_disabled);
  g_test_add_func ("/config/leftclick/invalid", test_config_leftclick_invalid);
  g_test_add_func ("/config/leftclick/manual", test_config_leftclick_manual);
  g_test_add_func ("/config/leftclick/pac", test_config_leftclick_pac);
  g_test_add_func ("/config/leftclick/wpad", test_config_leftclick_wpad);

  return g_test_run ();
}
