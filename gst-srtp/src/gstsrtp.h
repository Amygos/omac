#include <srtp/srtp.h>
#include <srtp/srtp_priv.h>
#include <gst/gst.h>

#ifndef __GST_SRTP_H__
#define __GST_SRTP_H__

G_BEGIN_DECLS
extern void srtp_perror_status(GstDebugCategory *cat, err_status_t err_status);

extern gboolean srtp_check_mkey_len(GstDebugCategory *cat, gchar *mkey, srtp_profile_t profile);

#define SRTP_PROFILE (srtp_profile_get_type())
extern GType srtp_profile_get_type (void);

G_END_DECLS

#endif /* __GST_SRTP_H__ */
