/* config-leftclick.c
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <gio/gio.h>
#include <glib/gmarkup.h>

#include "config-leftclick.h"

#include "glib.h"
#include "px-manager.h"
#include "px-plugin-config.h"

struct _PxConfigLeftClick {
  GObject parent_instance;
  GFileMonitor *monitor;

  char *config_file;
  gboolean available;

  const char *current_element;
  gboolean proxy_enabled;
  char *proxy_server;
  char *proxy_port;
  GStrv proxy_blacklist;
};

static void px_config_iface_init (PxConfigInterface *iface);

G_DEFINE_FINAL_TYPE_WITH_CODE (PxConfigLeftClick, px_config_leftclick,
                               G_TYPE_OBJECT,
                               G_IMPLEMENT_INTERFACE (PX_TYPE_CONFIG,
                                                      px_config_iface_init))

enum { PROP_0, PROP_CONFIG_OPTION };

static void
start_element (GMarkupParseContext  *context,
               const gchar          *element_name,
               const gchar         **attribute_names,
               const gchar         **attribute_values,
               gpointer              user_data,
               GError              **error)
{
  PxConfigLeftClick *config = (PxConfigLeftClick *)user_data;

  /* Store the current element name */
  config->current_element = element_name;
}

static void
end_element (GMarkupParseContext  *context,
             const gchar          *element_name,
             gpointer              user_data,
             GError              **error)
{
  /* No action needed on end element */
}

static void
text (GMarkupParseContext  *context,
      const gchar          *text,
      gsize                 text_length,
      gpointer              user_data,
      GError              **error)
{
  PxConfigLeftClick *config = (PxConfigLeftClick *)user_data;

  if (g_strcmp0 (config->current_element, "player_proxy") == 0) {
    if (g_strcmp0 (text, "ON") == 0) {
      config->proxy_enabled = TRUE;
    } else if (g_strcmp0 (text, "OFF") == 0) {
      config->proxy_enabled = FALSE;
    }
  } else if (g_strcmp0 (config->current_element, "player_proxy_server") == 0) {
    config->proxy_server = g_strndup (text, text_length);
  } else if (g_strcmp0 (config->current_element, "player_proxy_port") == 0) {
    config->proxy_port = g_strndup (text, text_length);
  } else if (g_strcmp0 (config->current_element, "player_proxy_blacklist") == 0) {
    /* Split the comma-separated string into a GStrv */
    gchar *blacklist_str = g_strndup (text, text_length);
    config->proxy_blacklist = g_strsplit (blacklist_str, ",", -1);
    g_free (blacklist_str);
  }
}

static void px_config_leftclick_set_config_file (PxConfigLeftClick *self,
                                                 const char        *config_file);

static void
on_file_changed (GFileMonitor      *monitor,
                 GFile             *file,
                 GFile             *other_file,
                 GFileMonitorEvent  event_type,
                 gpointer           user_data)
{
  PxConfigLeftClick *self = PX_CONFIG_LEFTCLICK (user_data);

  g_debug ("%s: Reloading configuration", __FUNCTION__);
  px_config_leftclick_set_config_file (self, g_file_get_path (file));
}

static void
px_config_leftclick_set_config_file (PxConfigLeftClick *self,
                                     const char        *config_file)
{
  g_autoptr (GFile) file = NULL;
  g_autoptr (GError) error = NULL;
  gchar *xml_data;
  GMarkupParser parser = {start_element, end_element, text, NULL, NULL};
  GMarkupParseContext *context =
    g_markup_parse_context_new (&parser, 0, self, NULL);
  gboolean result;

  g_clear_pointer (&self->config_file, g_free);
  self->config_file =
    g_strdup (config_file ? config_file : "/etc/LeftClick/config.xml");
  self->available = FALSE;

  /* Read the LeftClick configuration XML file */
  g_file_get_contents (self->config_file, &xml_data, NULL, &error);
  if (error) {
    g_debug ("%s: Could not read file %s: %s", __FUNCTION__, self->config_file,
             error->message);
    return;
  }

  g_clear_object (&self->monitor);
  file = g_file_new_for_path (self->config_file);
  self->monitor = g_file_monitor (file, G_FILE_MONITOR_NONE, NULL, &error);
  if (!self->monitor)
    g_warning ("Could not add a file monitor for %s, error: %s",
               g_file_get_uri (file), error->message);
  else
    g_signal_connect_object (G_OBJECT (self->monitor), "changed",
                             G_CALLBACK (on_file_changed), self, 0);

  /* Parse the content */
  result = g_markup_parse_context_parse (context, xml_data, -1, &error);
  if (!result) {
    g_debug ("%s: Error parsing XML: %s", __FUNCTION__, error->message);
    g_markup_parse_context_free (context);
    return;
  }

  result = g_markup_parse_context_end_parse (context, &error);
  if (!result) {
    g_debug ("%s: Error ending parsing XML: %s", __FUNCTION__, error->message);
    g_markup_parse_context_free (context);
    return;
  }

  self->available = TRUE;
}

static void px_config_leftclick_init (PxConfigLeftClick *self) {}

static void
px_config_leftclick_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  PxConfigLeftClick *config = PX_CONFIG_LEFTCLICK (object);

  switch (prop_id) {
    case PROP_CONFIG_OPTION:
      px_config_leftclick_set_config_file (config, g_value_dup_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
px_config_leftclick_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  PxConfigLeftClick *config = PX_CONFIG_LEFTCLICK (object);

  switch (prop_id) {
    case PROP_CONFIG_OPTION:
      g_value_set_string (value, config->config_file);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
px_config_leftclick_dispose (GObject *object)
{
  PxConfigLeftClick *self = PX_CONFIG_LEFTCLICK (object);

  g_clear_object (&self->monitor);
  g_clear_pointer (&self->proxy_blacklist, g_strfreev);
  g_clear_pointer (&self->config_file, g_free);

  G_OBJECT_CLASS (px_config_leftclick_parent_class)->dispose (object);
}

static void
px_config_leftclick_class_init (PxConfigLeftClickClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = px_config_leftclick_dispose;
  object_class->set_property = px_config_leftclick_set_property;
  object_class->get_property = px_config_leftclick_get_property;

  g_object_class_override_property (object_class, PROP_CONFIG_OPTION,
                                    "config-option");
}

static void
px_config_leftclick_get_config (PxConfig     *config,
                                GUri         *uri,
                                GStrvBuilder *builder)
{
  PxConfigLeftClick *self = PX_CONFIG_LEFTCLICK (config);
  const char *scheme = g_uri_get_scheme (uri);
  g_autofree char *proxy = NULL;

  if (!self->proxy_enabled)
    return;

  if (px_manager_is_ignore (uri, self->proxy_blacklist))
    return;

  if ((g_strcmp0 (scheme, "https") == 0) || (g_strcmp0 (scheme, "http") == 0)) {
    if (g_str_has_prefix (self->proxy_server, "pac+http://")) {
      proxy = g_strdup (self->proxy_server);
    } else if (g_str_has_prefix (self->proxy_server, "wpad://")) {
      proxy = g_strdup (self->proxy_server);
    } else {
      GString *tmp = g_string_new ("");

      g_string_printf (tmp, "http://%s", self->proxy_server);
      if (self->proxy_port)
        g_string_append_printf (tmp, ":%s", self->proxy_port);

      proxy = g_string_free_and_steal (tmp);
    }
  }

  if (proxy)
    px_strv_builder_add_proxy (builder, proxy);
}

static void
px_config_iface_init (PxConfigInterface *iface)
{
  iface->name = "config-leftclick";
  iface->priority = PX_CONFIG_PRIORITY_FIRST;
  iface->get_config = px_config_leftclick_get_config;
}
