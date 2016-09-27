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
 * @brief   NDN name and name component interface.
 *
 * @author  Wentao Shang <wentaoshang@gmail.com>
 */
#ifndef NDN_NAME_H_
#define NDN_NAME_H_

#include <inttypes.h>
#include <sys/types.h>

#include "net/ndn/ndn-constants.h"
#include "net/ndn/encoding/block.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Type to represent a name component. An alias for @ref ndn_block_t.
 */
typedef ndn_block_t ndn_name_component_t;


/**
 * @brief   Compares two name components based on the canonical order.
 *
 * @param[in]  lhs    Left-hand-side component.
 * @param[in]  rhs    Right-hand-side component.
 *
 * @return  0 if @p lhs == @p rhs.
 * @return  1 if @p lhs > @p rhs.
 * @return  -1 if @p lhs < @p rhs.
 * @return  -2 if @p lhs or @p rhs is NULL or invalid.
 */
int ndn_name_component_compare(ndn_name_component_t* lhs, ndn_name_component_t* rhs);

/**
 * @brief   Encodes a name component into caller-supplied buffer
 *          following the TLV wire format.
 *
 * @param[in]  comp      Name component to be encoded.
 * @param[out] buf       Pointer to the caller-supplied memory buffer.
 * @param[in]  len       Size of the buffer.
 *
 * @return  Number of bytes written to the buffer, if success.
 * @return  0, if the component is empty.
 * @return  -1 if the buffer is not big enough to store the encoded name.
 * @return  -1 if @p comp is invalid.
 * @return  -1 if @p comp or @p buf is NULL or @p len <= 0.
 */
int ndn_name_component_wire_encode(ndn_name_component_t* comp, uint8_t* buf, int len);


/**
 * @brief   Type to represent a name.
 * @details The owner of this structure owns the memory pointed to by @p comps,
 *          and is responsible for freeing the memory after use.
 */
typedef struct ndn_name {
    int size;                       /**< number of the components */
    ndn_name_component_t* comps;    /**< pointer to the array of components */
} ndn_name_t;


/**
 * @brief   Compares two names based on the canonical order.
 *
 * @param[in]  lhs    Left-hand-side name.
 * @param[in]  rhs    Right-hand-side name.
 *
 * @return  0 if @p lhs == @p rhs.
 * @return  1 if @p lhs > @p rhs.
 * @return  -1 if @p lhs < @p rhs.
 * @return  -2 if @p lhs or @p rhs is NULL or invalid.
 */
int ndn_name_compare(ndn_name_t* lhs, ndn_name_t* rhs);

/**
 * @brief   Gets the n-th component from the name. This function does not make a copy
 *          of the content of the name component.
 *
 * @param[in]  name      Name where the component is retrieved.
 * @param[in]  pos       Position of the component (zero-indexed). If negative, @p pos
 *                       represents the offset from the end of the name (i.e., -1 means 
 *                       last component).
 * @param[out] comp      Caller-supplied structure for storing the retrieved component.
 *                       This structure is invalidated once @p name is released. If
 *                       @p comp->buf is not NULL, the old memory pointer will be lost.
 *
 * @return  0 if success.
 * @return  -1 if @p pos >= @p name.size or @p pos < @p -(name.size).
 * @return  -1 if @p name or @p comp is NULL.
 */
int ndn_name_get_component(ndn_name_t* name, int pos, ndn_name_component_t* comp);

/**
 * @brief   Computes the total length of the TLV encoding of a name.
 *
 * @param[in]  name      Name to be encoded.
 *
 * @return  Total length of the TLV encoding, if success.
 * @return  0, if @p name is empty.
 * @return  -1, if @p name is NULL.
 * @return  -1, if any of the components is empty or invalid.
 */
int ndn_name_total_length(ndn_name_t* name);

/**
 * @brief   Encodes a name into caller-supplied buffer following the TLV wire format.
 *          Does nothing if the name is empty.
 *
 * @param[in]  name      Name to be encoded.
 * @param[out] buf       Pointer to the caller-supplied memory buffer.
 * @param[in]  len       Size of the buffer.
 *
 * @return  Number of bytes written to the buffer, if success.
 * @return  -1 if the buffer is not big enough to store the encoded name.
 * @return  -1 if @p name is invalid.
 * @return  -1 if @p name or @p buf is NULL.
 */
int ndn_name_wire_encode(ndn_name_t* name, uint8_t* buf, int len);

/**
 * @brief   Gets the number of name components in a TLV-encoded NDN name.
 *
 * @param[in]  block  TLV block containing a TLV-encoded NDN name.
 *
 * @return  Number of name components in the name.
 * @return  -1, if @p block is NULL.
 * @return  -1, if @p pkt does not contain a valid name.
 */
int ndn_name_get_size_from_block(ndn_block_t* block);

/**
 * @brief   Gets the n-th name component from a TLV-encoded NDN name.
 *
 * @param[in]  block  TLV block containing a TLV-encoded NDN name.
 * @param[in]  pos    Position of the name component to be retrieved (zero-indexed).
 *                    Cannot be negative.
 * @param[out] comp   Place to hold the name component structure.
 *                    This structure is invalidated once @p pkt is released. If
 *                    @p comp->buf is not NULL, the old memory pointer will be lost.
 *
 *
 * @return  0, if success.
 * @return  -1, if @p block or @p comp is NULL.
 * @return  -1, if @p block does not contain a valid name.
 * @return  -1, if @p pos >= the total number of name components.
 */
int ndn_name_get_component_from_block(ndn_block_t* block, int pos, ndn_name_component_t* comp);

/**
 * @brief   Compares two TLV-encoded names based on the canonical order.
 *
 * @param[in]  lhs    Left-hand-side name.
 * @param[in]  rhs    Right-hand-side name.
 *
 * @return  0, if @p lhs == @p rhs.
 * @return  1, if @p lhs > @p rhs and @p rhs is not a prefix of @p lhs.
 * @return  2, if @p lhs > @p rhs and @p rhs is a proper prefix of @p lhs.
 * @return  -1, if @p lhs < @p rhs and @p lhs is not a prefix of @p rhs.
 * @return  -2, if @p lhs < @p rhs and @p lhs is a proper prefix of @p rhs.
 * @return  3, if @p lhs is NULL or invalid.
 * @return  -3, if @p rhs is NULL or invalid.
 */
int ndn_name_compare_block(ndn_block_t* lhs, ndn_block_t* rhs);

#ifdef __cplusplus
}
#endif

#endif /* NDN_NAME_H_ */
/** @} */
