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

#include <string.h>
#include <glib.h>
#include <glib-object.h>

#include "matemixer-device.h"
#include "matemixer-device-profile.h"
#include "matemixer-stream.h"
#include "matemixer-switch.h"

/**
 * SECTION:matemixer-device
 * @short_description: Hardware or software device in the sound system
 * @include: libmatemixer/matemixer.h
 */

struct _MateMixerDevicePrivate
{
    gchar                  *name;
    gchar                  *label;
    gchar                  *icon;
    GList                  *streams;
    GList                  *switches;
    GList                  *profiles;
    MateMixerDeviceProfile *profile;
};

enum {
    PROP_0,
    PROP_NAME,
    PROP_LABEL,
    PROP_ICON,
    PROP_ACTIVE_PROFILE,
    N_PROPERTIES
};

static GParamSpec *properties[N_PROPERTIES] = { NULL, };

enum {
    STREAM_ADDED,
    STREAM_REMOVED,
    SWITCH_ADDED,
    SWITCH_REMOVED,
    N_SIGNALS
};

static guint signals[N_SIGNALS] = { 0, };

static void mate_mixer_device_class_init   (MateMixerDeviceClass *klass);

static void mate_mixer_device_get_property (GObject              *object,
                                            guint                 param_id,
                                            GValue               *value,
                                            GParamSpec           *pspec);
static void mate_mixer_device_set_property (GObject              *object,
                                            guint                 param_id,
                                            const GValue         *value,
                                            GParamSpec           *pspec);

static void mate_mixer_device_init         (MateMixerDevice      *device);
static void mate_mixer_device_dispose      (GObject              *object);
static void mate_mixer_device_finalize     (GObject              *object);

G_DEFINE_ABSTRACT_TYPE (MateMixerDevice, mate_mixer_device, G_TYPE_OBJECT)

static MateMixerStream *       mate_mixer_device_real_get_stream  (MateMixerDevice *device,
                                                                   const gchar     *name);
static MateMixerSwitch *       mate_mixer_device_real_get_switch  (MateMixerDevice *device,
                                                                   const gchar     *name);
static MateMixerDeviceProfile *mate_mixer_device_real_get_profile (MateMixerDevice *device,
                                                                   const gchar     *name);

static void
mate_mixer_device_class_init (MateMixerDeviceClass *klass)
{
    GObjectClass *object_class;

    klass->get_stream  = mate_mixer_device_real_get_stream;
    klass->get_switch  = mate_mixer_device_real_get_switch;
    klass->get_profile = mate_mixer_device_real_get_profile;

    object_class = G_OBJECT_CLASS (klass);
    object_class->dispose      = mate_mixer_device_dispose;
    object_class->finalize     = mate_mixer_device_finalize;
    object_class->get_property = mate_mixer_device_get_property;
    object_class->set_property = mate_mixer_device_set_property;

    properties[PROP_NAME] =
        g_param_spec_string ("name",
                             "Name",
                             "Name of the device",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_LABEL] =
        g_param_spec_string ("label",
                             "Label",
                             "Label of the device",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_ICON] =
        g_param_spec_string ("icon",
                             "Icon",
                             "Name of the sound device icon",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_CONSTRUCT_ONLY |
                             G_PARAM_STATIC_STRINGS);

    properties[PROP_ACTIVE_PROFILE] =
        g_param_spec_object ("active-profile",
                             "Active profile",
                             "The currently active profile of the device",
                             MATE_MIXER_TYPE_DEVICE_PROFILE,
                             G_PARAM_READABLE |
                             G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPERTIES, properties);

    signals[STREAM_ADDED] =
        g_signal_new ("stream-added",
                      G_TYPE_FROM_CLASS (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (MateMixerDeviceClass, stream_added),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__STRING,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_STRING);

    signals[STREAM_REMOVED] =
        g_signal_new ("stream-removed",
                      G_TYPE_FROM_CLASS (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (MateMixerDeviceClass, stream_removed),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__STRING,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_STRING);

    signals[SWITCH_ADDED] =
        g_signal_new ("switch-added",
                      G_TYPE_FROM_CLASS (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (MateMixerDeviceClass, switch_added),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__STRING,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_STRING);

    signals[SWITCH_REMOVED] =
        g_signal_new ("switch-removed",
                      G_TYPE_FROM_CLASS (object_class),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (MateMixerDeviceClass, switch_removed),
                      NULL,
                      NULL,
                      g_cclosure_marshal_VOID__STRING,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_STRING);

    g_type_class_add_private (object_class, sizeof (MateMixerDevicePrivate));
}

static void
mate_mixer_device_get_property (GObject    *object,
                                guint       param_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
    MateMixerDevice *device;

    device = MATE_MIXER_DEVICE (object);

    switch (param_id) {
    case PROP_NAME:
        g_value_set_string (value, device->priv->name);
        break;
    case PROP_LABEL:
        g_value_set_string (value, device->priv->label);
        break;
    case PROP_ICON:
        g_value_set_string (value, device->priv->icon);
        break;
    case PROP_ACTIVE_PROFILE:
        g_value_set_object (value, device->priv->profile);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
        break;
    }
}

static void
mate_mixer_device_set_property (GObject      *object,
                                guint         param_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
    MateMixerDevice *device;

    device = MATE_MIXER_DEVICE (object);

    switch (param_id) {
    case PROP_NAME:
        /* Construct-only string */
        device->priv->name = g_strdup (g_value_get_string (value));
        break;
    case PROP_LABEL:
        /* Construct-only string */
        device->priv->label = g_strdup (g_value_get_string (value));
        break;
    case PROP_ICON:
        /* Construct-only string */
        device->priv->icon = g_strdup (g_value_get_string (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
        break;
    }
}

static void
mate_mixer_device_init (MateMixerDevice *device)
{
    device->priv = G_TYPE_INSTANCE_GET_PRIVATE (device,
                                                MATE_MIXER_TYPE_DEVICE,
                                                MateMixerDevicePrivate);
}

static void
mate_mixer_device_dispose (GObject *object)
{
    MateMixerDevice *device;

    device = MATE_MIXER_DEVICE (object);

    if (device->priv->streams != NULL) {
        g_list_free_full (device->priv->streams, g_object_unref);
        device->priv->streams = NULL;
    }
    if (device->priv->switches != NULL) {
        g_list_free_full (device->priv->switches, g_object_unref);
        device->priv->switches = NULL;
    }
    if (device->priv->profiles != NULL) {
        g_list_free_full (device->priv->profiles, g_object_unref);
        device->priv->profiles = NULL;
    }

    g_clear_object (&device->priv->profile);

    G_OBJECT_CLASS (mate_mixer_device_parent_class)->dispose (object);
}

static void
mate_mixer_device_finalize (GObject *object)
{
    MateMixerDevice *device;

    device = MATE_MIXER_DEVICE (object);

    g_free (device->priv->name);
    g_free (device->priv->label);
    g_free (device->priv->icon);

    G_OBJECT_CLASS (mate_mixer_device_parent_class)->finalize (object);
}

/**
 * mate_mixer_device_get_name:
 * @device: a #MateMixerDevice
 */
const gchar *
mate_mixer_device_get_name (MateMixerDevice *device)
{
    g_return_val_if_fail (MATE_MIXER_IS_DEVICE (device), NULL);

    return device->priv->name;
}

/**
 * mate_mixer_device_get_description:
 * @device: a #MateMixerDevice
 */
const gchar *
mate_mixer_device_get_label (MateMixerDevice *device)
{
    g_return_val_if_fail (MATE_MIXER_IS_DEVICE (device), NULL);

    return device->priv->label;
}

/**
 * mate_mixer_device_get_icon:
 * @device: a #MateMixerDevice
 */
const gchar *
mate_mixer_device_get_icon (MateMixerDevice *device)
{
    g_return_val_if_fail (MATE_MIXER_IS_DEVICE (device), NULL);

    return device->priv->icon;
}

/**
 * mate_mixer_device_get_profile:
 * @device: a #MateMixerDevice
 * @name: a profile name
 */
MateMixerDeviceProfile *
mate_mixer_device_get_profile (MateMixerDevice *device, const gchar *name)
{
    return MATE_MIXER_DEVICE_GET_CLASS (device)->get_profile (device, name);
}

/**
 * mate_mixer_device_get_stream:
 * @device: a #MateMixerDevice
 * @name: a profile name
 */
MateMixerStream *
mate_mixer_device_get_stream (MateMixerDevice *device, const gchar *name)
{
    return MATE_MIXER_DEVICE_GET_CLASS (device)->get_stream (device, name);
}

/**
 * mate_mixer_device_get_switch:
 * @device: a #MateMixerDevice
 * @name: a profile name
 */
MateMixerSwitch *
mate_mixer_device_get_switch (MateMixerDevice *device, const gchar *name)
{
    return MATE_MIXER_DEVICE_GET_CLASS (device)->get_switch (device, name);
}

/**
 * mate_mixer_device_list_streams:
 * @device: a #MateMixerDevice
 */
const GList *
mate_mixer_device_list_streams (MateMixerDevice *device)
{
    g_return_val_if_fail (MATE_MIXER_IS_DEVICE (device), NULL);

    if (device->priv->streams == NULL) {
        MateMixerDeviceClass *klass = MATE_MIXER_DEVICE_GET_CLASS (device);

        if (klass->list_streams != NULL)
            device->priv->streams = klass->list_streams (device);
    }

    return (const GList *) device->priv->streams;
}

/**
 * mate_mixer_device_list_switches:
 * @device: a #MateMixerDevice
 */
const GList *
mate_mixer_device_list_switches (MateMixerDevice *device)
{
    g_return_val_if_fail (MATE_MIXER_IS_DEVICE (device), NULL);

    if (device->priv->switches == NULL) {
        MateMixerDeviceClass *klass = MATE_MIXER_DEVICE_GET_CLASS (device);

        if (klass->list_switches != NULL)
            device->priv->switches = klass->list_switches (device);
    }

    return (const GList *) device->priv->switches;
}

/**
 * mate_mixer_device_list_profiles:
 * @device: a #MateMixerDevice
 */
const GList *
mate_mixer_device_list_profiles (MateMixerDevice *device)
{
    g_return_val_if_fail (MATE_MIXER_IS_DEVICE (device), NULL);

    if (device->priv->profiles == NULL) {
        MateMixerDeviceClass *klass = MATE_MIXER_DEVICE_GET_CLASS (device);

        if (klass->list_profiles != NULL)
            device->priv->profiles = klass->list_profiles (device);
    }

    return (const GList *) device->priv->profiles;
}

/**
 * mate_mixer_device_get_active_profile:
 * @device: a #MateMixerDevice
 */
MateMixerDeviceProfile *
mate_mixer_device_get_active_profile (MateMixerDevice *device)
{
    g_return_val_if_fail (MATE_MIXER_IS_DEVICE (device), NULL);

    return device->priv->profile;
}

/**
 * mate_mixer_device_set_active_profile:
 * @device: a #MateMixerDevice
 * @profile: a #MateMixerDeviceProfile
 */
gboolean
mate_mixer_device_set_active_profile (MateMixerDevice        *device,
                                      MateMixerDeviceProfile *profile)
{
    g_return_val_if_fail (MATE_MIXER_IS_DEVICE (device), FALSE);
    g_return_val_if_fail (MATE_MIXER_IS_DEVICE_PROFILE (profile), FALSE);

    if (profile != device->priv->profile) {
        MateMixerDeviceClass *klass;

        klass = MATE_MIXER_DEVICE_GET_CLASS (device);

        if (klass->set_active_profile == NULL ||
            klass->set_active_profile (device, profile) == FALSE)
            return FALSE;

        if (G_LIKELY (device->priv->profile != NULL))
            g_object_unref (device->priv->profile);

        device->priv->profile = g_object_ref (profile);
    }

    return TRUE;
}

static MateMixerStream *
mate_mixer_device_real_get_stream (MateMixerDevice *device, const gchar *name)
{
    const GList *list;

    g_return_val_if_fail (MATE_MIXER_IS_DEVICE (device), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    list = mate_mixer_device_list_streams (device);
    while (list != NULL) {
        MateMixerStream *stream = MATE_MIXER_STREAM (list->data);

        if (strcmp (name, mate_mixer_stream_get_name (stream)) == 0)
            return stream;

        list = list->next;
    }
    return NULL;
}

static MateMixerSwitch *
mate_mixer_device_real_get_switch (MateMixerDevice *device, const gchar *name)
{
    const GList *list;

    g_return_val_if_fail (MATE_MIXER_IS_DEVICE (device), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    list = mate_mixer_device_list_switches (device);
    while (list != NULL) {
        MateMixerSwitch *swtch = MATE_MIXER_SWITCH (list->data);

        if (strcmp (name, mate_mixer_switch_get_name (swtch)) == 0)
            return swtch;

        list = list->next;
    }
    return NULL;
}

static MateMixerDeviceProfile *
mate_mixer_device_real_get_profile (MateMixerDevice *device, const gchar *name)
{
    const GList *list;

    g_return_val_if_fail (MATE_MIXER_IS_DEVICE (device), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    list = mate_mixer_device_list_profiles (device);
    while (list != NULL) {
        MateMixerDeviceProfile *profile = MATE_MIXER_DEVICE_PROFILE (list->data);

        if (strcmp (name, mate_mixer_device_profile_get_name (profile)) == 0)
            return profile;

        list = list->next;
    }
    return NULL;
}
