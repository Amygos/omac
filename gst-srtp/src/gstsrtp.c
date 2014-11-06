/* 
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

#include "gstsrtp.h"

void srtp_perror_status(GstDebugCategory *cat, err_status_t err_status);

gboolean srtp_check_mkey_len(GstDebugCategory *cat, gchar *mkey, srtp_profile_t profile);


void srtp_perror_status(GstDebugCategory *cat, err_status_t err_status)
{
	switch(err_status)
	{
		case err_status_ok:
			GST_CAT_LOG(cat, "nothing to report\n");
			break;
		case err_status_fail:
			GST_CAT_ERROR(cat, "unspecified failure.\n");
			break;
		case err_status_bad_param:
			GST_CAT_ERROR(cat, "unsupported parameter.\n");
			break;
		case err_status_alloc_fail:
			GST_CAT_ERROR(cat, "couldn't allocate memory.\n");
			break;
		case err_status_dealloc_fail:
			GST_CAT_ERROR(cat, "couldn't deallocate properly.\n");
			break;
		case err_status_init_fail:
			GST_CAT_ERROR(cat, "couldn't initialize.\n");
			break;
		case err_status_terminus:
			GST_CAT_ERROR(cat, "can't process as much data as requested.\n");
			break;
		case err_status_auth_fail:
			GST_CAT_ERROR(cat, "authentication failure.\n");
			break;
		case err_status_cipher_fail:
			GST_CAT_ERROR(cat, "cipher failure.\n");
			break;
		case err_status_replay_fail:
			GST_CAT_ERROR(cat, "replay check failed (bad index).\n");
			break;
		case err_status_replay_old:
			GST_CAT_ERROR(cat, "replay check failed (index too old).\n");
			break;
		case err_status_algo_fail:
			GST_CAT_ERROR(cat, "algorithm failed test routine.\n");
			break;
		case err_status_no_such_op:
			GST_CAT_ERROR(cat, "unsupported operation.\n");
			break;
		case err_status_no_ctx:
			GST_CAT_ERROR(cat, "no appropriate context found.\n");
			break;
		case err_status_cant_check:
			GST_CAT_ERROR(cat, "unable to perform desired validation.\n");
			break;
		case err_status_key_expired:
			GST_CAT_ERROR(cat, "can't use key any more.\n");
			break;
		case err_status_socket_err:
			GST_CAT_ERROR(cat, "error in use of socket.\n");
			break;
		case err_status_signal_err:
			GST_CAT_ERROR(cat, "error in use POSIX signals.\n");
			break;
		case err_status_nonce_bad:
			GST_CAT_ERROR(cat, "nonce check failed.\n");
			break;
		case err_status_read_fail:
			GST_CAT_ERROR(cat, "couldn't read data.\n");
			break;
		case err_status_write_fail:
			GST_CAT_ERROR(cat, "couldn't write data.\n");
			break;
		case err_status_parse_err:
			GST_CAT_ERROR(cat, "error pasring data.\n");
			break;
		case err_status_encode_err:
			GST_CAT_ERROR(cat, "error encoding data.\n");
			break;
		case err_status_semaphore_err:
			GST_CAT_ERROR(cat, "error while using semaphores.\n");
			break;
		case err_status_pfkey_err:
			GST_CAT_ERROR(cat, "error while using pfkey.\n");
			break;
		default:
			GST_CAT_ERROR(cat, "unknow error\n");
			break;
	}
}

GType
srtp_profile_get_type (void)
{
  static GType srtp_profile_type = 0;
  static const GEnumValue srtp_profile[] = {
    {srtp_profile_aes128_cm_sha1_80, "Cipher: AES128 CM, Auth: HMAC-SHA1 80 ",
        "aes128_cm_sha1_80"},
    {srtp_profile_aes128_cm_sha1_32, "Cipher: AES128 CM, Auth: HMAC-SHA1 32 ",
        "aes128_cm_sha1_32"},
    {srtp_profile_aes256_cm_sha1_80, "Cipher: AES256 CM, Auth: HMAC-SHA1 80 ",
        "aes256_cm_sha1_80"},
    {srtp_profile_aes256_cm_sha1_32, "Cipher: AES256 CM, Auth: HMAC-SHA1 32 ",
        "aes256_cm_sha1_32"},
    {srtp_profile_null_sha1_80, "Cipher: NULL, Auth: HMAC-SHA1 80 ",
        "null_sha1_80"},
    {srtp_profile_null_sha1_32, "Cipher: NULL, Auth: HMAC-SHA1 32 ",
        "null_sha1_32"},
    {0, NULL, NULL},
  };

  if (!srtp_profile_type) {
    srtp_profile_type =
        g_enum_register_static ("srtp_profile_t", srtp_profile);
  }
  return srtp_profile_type;
}

gboolean 
srtp_check_mkey_len(GstDebugCategory *cat, gchar *mkey, srtp_profile_t profile)
{
  size_t mkey_len, key_len, salt_len;
  gboolean ret = TRUE;


  mkey_len = strlen(mkey);
  key_len = srtp_profile_get_master_key_length(profile);
  salt_len = srtp_profile_get_master_salt_length(profile);

  if(mkey_len <  key_len + salt_len){
    ret = FALSE;
    GST_CAT_ERROR(cat, "Key \"%s\" too short! Must be %d char.",
		    mkey, key_len + salt_len);
  }
  else if(mkey_len >  key_len + salt_len)
    GST_CAT_WARNING(cat, "Key  \"%s\" too long! Only first %d char will be use.",
		    mkey, key_len + salt_len);

  return ret;
}
