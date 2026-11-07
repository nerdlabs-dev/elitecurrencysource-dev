// Copyright (c) 8888 Satoshi Nakamoto
// Copyright (c) 8888 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "main.h"
#include "bitcoinrpc.h"
#include "txdb.h"

using namespace json_spirit;
using namespace std;

extern void TxToJSON(const CTransaction& tx, const uint256 hashBlock, json_spirit::Object& entry);
extern enum Checkpoints::CPMode CheckpointsMode;

double GetDifficulty(const CBlockIndex* blockindex)
{
    // Floating point number that is a multiple of the minimum difficulty,
    // minimum difficulty = 1.0.
    if (blockindex == NULL)
    {
        if (pindexBest == NULL)
            return 1.0;
        else
            blockindex = GetLastBlockIndex(pindexBest, false);
    }

    int nShift = (blockindex->nBits >> 24) & 0xff;

    double dDiff =
        (double)0x0000ffff / (double)(blockindex->nBits & 0x00ffffff);

    while (nShift < 29)
    {
        dDiff *= 256.0;
        nShift++;
    }
    while (nShift > 29)
    {
        dDiff /= 256.0;
        nShift--;
    }

    return dDiff;
}

double GetPoWMHashPS()
{
    if (pindexBest->nHeight >= LAST_POW_BLOCK)
        return 0;

    int nPoWInterval = 72;
    int64_t nTargetSpacingWorkMin = 30, nTargetSpacingWork = 30;

    CBlockIndex* pindex = pindexGenesisBlock;
    CBlockIndex* pindexPrevWork = pindexGenesisBlock;

    while (pindex)
    {
        if (pindex->IsProofOfWork())
        {
            int64_t nActualSpacingWork = pindex->GetBlockTime() - pindexPrevWork->GetBlockTime();
            nTargetSpacingWork = ((nPoWInterval - 1) * nTargetSpacingWork + nActualSpacingWork + nActualSpacingWork) / (nPoWInterval + 1);
            nTargetSpacingWork = max(nTargetSpacingWork, nTargetSpacingWorkMin);
            pindexPrevWork = pindex;
        }

        pindex = pindex->pnext;
    }

    return GetDifficulty() * 4294.967296 / nTargetSpacingWork;
}

double GetPoSKernelPS()
{
    int nPoSInterval = 72;
    double dStakeKernelsTriedAvg = 0;
    int nStakesHandled = 0, nStakesTime = 0;

    CBlockIndex* pindex = pindexBest;;
    CBlockIndex* pindexPrevStake = NULL;

    while (pindex && nStakesHandled < nPoSInterval)
    {
        if (pindex->IsProofOfStake())
        {
            dStakeKernelsTriedAvg += GetDifficulty(pindex) * 4294967296.0;
            nStakesTime += pindexPrevStake ? (pindexPrevStake->nTime - pindex->nTime) : 0;
            pindexPrevStake = pindex;
            nStakesHandled++;
        }

        pindex = pindex->pprev;
    }

    return nStakesTime ? dStakeKernelsTriedAvg / nStakesTime : 0;
}

Object blockToJSON(const CBlock& block, const CBlockIndex* blockindex, bool fPrintTransactionDetail)
{
    Object result;
    result.push_back(Pair("hash", block.GetHash().GetHex()));
    CMerkleTx txGen(block.vtx[0]);
    txGen.SetMerkleBranch(&block);
    result.push_back(Pair("confirmations", (int)txGen.GetDepthInMainChain()));
    result.push_back(Pair("size", (int)::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION)));
    result.push_back(Pair("height", blockindex->nHeight));
    result.push_back(Pair("version", block.nVersion));
    result.push_back(Pair("merkleroot", block.hashMerkleRoot.GetHex()));
    result.push_back(Pair("mint", ValueFromAmount(blockindex->nMint)));
    result.push_back(Pair("time", (boost::int64_t)block.GetBlockTime()));
    result.push_back(Pair("mediantime", (boost::int64_t)blockindex->GetMedianTimePast()));
    result.push_back(Pair("nonce", (boost::uint64_t)block.nNonce));
    result.push_back(Pair("bits", HexBits(block.nBits)));
    result.push_back(Pair("difficulty", GetDifficulty(blockindex)));
    result.push_back(Pair("blocktrust", leftTrim(blockindex->GetBlockTrust().GetHex(), '0')));
    result.push_back(Pair("chaintrust", leftTrim(blockindex->nChainTrust.GetHex(), '0')));
    result.push_back(Pair("chainwork", blockindex->nChainTrust.GetHex()));
    result.push_back(Pair("nTx", (int)block.vtx.size()));
    if (blockindex->pprev)
        result.push_back(Pair("previousblockhash", blockindex->pprev->GetBlockHash().GetHex()));
    if (blockindex->pnext)
        result.push_back(Pair("nextblockhash", blockindex->pnext->GetBlockHash().GetHex()));

    result.push_back(Pair("flags", strprintf("%s%s", blockindex->IsProofOfStake()? "proof-of-stake" : "proof-of-work", blockindex->GeneratedStakeModifier()? " stake-modifier": "")));
    result.push_back(Pair("proofhash", blockindex->hashProof.GetHex()));
    result.push_back(Pair("entropybit", (int)blockindex->GetStakeEntropyBit()));
    result.push_back(Pair("modifier", strprintf("%016" PRIx64, blockindex->nStakeModifier)));
    result.push_back(Pair("modifierchecksum", strprintf("%08x", blockindex->nStakeModifierChecksum)));
    Array txinfo;
    BOOST_FOREACH (const CTransaction& tx, block.vtx)
    {
        if (fPrintTransactionDetail)
        {
            Object entry;

            TxToJSON(tx, 0, entry); // (adds txid; it was added twice)

            txinfo.push_back(entry);
        }
        else
            txinfo.push_back(tx.GetHash().GetHex());
    }

    result.push_back(Pair("tx", txinfo));

    if (block.IsProofOfStake())
        result.push_back(Pair("signature", HexStr(block.vchBlockSig.begin(), block.vchBlockSig.end())));

    return result;
}

Value getbestblockhash(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getbestblockhash\n"
            "Returns the hash of the best block in the longest block chain.");

    return hashBestChain.GetHex();
}

Value getblockcount(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getblockcount\n"
            "Returns the number of blocks in the longest block chain.");

    return nBestHeight;
}


Value getdifficulty(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getdifficulty\n"
            "Returns the difficulty as a multiple of the minimum difficulty.");

    Object obj;
    obj.push_back(Pair("proof-of-work",        GetDifficulty()));
    obj.push_back(Pair("proof-of-stake",       GetDifficulty(GetLastBlockIndex(pindexBest, true))));
    obj.push_back(Pair("search-interval",      (int)nLastCoinStakeSearchInterval));
    return obj;
}


Value settxfee(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 1 || AmountFromValue(params[0]) < MIN_TX_FEE)
        throw runtime_error(
            "settxfee <amount>\n"
            "<amount> is a real and is rounded to the nearest 0.01");

    nTransactionFee = AmountFromValue(params[0]);
    nTransactionFee = (nTransactionFee / CENT) * CENT;  // round to cent

    return true;
}

Value getrawmempool(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getrawmempool\n"
            "Returns all transaction ids in memory pool.");

    vector<uint256> vtxid;
    mempool.queryHashes(vtxid);

    Array a;
    BOOST_FOREACH(const uint256& hash, vtxid)
        a.push_back(hash.ToString());

    return a;
}

Value getblockhash(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 1)
        throw runtime_error(
            "getblockhash <index>\n"
            "Returns hash of block in best-block-chain at <index>.");

    int nHeight = ParamInt(params[0], "index");
    if (nHeight < 0 || nHeight > nBestHeight)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Block number out of range.");

    CBlockIndex* pblockindex = FindBlockByHeight(nHeight);
    return pblockindex->phashBlock->GetHex();
}

// getblock's second parameter: the original boolean txinfo, or Bitcoin's
// verbosity 0 (hex), 1 (txids) or 2 (transaction details)
static int BlockVerbosity(const Array& params)
{
    if (params.size() < 2)
        return 1;
    if (params[1].type() == bool_type)
        return params[1].get_bool() ? 2 : 1;
    if (params[1].type() == str_type && (params[1].get_str() == "true" || params[1].get_str() == "false"))
        return params[1].get_str() == "true" ? 2 : 1;
    int nVerbosity = ParamInt(params[1], "verbosity");
    if (nVerbosity < 0 || nVerbosity > 2)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "verbosity must be 0, 1 or 2");
    return nVerbosity;
}

static Value BlockResult(CBlockIndex* pblockindex, int nVerbosity)
{
    CBlock block;
    if (!block.ReadFromDisk(pblockindex, true))
        throw JSONRPCError(RPC_INTERNAL_ERROR, "Can't read block from disk");

    if (nVerbosity == 0)
    {
        CDataStream ssBlock(SER_NETWORK, PROTOCOL_VERSION);
        ssBlock << block;
        return HexStr(ssBlock.begin(), ssBlock.end());
    }
    return blockToJSON(block, pblockindex, nVerbosity >= 2);
}

Value getblock(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getblock <hash> [verbosity=1]\n"
            "Returns details of a block with given block-hash.\n"
            "verbosity 0: the serialized block as hex; 1: details with the list of\n"
            "transaction ids; 2: details with each transaction decoded.\n"
            "(true and false still work, for 2 and 1.)");

    std::string strHash = params[0].get_str();
    uint256 hash(strHash);

    LOCK(cs_main);
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
    if (mi == mapBlockIndex.end() || mi->second == NULL)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");

    return BlockResult(mi->second, BlockVerbosity(params));
}

Value getblockbynumber(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getblockbynumber <number> [verbosity=1]\n"
            "Returns details of a block with given block-number (same output as getblock).");

    int nHeight = ParamInt(params[0], "number");
    LOCK(cs_main);
    if (nHeight < 0 || nHeight > nBestHeight)
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Block number out of range.");

    return BlockResult(FindBlockByHeight(nHeight), BlockVerbosity(params));
}

Value getblockheader(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "getblockheader <hash> [verbose=true]\n"
            "Returns the block header: as an object, or as hex (80 bytes) if verbose is false.");

    uint256 hash(params[0].get_str());
    bool fVerbose = params.size() > 1 ? ParamBool(params[1], "verbose") : true;

    LOCK(cs_main);
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hash);
    if (mi == mapBlockIndex.end() || mi->second == NULL)
        throw JSONRPCError(RPC_INVALID_ADDRESS_OR_KEY, "Block not found");
    CBlockIndex* pindex = mi->second;

    if (!fVerbose)
    {
        CDataStream ssHeader(SER_NETWORK, PROTOCOL_VERSION);
        ssHeader << pindex->nVersion << (pindex->pprev ? pindex->pprev->GetBlockHash() : uint256(0))
                 << pindex->hashMerkleRoot << pindex->nTime << pindex->nBits << pindex->nNonce;
        return HexStr(ssHeader.begin(), ssHeader.end());
    }

    Object result;
    result.push_back(Pair("hash", pindex->GetBlockHash().GetHex()));
    result.push_back(Pair("confirmations", pindex->IsInMainChain() ? nBestHeight - pindex->nHeight + 1 : -1));
    result.push_back(Pair("height", pindex->nHeight));
    result.push_back(Pair("version", pindex->nVersion));
    result.push_back(Pair("merkleroot", pindex->hashMerkleRoot.GetHex()));
    result.push_back(Pair("time", (boost::int64_t)pindex->GetBlockTime()));
    result.push_back(Pair("mediantime", (boost::int64_t)pindex->GetMedianTimePast()));
    result.push_back(Pair("nonce", (boost::uint64_t)pindex->nNonce));
    result.push_back(Pair("bits", HexBits(pindex->nBits)));
    result.push_back(Pair("difficulty", GetDifficulty(pindex)));
    result.push_back(Pair("chainwork", pindex->nChainTrust.GetHex()));
    result.push_back(Pair("flags", pindex->IsProofOfStake() ? "proof-of-stake" : "proof-of-work"));
    if (pindex->pprev)
        result.push_back(Pair("previousblockhash", pindex->pprev->GetBlockHash().GetHex()));
    if (pindex->pnext)
        result.push_back(Pair("nextblockhash", pindex->pnext->GetBlockHash().GetHex()));
    return result;
}

// Rough sync progress: how far the tip's time is between the genesis block
// and now (blocks are evenly spaced in time on this chain, at least roughly)
static double VerificationProgress(const CBlockIndex* pindex)
{
    if (pindex == NULL || pindexGenesisBlock == NULL)
        return 0;
    int64_t nNow = GetAdjustedTime();
    int64_t nSpan = nNow - pindexGenesisBlock->GetBlockTime();
    if (nSpan <= 0)
        return 1;
    double dProgress = (double)(pindex->GetBlockTime() - pindexGenesisBlock->GetBlockTime()) / (double)nSpan;
    if (nNow - pindex->GetBlockTime() < 60 * 60)
        dProgress = 1;  // tip is less than an hour old: caught up
    return std::max(0.0, std::min(1.0, dProgress));
}

Value getblockchaininfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getblockchaininfo\n"
            "Returns information about the block chain: chain, blocks, headers,\n"
            "bestblockhash, difficulty (proof-of-stake), difficulty_pow, mediantime,\n"
            "verificationprogress, initialblockdownload, chainwork, moneysupply,\n"
            "maxmoney, pruned, warnings.");

    LOCK(cs_main);
    Object obj;
    obj.push_back(Pair("chain", fTestNet ? "test" : "main"));
    obj.push_back(Pair("blocks", (int)nBestHeight));
    obj.push_back(Pair("headers", (int)nBestHeight));  // no headers-first sync: same as blocks
    obj.push_back(Pair("bestblockhash", hashBestChain.GetHex()));
    obj.push_back(Pair("difficulty", GetDifficulty(GetLastBlockIndex(pindexBest, true))));
    obj.push_back(Pair("difficulty_pow", GetDifficulty()));
    obj.push_back(Pair("mediantime", pindexBest ? (boost::int64_t)pindexBest->GetMedianTimePast() : (boost::int64_t)0));
    obj.push_back(Pair("verificationprogress", VerificationProgress(pindexBest)));
    obj.push_back(Pair("initialblockdownload", IsInitialBlockDownload()));
    obj.push_back(Pair("chainwork", pindexBest ? pindexBest->nChainTrust.GetHex() : uint256(0).GetHex()));
    obj.push_back(Pair("moneysupply", pindexBest ? ValueFromAmount(pindexBest->nMoneySupply) : ValueFromAmount(0)));
    obj.push_back(Pair("maxmoney", ValueFromAmount(MAX_MONEY)));
    obj.push_back(Pair("pruned", false));
    obj.push_back(Pair("warnings", GetWarnings("statusbar")));
    return obj;
}

Value getmempoolinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getmempoolinfo\n"
            "Returns the number of transactions in the memory pool (size) and their\n"
            "total serialized size in bytes.");

    Object obj;
    uint64_t nBytes = 0;
    unsigned long nSize = 0;
    {
        LOCK(mempool.cs);
        nSize = mempool.mapTx.size();
        for (map<uint256, CTransaction>::const_iterator it = mempool.mapTx.begin(); it != mempool.mapTx.end(); ++it)
            nBytes += ::GetSerializeSize(it->second, SER_NETWORK, PROTOCOL_VERSION);
    }
    obj.push_back(Pair("size", (boost::int64_t)nSize));
    obj.push_back(Pair("bytes", (boost::int64_t)nBytes));
    return obj;
}

extern void ScriptPubKeyToJSON(const CScript& scriptPubKey, Object& out, bool fIncludeHex);

Value gettxout(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 2 || params.size() > 3)
        throw runtime_error(
            "gettxout <txid> <n> [includemempool=true]\n"
            "Returns details about an unspent transaction output, or nothing if it\n"
            "is spent or doesn't exist. With includemempool, outputs spent by\n"
            "memory pool transactions count as spent and memory pool outputs are found.");

    uint256 hash(params[0].get_str());
    int n = ParamInt(params[1], "n");
    bool fMempool = params.size() > 2 ? ParamBool(params[2], "includemempool") : true;
    if (n < 0)
        return Value::null;

    LOCK(cs_main);
    CTransaction tx;
    int nConfirmations = 0;

    if (fMempool && mempool.exists(hash))
    {
        tx = mempool.lookup(hash);
    }
    else
    {
        CTxDB txdb("r");
        CTxIndex txindex;
        if (!txdb.ReadTxIndex(hash, txindex))
            return Value::null;
        if (n >= (int)txindex.vSpent.size() || !txindex.vSpent[n].IsNull())
            return Value::null;
        uint256 hashBlock = 0;
        if (!GetTransaction(hash, tx, hashBlock))
            return Value::null;
        map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
        if (mi != mapBlockIndex.end() && mi->second && mi->second->IsInMainChain())
            nConfirmations = nBestHeight - mi->second->nHeight + 1;
    }

    if (n >= (int)tx.vout.size())
        return Value::null;
    if (fMempool)
    {
        LOCK(mempool.cs);
        if (mempool.mapNextTx.count(COutPoint(hash, n)))
            return Value::null;  // spent by a memory pool transaction
    }

    Object ret;
    ret.push_back(Pair("bestblock", hashBestChain.GetHex()));
    ret.push_back(Pair("confirmations", nConfirmations));
    ret.push_back(Pair("value", ValueFromAmount(tx.vout[n].nValue)));
    Object o;
    ScriptPubKeyToJSON(tx.vout[n].scriptPubKey, o, true);
    ret.push_back(Pair("scriptPubKey", o));
    ret.push_back(Pair("version", tx.nVersion));
    ret.push_back(Pair("coinbase", tx.IsCoinBase()));
    ret.push_back(Pair("coinstake", tx.IsCoinStake()));
    return ret;
}

Value gettxoutsetinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "gettxoutsetinfo\n"
            "Returns statistics about the unspent transaction outputs:\n"
            "height, bestblock, transactions (with unspent outputs), txouts\n"
            "(unspent outputs; includes the empty first output of each coinstake),\n"
            "total_amount (coins in unspent outputs, the money supply).\n"
            "Scans the transaction index, so the first call after a new block can\n"
            "take a while; the result is cached until the next block.");

    static CCriticalSection cs_cache;
    static uint256 hashCached = 0;
    static Object cached;

    const leveldb::Snapshot* snapshot = NULL;
    int nHeight;
    uint256 hashBlock;
    int64_t nMoneySupply;
    CTxDB txdb("r");
    {
        // Take the database snapshot together with the best block, so the
        // numbers belong to that block; then scan without holding cs_main
        LOCK(cs_main);
        nHeight = nBestHeight;
        hashBlock = hashBestChain;
        nMoneySupply = pindexBest ? pindexBest->nMoneySupply : 0;
        {
            LOCK(cs_cache);
            if (hashCached == hashBlock)
                return cached;
        }
        snapshot = txdb.GetSnapshot();
    }

    uint64_t nTransactions = 0, nTxOuts = 0;
    bool fOk = txdb.ScanTxIndex(snapshot, nTransactions, nTxOuts);
    txdb.ReleaseSnapshot(snapshot);
    if (!fOk)
        throw JSONRPCError(RPC_DATABASE_ERROR, "Unable to read the transaction index");

    Object ret;
    ret.push_back(Pair("height", nHeight));
    ret.push_back(Pair("bestblock", hashBlock.GetHex()));
    ret.push_back(Pair("transactions", (boost::int64_t)nTransactions));
    ret.push_back(Pair("txouts", (boost::int64_t)nTxOuts));
    ret.push_back(Pair("total_amount", ValueFromAmount(nMoneySupply)));

    LOCK(cs_cache);
    hashCached = hashBlock;
    cached = ret;
    return ret;
}

Value getsupply(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getsupply\n"
            "Returns the current money supply (coins in existence).");

    LOCK(cs_main);
    return ValueFromAmount(pindexBest ? pindexBest->nMoneySupply : 0);
}

Value getmaxmoney(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getmaxmoney\n"
            "Returns the largest amount the network accepts (MAX_MONEY).");

    return ValueFromAmount(MAX_MONEY);
}

// ppcoin: get information of sync-checkpoint
Value getcheckpoint(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getcheckpoint\n"
            "Show info of synchronized checkpoint.\n");

    Object result;
    CBlockIndex* pindexCheckpoint;

    result.push_back(Pair("synccheckpoint", Checkpoints::hashSyncCheckpoint.ToString().c_str()));
    pindexCheckpoint = mapBlockIndex[Checkpoints::hashSyncCheckpoint];
    result.push_back(Pair("height", pindexCheckpoint->nHeight));
    result.push_back(Pair("timestamp", DateTimeStrFormat(pindexCheckpoint->GetBlockTime()).c_str()));

    // Check that the block satisfies synchronized checkpoint
    if (CheckpointsMode == Checkpoints::STRICT)
        result.push_back(Pair("policy", "strict"));

    if (CheckpointsMode == Checkpoints::ADVISORY)
        result.push_back(Pair("policy", "advisory"));

    if (CheckpointsMode == Checkpoints::PERMISSIVE)
        result.push_back(Pair("policy", "permissive"));

    if (mapArgs.count("-checkpointkey"))
        result.push_back(Pair("checkpointmaster", true));

    return result;
}
