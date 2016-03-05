/*
 * Copyright (C) 2016 Wentao Shang
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    net_ndn    NDN packet processing
 * @ingroup     net
 * @brief       NDN packet sending and receiving.
 * @{
 *
 * @file
 * @brief   NDN PIT implementation.
 *
 * @author  Wentao Shang <wentaoshang@gmail.com>
 */
#ifndef NDN_PIT_H_
#define NDN_PIT_H_

#include "kernel_types.h"
#include "xtimer.h"
#include "net/ndn/encoding/shared_block.h"
#include "net/ndn/face_table.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Type to represent the PIT entry.
 */
typedef struct ndn_pit_entry {
    struct ndn_pit_entry *prev;
    struct ndn_pit_entry *next;
    ndn_shared_block_t *shared_pi;  /**< shared TLV block of the pending interest */
    xtimer_t timer;                 /**< xtimer struct */
    msg_t timer_msg;                /**< special message to indicate timeout event */

    // List of incoming faces
    _face_list_entry_t *face_list;
    int face_list_size;
} ndn_pit_entry_t;

/**
 * @brief      Adds an entry to PIT.
 * 
 * @param[in]  face_id    ID of the incoming face.
 * @param[in]  face_type  Type of the incoming face.
 * @param[in]  block      TLV block of the Interest packet to add.
 *
 * @return     Pointer to the new PIT entry, if success. Caller need to 
 *             set (or reset) the timer using the returned entry.
 * @retrun     NULL, if out of memory.
 */
ndn_pit_entry_t* ndn_pit_add(kernel_pid_t face_id, int face_type,
			     ndn_block_t* block);

/**
 * @brief  Removes the expired entry from PIT based on the @p msg pointer.
 *
 * @param[in]  msg    Message that identifies the expired PIT entry.
 */
void ndn_pit_timeout(msg_t *msg);

/**
 * @brief  Matches data against PIT and forwards the data to all incoming
 *         faces.
 *
 * @param[in]  pkt    Packet snip of the data packet.
 *
 * @return  Shared pointer to data (created from the packet), if a match is
 *          found. This pointer will be used later to insert into CS.
 * @return  NULL, if no matching PIT entry is found.
 * @return  NULL, if @p pkt is invalid.
 */
ndn_shared_block_t* ndn_pit_match_data(gnrc_pktsnip_t* pkt);

/**
 * @brief    Initializes the pending interest table.
 */
void ndn_pit_init(void);


#ifdef __cplusplus
}
#endif

#endif /* NDN_PIT_H_ */
/** @} */
