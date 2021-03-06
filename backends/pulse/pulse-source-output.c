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
#include <libmatemixer/matemixer.h>
#include <libmatemixer/matemixer-private.h>

#include <pulse/pulseaudio.h>

#include "pulse-connection.h"
#include "pulse-helpers.h"
#include "pulse-monitor.h"
#include "pulse-source.h"
#include "pulse-source-output.h"
#include "pulse-stream.h"
#include "pulse-stream-control.h"

static void pulse_source_output_class_init (PulseSourceOutputClass *klass);
static void pulse_source_output_init       (PulseSourceOutput      *output);

G_DEFINE_TYPE (PulseSourceOutput, pulse_source_output, PULSE_TYPE_STREAM_CONTROL);

static gboolean      pulse_source_output_set_mute       (PulseStreamControl *psc,
                                                         gboolean            mute);
static gboolean      pulse_source_output_set_volume     (PulseStreamControl *psc,
                                                         pa_cvolume         *cvolume);
static PulseMonitor *pulse_source_output_create_monitor (PulseStreamControl *psc);

static void
pulse_source_output_class_init (PulseSourceOutputClass *klass)
{
    PulseStreamControlClass *control_class;

    control_class = PULSE_STREAM_CONTROL_CLASS (klass);
    control_class->set_mute       = pulse_source_output_set_mute;
    control_class->set_volume     = pulse_source_output_set_volume;
    control_class->create_monitor = pulse_source_output_create_monitor;
}

static void
pulse_source_output_init (PulseSourceOutput *output)
{
}

PulseSourceOutput *
pulse_source_output_new (PulseSource                 *source,
                         const pa_source_output_info *info)
{
    PulseSourceOutput *output;
    gchar             *name;
    const gchar       *prop;
    MateMixerAppInfo  *app_info = NULL;

    MateMixerStreamControlFlags flags = MATE_MIXER_STREAM_CONTROL_MUTE_READABLE |
                                        MATE_MIXER_STREAM_CONTROL_MUTE_WRITABLE |
                                        MATE_MIXER_STREAM_CONTROL_HAS_MONITOR;
    MateMixerStreamControlRole  role  = MATE_MIXER_STREAM_CONTROL_ROLE_UNKNOWN;

    MateMixerStreamControlMediaRole media_role = MATE_MIXER_STREAM_CONTROL_MEDIA_ROLE_UNKNOWN;

    g_return_val_if_fail (PULSE_IS_SOURCE (source), NULL);
    g_return_val_if_fail (info != NULL, NULL);

    /* Many mixer applications query the Pulse client list and use the client
     * name here, but we use the name only as an identifier, so let's avoid
     * this unnecessary overhead and use a custom name.
     * Also make sure to make the name unique by including the PulseAudio index. */
    name = g_strdup_printf ("pulse-input-control-%lu", (gulong) info->index);

#if PA_CHECK_VERSION(1, 0, 0)
    if (info->has_volume) {
        flags |=
            MATE_MIXER_STREAM_CONTROL_VOLUME_READABLE |
            MATE_MIXER_STREAM_CONTROL_HAS_DECIBEL;

        if (info->volume_writable)
            flags |= MATE_MIXER_STREAM_CONTROL_VOLUME_WRITABLE;
    }
#else
    /* Pre-1.0 PulseAudio does not include the has_volume and volume_writable
     * fields, but does include the volume info, so let's give it a try */
    flags |=
        MATE_MIXER_STREAM_CONTROL_VOLUME_READABLE |
        MATE_MIXER_STREAM_CONTROL_VOLUME_WRITABLE |
        MATE_MIXER_STREAM_CONTROL_HAS_DECIBEL;
#endif

    if (info->client != PA_INVALID_INDEX) {
        app_info = _mate_mixer_app_info_new ();

        role = MATE_MIXER_STREAM_CONTROL_ROLE_APPLICATION;

        prop = pa_proplist_gets (info->proplist, PA_PROP_APPLICATION_NAME);
        if (prop != NULL)
            _mate_mixer_app_info_set_name (app_info, prop);

        prop = pa_proplist_gets (info->proplist, PA_PROP_APPLICATION_ID);
        if (prop != NULL)
            _mate_mixer_app_info_set_id (app_info, prop);

        prop = pa_proplist_gets (info->proplist, PA_PROP_APPLICATION_VERSION);
        if (prop != NULL)
            _mate_mixer_app_info_set_version (app_info, prop);

        prop = pa_proplist_gets (info->proplist, PA_PROP_APPLICATION_ICON_NAME);
        if (prop != NULL)
            _mate_mixer_app_info_set_icon (app_info, prop);
    }

    prop = pa_proplist_gets (info->proplist, PA_PROP_MEDIA_ROLE);
    if (prop != NULL)
        media_role = pulse_convert_media_role_name (prop);

    output = g_object_new (PULSE_TYPE_SOURCE_OUTPUT,
                          "name", name,
                          "label", info->name,
                          "flags", flags,
                          "role", role,
                          "media-role", media_role,
                          "index", info->index,
                          "stream", source,
                          NULL);
    g_free (name);

    if (app_info != NULL)
        pulse_stream_control_set_app_info (PULSE_STREAM_CONTROL (output), app_info);

    pulse_source_output_update (output, info);
    return output;
}

void
pulse_source_output_update (PulseSourceOutput           *output,
                            const pa_source_output_info *info)
{
    g_return_if_fail (PULSE_IS_SOURCE_OUTPUT (output));
    g_return_if_fail (info != NULL);

    /* Let all the information update before emitting notify signals */
    g_object_freeze_notify (G_OBJECT (output));

    _mate_mixer_stream_control_set_mute (MATE_MIXER_STREAM_CONTROL (output),
                                         info->mute ? TRUE : FALSE);

#if PA_CHECK_VERSION(1, 0, 0)
    pulse_stream_control_set_channel_map (PULSE_STREAM_CONTROL (output),
                                          &info->channel_map);

    if (info->has_volume)
        pulse_stream_control_set_cvolume (PULSE_STREAM_CONTROL (output),
                                          &info->volume,
                                          0);
    else
        pulse_stream_control_set_cvolume (PULSE_STREAM_CONTROL (output),
                                          NULL,
                                          0);
#else
    pulse_stream_control_set_channel_map (PULSE_STREAM_CONTROL (output),
                                          &info->channel_map);

    pulse_stream_control_set_volume (PULSE_STREAM_CONTROL (output),
                                     &info->volume,
                                     0);
#endif

    g_object_thaw_notify (G_OBJECT (output));
}

static gboolean
pulse_source_output_set_mute (PulseStreamControl *psc, gboolean mute)
{
    g_return_val_if_fail (PULSE_IS_SOURCE_OUTPUT (psc), FALSE);

    return pulse_connection_set_source_output_mute (PULSE_STREAM_CONTROL_GET_CONNECTION (psc),
                                                    pulse_stream_control_get_index (psc),
                                                    mute);
}

static gboolean
pulse_source_output_set_volume (PulseStreamControl *psc, pa_cvolume *cvolume)
{
    g_return_val_if_fail (PULSE_IS_SOURCE_OUTPUT (psc), FALSE);
    g_return_val_if_fail (cvolume != NULL, FALSE);

    return pulse_connection_set_source_output_volume (PULSE_STREAM_CONTROL_GET_CONNECTION (psc),
                                                      pulse_stream_control_get_index (psc),
                                                      cvolume);
}

static PulseMonitor *
pulse_source_output_create_monitor (PulseStreamControl *psc)
{
    g_return_val_if_fail (PULSE_IS_SOURCE_OUTPUT (psc), NULL);

    return pulse_connection_create_monitor (PULSE_STREAM_CONTROL_GET_CONNECTION (psc),
                                            PULSE_STREAM_CONTROL_GET_STREAM_INDEX (psc),
                                            PA_INVALID_INDEX);
}
