// Copyright (c) 2026 The Elite developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ELITE_BLOCKMAP_H
#define ELITE_BLOCKMAP_H

#include "uint256.h"

#include <cstring>
#include <unordered_map>

class CBlockIndex;

/** Hashes a block hash for the block index: its first 8 bytes. Block hashes
 *  are already random, and a block only gets into the index once it has
 *  passed its proof-of-work or proof-of-stake checks. */
struct BlockHasher
{
    size_t operator()(const uint256& hash) const
    {
        uint64_t n;
        memcpy(&n, hash.begin(), sizeof(n));
        return (size_t)n;
    }
};

/** The block index (mapBlockIndex). A hash table rather than a sorted map:
 *  with millions of blocks, each lookup in a sorted map is ~23 steps through
 *  memory that's no longer in the CPU cache, which made loading the block
 *  index at startup slow. Nothing relies on the order. */
typedef std::unordered_map<uint256, CBlockIndex*, BlockHasher> BlockMap;

#endif
