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
 * @brief   NDN Interest interface.
 *
 * @author  Wentao Shang <wentaoshang@gmail.com>
 */
#ifndef NDN_INTEREST_H_
#define NDN_INTEREST_H_

#include <inttypes.h>
#include <sys/types.h>

#include "net/gnrc/pktbuf.h"
#include "net/ndn/ndn-constants.h"
#include "net/ndn/encoding/block.h"
#include "net/ndn/encoding/shared_block.h"
#include "net/ndn/encoding/name.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Creates a shared TLV block that contains the encoded Interest packet.
 *
 * @param[in]  name       Name of the Interest.
 * @param[in]  selectors  Selectors of the Interest. Can be NULL if omitted.
 * @param[in]  lifetime   Lifetime of the Interest.
 *
 * @return  Pointer to the shared block, if success.
 * @return  -1, if @p name is NULL or invalid.
 * @return  -1, if out of memory.
 */
ndn_shared_block_t* ndn_interest_create(ndn_name_t* name, void* selectors,
					uint32_t lifetime);

/**
 * @brief    Creates a packet snip for the Interest.
 *
 * @details  This function does not check the validity of the block. It simply
 *           copies the memory into the packet buffer.
 *
 * @param[in]  block    TLV block of the encoded Interest.
 *
 * @return  Packet snip containing the interest.
 * @return  NULL, if @p block is NULL or invalid.
 * @return  NULL, if out of memory of packet buffer.
 */
gnrc_pktsnip_t* ndn_interest_create_packet(ndn_block_t* block);

/**
 * @brief  Retrieves the TLV-encoded name from an Interest TLV block.
 *
 * @param[in]  block      TLV block containing the Interest packet.
 * @param[out] name       Place to store the TLV block of the name.
 *
 * @return  0, if success.
 * @return  -1, if @p block or @p name is NULL.
 * @return  -1, if @p block is invalid or incomplete.
 */
int ndn_interest_get_name(ndn_block_t* block, ndn_block_t* name);

/**
 * @brief  Retrieves the nonce value from an Interest TLV block.
 *
 * @param[in]  block      TLV block containing the Interest packet.
 * @param[out] nonce      Place to store the nonce value.
 *
 * @return  0, if success.
 * @return  -1, if @p block or @p nonce is NULL.
 * @return  -1, if @p block is invalid or incomplete.
 */
int ndn_interest_get_nonce(ndn_block_t* block, uint32_t* nonce);

/**
 * @brief  Retrieves the lifetime value from an Interest TLV block.
 *
 * @param[in]  block      TLV block containing the Interest packet.
 * @param[out] life       Place to store the lifetime value.
 *
 * @return  0, if success.
 * @return  -1, if @p block or @p life is NULL.
 * @return  -1, if @p block is invalid or incomplete.
 */
int ndn_interest_get_lifetime(ndn_block_t* block, uint32_t* life);


#ifdef __cplusplus
}
#endif

#endif /* NDN_INTEREST_H_ */
/** @} */
