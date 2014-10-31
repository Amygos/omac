/* 
 * GStreamer
 * Copyright (C) 2006 Stefan Kost <ensonic@users.sf.net>
 * Copyright (C) 2014 Matteo Valentini <amygos@paranoici.org>>
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
 
#ifndef __GST_RESCHED_H__
#define __GST_RESCHED_H__

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>

G_BEGIN_DECLS

#define GST_TYPE_RESCHED \
  (gst_resched_get_type())
#define GST_RESCHED(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RESCHED,Gstresched))
#define GST_RESCHED_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RESCHED,GstreschedClass))
#define GST_IS_RESCHED(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RESCHED))
#define GST_IS_RESCHED_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RESCHED))

typedef struct _Gstresched      Gstresched;
typedef struct _GstreschedClass GstreschedClass;

struct _Gstresched {
  GstBaseTransform element;

  guint delay;

};

struct _GstreschedClass {
  GstBaseTransformClass parent_class;
};

GType gst_resched_get_type (void);

G_END_DECLS

#endif /* __GST_RESCHED_H__ */
