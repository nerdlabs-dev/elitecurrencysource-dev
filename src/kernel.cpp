// Copyright (c) 2012-2013 The PPCoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/assign/list_of.hpp>

#include "kernel.h"
#include "txdb.h"

using namespace std;

typedef std::map<int, unsigned int> MapModifierCheckpoints;

// Hard checkpoints of stake modifiers to ensure they are deterministic
static std::map<int, unsigned int> mapStakeModifierCheckpoints =
    boost::assign::map_list_of
        (       0, 0xfd11f4e7 )
        (   25000, 0xb3b2a77c )
        (   50000, 0x4a424815 )
        (   75000, 0xccd75373 )
        (  100000, 0x73a121bb )
        (  125000, 0x86abd049 )
        (  150000, 0x91946a75 )
        (  175000, 0xa4d4c9a3 )
        (  200000, 0xc97afbe5 )
        (  225000, 0x65440f2a )
        (  250000, 0x004984d6 )
        (  275000, 0xf0db794b )
        (  300000, 0xdf353878 )
        (  325000, 0x84dd86a5 )
        (  350000, 0x57bc4b79 )
        (  375000, 0x4752b57d )
        (  400000, 0x8140977f )
        (  425000, 0x9de5101b )
        (  450000, 0x3dacefa4 )
        (  475000, 0xfca7f636 )
        (  500000, 0x4bcafd1c )
        (  525000, 0x6ad90123 )
        (  550000, 0x862cfea1 )
        (  575000, 0xfd7f649a )
        (  600000, 0xd82ef39e )
        (  625000, 0x2fb0a1e9 )
        (  650000, 0x0088886d )
        (  675000, 0x1f567b9f )
        (  700000, 0x8fdb6bbe )
        (  725000, 0x96ebf732 )
        (  750000, 0x1fb9633e )
        (  775000, 0x41cd78a1 )
        (  800000, 0x098d3d46 )
        (  825000, 0x77a8ff6b )
        (  850000, 0x2a23d757 )
        (  875000, 0x9c27ef37 )
        (  900000, 0x5e7998eb )
        (  925000, 0x6f6d5e69 )
        (  950000, 0xd232c5c0 )
        (  975000, 0x2cec616d )
        ( 1000000, 0xe8910a68 )
        ( 1025000, 0x3dce725e )
        ( 1050000, 0xe62311bd )
        ( 1075000, 0xa832d9a7 )
        ( 1100000, 0xcb3c8ba1 )
        ( 1125000, 0x3e334cb9 )
        ( 1150000, 0x4aff40ad )
        ( 1175000, 0x74961150 )
        ( 1200000, 0xf128409d )
        ( 1225000, 0xe4e96147 )
        ( 1250000, 0xb522418c )
        ( 1275000, 0x82ff1545 )
        ( 1300000, 0x3788b649 )
        ( 1325000, 0x9385cd97 )
        ( 1350000, 0x6a82dd45 )
        ( 1375000, 0x4be1474b )
        ( 1400000, 0xd393a2d5 )
        ( 1425000, 0xc4306fdc )
        ( 1450000, 0xd14aa35e )
        ( 1475000, 0x97569453 )
        ( 1500000, 0x9cee495d )
        ( 1525000, 0x72ffdf9f )
        ( 1550000, 0x76bd5426 )
        ( 1575000, 0xd2c79454 )
        ( 1600000, 0xec6ab18a )
        ( 1625000, 0x40c51bc1 )
        ( 1650000, 0x85250562 )
        ( 1675000, 0xe82eb06c )
        ( 1700000, 0xc601e820 )
        ( 1725000, 0x92e063fa )
        ( 1750000, 0xea233429 )
        ( 1775000, 0x53caec00 )
        ( 1800000, 0x6432454c )
        ( 1825000, 0x64cfaec5 )
        ( 1850000, 0x95a2fec5 )
        ( 1875000, 0xdc9a990b )
        ( 1900000, 0xf7e5b06e )
        ( 1925000, 0xe0594f6c )
        ( 1950000, 0x2219b45d )
        ( 1975000, 0x956afe70 )
        ( 2000000, 0xf7ae7349 )
        ( 2025000, 0x877b7425 )
        ( 2050000, 0xc71797be )
        ( 2075000, 0x19c7da49 )
        ( 2100000, 0xc70a282e )
        ( 2125000, 0xaf95cb3a )
        ( 2150000, 0x4d9dd822 )
        ( 2175000, 0xeb210143 )
        ( 2200000, 0xc77a25aa )
        ( 2225000, 0x9289c8fd )
        ( 2250000, 0xe5d13f34 )
        ( 2275000, 0xe7e9ab72 )
        ( 2300000, 0x81abf8a8 )
        ( 2325000, 0x6b0bf64a )
        ( 2350000, 0x8c217f1e )
        ( 2375000, 0xd526c759 )
        ( 2400000, 0xfe1db2e6 )
        ( 2425000, 0x4bfc0fba )
        ( 2450000, 0x80145a7d )
        ( 2475000, 0x1f8d2abc )
        ( 2500000, 0x5e9108ac )
        ( 2525000, 0x99441e3c )
        ( 2550000, 0x3e4ce610 )
        ( 2575000, 0xb4aa1f31 )
        ( 2600000, 0x74200434 )
        ( 2625000, 0xf58b718a )
        ( 2650000, 0xdd17d8c4 )
        ( 2675000, 0x16caeeee )
        ( 2700000, 0x5baa866f )
        ( 2725000, 0x2031ab11 )
        ( 2750000, 0x3c02b1a1 )
        ( 2775000, 0xbeca788e )
        ( 2800000, 0x0ed1faf5 )
        ( 2825000, 0x41673c21 )
        ( 2850000, 0xd3810b52 )
        ( 2875000, 0x7b44682b )
        ( 2900000, 0x899cc73a )
        ( 2925000, 0x2ee98089 )
        ( 2950000, 0x9976212f )
        ( 2975000, 0x4bbd9adc )
        ( 3000000, 0xa7f49bc8 )
        ( 3025000, 0x24f8248a )
        ( 3050000, 0x04721664 )
        ( 3075000, 0xfba1fe1b )
        ( 3100000, 0xc309b924 )
        ( 3125000, 0x1d1d9f3d )
        ( 3150000, 0x31f905b6 )
        ( 3175000, 0x0ad14f1a )
        ( 3200000, 0x50d8fc54 )
        ( 3225000, 0x80a8bc58 )
        ( 3250000, 0xd9bf5c47 )
        ( 3275000, 0x232e7b6b )
        ( 3300000, 0x31ad492d )
        ( 3325000, 0x275f85e9 )
        ( 3350000, 0xb0b8a80f )
        ( 3375000, 0x9fda8cbf )
        ( 3400000, 0x8c38666e )
        ( 3425000, 0xe926d57c )
        ( 3450000, 0x04b0136e )
        ( 3475000, 0xefe761e9 )
        ( 3500000, 0xfd949cd3 )
        ( 3525000, 0xdc36a311 )
        ( 3550000, 0x9b06bd34 )
        ( 3575000, 0x70ec54e1 )
        ( 3600000, 0xf77db1cf )
        ( 3625000, 0xaca39b12 )
        ( 3650000, 0xdcfd95d5 )
        ( 3675000, 0x10127797 )
        ( 3700000, 0x2a0fa344 )
        ( 3725000, 0xcfe02f21 )
        ( 3750000, 0x85b5cf13 )
        ( 3775000, 0xc7dac369 )
        ( 3800000, 0xb48eaf5b )
        ( 3825000, 0xcc708d25 )
        ( 3850000, 0xd6c60724 )
        ( 3875000, 0x47392a14 )
        ( 3900000, 0x74d79dc7 )
        ( 3925000, 0x8b3c7f75 )
        ( 3950000, 0x93fbea9e )
        ( 3975000, 0xa3b26ab1 )
        ( 4000000, 0x70b089d4 )
        ( 4025000, 0x4cb52ab5 )
        ( 4050000, 0x1b428068 )
        ( 4075000, 0xb9c2ccdc )
        ( 4100000, 0x402a63cf )
        ( 4125000, 0x49551c34 )
        ( 4150000, 0xab045e0e )
        ( 4175000, 0xa80bddff )
        ( 4200000, 0xe6d92ce0 )
        ( 4225000, 0xec83c226 )
        ( 4250000, 0x01a8aba7 )
        ( 4275000, 0x72bad6e4 )
        ( 4300000, 0x2ed734d7 )
        ( 4325000, 0x3d6cd36c )
        ( 4350000, 0xad0494a1 )
        ( 4375000, 0xa5941687 )
        ( 4400000, 0x75ff1799 )
        ( 4425000, 0xb9ffaeba )
        ( 4450000, 0x73e25e9c )
        ( 4475000, 0xcfc28316 )
        ( 4500000, 0xd4d1998e )
        ( 4525000, 0x922ac2b4 )
        ( 4550000, 0xd3246d3a )
        ( 4575000, 0x1913c05e )
        ( 4600000, 0x22c7a119 )
        ( 4625000, 0xd9377408 )
        ( 4650000, 0x2a00268a )
        ( 4675000, 0xe08d3d51 )
        ( 4700000, 0x11973c21 )
        ( 4725000, 0xcb7dfe5f )
        ( 4750000, 0x585cd598 )
        ( 4775000, 0xbe7d985c )
        ( 4800000, 0xafd81d35 )
        ( 4825000, 0x6207b5e4 )
        ( 4850000, 0x61f28b3c )
        ( 4875000, 0xb406f877 )
        ( 4900000, 0x4d388a4b )
        ( 4925000, 0x59b365ca )
        ( 4950000, 0x1ec9da3d )
        ( 4975000, 0x5259a107 )
        ( 5000000, 0x4e615849 )
        ( 5025000, 0x72ab50bb )
        ( 5050000, 0x283f170b )
        ( 5075000, 0x43e5a6c3 )
        ( 5100000, 0x9d0d1244 )
        ( 5125000, 0xb2dd1b41 )
        ( 5150000, 0xab9c328a )
        ( 5175000, 0xa00012ee )
        ( 5200000, 0x66bc5813 )
        ( 5225000, 0x3c830e01 )
        ( 5250000, 0x8b1c01ce )
        ( 5275000, 0x167094e1 )
        ( 5300000, 0x2045d6eb )
        ( 5325000, 0x75f97beb )
        ( 5350000, 0xbea003ea )
        ( 5375000, 0x6c51aabf )
        ( 5400000, 0x9107dcc9 )
        ( 5425000, 0xb7b45469 )
        ( 5450000, 0x057c182d )
        ( 5475000, 0xbb621530 )
        ( 5500000, 0x45129984 )
        ( 5525000, 0x82823b82 )
        ( 5550000, 0x25ceb9fb )
        ( 5575000, 0x93b10a3a )
        ( 5600000, 0x9bc8f545 )
        ( 5625000, 0x4a34866b )
        ( 5650000, 0x0a0f5ec5 )
        ( 5675000, 0x7e3cf830 )
        ( 5700000, 0xf3f88a06 )
        ( 5725000, 0x195fac21 )
    ;

// Hard checkpoints of stake modifiers to ensure they are deterministic (testNet)
static std::map<int, unsigned int> mapStakeModifierCheckpointsTestNet =
    boost::assign::map_list_of
        ( 0, 0xfd11f4e7 )
    ;

// Get time weight
int64_t GetWeight(int64_t nIntervalBeginning, int64_t nIntervalEnd)
{
    // Kernel hash weight starts from 0 at the min age
    // this change increases active coins participating the hash and helps
    // to secure the network when proof-of-stake difficulty is low

    return min(nIntervalEnd - nIntervalBeginning - nStakeMinAge, (int64_t)nStakeMaxAge);
}

// Get the last stake modifier and its generation time from a given block
static bool GetLastStakeModifier(const CBlockIndex* pindex, uint64_t& nStakeModifier, int64_t& nModifierTime)
{
    if (!pindex)
        return error("GetLastStakeModifier: null pindex");
    while (pindex && pindex->pprev && !pindex->GeneratedStakeModifier())
        pindex = pindex->pprev;
    if (!pindex->GeneratedStakeModifier())
        return error("GetLastStakeModifier: no generation at genesis block");
    nStakeModifier = pindex->nStakeModifier;
    nModifierTime = pindex->GetBlockTime();
    return true;
}

// Get selection interval section (in seconds)
static int64_t GetStakeModifierSelectionIntervalSection(int nSection)
{
    assert (nSection >= 0 && nSection < 64);
    return (nModifierInterval * 63 / (63 + ((63 - nSection) * (MODIFIER_INTERVAL_RATIO - 1))));
}

// Get stake modifier selection interval (in seconds)
static int64_t GetStakeModifierSelectionInterval()
{
    int64_t nSelectionInterval = 0;
    for (int nSection=0; nSection<64; nSection++)
        nSelectionInterval += GetStakeModifierSelectionIntervalSection(nSection);
    return nSelectionInterval;
}

// select a block from the candidate blocks in vSortedByTimestamp, excluding
// already selected blocks in vSelectedBlocks, and with timestamp up to
// nSelectionIntervalStop.
static bool SelectBlockFromCandidates(vector<pair<int64_t, uint256> >& vSortedByTimestamp, map<uint256, const CBlockIndex*>& mapSelectedBlocks,
    int64_t nSelectionIntervalStop, uint64_t nStakeModifierPrev, const CBlockIndex** pindexSelected)
{
    bool fSelected = false;
    uint256 hashBest = 0;
    *pindexSelected = (const CBlockIndex*) 0;
    BOOST_FOREACH(const PAIRTYPE(int64_t, uint256)& item, vSortedByTimestamp)
    {
        if (!mapBlockIndex.count(item.second))
            return error("SelectBlockFromCandidates: failed to find block index for candidate block %s", item.second.ToString().c_str());
        const CBlockIndex* pindex = mapBlockIndex[item.second];
        if (fSelected && pindex->GetBlockTime() > nSelectionIntervalStop)
            break;
        if (mapSelectedBlocks.count(pindex->GetBlockHash()) > 0)
            continue;
        // compute the selection hash by hashing its proof-hash and the
        // previous proof-of-stake modifier
        CDataStream ss(SER_GETHASH, 0);
        ss << pindex->hashProof << nStakeModifierPrev;
        uint256 hashSelection = Hash(ss.begin(), ss.end());
        // the selection hash is divided by 2**32 so that proof-of-stake block
        // is always favored over proof-of-work block. this is to preserve
        // the energy efficiency property
        if (pindex->IsProofOfStake())
            hashSelection >>= 32;
        if (fSelected && hashSelection < hashBest)
        {
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*) pindex;
        }
        else if (!fSelected)
        {
            fSelected = true;
            hashBest = hashSelection;
            *pindexSelected = (const CBlockIndex*) pindex;
        }
    }
    if (fDebug && GetBoolArg("-printstakemodifier"))
        printf("SelectBlockFromCandidates: selection hash=%s\n", hashBest.ToString().c_str());
    return fSelected;
}

// Stake Modifier (hash modifier of proof-of-stake):
// The purpose of stake modifier is to prevent a txout (coin) owner from
// computing future proof-of-stake generated by this txout at the time
// of transaction confirmation. To meet kernel protocol, the txout
// must hash with a future stake modifier to generate the proof.
// Stake modifier consists of bits each of which is contributed from a
// selected block of a given block group in the past.
// The selection of a block is based on a hash of the block's proof-hash and
// the previous stake modifier.
// Stake modifier is recomputed at a fixed time interval instead of every
// block. This is to make it difficult for an attacker to gain control of
// additional bits in the stake modifier, even after generating a chain of
// blocks.
bool ComputeNextStakeModifier(const CBlockIndex* pindexPrev, uint64_t& nStakeModifier, bool& fGeneratedStakeModifier)
{
    nStakeModifier = 0;
    fGeneratedStakeModifier = false;
    if (!pindexPrev)
    {
        fGeneratedStakeModifier = true;
        return true;  // genesis block's modifier is 0
    }
    // First find current stake modifier and its generation block time
    // if it's not old enough, return the same stake modifier
    int64_t nModifierTime = 0;
    if (!GetLastStakeModifier(pindexPrev, nStakeModifier, nModifierTime))
        return error("ComputeNextStakeModifier: unable to get last modifier");
    if (fDebug)
    {
        printf("ComputeNextStakeModifier: prev modifier=0x%016" PRIx64 " time=%s\n", nStakeModifier, DateTimeStrFormat(nModifierTime).c_str());
    }
    if (nModifierTime / nModifierInterval >= pindexPrev->GetBlockTime() / nModifierInterval)
        return true;

    // Sort candidate blocks by timestamp
    vector<pair<int64_t, uint256> > vSortedByTimestamp;
    vSortedByTimestamp.reserve(64 * nModifierInterval / nTargetSpacing);
    int64_t nSelectionInterval = GetStakeModifierSelectionInterval();
    int64_t nSelectionIntervalStart = (pindexPrev->GetBlockTime() / nModifierInterval) * nModifierInterval - nSelectionInterval;
    const CBlockIndex* pindex = pindexPrev;
    while (pindex && pindex->GetBlockTime() >= nSelectionIntervalStart)
    {
        vSortedByTimestamp.push_back(make_pair(pindex->GetBlockTime(), pindex->GetBlockHash()));
        pindex = pindex->pprev;
    }
    int nHeightFirstCandidate = pindex ? (pindex->nHeight + 1) : 0;
    reverse(vSortedByTimestamp.begin(), vSortedByTimestamp.end());
    sort(vSortedByTimestamp.begin(), vSortedByTimestamp.end());

    // Select 64 blocks from candidate blocks to generate stake modifier
    uint64_t nStakeModifierNew = 0;
    int64_t nSelectionIntervalStop = nSelectionIntervalStart;
    map<uint256, const CBlockIndex*> mapSelectedBlocks;
    for (int nRound=0; nRound<min(64, (int)vSortedByTimestamp.size()); nRound++)
    {
        // add an interval section to the current selection round
        nSelectionIntervalStop += GetStakeModifierSelectionIntervalSection(nRound);
        // select a block from the candidates of current round
        if (!SelectBlockFromCandidates(vSortedByTimestamp, mapSelectedBlocks, nSelectionIntervalStop, nStakeModifier, &pindex))
            return error("ComputeNextStakeModifier: unable to select block at round %d", nRound);
        // write the entropy bit of the selected block
        nStakeModifierNew |= (((uint64_t)pindex->GetStakeEntropyBit()) << nRound);
        // add the selected block from candidates to selected list
        mapSelectedBlocks.insert(make_pair(pindex->GetBlockHash(), pindex));
        if (fDebug && GetBoolArg("-printstakemodifier"))
            printf("ComputeNextStakeModifier: selected round %d stop=%s height=%d bit=%d\n", nRound, DateTimeStrFormat(nSelectionIntervalStop).c_str(), pindex->nHeight, pindex->GetStakeEntropyBit());
    }

    // Print selection map for visualization of the selected blocks
    if (fDebug && GetBoolArg("-printstakemodifier"))
    {
        string strSelectionMap = "";
        // '-' indicates proof-of-work blocks not selected
        strSelectionMap.insert(0, pindexPrev->nHeight - nHeightFirstCandidate + 1, '-');
        pindex = pindexPrev;
        while (pindex && pindex->nHeight >= nHeightFirstCandidate)
        {
            // '=' indicates proof-of-stake blocks not selected
            if (pindex->IsProofOfStake())
                strSelectionMap.replace(pindex->nHeight - nHeightFirstCandidate, 1, "=");
            pindex = pindex->pprev;
        }
        BOOST_FOREACH(const PAIRTYPE(uint256, const CBlockIndex*)& item, mapSelectedBlocks)
        {
            // 'S' indicates selected proof-of-stake blocks
            // 'W' indicates selected proof-of-work blocks
            strSelectionMap.replace(item.second->nHeight - nHeightFirstCandidate, 1, item.second->IsProofOfStake()? "S" : "W");
        }
        printf("ComputeNextStakeModifier: selection height [%d, %d] map %s\n", nHeightFirstCandidate, pindexPrev->nHeight, strSelectionMap.c_str());
    }
    if (fDebug)
    {
        printf("ComputeNextStakeModifier: new modifier=0x%016" PRIx64 " time=%s\n", nStakeModifierNew, DateTimeStrFormat(pindexPrev->GetBlockTime()).c_str());
    }

    nStakeModifier = nStakeModifierNew;
    fGeneratedStakeModifier = true;
    return true;
}

// The stake modifier used to hash for a stake kernel is chosen as the stake
// modifier about a selection interval later than the coin generating the kernel
static bool GetKernelStakeModifier(uint256 hashBlockFrom, uint64_t& nStakeModifier, int& nStakeModifierHeight, int64_t& nStakeModifierTime, bool fPrintProofOfStake)
{
    nStakeModifier = 0;
    if (!mapBlockIndex.count(hashBlockFrom))
        return error("GetKernelStakeModifier() : block not indexed");
    const CBlockIndex* pindexFrom = mapBlockIndex[hashBlockFrom];
    nStakeModifierHeight = pindexFrom->nHeight;
    nStakeModifierTime = pindexFrom->GetBlockTime();
    int64_t nStakeModifierSelectionInterval = GetStakeModifierSelectionInterval();
    const CBlockIndex* pindex = pindexFrom;
    // loop to find the stake modifier later by a selection interval
    while (nStakeModifierTime < pindexFrom->GetBlockTime() + nStakeModifierSelectionInterval)
    {
        if (!pindex->pnext)
        {   // reached best block; may happen if node is behind on block chain
            if (fPrintProofOfStake || (pindex->GetBlockTime() + nStakeMinAge - nStakeModifierSelectionInterval > GetAdjustedTime()))
                return error("GetKernelStakeModifier() : reached best block %s at height %d from block %s",
                    pindex->GetBlockHash().ToString().c_str(), pindex->nHeight, hashBlockFrom.ToString().c_str());
            else
                return false;
        }
        pindex = pindex->pnext;
        if (pindex->GeneratedStakeModifier())
        {
            nStakeModifierHeight = pindex->nHeight;
            nStakeModifierTime = pindex->GetBlockTime();
        }
    }
    nStakeModifier = pindex->nStakeModifier;
    return true;
}

// ppcoin kernel protocol
// coinstake must meet hash target according to the protocol:
// kernel (input 0) must meet the formula
//     hash(nStakeModifier + txPrev.block.nTime + txPrev.offset + txPrev.nTime + txPrev.vout.n + nTime) < bnTarget * nCoinDayWeight
// this ensures that the chance of getting a coinstake is proportional to the
// amount of coin age one owns.
// The reason this hash is chosen is the following:
//   nStakeModifier: scrambles computation to make it very difficult to precompute
//                  future proof-of-stake at the time of the coin's confirmation
//   txPrev.block.nTime: prevent nodes from guessing a good timestamp to
//                       generate transaction for future advantage
//   txPrev.offset: offset of txPrev inside block, to reduce the chance of
//                  nodes generating coinstake at the same time
//   txPrev.nTime: reduce the chance of nodes generating coinstake at the same
//                 time
//   txPrev.vout.n: output number of txPrev, to reduce the chance of nodes
//                  generating coinstake at the same time
//   block/tx hash should not be used here as they can be generated in vast
//   quantities so as to generate blocks faster, degrading the system back into
//   a proof-of-work situation.
//
bool CheckStakeKernelHash(unsigned int nBits, const CBlock& blockFrom, unsigned int nTxPrevOffset, const CTransaction& txPrev, const COutPoint& prevout, unsigned int nTimeTx, uint256& hashProofOfStake, uint256& targetProofOfStake, bool fPrintProofOfStake)
{
    if (nTimeTx < txPrev.nTime)  // Transaction timestamp violation
        return error("CheckStakeKernelHash() : nTime violation");

    unsigned int nTimeBlockFrom = blockFrom.GetBlockTime();
    if (nTimeBlockFrom + nStakeMinAge > nTimeTx) // Min age requirement
        return error("CheckStakeKernelHash() : min age violation");

    CBigNum bnTargetPerCoinDay;
    bnTargetPerCoinDay.SetCompact(nBits);
    int64_t nValueIn = txPrev.vout[prevout.n].nValue;

    uint256 hashBlockFrom = blockFrom.GetHash();

    CBigNum bnCoinDayWeight = CBigNum(nValueIn) * GetWeight((int64_t)txPrev.nTime, (int64_t)nTimeTx) / COIN / (24 * 60 * 60);
    targetProofOfStake = (bnCoinDayWeight * bnTargetPerCoinDay).getuint256();

    // Calculate hash
    CDataStream ss(SER_GETHASH, 0);
    uint64_t nStakeModifier = 0;
    int nStakeModifierHeight = 0;
    int64_t nStakeModifierTime = 0;

    if (!GetKernelStakeModifier(hashBlockFrom, nStakeModifier, nStakeModifierHeight, nStakeModifierTime, fPrintProofOfStake))
        return false;
    ss << nStakeModifier;

    ss << nTimeBlockFrom << nTxPrevOffset << txPrev.nTime << prevout.n << nTimeTx;
    hashProofOfStake = Hash(ss.begin(), ss.end());
    if (fPrintProofOfStake)
    {
        printf("CheckStakeKernelHash() : using modifier 0x%016" PRIx64 " at height=%d timestamp=%s for block from height=%d timestamp=%s\n",
            nStakeModifier, nStakeModifierHeight,
            DateTimeStrFormat(nStakeModifierTime).c_str(),
            mapBlockIndex[hashBlockFrom]->nHeight,
            DateTimeStrFormat(blockFrom.GetBlockTime()).c_str());
        printf("CheckStakeKernelHash() : check modifier=0x%016" PRIx64 " nTimeBlockFrom=%u nTxPrevOffset=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProof=%s\n",
            nStakeModifier,
            nTimeBlockFrom, nTxPrevOffset, txPrev.nTime, prevout.n, nTimeTx,
            hashProofOfStake.ToString().c_str());
    }

    // Now check if proof-of-stake hash meets target protocol
    if (CBigNum(hashProofOfStake) > bnCoinDayWeight * bnTargetPerCoinDay)
        return false;
    if (fDebug && !fPrintProofOfStake)
    {
        printf("CheckStakeKernelHash() : using modifier 0x%016" PRIx64 " at height=%d timestamp=%s for block from height=%d timestamp=%s\n",
            nStakeModifier, nStakeModifierHeight,
            DateTimeStrFormat(nStakeModifierTime).c_str(),
            mapBlockIndex[hashBlockFrom]->nHeight,
            DateTimeStrFormat(blockFrom.GetBlockTime()).c_str());
        printf("CheckStakeKernelHash() : pass modifier=0x%016" PRIx64 " nTimeBlockFrom=%u nTxPrevOffset=%u nTimeTxPrev=%u nPrevout=%u nTimeTx=%u hashProof=%s\n",
            nStakeModifier,
            nTimeBlockFrom, nTxPrevOffset, txPrev.nTime, prevout.n, nTimeTx,
            hashProofOfStake.ToString().c_str());
    }
    return true;
}

// Check kernel hash target and coinstake signature
bool CheckProofOfStake(const CTransaction& tx, unsigned int nBits, uint256& hashProofOfStake, uint256& targetProofOfStake)
{
    if (!tx.IsCoinStake())
        return error("CheckProofOfStake() : called on non-coinstake %s", tx.GetHash().ToString().c_str());

    // Kernel (input 0) must match the stake hash target per coin age (nBits)
    const CTxIn& txin = tx.vin[0];

    // First try finding the previous transaction in database
    CTxDB txdb("r");
    CTransaction txPrev;
    CTxIndex txindex;
    if (!txPrev.ReadFromDisk(txdb, txin.prevout, txindex))
        return tx.DoS(1, error("CheckProofOfStake() : INFO: read txPrev failed"));  // previous transaction not in main chain, may occur during initial download

    // Verify signature
    if (!VerifySignature(txPrev, tx, 0, 0))
        return tx.DoS(100, error("CheckProofOfStake() : VerifySignature failed on coinstake %s", tx.GetHash().ToString().c_str()));

    // Read block header
    CBlock block;
    if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
        return fDebug? error("CheckProofOfStake() : read block failed") : false; // unable to read block of previous transaction

    if (!CheckStakeKernelHash(nBits, block, txindex.pos.nTxPos - txindex.pos.nBlockPos, txPrev, txin.prevout, tx.nTime, hashProofOfStake, targetProofOfStake, fDebug))
        return tx.DoS(1, error("CheckProofOfStake() : INFO: check kernel failed on coinstake %s, hashProof=%s", tx.GetHash().ToString().c_str(), hashProofOfStake.ToString().c_str())); // may occur during initial download or if behind on block chain sync

    return true;
}

// Check whether the coinstake timestamp meets protocol
bool CheckCoinStakeTimestamp(int64_t nTimeBlock, int64_t nTimeTx)
{
    // v0.3 protocol
    return (nTimeBlock == nTimeTx);
}

// Get stake modifier checksum
unsigned int GetStakeModifierChecksum(const CBlockIndex* pindex)
{
    assert (pindex->pprev || pindex->GetBlockHash() == (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet));
    // Hash previous checksum with flags, hashProofOfStake and nStakeModifier
    CDataStream ss(SER_GETHASH, 0);
    if (pindex->pprev)
        ss << pindex->pprev->nStakeModifierChecksum;
    ss << pindex->nFlags << (pindex->IsProofOfStake() ? pindex->hashProof : 0) << pindex->nStakeModifier;
    uint256 hashChecksum = Hash(ss.begin(), ss.end());
    hashChecksum >>= (256 - 32);
    return hashChecksum.Get64();
}

// Check stake modifier hard checkpoints
bool CheckStakeModifierCheckpoints(int nHeight, unsigned int nStakeModifierChecksum)
{
    MapModifierCheckpoints& checkpoints = (fTestNet ? mapStakeModifierCheckpointsTestNet : mapStakeModifierCheckpoints);

    if (checkpoints.count(nHeight))
        return nStakeModifierChecksum == checkpoints[nHeight];
    return true;
}
