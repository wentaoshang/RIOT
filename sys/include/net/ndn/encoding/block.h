/*
 * Copyright (C) 2016 Wentao Shang
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_ndn_encoding    NDN packet encoding
 * @ingroup     net_ndn
 * @brief       NDN TLV packet encoding and decoding.
 * @{
 *
 * @file
 * @brief   NDN TLV block utilities.
 *
 * @author  Wentao Shang <wentaoshang@gmail.com>
 */
#ifndef NDN_BLOCK_H_
#define NDN_BLOCK_H_

#include <inttypes.h>
#include <sys/types.h>

#include "net/ndn/ndn-constants.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Type to represent a block of memory storing TLV-encoded data.
 * @details This structure does not own the memory pointed by @p buf.
 *          The user must make sure the memory pointed by @p buf is still valid
 *          as long as this structure is in use.
 */
typedef struct ndn_block {
    const uint8_t* buf;      /**< pointer to the memory buffer */
    int len;                 /**< size of the buffer */
} ndn_block_t;

/**
 * @brief   Reads a variable-length encoded integer in the beginning of a buffer.
 *
 * @param[in]  buf       Buffer to read from.
 * @param[in]  len       Size of the TLV block pointed by @p buf.
 * @param[out] num       Place to store the result.
 *
 * @return  The number of bytes occupied by the encoded number.
 * @return  -1, if the encoded number is incomplete.
 * @return  -1, if @p buf or @p num is NULL.
 * @return  -1, if the stored number is longer than 32-bit.
 */
int ndn_block_get_var_number(const uint8_t* buf, int len, uint32_t* num);

/**
 * @brief   Writes a non-negative integer into a buffer using
 *          variable-length encoding.
 *
 * @param[in]  num       Number to encode.
 * @param[out] buf       Buffer to write @p num into.
 * @param[in]  len       Size of the TLV block pointed by @p buf.
 *
 * @return  The number of bytes written into the buffer.
 * @return  -1, if there is not enough space to write @p num.
 * @return  -1, if @p num is invalid or @p buf is NULL.
 */
int ndn_block_put_var_number(uint32_t num, uint8_t* buf, int len);

/**
 * @brief   Computes the length of the variable-length encoded
 *          non-negative 32-bit integer.
 *
 * @param[in] num       Non-negative integer to be encoded.
 *
 * @return  Length of the variable-length encoded non-negative integer.
 */
int ndn_block_var_number_length(uint32_t num);

/**
 * @brief   Computes the total length of the TLV block.
 *
 * @param[in] type      Type value of the TLV block.
 * @param[in] length    Length value of the TLV block.
 *
 * @return  Total length of the TLV block.
 */
int ndn_block_total_length(uint32_t type, uint32_t length);


/**
 * @brief   Computes the length of the encoded non-negative 32-bit integer.
 *
 * @param[in] num       Non-negative integer to be encoded.
 *
 * @return  Length of the encoded non-negative integer.
 */
int ndn_block_integer_length(uint32_t num);

/**
 * @brief   Writes an non-negative integer into a caller-supplied buffer
 *          using NDN non-negative integer encoding format.
 *
 * @param[in]  num       Non-negative integer to be encoded.
 * @param[out] buf       Buffer to write into.
 * @param[in]  len       Size of the buffer.
 *
 * @return  Number of bytes written, if success.
 * @return  -1, if @p buf is NULL or not big enough to hold the encoded integer.
 */
int ndn_block_put_integer(uint32_t num, uint8_t* buf, int len);


#ifdef __cplusplus
}
#endif

#endif /* NDN_BLOCK_H_ */
/** @} */
