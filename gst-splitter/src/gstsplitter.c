/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2014 Matteo Valentini <amygos@paranoici.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
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
 * SECTION:element-splitter
 *
 * FIXME:Describe splitter here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! splitter ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstsplitter.h"

GST_DEBUG_CATEGORY_STATIC (gst_splitter_debug);
#define GST_CAT_DEFAULT gst_splitter_debug

#define SPLITTER_MODE (splitter_mode_get_type())
GType splitter_mode_get_type (void);

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_MODE,
  PROP_COUNTER,
  PROP_RAND_SEED,
  PROP_UD_RANGE
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("ANY")
    );

GST_BOILERPLATE (Gstsplitter, gst_splitter, GstElement,
    GST_TYPE_ELEMENT);

static void gst_splitter_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_splitter_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

//static gboolean gst_splitter_set_caps (GstPad * pad, GstCaps * caps);
static GstFlowReturn gst_splitter_chain (GstPad * pad, GstBuffer * buf);

static gboolean
primary_src_select (Gstsplitter *filter, GstBuffer *buf);

static gboolean
counter_mode_select (Gstsplitter *filter);

static gboolean
ud_mode_select(Gstsplitter *filter);

static gboolean
i_frame_mode_select(GstBuffer *buf);


static gboolean
is_FU_A (GstBuffer * buf);

static gboolean
is_IDR(GstBuffer * buf);

/* GObject vmethod implementations */

static void
gst_splitter_base_init (gpointer gclass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "splitter",
    "FIXME:Generic",
    "FIXME:Generic Template Element",
    "Matteo Valentini <amygos@paranoici.org>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the splitter's class */
static void
gst_splitter_class_init (GstsplitterClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_splitter_set_property;
  gobject_class->get_property = gst_splitter_get_property;

  g_object_class_install_property (gobject_class, PROP_MODE,
      g_param_spec_enum ("mode", "Mode",
          "Spliter mode", SPLITTER_MODE,
          SPLITTER_MODE_COUNTER, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, PROP_COUNTER,
      g_param_spec_uint ("counter", "Counter", "Spliter counter",
          0,  G_MAXUINT, 10, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, PROP_RAND_SEED,
      g_param_spec_uint ("rand_seed", "Rand Seed", "Seed for random ud, 0 for auto",
          0,  G_MAXUINT, 0, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, PROP_UD_RANGE,
      g_param_spec_int ("ud_range", "UD range", "UD range",
          2,  G_MAXINT32, 2, G_PARAM_READWRITE));

}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_splitter_init (Gstsplitter * filter,
    GstsplitterClass * gclass)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  //gst_pad_set_setcaps_function (filter->sinkpad,
  //                              GST_DEBUG_FUNCPTR(gst_splitter_set_caps));
  gst_pad_set_getcaps_function (filter->sinkpad,
                                GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));
  gst_pad_set_chain_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_splitter_chain));


  filter->primary_srcpad = gst_pad_new_from_static_template (&src_factory, 
		  "primary");
  gst_pad_set_getcaps_function (filter->primary_srcpad,
                                GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));

 filter->secondary_srcpad = gst_pad_new_from_static_template (&src_factory, 
		 "secondary");
 gst_pad_set_getcaps_function (filter->secondary_srcpad,
                               GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));


  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->primary_srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->secondary_srcpad);
}

static void
gst_splitter_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  Gstsplitter *filter = GST_SPLITTER (object);

  switch (prop_id) {
    case PROP_MODE:
      filter->mode = g_value_get_enum(value);
      break;
    case PROP_COUNTER:
      filter->counter_value = filter->counter = g_value_get_uint (value);
      break;
    case PROP_RAND_SEED:
      filter->seed = g_value_get_uint (value);
      if(filter->seed)
        filter->grand = g_rand_new_with_seed(filter->seed);
      else
        filter->grand = g_rand_new();
      break;
    case PROP_UD_RANGE:
      filter->ud_range = g_value_get_int (value);
      break; 
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_splitter_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gstsplitter *filter = GST_SPLITTER (object);

  switch (prop_id) {
    case PROP_MODE:
      g_value_set_enum (value, filter->mode);
      break;
    case PROP_COUNTER:
      g_value_set_uint (value, filter->counter);
      break;
    case PROP_RAND_SEED:
      g_value_set_uint (value, filter->seed);
      break;
    case PROP_UD_RANGE:
      g_value_set_int (value, filter->ud_range);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles the link with other elements */
/*static gboolean
gst_splitter_set_caps (GstPad * pad, GstCaps * caps)
{
  Gstsplitter *filter;
  GstPad *otherpad;

  filter = GST_SPLITTER (gst_pad_get_parent (pad));
  otherpad = (pad == filter->srcpad) ? filter->sinkpad : filter->srcpad;
  gst_object_unref (filter);

  return gst_pad_set_caps (otherpad, caps);
}
*/
/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_splitter_chain (GstPad * pad, GstBuffer * buf)
{
  Gstsplitter *filter;
  GstFlowReturn flow_ret;

  filter = GST_SPLITTER (GST_OBJECT_PARENT (pad));


  if(primary_src_select(filter, buf))
    flow_ret = gst_pad_push (filter->primary_srcpad, buf);
  else 
    flow_ret = gst_pad_push (filter->secondary_srcpad, buf);

  return flow_ret;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
splitter_init (GstPlugin * splitter)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template splitter' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_splitter_debug, "splitter",
      0, "Template splitter");

  return gst_element_register (splitter, "splitter", GST_RANK_NONE,
      GST_TYPE_SPLITTER);
}

static gboolean
primary_src_select (Gstsplitter *filter, GstBuffer * buf)
{
  gboolean ret = FALSE;

  switch (filter->mode) {
    case SPLITTER_MODE_COUNTER:
      ret = counter_mode_select(filter);
      break;
    case SPLITTER_MODE_UD:
      ret = ud_mode_select(filter);
      break;
    case SPLITTER_MODE_I_FRAME:
      ret = i_frame_mode_select(buf);
      break;
  }

  return ret;
}

static gboolean
counter_mode_select (Gstsplitter *filter)
{
  gboolean ret;

  if(!(filter->counter)){
    filter->counter = filter->counter_value;
    ret = FALSE;
  }
  else{
    filter->counter--;
    ret = TRUE;
  }

  return ret;
}

static gboolean
ud_mode_select(Gstsplitter *filter)
{
  gboolean ret = FALSE;

  if(g_rand_int_range(filter->grand, 0, filter->ud_range))
    ret = TRUE;

  return ret;
}

#define NRI_MASK 0x60
/*
#define NRI_0 
#define NRI_1
#define NRI_2
#define NRI_3
*/
#define NRI_VAL(nalu) (nalu[0] & NRI_MASK)


enum
{
  Type_N_IDR = 0x01,
  Type_P_A,
  Type_P_B,
  Type_P_C,
  Type_IDR,
  Type_STAP_A = 0x18,
  Type_STAP_B,
  Type_MTAP16,
  Type_MTAP24,
  Type_FU_A,
  Type_FU_B
};
#define Type_MASK 0x1f
#define Type_VAL(nalu) (nalu[0] & Type_MASK)
#define FU_Type_VAL(nalu) (nalu[1] & Type_MASK)

static gboolean
i_frame_mode_select(GstBuffer *buf)
{

  guint8 *rtp_pkt = GST_BUFFER_DATA(buf);
  guint8 *nalu;
  gboolean ret = TRUE;

  /* Skip rtp header and get pointer to Network Abstraction Layer Unit */
  nalu = rtp_pkt + 12;

  /* Determinate if NRI value is greater than zero */ 
  if(NRI_VAL(nalu)){
    /* Determinate type of NALU */
    switch(Type_VAL(nalu)){
      case Type_FU_A:
      case Type_FU_B:
        /* Determinate if fragment is IDR */
        if (FU_Type_VAL(nalu) == Type_IDR)
          ret = FALSE;
        break;
    }
  }

  return ret;
}

GType splitter_mode_get_type (void)
{
  static GType splitter_mode_type = 0;
  static const GEnumValue splitter_mode[] = {
    {SPLITTER_MODE_COUNTER, "Counter Mode", "counter_mode"},
    {SPLITTER_MODE_UD, "UD mode", "ud_mode"},
    {SPLITTER_MODE_I_FRAME, "I-Frame mode ", "i_frame_mode"},
    {0, NULL, NULL},
  };

  if (!splitter_mode_type) {
    splitter_mode_type =
        g_enum_register_static ("GstSpliterMode", splitter_mode);
  }
  return splitter_mode_type;

}

//static GstFlowReturn
//gst_spiter_counter_mode(Gstsplitter *filter, 
/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "splitter"
#endif

/* gstreamer looks for this structure to register splitters
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "splitter",
    "splitter",
    splitter_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "https://amygos.github.io/omac/"
)
