/*
 * Copyright (C) 2016 Wentao Shang
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_ndn_encoding
 * @{
 *
 * @file
 *
 * @author  Wentao Shang <wentaoshang@gmail.com>
 */
#include <stdlib.h>
#include <string.h>

#include "hashes/sha256.h"
#include "net/gnrc/nettype.h"
#include "uECC.h"

#include "net/ndn/encoding/data.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

ndn_shared_block_t* ndn_data_create(ndn_block_t* name,
				    ndn_metainfo_t* metainfo,
				    ndn_block_t* content,
				    uint8_t sig_type,
				    ndn_block_t* key_name,
				    const unsigned char* key,
				    size_t key_len)
{
    if (name == NULL || name->buf == NULL || name->len <= 0 ||
	metainfo == NULL || content == NULL || content->buf == NULL ||
	content->len < 0)
	return NULL;

    if (sig_type != NDN_SIG_TYPE_DIGEST_SHA256 &&
	sig_type != NDN_SIG_TYPE_ECDSA_SHA256 &&
	sig_type != NDN_SIG_TYPE_HMAC_SHA256)
	return NULL;

    if (sig_type != NDN_SIG_TYPE_DIGEST_SHA256 && key == NULL)
	return NULL;

    if (sig_type == NDN_SIG_TYPE_ECDSA_SHA256 && key_len != 32)
	return NULL;

    if (key != NULL && key_len <= 0)
	return NULL;

    int ml = ndn_metainfo_total_length(metainfo);
    if (ml <= 0) return NULL;

    int cl = ndn_block_total_length(NDN_TLV_CONTENT, content->len);

    int kl = 0;
    if (key_name != NULL)
	kl = ndn_block_total_length(NDN_TLV_KEY_LOCATOR, key_name->len);

    int dl = name->len + ml + cl
	+ ndn_block_total_length(NDN_TLV_SIGNATURE_INFO, 3 + kl);
    if (sig_type == NDN_SIG_TYPE_ECDSA_SHA256)
	// ecc p256 signature length is 64 bytes (plus 2 byte header)
	dl += 66;
    else
	// sha256 and hmac signature length is 32 bytes (plus 2 byte header)
	dl += 34;

    ndn_block_t data;
    data.len = ndn_block_total_length(NDN_TLV_DATA, dl);
    uint8_t* buf = (uint8_t*)malloc(data.len);
    if (buf == NULL) {
	DEBUG("ndn_encoding: cannot allocate memory for data block\n");
	return NULL;
    }
    data.buf = buf;

    int l, r = data.len;
    // Write data type and length
    buf[0] = NDN_TLV_DATA;
    l = ndn_block_put_var_number(dl, buf + 1, r - 1);
    buf += l + 1;
    r -= l + 1;
    assert(r == dl);

    // Write name
    memcpy(buf, name->buf, name->len);
    buf += name->len;
    r -= name->len;

    // Write metainfo
    ndn_metainfo_wire_encode(metainfo, buf, ml);
    buf += ml;
    r -= ml;

    // Write content
    buf[0] = NDN_TLV_CONTENT;
    l = ndn_block_put_var_number(content->len, buf + 1, r - 1);
    buf += l + 1;
    r -= l + 1;
    memcpy(buf, content->buf, content->len);
    buf += content->len;
    r -= content->len;

    // Write signature info
    buf[0] = NDN_TLV_SIGNATURE_INFO;
    l = ndn_block_put_var_number(3 + kl, buf + 1, r - 1);
    buf += l + 1;
    r -= l + 1;

    // Write signature type
    buf[0] = NDN_TLV_SIGNATURE_TYPE;
    buf[1] = 1;
    buf[2] = sig_type;
    buf += 3;
    r -= 3;

    // Write key locator
    if (key_name != NULL) {
	assert(kl > 0);
	buf[0] = NDN_TLV_KEY_LOCATOR;
	l = ndn_block_put_var_number(key_name->len, buf + 1, r - 1);
	assert(kl == 1 + l + key_name->len);
	buf += l + 1;
	r -= l + 1;
	memcpy(buf, key_name->buf, key_name->len);
	buf += key_name->len;
	r -= key_name->len;
    }

    // Write signature value
    buf[0] = NDN_TLV_SIGNATURE_VALUE;

    switch (sig_type) {
	case NDN_SIG_TYPE_DIGEST_SHA256:
	    buf[1] = 32;
	    sha256(data.buf + 2, dl - 34, buf + 2);
	    break;

	case NDN_SIG_TYPE_HMAC_SHA256:
	    buf[1] = 32;
	    hmac_sha256(key, key_len, (const unsigned*)(data.buf + 2),
			dl - 34, buf + 2);
	    break;

	case NDN_SIG_TYPE_ECDSA_SHA256:
	{
	    buf[1] = 64;
	    uint8_t h[32] = {0};
	    sha256(data.buf + 2, dl - 66, h);
	    uECC_Curve curve = uECC_secp256r1();
	    if (uECC_sign(key, h, sizeof(h), buf + 2, curve) == 0) {
		free(buf);
		return NULL;
	    }
	}
	break;

	default:
	    break;
    }

    ndn_shared_block_t* sd = ndn_shared_block_create_by_move(&data);
    if (sd == NULL) {
	free((void*)data.buf);
	return NULL;
    }
    return sd;
}

ndn_shared_block_t* ndn_data_create2(ndn_name_t* name,
				     ndn_metainfo_t* metainfo,
				     ndn_block_t* content,
				     uint8_t sig_type,
				     ndn_name_t* key_name,
				     const unsigned char* key,
				     size_t key_len)
{
    if (name == NULL || metainfo == NULL || content == NULL)
	return NULL;

    if (content->buf == NULL || content->len < 0)
	return NULL;


    if (sig_type != NDN_SIG_TYPE_DIGEST_SHA256 &&
	sig_type != NDN_SIG_TYPE_ECDSA_SHA256 &&
	sig_type != NDN_SIG_TYPE_HMAC_SHA256)
	return NULL;

    if (sig_type != NDN_SIG_TYPE_DIGEST_SHA256 && key == NULL)
	return NULL;

    if (sig_type == NDN_SIG_TYPE_ECDSA_SHA256 && key_len != 32)
	return NULL;

    if (key != NULL && key_len <= 0)
	return NULL;

    int nl = ndn_name_total_length(name);
    if (nl <= 0) return NULL;

    int ml = ndn_metainfo_total_length(metainfo);
    if (ml <= 0) return NULL;

    int cl = ndn_block_total_length(NDN_TLV_CONTENT, content->len);

    int kl = 0;
    int knl = 0;
    if (key_name != NULL) {
	knl = ndn_name_total_length(key_name);
	if (knl <= 0) return NULL;
	kl = ndn_block_total_length(NDN_TLV_KEY_LOCATOR, knl);
    }

    int dl = nl + ml + cl
	+ ndn_block_total_length(NDN_TLV_SIGNATURE_INFO, 3 + kl);
    if (sig_type == NDN_SIG_TYPE_ECDSA_SHA256)
	// ecc p256 signature length is 64 bytes (plus 2 byte header)
	dl += 66;
    else
	// sha256 and hmac signature length is 32 bytes (plus 2 byte header)
	dl += 34;

    ndn_block_t data;
    data.len = ndn_block_total_length(NDN_TLV_DATA, dl);
    uint8_t* buf = (uint8_t*)malloc(data.len);
    if (buf == NULL) {
	DEBUG("ndn_encoding: cannot allocate memory for data block\n");
	return NULL;
    }
    data.buf = buf;

    int l, r = data.len;
    // Write data type and length
    buf[0] = NDN_TLV_DATA;
    l = ndn_block_put_var_number(dl, buf + 1, r - 1);
    buf += l + 1;
    r -= l + 1;
    assert(r == dl);

    // Write name
    ndn_name_wire_encode(name, buf, nl);
    buf += nl;
    r -= nl;

    // Write metainfo
    ndn_metainfo_wire_encode(metainfo, buf, ml);
    buf += ml;
    r -= ml;

    // Write content
    buf[0] = NDN_TLV_CONTENT;
    l = ndn_block_put_var_number(content->len, buf + 1, r - 1);
    buf += l + 1;
    r -= l + 1;
    memcpy(buf, content->buf, content->len);
    buf += content->len;
    r -= content->len;

    // Write signature info
    buf[0] = NDN_TLV_SIGNATURE_INFO;
    l = ndn_block_put_var_number(3 + kl, buf + 1, r - 1);
    buf += l + 1;
    r -= l + 1;

    // Write signature type
    buf[0] = NDN_TLV_SIGNATURE_TYPE;
    buf[1] = 1;
    buf[2] = sig_type;
    buf += 3;
    r -= 3;

    // Write key locator
    if (key_name != NULL) {
	assert(kl > 0);
	buf[0] = NDN_TLV_KEY_LOCATOR;
	l = ndn_block_put_var_number(knl, buf + 1, r - 1);
	assert(kl == 1 + l + knl);
	buf += l + 1;
	r -= l + 1;
	ndn_name_wire_encode(key_name, buf, knl);
	buf += knl;
	r -= knl;
    }

    // Write signature value
    buf[0] = NDN_TLV_SIGNATURE_VALUE;

    switch (sig_type) {
	case NDN_SIG_TYPE_DIGEST_SHA256:
	    buf[1] = 32;
	    sha256(data.buf + 2, dl - 34, buf + 2);
	    break;

	case NDN_SIG_TYPE_HMAC_SHA256:
	    buf[1] = 32;
	    hmac_sha256(key, key_len, (const unsigned*)(data.buf + 2),
			dl - 34, buf + 2);
	    break;

	case NDN_SIG_TYPE_ECDSA_SHA256:
	{
	    buf[1] = 64;
	    uint8_t h[32] = {0};
	    sha256(data.buf + 2, dl - 66, h);
	    uECC_Curve curve = uECC_secp256r1();
	    if (uECC_sign(key, h, sizeof(h), buf + 2, curve) == 0) {
		free(buf);
		return NULL;
	    }
	}
	break;

	default:
	    break;
    }

    ndn_shared_block_t* sd = ndn_shared_block_create_by_move(&data);
    if (sd == NULL) {
	free((void*)data.buf);
	return NULL;
    }
    return sd;
}

int ndn_data_get_name(ndn_block_t* block, ndn_block_t* name)
{
    if (name == NULL || block == NULL) return -1;

    const uint8_t* buf = block->buf;
    int len = block->len;
    uint32_t num;
    int l;

    /* read data type */
    if (*buf != NDN_TLV_DATA) return -1;
    buf += 1;
    len -= 1;

    /* read data length */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l;
    len -= l;

    if ((int)num > len) return -1;  // incomplete packet

    /* read name type */
    if (*buf != NDN_TLV_NAME) return -1;
    buf += 1;
    len -= 1;

    /* read name length */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;

    if ((int)num > len - l)  // name block is incomplete
	return -1;

    name->buf = buf - 1;
    name->len = (int)num + l + 1;
    return 0;
}

int ndn_data_get_metainfo(ndn_block_t* block, ndn_metainfo_t* meta)
{
    if (block == NULL || meta == NULL) return -1;

    const uint8_t* buf = block->buf;
    int len = block->len;
    uint32_t num;
    int l;

    /* read data type */
    if (*buf != NDN_TLV_DATA) return -1;
    buf += 1;
    len -= 1;

    /* read data length */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l;
    len -= l;

    if ((int)num > len) return -1;  // incomplete packet

    /* read name type */
    if (*buf != NDN_TLV_NAME) return -1;
    buf += 1;
    len -= 1;

    /* read name length and skip value */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l + (int)num;
    len -= l + (int)num;

    if (ndn_metainfo_from_block(buf, len, meta) <= 0) return -1;
    else return 0;
}

int ndn_data_get_content(ndn_block_t* block, ndn_block_t* content)
{
    if (block == NULL || content == NULL) return -1;

    const uint8_t* buf = block->buf;
    int len = block->len;
    uint32_t num;
    int l;

    /* read data type */
    if (*buf != NDN_TLV_DATA) return -1;
    buf += 1;
    len -= 1;

    /* read data length */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l;
    len -= l;

    if ((int)num > len) return -1;  // incomplete packet

    /* read name type */
    if (*buf != NDN_TLV_NAME) return -1;
    buf += 1;
    len -= 1;

    /* read name length and skip value */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l + (int)num;
    len -= l + (int)num;

    /* read metainfo type */
    if (*buf != NDN_TLV_METAINFO) return -1;
    buf += 1;
    len -= 1;

    /* read metainfo length and skip value */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l + (int)num;
    len -= l + (int)num;

    /* read content type */
    if (*buf != NDN_TLV_CONTENT) return -1;
    buf += 1;
    len -= 1;

    /* read content length */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;

    if ((int)num > len - l)  // content block is incomplete
	return -1;

    content->buf = buf - 1;
    content->len = (int)num + l + 1;
    return 0;
}

int ndn_data_get_key_locator(ndn_block_t* block, ndn_block_t* key_name)
{
    if (block == NULL || key_name == NULL) return -1;

    const uint8_t* buf = block->buf;
    int len = block->len;
    uint32_t num;
    int l;

    /* read data type */
    if (*buf != NDN_TLV_DATA) return -1;
    buf += 1;
    len -= 1;

    /* read data length */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l;
    len -= l;

    if ((int)num > len) return -1;  // incomplete packet

    /* read name type */
    if (*buf != NDN_TLV_NAME) return -1;
    buf += 1;
    len -= 1;

    /* read name length and skip value */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l + (int)num;
    len -= l + (int)num;

    /* read metainfo type */
    if (*buf != NDN_TLV_METAINFO) return -1;
    buf += 1;
    len -= 1;

    /* read metainfo length and skip value */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l + (int)num;
    len -= l + (int)num;

    /* read content type */
    if (*buf != NDN_TLV_CONTENT) return -1;
    buf += 1;
    len -= 1;

    /* read content length and skip value */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l + (int)num;
    len += l + (int)num;

    // read signature info type
    if (*buf != NDN_TLV_SIGNATURE_INFO) return -1;
    buf += 1;
    len -= 1;

    // read signature info length
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l;
    len -= l;

    // read signature type type
    if (*buf != NDN_TLV_SIGNATURE_TYPE) return -1;
    buf += 1;
    len -= 1;

    // read signature type length and skip value
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l + (int)num;
    len -= l + (int)num;

    // read key locator type
    if (*buf != NDN_TLV_KEY_LOCATOR) return -2;
    buf += 1;
    len -= 1;

    // read key locator length
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l;
    len -= l;

    // read key name type
    if (*buf != NDN_TLV_NAME) return -3;
    buf += 1;
    len -= 1;

    // read key name length
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    if (len < (int)num)  // invalid length
	return -1;

    key_name->buf = buf - 1;
    key_name->len = (int)num + l + 1;
    return 0;
}

int ndn_data_verify_signature(ndn_block_t* block,
			      const unsigned char* key,
			      size_t key_len)
{
    if (block == NULL) return -1;

    const uint8_t* buf = block->buf;
    int len = block->len;
    uint32_t num;
    int l;
    uint32_t algorithm;

    /* read data type */
    if (*buf != NDN_TLV_DATA) return -1;
    buf += 1;
    len -= 1;

    /* read data length */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l;
    len -= l;

    if ((int)num > len) return -1;  // incomplete packet

    const uint8_t* sig_start = buf;

    /* read name type */
    if (*buf != NDN_TLV_NAME) return -1;
    buf += 1;
    len -= 1;

    /* read name length and skip value */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l + (int)num;
    len -= l + (int)num;

    /* read metainfo type */
    if (*buf != NDN_TLV_METAINFO) return -1;
    buf += 1;
    len -= 1;

    /* read metainfo length and skip value */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l + (int)num;
    len -= l + (int)num;

    /* read content type */
    if (*buf != NDN_TLV_CONTENT) return -1;
    buf += 1;
    len -= 1;

    /* read content length and skip value */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l + (int)num;
    len -= l + (int)num;

    /* read signature info type */
    if (*buf != NDN_TLV_SIGNATURE_INFO) return -1;
    buf += 1;
    len -= 1;

    /* read signature info length */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l;
    len -= l;

    ndn_block_t sig_value = { buf + (int)num, len - (int)num };

    /* read signature type type */
    if (*buf != NDN_TLV_SIGNATURE_TYPE) return -1;
    buf += 1;
    len -= 1;

    /* read signature type length */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l;
    len -= l;

    /* read integer */
    l = ndn_block_get_integer(buf, (int)num, &algorithm);
    if (l < 0) return -1;
    buf += l;
    len -= l;

    if (algorithm != NDN_SIG_TYPE_DIGEST_SHA256 &&
	algorithm != NDN_SIG_TYPE_HMAC_SHA256 &&
	algorithm != NDN_SIG_TYPE_ECDSA_SHA256) {
	DEBUG("ndn_encoding: unknown signature type, cannot verify\n");
	return -1;
    }

    // skip to signature value
    buf = sig_value.buf;
    len = sig_value.len;

    /* read signature value type */
    if (*buf != NDN_TLV_SIGNATURE_VALUE) return -1;
    buf += 1;
    len -= 1;

    /* read signature value length */
    l = ndn_block_get_var_number(buf, len, &num);
    if (l < 0) return -1;
    buf += l;
    len -= l;

    /* verify signature */
    switch (algorithm) {
	case NDN_SIG_TYPE_DIGEST_SHA256:
	{
	    if (num != 32) {
		DEBUG("ndn_encoding: invalid digest sig value length (%u)\n",
		      num);
		return -1;
	    }
	    uint8_t h[32] = {0};
	    sha256(sig_start, sig_value.buf - sig_start, h);
	    if (memcmp(h, sig_value.buf + 2, sizeof(h)) != 0) {
		DEBUG("ndn_encoding: fail to verify DigestSha256 signature\n");
		return -1;
	    }
	    else
		return 0;
	}

	case NDN_SIG_TYPE_HMAC_SHA256:
	{
	    if (num != 32) {
		DEBUG("ndn_encoding: invalid hmac sig value length (%u)\n",
		      num);
		return -1;
	    }
	    uint8_t h[32] = {0};
	    if (key == NULL || key_len <= 0) {
		DEBUG("ndn_encoding: no hmac key, cannot verify signature\n");
		return -1;
	    }
	    hmac_sha256(key, key_len, (const unsigned*)sig_start,
			sig_value.buf - sig_start, h);
	    if (memcmp(h, sig_value.buf + 2, sizeof(h)) != 0) {
		DEBUG("ndn_encoding: fail to verify HMAC_SHA256 signature\n");
		return -1;
	    }
	    else
		return 0;
	}

	case NDN_SIG_TYPE_ECDSA_SHA256:
	{
	    if (num != 64) {
		DEBUG("ndn_encoding: invalid ecdsa sig value length (%u)\n",
		      num);
		return -1;
	    }
	    if (key == NULL || key_len != 64) {
		DEBUG("ndn_encoding: invalid ecdsa key\n");
		return -1;
	    }
	    uint8_t h[32] = {0};
	    sha256(sig_start, sig_value.buf - sig_start, h);
	    uECC_Curve curve = uECC_secp256r1();
	    if (uECC_verify(key, h, sizeof(h),
			    sig_value.buf + 2, curve) == 0) {
		DEBUG("ndn_encoding: fail to verify ECDSA_SHA256 signature\n");
		return -1;
	    }
	    else
		return 0;
	}

	default:
	    break;
    }
    return -1; // never reach here
}

/** @} */
