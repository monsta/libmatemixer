/*
 * Copyright (C) 2014 Michal Ratajsky <michal.ratajsky@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the licence, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <glib-object.h>

#include "matemixer-enums.h"
#include "matemixer-enum-types.h"
#include "matemixer-port.h"

struct _MateMixerPortPrivate
{
    gchar               *name;
    gchar               *description;
    gchar               *icon;
    guint32              priority;
    MateMixerPortStatus  status;
};

enum
{
    PROP_0,
    PROP_NAME,
    PROP_DESCRIPTION,
    PROP_ICON,
    PROP_PRIORITY,
    PROP_STATUS,
    N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = { NULL, };

static void mate_mixer_port_class_init (MateMixerPortClass *klass);
static void mate_mixer_port_init       (MateMixerPort      *port);
static void mate_mixer_port_finalize   (GObject            *object);

G_DEFINE_TYPE (MateMixerPort, mate_mixer_port, G_TYPE_OBJECT);

static void
mate_mixer_port_get_property (GObject    *object,
                              guint       param_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    MateMixerPort *port;

    port = MATE_MIXER_PORT (object);

    switch (param_id) {
    case PROP_NAME:
        g_value_set_string (value, port->priv->name);
        break;
    case PROP_DESCRIPTION:
        g_value_set_string (value, port->priv->description);
        break;
    case PROP_ICON:
        g_value_set_string (value, port->priv->icon);
        break;
    case PROP_PRIORITY:
        g_value_set_uint (value, port->priv->priority);
        break;
    case PROP_STATUS:
        g_value_set_enum (value, port->priv->status);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
        break;
    }
}

static void
mate_mixer_port_set_property (GObject      *object,
                              guint         param_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    MateMixerPort *port;

    port = MATE_MIXER_PORT (object);

    switch (param_id) {
    case PROP_NAME:
        port->priv->name = g_strdup (g_value_get_string (value));
        break;
    case PROP_DESCRIPTION:
        port->priv->description = g_strdup (g_value_get_string (value));
        break;
    case PROP_ICON:
        port->priv->icon = g_strdup (g_value_get_string (value));
        break;
    case PROP_PRIORITY:
        port->priv->priority = g_value_get_uint (value);
        break;
    case PROP_STATUS:
        port->priv->status = g_value_get_enum (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
        break;
    }
}

static void
mate_mixer_port_class_init (MateMixerPortClass *klass)
{
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize     = mate_mixer_port_finalize;
    object_class->get_property = mate_mixer_port_get_property;
    object_class->set_property = mate_mixer_port_set_property;

    properties[PROP_NAME] =
        g_param_spec_string ("name",
                             "Name",
                             "Name of the port",
                             NULL,
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_DESCRIPTION] =
        g_param_spec_string ("description",
                             "Description",
                             "Description of the port",
                             NULL,
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_ICON] =
        g_param_spec_string ("icon",
                             "Icon",
                             "Name of the port icon",
                             NULL,
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_PRIORITY] =
        g_param_spec_uint ("priority",
                           "Priority",
                           "Priority of the port",
                           0,
                           G_MAXUINT32,
                           0,
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS);

    properties[PROP_STATUS] =
        g_param_spec_enum ("status",
                           "Status",
                           "Status for the port",
                           MATE_MIXER_TYPE_PORT_STATUS,
                           MATE_MIXER_PORT_UNKNOWN_STATUS,
                           G_PARAM_CONSTRUCT_ONLY |
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPERTIES, properties);

    g_type_class_add_private (object_class, sizeof (MateMixerPortPrivate));
}

static void
mate_mixer_port_init (MateMixerPort *port)
{
    port->priv = G_TYPE_INSTANCE_GET_PRIVATE (port,
                                              MATE_MIXER_TYPE_PORT,
                                              MateMixerPortPrivate);
}

static void
mate_mixer_port_finalize (GObject *object)
{
    MateMixerPort *port;

    port = MATE_MIXER_PORT (object);

    g_free (port->priv->name);
    g_free (port->priv->description);
    g_free (port->priv->icon);

    G_OBJECT_CLASS (mate_mixer_port_parent_class)->finalize (object);
}

MateMixerPort *
mate_mixer_port_new (const gchar         *name,
                     const gchar         *description,
                     const gchar         *icon,
                     guint32              priority,
                     MateMixerPortStatus  status)
{
    return g_object_new (MATE_MIXER_TYPE_PORT,
                         "name", name,
                         "description", description,
                         "icon", icon,
                         "priority", priority,
                         "status", status,
                         NULL);
}

const gchar *
mate_mixer_port_get_name (MateMixerPort *port)
{
    g_return_val_if_fail (MATE_MIXER_IS_PORT (port), NULL);

    return port->priv->name;
}

const gchar *
mate_mixer_port_get_description (MateMixerPort *port)
{
    g_return_val_if_fail (MATE_MIXER_IS_PORT (port), NULL);

    return port->priv->description;
}

const gchar *
mate_mixer_port_get_icon (MateMixerPort *port)
{
    g_return_val_if_fail (MATE_MIXER_IS_PORT (port), NULL);

    return port->priv->icon;
}

guint32
mate_mixer_port_get_priority (MateMixerPort *port)
{
    g_return_val_if_fail (MATE_MIXER_IS_PORT (port), 0);

    return port->priv->priority;
}

MateMixerPortStatus
mate_mixer_port_get_status (MateMixerPort *port)
{
    g_return_val_if_fail (MATE_MIXER_IS_PORT (port), MATE_MIXER_PORT_UNKNOWN_STATUS);

    return port->priv->status;
}