/*
 * GStreamer
 * Copyright (C) 2006 Stefan Kost <ensonic@users.sf.net>
 * Copyright (C) 2014 Matteo Valentini  <amygos@paranoici.org>
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-resched
 *
 * FIXME:Describe resched here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! resched ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include <gst/controller/gstcontroller.h>

#include "gstresched.h"

GST_DEBUG_CATEGORY_STATIC (gst_resched_debug);
#define GST_CAT_DEFAULT gst_resched_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_DELAY,
};

/* the capabilities of the inputs and outputs.
 *
 * FIXME:describe the real formats here.
 */
static GstStaticPadTemplate sink_template =
GST_STATIC_PAD_TEMPLATE (
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("ANY")
);

static GstStaticPadTemplate src_template =
GST_STATIC_PAD_TEMPLATE (
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_STATIC_CAPS ("ANY")
);

/* debug category for fltering log messages
 *
 * FIXME:exchange the string 'Template resched' with your description
 */
#define DEBUG_INIT(bla) \
  GST_DEBUG_CATEGORY_INIT (gst_resched_debug, "resched", 0, "resched");

GST_BOILERPLATE_FULL (Gstresched, gst_resched, GstBaseTransform,
    GST_TYPE_BASE_TRANSFORM, DEBUG_INIT);

static void gst_resched_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_resched_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static GstFlowReturn gst_resched_transform_ip (GstBaseTransform * base,
    GstBuffer * outbuf);

/* GObject vmethod implementations */

static void
gst_resched_base_init (gpointer klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_set_details_simple (element_class,
    "resched",
    "Generic/Filter",
    "FIXME:Generic Template Filter",
    " <amygos@paranoici.org>>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_template));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_template));
}
	
/* initialize the resched's class */
static void
gst_resched_class_init (GstreschedClass * klass)
{
  GObjectClass *gobject_class;

  gobject_class = (GObjectClass *) klass;
  gobject_class->set_property = gst_resched_set_property;
  gobject_class->get_property = gst_resched_get_property;

  g_object_class_install_property (gobject_class, PROP_DELAY,
    g_param_spec_uint ("delay", "Delay", "Set buffer's timestamp delay",
          0, G_MAXUINT, 50, G_PARAM_READWRITE | GST_PARAM_CONTROLLABLE));

  GST_BASE_TRANSFORM_CLASS (klass)->transform_ip =
      GST_DEBUG_FUNCPTR (gst_resched_transform_ip);
}

/* initialize the new element
 * initialize instance structure
 */
static void
gst_resched_init (Gstresched *filter, GstreschedClass * klass)
{
  filter->delay = 50;
}

static void
gst_resched_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  Gstresched *filter = GST_RESCHED (object);

  switch (prop_id) {
    case PROP_DELAY:
      filter->delay = g_value_get_uint (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_resched_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gstresched *filter = GST_RESCHED (object);

  switch (prop_id) {
    case PROP_DELAY:
      g_value_set_uint (value, filter->delay);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstBaseTransform vmethod implementations */

/* this function does the actual processing
 */
static GstFlowReturn
gst_resched_transform_ip (GstBaseTransform * base, GstBuffer * outbuf)
{
  Gstresched *filter = GST_RESCHED (base);

  if (GST_CLOCK_TIME_IS_VALID (GST_BUFFER_TIMESTAMP (outbuf)))
    gst_object_sync_values (G_OBJECT (filter), GST_BUFFER_TIMESTAMP (outbuf));

  /* Set timestamp for correct stream buffer order */
  GST_BUFFER_TIMESTAMP(outbuf) = 
    (gst_clock_get_time(GST_ELEMENT(filter)->clock) - 
      (GST_ELEMENT (filter)->base_time)) +
    ((GST_MSECOND) * filter->delay);


  return GST_FLOW_OK;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
resched_init (GstPlugin * resched)
{
  /* initialize gst controller library */
  gst_controller_init(NULL, NULL);

  return gst_element_register (resched, "resched", GST_RANK_NONE,
      GST_TYPE_RESCHED);
}

/* gstreamer looks for this structure to register rescheds
 *
 * FIXME:exchange the string 'Template resched' with you resched description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "resched",
    "resched",
    resched_init,
    VERSION,
    "LGPL",
    "Open Multicast Access Control",
    "https://amygos.github.io/omac/"
)
