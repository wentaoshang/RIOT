/*
 * Copyright (C) 2016 Wentao Shang
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     net_ndn
 * @{
 *
 * @file
 *
 * @author  Wentao Shang <wentaoshang@gmail.com>
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "net/ndn/encoding/shared_block.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

ndn_shared_block_t* ndn_shared_block_create(ndn_block_t* block)
{
    if (block == NULL || block->buf == NULL || block->len <= 0)
	return NULL;

    ndn_shared_block_t* shared =
	(ndn_shared_block_t*)malloc(sizeof(ndn_shared_block_t));
    if (shared == NULL) {
	DEBUG("ndn: cannot allocate memory for shared block\n");
	return NULL;
    }

    memset(shared, 0, sizeof(ndn_shared_block_t));

    uint8_t* nbuf = (uint8_t*)malloc(block->len);
    if (nbuf == NULL) {
	DEBUG("ndn: cannot allocate memory for shared block content\n");
	free(shared);
	return NULL;
    }
    memcpy(nbuf, block->buf, block->len);
    shared->block.buf = nbuf;
    shared->block.len = block->len;
    atomic_set_to_one(&shared->ref);
    return shared;
}

ndn_shared_block_t* ndn_shared_block_create_by_move(ndn_block_t* block)
{
    if (block == NULL || block->buf == NULL || block->len <= 0)
	return NULL;

    ndn_shared_block_t* shared =
	(ndn_shared_block_t*)malloc(sizeof(ndn_shared_block_t));
    if (shared == NULL) {
	DEBUG("ndn: cannot allocate memory for shared block\n");
	return NULL;
    }

    memset(shared, 0, sizeof(ndn_shared_block_t));

    // "Move" memory into the shared block
    shared->block.buf = block->buf;
    shared->block.len = block->len;
    block->buf = NULL;
    block->len = 0;
    atomic_set_to_one(&shared->ref);
    return shared;
}

void ndn_shared_block_release(ndn_shared_block_t* shared)
{
    assert(shared != NULL);
    int ref = atomic_dec(&shared->ref) - 1;
    DEBUG("ndn: decrement shared block ref to %d\n", ref);
    if (ref == 0) {
	/* no one is using this block; free the memory. */
	DEBUG("ndn: free shared block memory\n");
	free((void*)shared->block.buf);
	free(shared);
    }
    return;
}

ndn_shared_block_t* ndn_shared_block_copy(ndn_shared_block_t* shared)
{
    assert(shared != NULL);
    int ref = atomic_inc(&shared->ref) + 1;
    DEBUG("ndn: increase shared block ref to %d\n", ref);
    (void)ref;
    return shared;
}


/** @} */
