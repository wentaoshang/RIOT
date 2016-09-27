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
 * @brief      Adds an Interest to PIT and set (or reset) the PIT entry timer.
 * 
 * @param[in]  face_id    ID of the incoming face.
 * @param[in]  face_type  Type of the incoming face.
 * @param[in]  si         Shared TLV block of the Interest packet to add. Will
 *                        be "copied" into the new PIT entry.
 *
 * @return     0, if success.
 * @return     1, if Interet with the same name and selectors already exists.
 * @return     -1, if @p si is invlid.
 * @retrun     -1, if out of memory.
 */
int ndn_pit_add(kernel_pid_t face_id, int face_type, ndn_shared_block_t* si);

/**
 * @brief  Removes the expired entry from PIT based on the @p msg pointer.
 *
 * @param[in]  msg    Message that identifies the expired PIT entry.
 */
void ndn_pit_timeout(msg_t *msg);

/**
 * @brief   Matches data against PIT and forwards the data to all incoming
 *          faces.
 * @details This function will not take ownership of @p sd.
 *
 * @param[in]  sd    Shared block pointer of the data packet.
 *
 * @return  0, if a match is found.
 * @return  -1, if no matching PIT entry is found.
 * @return  -1, if @p sd is invalid.
 */
int ndn_pit_match_data(ndn_shared_block_t* sd);

/**
 * @brief    Initializes the pending interest table.
 */
void ndn_pit_init(void);


#ifdef __cplusplus
}
#endif

#endif /* NDN_PIT_H_ */
/** @} */
