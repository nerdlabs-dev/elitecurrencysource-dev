// Copyright (c) 8888 Bitcoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "net.h"
#include "bitcoinrpc.h"
#include "alert.h"
#include "wallet.h"
#include "db.h"
#include "walletdb.h"
#include "version.h"

#include <algorithm>

using namespace json_spirit;
using namespace std;

Value getconnectioncount(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getconnectioncount\n"
            "Returns the number of connections to other nodes.");

    LOCK(cs_vNodes);
    return (int)vNodes.size();
}

static void CopyNodeStats(std::vector<CNodeStats>& vstats)
{
    vstats.clear();

    LOCK(cs_vNodes);
    vstats.reserve(vNodes.size());
    BOOST_FOREACH(CNode* pnode, vNodes) {
        CNodeStats stats;
        pnode->copyStats(stats);
        vstats.push_back(stats);
    }
}

Value getpeerinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getpeerinfo\n"
            "Returns data about each connected network node:\n"
            "id, addr, addrlocal, services, lastsend, lastrecv, bytessent, bytesrecv,\n"
            "conntime, pingtime, pingwait (seconds), version, subver, inbound,\n"
            "addnode, startingheight, banscore, syncnode.");

    vector<CNodeStats> vstats;
    CopyNodeStats(vstats);

    Array ret;

    BOOST_FOREACH(const CNodeStats& stats, vstats) {
        Object obj;

        obj.push_back(Pair("id", stats.nodeid));
        obj.push_back(Pair("addr", stats.addrName));
        if (!stats.addrLocal.empty())
            obj.push_back(Pair("addrlocal", stats.addrLocal));
        obj.push_back(Pair("services", strprintf("%016" PRIx64, stats.nServices)));
        obj.push_back(Pair("lastsend", (boost::int64_t)stats.nLastSend));
        obj.push_back(Pair("lastrecv", (boost::int64_t)stats.nLastRecv));
        obj.push_back(Pair("bytessent", (boost::int64_t)stats.nSendBytes));
        obj.push_back(Pair("bytesrecv", (boost::int64_t)stats.nRecvBytes));
        obj.push_back(Pair("conntime", (boost::int64_t)stats.nTimeConnected));
        if (stats.dPingTime > 0)
            obj.push_back(Pair("pingtime", stats.dPingTime));
        if (stats.dPingWait > 0)
            obj.push_back(Pair("pingwait", stats.dPingWait));
        obj.push_back(Pair("version", stats.nVersion));
        obj.push_back(Pair("subver", stats.strSubVer));
        obj.push_back(Pair("inbound", stats.fInbound));
        obj.push_back(Pair("addnode", stats.fAddedNode));
        obj.push_back(Pair("startingheight", stats.nStartingHeight));
        obj.push_back(Pair("banscore", stats.nMisbehavior));
        obj.push_back(Pair("syncnode", stats.fSyncNode));

        ret.push_back(obj);
    }

    return ret;
}

Value ping(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "ping\n"
            "Asks every connected peer to answer a ping; the round trip shows\n"
            "as pingtime in getpeerinfo (pingwait while it's outstanding).");

    LOCK(cs_vNodes);
    BOOST_FOREACH(CNode* pnode, vNodes)
        pnode->fPingQueued = true;
    return Value::null;
}

Value addnode(const Array& params, bool fHelp)
{
    string strCommand;
    if (params.size() == 2)
        strCommand = params[1].get_str();
    if (fHelp || params.size() != 2 ||
        (strCommand != "onetry" && strCommand != "add" && strCommand != "remove"))
        throw runtime_error(
            "addnode <node> <add|remove|onetry>\n"
            "add: keep a connection open to <node> (ip, ip:port or host name), retrying\n"
            "     every 2 minutes, like -addnode. Not saved over a restart; put\n"
            "     addnode=<node> in 1337.conf for that.\n"
            "remove: stop keeping a connection to <node> (see disconnectnode to drop it now).\n"
            "onetry: connect to <node> once.");

    string strNode = params[0].get_str();

    if (strCommand == "onetry")
    {
        CAddress addr;
        if (!OpenNetworkConnection(addr, NULL, strNode.c_str()))
            throw JSONRPCError(RPC_CLIENT_NODE_NOT_CONNECTED, "Error: Unable to connect to " + strNode);
        return Value::null;
    }

    {
        LOCK(cs_vAddedNodes);
        list<string>::iterator it = std::find(vAddedNodes.begin(), vAddedNodes.end(), strNode);

        if (strCommand == "add")
        {
            if (it != vAddedNodes.end())
                throw JSONRPCError(RPC_CLIENT_NODE_ALREADY_ADDED, "Error: Node already added");
            vAddedNodes.push_back(strNode);
        }
        else
        {
            if (it == vAddedNodes.end())
                throw JSONRPCError(RPC_CLIENT_NODE_NOT_ADDED, "Error: Node has not been added.");
            vAddedNodes.erase(it);
        }
    }
    if (strCommand == "add")
        WakeAddedConnections();

    return Value::null;
}

Value getaddednodeinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 2)
        throw runtime_error(
            "getaddednodeinfo [dns=true] [node]\n"
            "Returns the nodes added with addnode or -addnode (or just [node]).\n"
            "If dns is false, only the names are listed; otherwise each entry also\n"
            "shows whether it's connected and the addresses it resolves to.");

    bool fDns = params.size() > 0 ? ParamBool(params[0], "dns") : true;

    list<string> laddedNodes;
    {
        LOCK(cs_vAddedNodes);
        laddedNodes = vAddedNodes;
    }
    if (params.size() == 2)
    {
        string strNode = params[1].get_str();
        if (std::find(laddedNodes.begin(), laddedNodes.end(), strNode) == laddedNodes.end())
            throw JSONRPCError(RPC_CLIENT_NODE_NOT_ADDED, "Error: Node has not been added.");
        laddedNodes.clear();
        laddedNodes.push_back(strNode);
    }

    Array ret;
    BOOST_FOREACH(const string& strAddNode, laddedNodes)
    {
        Object obj;
        obj.push_back(Pair("addednode", strAddNode));
        if (fDns)
        {
            vector<CService> vservNode;
            Lookup(strAddNode.c_str(), vservNode, GetDefaultPort(), fNameLookup, 0);

            Array addresses;
            bool fConnected = false;
            {
                LOCK(cs_vNodes);
                // connected by name (through a proxy)?
                BOOST_FOREACH(CNode* pnode, vNodes)
                    if (pnode->addrName == strAddNode)
                    {
                        Object node;
                        node.push_back(Pair("address", pnode->addr.ToStringIPPort()));
                        node.push_back(Pair("connected", pnode->fInbound ? "inbound" : "outbound"));
                        addresses.push_back(node);
                        fConnected = true;
                    }
                BOOST_FOREACH(const CService& addrNode, vservNode)
                {
                    bool fFound = false;
                    BOOST_FOREACH(CNode* pnode, vNodes)
                        if (pnode->addr == addrNode && pnode->addrName != strAddNode)
                        {
                            Object node;
                            node.push_back(Pair("address", addrNode.ToStringIPPort()));
                            node.push_back(Pair("connected", pnode->fInbound ? "inbound" : "outbound"));
                            addresses.push_back(node);
                            fFound = fConnected = true;
                            break;
                        }
                    if (!fFound)
                    {
                        Object node;
                        node.push_back(Pair("address", addrNode.ToStringIPPort()));
                        node.push_back(Pair("connected", "false"));
                        addresses.push_back(node);
                    }
                }
            }
            obj.push_back(Pair("connected", fConnected));
            obj.push_back(Pair("addresses", addresses));
        }
        ret.push_back(obj);
    }
    return ret;
}

Value disconnectnode(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 1 || params.size() > 2)
        throw runtime_error(
            "disconnectnode <address> | disconnectnode \"\" <nodeid>\n"
            "Disconnects a peer, by the address shown in getpeerinfo (ip:port, or\n"
            "the name used to connect) or by its id. It may reconnect later; see\n"
            "addnode remove and setban.");

    string strAddr = params[0].get_str();
    int nNodeId = -1;
    if (params.size() == 2)
    {
        if (!strAddr.empty())
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Give either an address or a node id, not both");
        nNodeId = ParamInt(params[1], "nodeid");
        if (nNodeId < 0)
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Invalid node id");
    }
    else if (strAddr.empty())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Give an address or a node id");

    if (DisconnectNodes(strAddr, nNodeId) == 0)
        throw JSONRPCError(RPC_CLIENT_NODE_NOT_CONNECTED, "Node not found in connected nodes");
    return Value::null;
}

Value getnettotals(const Array& params, bool fHelp)
{
    if (fHelp || params.size() > 0)
        throw runtime_error(
            "getnettotals\n"
            "Returns bytes received and sent on all connections since startup,\n"
            "and the current time in milliseconds.");

    Object obj;
    obj.push_back(Pair("totalbytesrecv", (boost::uint64_t)CNode::nTotalBytesRecv));
    obj.push_back(Pair("totalbytessent", (boost::uint64_t)CNode::nTotalBytesSent));
    obj.push_back(Pair("timemillis", (boost::int64_t)GetTimeMillis()));
    return obj;
}

static const char* NetworkName(enum Network net)
{
    switch (net)
    {
    case NET_IPV4: return "ipv4";
    case NET_IPV6: return "ipv6";
    case NET_TOR:  return "onion";
    case NET_I2P:  return "i2p";
    default:       return "";
    }
}

Value getnetworkinfo(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "getnetworkinfo\n"
            "Returns information about the node's networking: version, subversion,\n"
            "protocolversion, localservices, timeoffset, connections, networks,\n"
            "relayfee, localaddresses, warnings.");

    Object obj;
    obj.push_back(Pair("version", (int)CLIENT_VERSION));
    obj.push_back(Pair("subversion", FormatSubVersion(CLIENT_NAME, CLIENT_VERSION, std::vector<string>())));
    obj.push_back(Pair("protocolversion", (int)PROTOCOL_VERSION));
    obj.push_back(Pair("localservices", strprintf("%016" PRIx64, nLocalServices)));
    obj.push_back(Pair("timeoffset", (boost::int64_t)GetTimeOffset()));
    {
        LOCK(cs_vNodes);
        int nInbound = 0;
        BOOST_FOREACH(CNode* pnode, vNodes)
            if (pnode->fInbound)
                nInbound++;
        obj.push_back(Pair("connections", (int)vNodes.size()));
        obj.push_back(Pair("connections_in", nInbound));
        obj.push_back(Pair("connections_out", (int)vNodes.size() - nInbound));
    }

    Array networks;
    for (int n = 0; n < NET_MAX; ++n)
    {
        enum Network network = static_cast<enum Network>(n);
        if (network == NET_UNROUTABLE)
            continue;
        proxyType proxy;
        Object net;
        net.push_back(Pair("name", NetworkName(network)));
        net.push_back(Pair("limited", IsLimited(network)));
        net.push_back(Pair("reachable", IsReachable(network)));
        net.push_back(Pair("proxy", GetProxy(network, proxy) ? proxy.first.ToStringIPPort() : string()));
        networks.push_back(net);
    }
    obj.push_back(Pair("networks", networks));
    obj.push_back(Pair("relayfee", ValueFromAmount(MIN_RELAY_TX_FEE)));

    Array localAddresses;
    std::vector<std::pair<CService, int> > vLocal;
    GetLocalAddresses(vLocal);
    for (unsigned int i = 0; i < vLocal.size(); i++)
    {
        Object rec;
        rec.push_back(Pair("address", vLocal[i].first.ToStringIP()));
        rec.push_back(Pair("port", vLocal[i].first.GetPort()));
        rec.push_back(Pair("score", vLocal[i].second));
        localAddresses.push_back(rec);
    }
    obj.push_back(Pair("localaddresses", localAddresses));
    obj.push_back(Pair("warnings", GetWarnings("statusbar")));
    return obj;
}

// setban takes a single address; "/32" (IPv4) or "/128" (IPv6) is accepted
// for compatibility with tools that always send a subnet
static CNetAddr ParseBanAddress(const string& strIn)
{
    string str = strIn;
    size_t nSlash = str.find('/');
    if (nSlash != string::npos)
    {
        string strMask = str.substr(nSlash + 1);
        str = str.substr(0, nSlash);
        CNetAddr addr(str, false);
        if (!(addr.IsValid() && ((addr.IsIPv4() && strMask == "32") || (!addr.IsIPv4() && strMask == "128"))))
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Error: only single addresses can be banned (no subnets)");
        return addr;
    }
    CNetAddr addr(str, false);
    if (!addr.IsValid())
        throw JSONRPCError(RPC_INVALID_PARAMETER, "Error: Invalid IP address");
    return addr;
}

Value setban(const Array& params, bool fHelp)
{
    string strCommand;
    if (params.size() >= 2)
        strCommand = params[1].get_str();
    if (fHelp || params.size() < 2 || params.size() > 4 || (strCommand != "add" && strCommand != "remove"))
        throw runtime_error(
            "setban <ip> <add|remove> [bantime] [absolute=false]\n"
            "add: ban the IP address (disconnecting it) for bantime seconds\n"
            "     (default -bantime, 24 hours), or until the unix time bantime if\n"
            "     absolute is true.\n"
            "remove: lift the ban.\n"
            "Bans last until the node restarts.");

    CNetAddr addr = ParseBanAddress(params[0].get_str());

    if (strCommand == "add")
    {
        if (CNode::IsBanned(addr))
            throw JSONRPCError(RPC_CLIENT_NODE_ALREADY_ADDED, "Error: IP already banned");

        int64_t nBanTime = params.size() > 2 ? ParamInt64(params[2], "bantime") : 0;
        bool fAbsolute = params.size() > 3 ? ParamBool(params[3], "absolute") : false;
        if (nBanTime <= 0)
            nBanTime = GetArg("-bantime", 60*60*24);
        int64_t nUntil = fAbsolute ? nBanTime : GetTime() + nBanTime;
        if (nUntil <= GetTime())
            throw JSONRPCError(RPC_INVALID_PARAMETER, "Error: ban time is in the past");
        CNode::Ban(addr, nUntil);

        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
            if ((CNetAddr)pnode->addr == addr)
                pnode->fDisconnect = true;
    }
    else
    {
        if (!CNode::Unban(addr))
            throw JSONRPCError(RPC_MISC_ERROR, "Error: Unban failed, the address wasn't banned");
    }
    return Value::null;
}

Value listbanned(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "listbanned\n"
            "Lists banned IP addresses, with banned_until (unix time).");

    std::map<CNetAddr, int64_t> banned;
    CNode::GetBanned(banned);

    Array ret;
    for (std::map<CNetAddr, int64_t>::const_iterator it = banned.begin(); it != banned.end(); ++it)
    {
        Object rec;
        rec.push_back(Pair("address", it->first.ToStringIP() + (it->first.IsIPv4() ? "/32" : "/128")));
        rec.push_back(Pair("banned_until", (boost::int64_t)it->second));
        ret.push_back(rec);
    }
    return ret;
}

Value clearbanned(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "clearbanned\n"
            "Lifts all bans.");

    CNode::ClearBanned();
    return Value::null;
}

Value uptime(const Array& params, bool fHelp)
{
    if (fHelp || params.size() != 0)
        throw runtime_error(
            "uptime\n"
            "Returns how many seconds the node has been running.");

    return (boost::int64_t)(GetTime() - nStartupTime);
}

// ppcoin: send alert.  
// There is a known deadlock situation with ThreadMessageHandler
// ThreadMessageHandler: holds cs_vSend and acquiring cs_main in SendMessages()
// ThreadRPCServer: holds cs_main and acquiring cs_vSend in alert.RelayTo()/PushMessage()/BeginMessage()
Value sendalert(const Array& params, bool fHelp)
{
    if (fHelp || params.size() < 6)
        throw runtime_error(
            "sendalert <message> <privatekey> <minver> <maxver> <priority> <id> [cancelupto]\n"
            "<message> is the alert text message\n"
            "<privatekey> is hex string of alert master private key\n"
            "<minver> is the minimum applicable internal client version\n"
            "<maxver> is the maximum applicable internal client version\n"
            "<priority> is integer priority number\n"
            "<id> is the alert id\n"
            "[cancelupto] cancels all alert id's up to this number\n"
            "Returns true or false.");

    CAlert alert;
    CKey key;

    alert.strStatusBar = params[0].get_str();
    alert.nMinVer = params[2].get_int();
    alert.nMaxVer = params[3].get_int();
    alert.nPriority = params[4].get_int();
    alert.nID = params[5].get_int();
    if (params.size() > 6)
        alert.nCancel = params[6].get_int();
    alert.nVersion = PROTOCOL_VERSION;
    alert.nRelayUntil = GetAdjustedTime() + 365*24*60*60;
    alert.nExpiration = GetAdjustedTime() + 365*24*60*60;

    CDataStream sMsg(SER_NETWORK, PROTOCOL_VERSION);
    sMsg << (CUnsignedAlert)alert;
    alert.vchMsg = vector<unsigned char>(sMsg.begin(), sMsg.end());

    vector<unsigned char> vchPrivKey = ParseHex(params[1].get_str());
    key.SetPrivKey(CPrivKey(vchPrivKey.begin(), vchPrivKey.end())); // if key is not correct openssl may crash
    if (!key.Sign(Hash(alert.vchMsg.begin(), alert.vchMsg.end()), alert.vchSig))
        throw runtime_error(
            "Unable to sign alert, check private key?\n");  
    if(!alert.ProcessAlert()) 
        throw runtime_error(
            "Failed to process alert.\n");
    // Relay alert
    {
        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
            alert.RelayTo(pnode);
    }

    Object result;
    result.push_back(Pair("strStatusBar", alert.strStatusBar));
    result.push_back(Pair("nVersion", alert.nVersion));
    result.push_back(Pair("nMinVer", alert.nMinVer));
    result.push_back(Pair("nMaxVer", alert.nMaxVer));
    result.push_back(Pair("nPriority", alert.nPriority));
    result.push_back(Pair("nID", alert.nID));
    if (alert.nCancel > 0)
        result.push_back(Pair("nCancel", alert.nCancel));
    return result;
}
