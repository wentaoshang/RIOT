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

#include "utlist.h"
#include "net/ndn/encoding/interest.h"
#include "net/ndn/encoding/data.h"

#include "net/ndn/cs.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

static ndn_cs_entry_t *_cs;

int ndn_cs_add(ndn_shared_block_t* data)
{
    assert(data != NULL);

    // allocate new entry
    ndn_cs_entry_t *entry = (ndn_cs_entry_t*)malloc(sizeof(ndn_cs_entry_t));
    if (entry == NULL) {
	DEBUG("ndn: cannot allocate pit entry\n");
	return -1;
    }

    entry->data = ndn_shared_block_copy(data);
    entry->prev = entry->next = NULL;

    DL_PREPEND(_cs, entry);
    DEBUG("ndn: add new cs entry\n");
    return 0;
}

ndn_shared_block_t* ndn_cs_match(ndn_block_t* interest)
{
    ndn_block_t iname;
    int r = ndn_interest_get_name(interest, &iname);
    if (r != 0) {
	DEBUG("ndn: cannot get name from interest for cs matching\n");
	return NULL;
    }

    ndn_cs_entry_t *entry;
    DL_FOREACH(_cs, entry) {
	ndn_block_t dname;
	r = ndn_data_get_name(&entry->data->block, &dname);
	assert(r == 0);

	r = ndn_name_compare_block(&iname, &dname);
	if (r == -2 || r == 0) {
	    // either iname is a prefix of dname, or they are the same
	    return ndn_shared_block_copy(entry->data);
	}
    }
    return NULL;
}

void ndn_cs_init(void)
{
    _cs = NULL;    
}


/** @} */
