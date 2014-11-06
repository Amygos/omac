/*
 *
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
 * SECTION:element-srtp_enc
 *
 * FIXME:Describe srtp_enc here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! srtp_enc ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstsrtpenc.h"


GST_DEBUG_CATEGORY_STATIC (gst_srtpenc_debug);
#define GST_CAT_DEFAULT gst_srtpenc_debug
#define DEFAULT_MASTER_KEY      NULL

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_PROFILE,
  PROP_SSRC,
  PROP_MKEY
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

GST_BOILERPLATE (Gstsrtpenc, gst_srtpenc, GstElement,
    GST_TYPE_ELEMENT);


static void gst_srtpenc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_srtpenc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static void gst_srtpenc_constructed(GObject *object);

static gboolean gst_srtpenc_set_caps (GstPad * pad, GstCaps * caps);
static GstFlowReturn gst_srtpenc_chain (GstPad * pad, GstBuffer * buf);

static GstStateChangeReturn gst_srtpenc_state_change (GstElement *element,
   GstStateChange transition);

static void srtp_event_handler(struct srtp_event_data_t *data);


/* GObject vmethod implementations */

static void
gst_srtpenc_base_init (gpointer gclass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "srtpenc",
    "FIXME:Generic",
    "FIXME:Generic Template Element",
    "Matteo Valentini <amygos@paranoici.org>");

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&sink_factory));
}

/* initialize the srtp_enc's class */
static void
gst_srtpenc_class_init (GstsrtpencClass * klass)
{	
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_srtpenc_set_property;
  gobject_class->get_property = gst_srtpenc_get_property;

  gstelement_class->change_state = gst_srtpenc_state_change;

  g_object_class_install_property (gobject_class, PROP_PROFILE,
      g_param_spec_enum ("profile", "Profile",
          "Set SRTP profile", SRTP_PROFILE,
          srtp_profile_aes128_cm_sha1_80, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, PROP_SSRC,
      g_param_spec_uint ("ssrc", "Ssrc",
          "Set specific ssrc", 0,4294967295,
          0, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, PROP_MKEY,
      g_param_spec_string ("key", "Key", "Master key",
          DEFAULT_MASTER_KEY, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_srtpenc_init (Gstsrtpenc * filter,
    GstsrtpencClass * gclass)
{


  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_setcaps_function (filter->sinkpad,
                                GST_DEBUG_FUNCPTR(gst_srtpenc_set_caps));
  gst_pad_set_getcaps_function (filter->sinkpad,
                                GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));
  gst_pad_set_chain_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_srtpenc_chain));

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  gst_pad_set_getcaps_function (filter->srcpad,
                                GST_DEBUG_FUNCPTR(gst_pad_proxy_getcaps));

  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);
  filter->silent = FALSE;

  /*Default ssrc type */
  filter->policy.ssrc.type = ssrc_any_outbound;

  /*default mkey*/
  filter->key = DEFAULT_MASTER_KEY;
}


static void
gst_srtpenc_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  Gstsrtpenc *filter = GST_SRTP_ENC (object);

  switch (prop_id) {
    case PROP_PROFILE:
      filter->profile = g_value_get_enum(value);
      break;
    case PROP_SSRC:
      filter->policy.ssrc.value = g_value_get_uint(value);
      filter->policy.ssrc.type = ssrc_specific;
      break;
    case PROP_MKEY:
      filter->key = g_value_dup_string(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_srtpenc_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  Gstsrtpenc *filter = GST_SRTP_ENC (object);

  switch (prop_id) {
    case PROP_PROFILE:
      g_value_set_enum (value, filter->profile);
      break;
    case PROP_SSRC:
      g_value_set_uint (value, filter->policy.ssrc.value);
      break;
    case PROP_MKEY:
      g_value_set_string (value, filter->key);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles the link with other elements */
static gboolean
gst_srtpenc_set_caps (GstPad * pad, GstCaps * caps)
{
  Gstsrtpenc *filter;
  GstPad *otherpad;

  filter = GST_SRTP_ENC (gst_pad_get_parent (pad));
  otherpad = (pad == filter->srcpad) ? filter->sinkpad : filter->srcpad;
  gst_object_unref (filter);

  return gst_pad_set_caps (otherpad, caps);
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_srtpenc_chain (GstPad * pad, GstBuffer * buf)
{
  Gstsrtpenc *filter;
  GstFlowReturn ret;
  err_status_t srtp_ret;
  guint new_size;
  GstBuffer *newbuf;

  int old_size = GST_BUFFER_SIZE(buf); 

  filter = GST_SRTP_ENC (GST_OBJECT_PARENT (pad));

  newbuf = gst_buffer_copy(buf);
  
  gst_buffer_unref(buf);
  if((filter->policy.rtp.sec_serv == sec_serv_auth) ||
      (filter->policy.rtp.sec_serv == sec_serv_conf_and_auth)){
    GST_BUFFER_DATA(newbuf) =
      GST_BUFFER_MALLOCDATA(newbuf) = 
        g_realloc(GST_BUFFER_MALLOCDATA(newbuf),
            GST_BUFFER_SIZE(newbuf) + SRTP_MAX_TRAILER_LEN );
  }


  srtp_ret = srtp_protect(filter->session,
		  GST_BUFFER_MALLOCDATA(newbuf), &(GST_BUFFER_SIZE(newbuf)));
  if(srtp_ret){
    /* TODO: put proper error message */
    printf("<-----\n");
    srtp_perror_status(gst_srtpenc_debug, srtp_ret);
    printf("RTP sequence number: %d\n",
        ntohs((((srtp_hdr_t *)GST_BUFFER_MALLOCDATA(newbuf))->seq)));
    printf("RTP mark: %d\n",
        ntohs((((srtp_hdr_t *)GST_BUFFER_MALLOCDATA(newbuf))->m)));
    printf("old size:%d\n",old_size);
    printf("new size:%d\n",GST_BUFFER_SIZE(newbuf));
    printf("data:%x\n",GST_BUFFER_DATA(newbuf));
    printf("mallocdata:%x\n",GST_BUFFER_MALLOCDATA(newbuf));
    printf("----->\n");

    gst_buffer_unref(newbuf);
    ret = GST_FLOW_OK;
  }
  else
   ret = gst_pad_push (filter->srcpad, newbuf);


  return ret;
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
srtp_enc_init (GstPlugin * srtp_enc)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template srtp_enc' with your description
   */
  GST_DEBUG_CATEGORY_INIT (gst_srtpenc_debug, "srtp_enc",
      0, "Template srtp_enc");

  return gst_element_register (srtp_enc, "srtp_enc", GST_RANK_NONE,
      GST_TYPE_SRTP_ENC);
}


static GstStateChangeReturn
gst_srtpenc_state_change(GstElement *element, GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  Gstsrtpenc *filter = GST_SRTP_ENC (element);
  GObjectClass *klass = G_OBJECT_GET_CLASS (element);

  static gboolean libsrtp_init = FALSE, libsrtp_shutdown = FALSE;
  
  err_status_t srtp_ret;

  switch(transition){
    case GST_STATE_CHANGE_NULL_TO_READY:
      GST_CLASS_LOCK(GST_OBJECT_CLASS (klass));
      if(!libsrtp_init){
        if((srtp_ret = srtp_init())){
           /* srtp_init error */
           ret = GST_STATE_CHANGE_FAILURE;
           srtp_perror_status(gst_srtpenc_debug, srtp_ret);
        }
        libsrtp_init = TRUE;
        GST_CLASS_UNLOCK(GST_OBJECT_CLASS (klass));
      }
      else
        GST_CLASS_UNLOCK(GST_OBJECT_CLASS (klass));
      break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
      /* set default rtcp policy, but not use*/
      crypto_policy_set_rtcp_default(&(filter->policy.rtcp));
      filter->policy.next = NULL;
      if(filter->profile != srtp_profile_reserved){
        crypto_policy_set_from_profile_for_rtp(&(filter->policy.rtp),
            filter->profile);
	if (filter->key && srtp_check_mkey_len(gst_srtpenc_debug, filter->key, filter->profile)){
            filter->policy.key = (uint8_t *)filter->key;
        //  filter->policy.key = (guchar *) GST_BUFFER_DATA (filter->key);
          if((srtp_ret = srtp_create(&(filter->session), &(filter->policy)))){
	    /* srtp_create error */
            ret = GST_STATE_CHANGE_FAILURE;
            srtp_perror_status(gst_srtpenc_debug, srtp_ret);
          }
        }
	else{
          /* mkey error */
          ret = GST_STATE_CHANGE_FAILURE;
        }
      }
      else{
	/* profile error */
        ret = GST_STATE_CHANGE_FAILURE;
         srtp_perror_status(gst_srtpenc_debug, srtp_ret);
      }
      break;
    default:
      break;
  }

  if(ret != GST_STATE_CHANGE_FAILURE){
    ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
    if(ret != GST_STATE_CHANGE_FAILURE){
      switch (transition) {
        case GST_STATE_CHANGE_PAUSED_TO_READY:
          if((srtp_ret = srtp_dealloc(filter->session))){
            /*srtp_dealloc error*/
            ret = GST_STATE_CHANGE_FAILURE;
            g_free(filter->key);
            srtp_perror_status(gst_srtpenc_debug, srtp_ret);
          }
          else
            g_free(filter->key);
	  break;
        case GST_STATE_CHANGE_READY_TO_NULL:
          GST_CLASS_LOCK(GST_OBJECT_CLASS (klass));
          if(!libsrtp_shutdown){
            if((srtp_ret = srtp_shutdown())){
              /*srtp_shoutdown error*/
              ret = GST_STATE_CHANGE_FAILURE;
              srtp_perror_status(gst_srtpenc_debug, srtp_ret);
            }
            libsrtp_shutdown = TRUE;
            GST_CLASS_UNLOCK(GST_OBJECT_CLASS (klass));
	  }
          else
            GST_CLASS_UNLOCK(GST_OBJECT_CLASS (klass));
	break;
	default:
	  break;
      }
    }
  }
  
  return ret;
}
//static void srtp_event_handler(struct srtp_event_data_t *data)
//{
//	printf("event: %d\n", data->event);
//}


/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "srtp_enc"
#endif

/* gstreamer looks for this structure to register srtp_encs
 *
 * exchange the string 'Template srtp_enc' with your srtp_enc description
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "srtp_enc",
    "srtp_enc",
    srtp_enc_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "https://amygos.github.io/omac/"
)
