// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcserver.h"

#include "main.h"
#include "net.h"
#include "netbase.h"
#include "protocol.h"
#include "sync.h"
#include "util.h"

#include <boost/foreach.hpp>
#include "json/json_spirit_value.h"

using namespace json_spirit;
using namespace std;
/**
 * ��ȡ����������
 * @param params �������
 * @param bHelp ���������Ϣ��
 * @return
 */
Value getconnectioncount(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
        throw runtime_error(
            "getconnectioncount\n"
            "\nReturns the number of connections to other nodes.\n"
            "\nbResult:\n"
            "n          (numeric) The connection count\n"
            "\nExamples:\n"
            + HelpExampleCli("getconnectioncount", "")
            + HelpExampleRpc("getconnectioncount", "")
        );
    }
    LOCK(g_cs_vNodes);
    return (int)g_vNodes.size();
}
/**
 * pingһ�����нڵ�
 * @param params �������
 * @param bHelp ���������Ϣ��
 * @return
 */
Value ping(const Array& params, bool bHelp) {
    if (bHelp || params.size() != 0) {
        throw runtime_error(
            "ping\n"
            "\nRequests that a ping be sent to all other nodes, to measure ping time.\n"
            "Results provided in getpeerinfo, pingtime and pingwait fields are decimal seconds.\n"
            "Ping command is handled in queue with all other commands, so it measures processing backlog, not just network ping.\n"
            "\nExamples:\n"
            + HelpExampleCli("ping", "")
            + HelpExampleRpc("ping", "")
        );
    }
	// Request that each node send a ping during next message processing pass
    LOCK(g_cs_vNodes);
    for (auto pNode : g_vNodes) {
        pNode->m_bPingQueued = true;
	}
	return Value::null;
}

static void CopyNodeStats(vector<CNodeStats>& vcNodeStats) {
	vcNodeStats.clear();
    LOCK(g_cs_vNodes);
    vcNodeStats.reserve(g_vNodes.size());
    for(auto pNode : g_vNodes) {
		CNodeStats cNodeStats;
		pNode->copyStats(cNodeStats);
		vcNodeStats.push_back(cNodeStats);
	}
}
/**
 * ���ÿ�����ӽڵ����Ϣ
 * @param params �������
 * @param bHelp ���������Ϣ��
 * @return
 */
Value getpeerinfo(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
        throw runtime_error(
            "getpeerinfo\n"
            "\nReturns data about each connected network node as a json array of objects.\n"
            "\nbResult:\n"
            "[\n"
            "  {\n"
            "    \"addr\":\"host:port\",      (string) The ip address and port of the peer\n"
            "    \"addrlocal\":\"ip:port\",   (string) local address\n"
            "    \"services\":\"00000001\",   (string) The services\n"
            "    \"lastsend\": ttt,           (numeric) The time in seconds since epoch (Jan 1 1970 GMT) of the last send\n"
            "    \"lastrecv\": ttt,           (numeric) The time in seconds since epoch (Jan 1 1970 GMT) of the last receive\n"
            "    \"bytessent\": n,            (numeric) The total bytes sent\n"
            "    \"bytesrecv\": n,            (numeric) The total bytes received\n"
            "    \"conntime\": ttt,           (numeric) The connection time in seconds since epoch (Jan 1 1970 GMT)\n"
            "    \"pingtime\": n,             (numeric) ping time\n"
            "    \"pingwait\": n,             (numeric) ping wait\n"
            "    \"version\": v,              (numeric) The peer version, such as 7001\n"
            "    \"subver\": \"/Satoshi:0.8.5/\",  (string) The string version\n"
            "    \"inbound\": true|false,     (boolean) Inbound (true) or Outbound (false)\n"
            "    \"startingheight\": n,       (numeric) The starting height (block) of the peer\n"
            "    \"banscore\": n,              (numeric) The ban score (stats.nMisbehavior)\n"
            "    \"syncnode\" : true|false     (booleamn) if sync node\n"
            "  }\n"
            "  ,...\n"
            "}\n"

            "\nExamples:\n"
            + HelpExampleCli("getpeerinfo", "")
            + HelpExampleRpc("getpeerinfo", "")
        );
    }
    vector<CNodeStats> vcStats;
    CopyNodeStats(vcStats);

	Array ret;

	for (const CNodeStats& stats : vcStats) {
		Object obj;
		ST_NodeStateStats tStatestats;
		bool bStateStats = GetNodeStateStats(stats.m_nNodeId, tStatestats);
		obj.push_back(Pair("addr", stats.m_strAddrName));
		if (!(stats.m_strAddrLocal.empty())) {
			obj.push_back(Pair("addrlocal", stats.m_strAddrLocal));
		}
		obj.push_back(Pair("services", strprintf("%08x", stats.m_ullServices)));
		obj.push_back(Pair("lastsend", stats.m_llLastSend));
		obj.push_back(Pair("lastrecv", stats.m_llLastRecv));
		obj.push_back(Pair("bytessent", stats.m_ullSendBytes));
		obj.push_back(Pair("bytesrecv", stats.m_ullRecvBytes));
		obj.push_back(Pair("conntime", stats.m_llTimeConnected));
		obj.push_back(Pair("pingtime", stats.m_dPingTime));
		if (stats.m_dPingWait > 0.0) {
			obj.push_back(Pair("pingwait", stats.m_dPingWait));
		}
		obj.push_back(Pair("version", stats.m_nVersion));
		// Use the sanitized form of subver here, to avoid tricksy remote peers from
		// corrupting or modifiying the JSON output by putting special characters in
		// their ver message.
		obj.push_back(Pair("subver", stats.m_strCleanSubVer));
		obj.push_back(Pair("inbound", stats.m_bInbound));
		obj.push_back(Pair("startingheight", stats.m_nStartingHeight));
		if (bStateStats) {
			obj.push_back(Pair("banscore", tStatestats.nMisbehavior));
		}
		obj.push_back(Pair("syncnode", stats.m_bSyncNode));

		ret.push_back(obj);
	}

	return ret;
}
/**
 * ��ӽڵ�
 * @param params �������
 * @param bHelp ���������Ϣ��
 * @return
 */
Value addnode(const Array& params, bool bHelp) {
	string strCommand;
	if (params.size() == 2) {
		strCommand = params[1].get_str();
	}
    if (bHelp || params.size() != 2 ||
        (strCommand != "onetry" && strCommand != "add" && strCommand != "remove")) {
        throw runtime_error(
            "addnode \"node\" \"add|remove|onetry\"\n"
            "\nAttempts add or remove a node from the addnode list.\n"
            "Or try a connection to a node once.\n"
            "\nArguments:\n"
            "1. \"node\"     (string, required) The node (see getpeerinfo for nodes)\n"
            "2. \"command\"  (string, required) 'add' to add a node to the list, 'remove' to remove a node from the list, 'onetry' to try a connection to the node once\n"
			"\nResult:\n"
        	"\nExamples:\n"
            + HelpExampleCli("addnode", "\"192.168.0.6:8333\" \"onetry\"")
            + HelpExampleRpc("addnode", "\"192.168.0.6:8333\", \"onetry\"")
        );
    }
	string strNode = params[0].get_str();

	if (strCommand == "onetry") {
		CAddress cAddr;
		ConnectNode(cAddr, strNode.c_str());
		return Value::null;
	}

    LOCK(g_cs_vAddedNodes);
    vector<string>::iterator it = g_vAddedNodes.begin();
	for (; it != g_vAddedNodes.end(); it++) {
		if (strNode == *it) {
			break;
		}
	}
	if (strCommand == "add") {
		if (it != g_vAddedNodes.end()) {
			throw JSONRPCError(RPC_CLIENT_NODE_ALREADY_ADDED, "Error: Node already added");
		}
		g_vAddedNodes.push_back(strNode);
	} else if (strCommand == "remove") {
		if (it == g_vAddedNodes.end()) {
			throw JSONRPCError(RPC_CLIENT_NODE_NOT_ADDED, "Error: Node has not been added.");
		}
		g_vAddedNodes.erase(it);
	}

	return Value::null;
}
/**
 * ������ӽڵ����Ϣ
 * @param params �������
 * @param bHelp ���������Ϣ��
 * @return
 */
Value getaddednodeinfo(const Array& params, bool bHelp) {
	if (bHelp || params.size() < 1 || params.size() > 2) {
        throw runtime_error(
            "getaddednodeinfo dns ( \"node\" )\n"
            "\nReturns information about the given added node, or all added nodes\n"
            "(note that onetry addnodes are not listed here)\n"
            "If dns is false, only a list of added nodes will be provided,\n"
            "otherwise connected information will also be available.\n"
            "\nArguments:\n"
            "1. dns        (boolean, required) If false, only a list of added nodes will be provided, otherwise connected information will also be available.\n"
            "2. \"node\"   (string, optional) If provided, return information about this specific node, otherwise all nodes are returned.\n"
            "\nResult:\n"
            "[\n"
            "  {\n"
            "    \"addednode\" : \"192.168.0.201\",   (string) The node ip address\n"
            "    \"connected\" : true|false,          (boolean) If connected\n"
            "    \"addresses\" : [\n"
            "       {\n"
            "         \"address\" : \"192.168.0.201:8333\",  (string) The Coin server host and port\n"
            "         \"connected\" : \"outbound\"           (string) connection, inbound or outbound\n"
            "       }\n"
            "       ,...\n"
            "     ]\n"
            "  }\n"
            "  ,...\n"
            "]\n"
            "\nExamples:\n"
            + HelpExampleCli("getaddednodeinfo", "true")
            + HelpExampleCli("getaddednodeinfo", "true \"192.168.0.201\"")
            + HelpExampleRpc("getaddednodeinfo", "true, \"192.168.0.201\"")
        );
    }
	bool bDns = params[0].get_bool();

	list<string> laddedNodes(0);
	if (params.size() == 1) {
		LOCK(g_cs_vAddedNodes);
		for (auto& strAddNode : g_vAddedNodes) {
			laddedNodes.push_back(strAddNode);
		}
	} else {
		string strNode = params[1].get_str();
		LOCK(g_cs_vAddedNodes);
		for (auto & strAddNode : g_vAddedNodes) {
			if (strAddNode == strNode) {
				laddedNodes.push_back(strAddNode);
				break;
			}
		}
		if (laddedNodes.size() == 0) {
			throw JSONRPCError(RPC_CLIENT_NODE_NOT_ADDED, "Error: Node has not been added.");
		}
	}

	Array ret;
	if (!bDns) {
		for (auto & strAddNode : laddedNodes) {
			Object obj;
			obj.push_back(Pair("addednode", strAddNode));
			ret.push_back(obj);
		}
		return ret;
	}

	list<pair<string, vector<CService> > > laddedAddreses(0);
	for (auto& strAddNode : laddedNodes) {
		vector<CService> vcServNode(0);
		if (Lookup(strAddNode.c_str(), vcServNode, SysCfg().GetDefaultPort(), g_bNameLookup, 0)) {
			laddedAddreses.push_back(make_pair(strAddNode, vcServNode));
		} else {
			Object obj;
			obj.push_back(Pair("addednode", strAddNode));
			obj.push_back(Pair("connected", false));
			Array addresses;
			obj.push_back(Pair("addresses", addresses));
		}
	}

	LOCK(g_cs_vNodes);
	for (list<pair<string, vector<CService> > >::iterator it = laddedAddreses.begin(); it != laddedAddreses.end();it++) {
		Object obj;
		obj.push_back(Pair("addednode", it->first));

		Array addresses;
		bool bConnected = false;
		for (auto& cAddrNode : it->second) {
			bool bFound = false;
			Object cNode;
			cNode.push_back(Pair("address", cAddrNode.ToString()));
			for (auto pNode : g_vNodes)
				if (pNode->m_cAddress == cAddrNode) {
					bFound = true;
					bConnected = true;
					cNode.push_back(Pair("connected", pNode->m_bInbound ? "inbound" : "outbound"));
					break;
				}
			if (!bFound) {
				cNode.push_back(Pair("connected", "false"));
			}
			addresses.push_back(cNode);
		}
		obj.push_back(Pair("connected", bConnected));
		obj.push_back(Pair("addresses", addresses));
		ret.push_back(obj);
	}

    return ret;
}
/**
 * ��ȡ������������͡����գ�ʱ����
 * @param params �������
 * @param bHelp ���������Ϣ��
 * @return
 */
Value getnettotals(const Array& params, bool bHelp) {
	if (bHelp || params.size() > 0) {
        throw runtime_error(
            "getnettotals\n"
            "\nReturns information about network traffic, including bytes in, bytes out,\n"
            "and current time.\n"
            "\nResult:\n"
            "{\n"
            "  \"totalbytesrecv\": n,   (numeric) Total bytes received\n"
            "  \"totalbytessent\": n,   (numeric) Total bytes sent\n"
            "  \"timemillis\": t        (numeric) Total cpu time\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getnettotals", "")
            + HelpExampleRpc("getnettotals", "")
       );
    }
	Object obj;
	obj.push_back(Pair("totalbytesrecv", CNode::GetTotalBytesRecv()));
	obj.push_back(Pair("totalbytessent", CNode::GetTotalBytesSent()));
	obj.push_back(Pair("timemillis", GetTimeMillis()));
	return obj;
}

/**
 * ���P2P�����״̬��Ϣ
 * @param params �������
 * @param bHelp ���������Ϣ��
 * @return
 */
Value getnetworkinfo(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 0) {
		throw runtime_error(
            "getnetworkinfo\n"
			"\nget various information about network.\n"
            "Returns an object containing various state info regarding P2P networking.\n"
            "\nResult:\n"
            "{\n"
            "  \"version\": xxxxx,           (numeric) the server version\n"
            "  \"protocolversion\": xxxxx,   (numeric) the protocol version\n"
            "  \"timeoffset\": xxxxx,        (numeric) the time offset\n"
            "  \"connections\": xxxxx,       (numeric) the number of connections\n"
            "  \"proxy\": \"host:port\",     (string, optional) the proxy used by the server\n"
            "  \"relayfee\": x.xxxx,         (numeric) minimum relay fee for non-free transactions in btc/kb\n"
            "  \"localaddresses\": [,        (array) list of local addresses\n"
            "    \"address\": \"xxxx\",      (string) network address\n"
            "    \"port\": xxx,              (numeric) network port\n"
            "    \"score\": xxx              (numeric) relative score\n"
            "  ]\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getnetworkinfo", "")
            + HelpExampleRpc("getnetworkinfo", "")
        );
    }
	proxyType proxy;
	GetProxy(NET_IPV4, proxy);

	Object obj;
	obj.push_back(Pair("version", (int) g_sClientVersion));
	obj.push_back(Pair("protocolversion", (int) g_sProtocolVersion));
	obj.push_back(Pair("timeoffset", GetTimeOffset()));
	obj.push_back(Pair("connections", (int) g_vNodes.size()));
	obj.push_back(Pair("proxy", (proxy.first.IsValid() ? proxy.first.ToStringIPPort() : string())));
	obj.push_back(Pair("relayfee", ValueFromAmount(CTransaction::m_sMinRelayTxFee)));
	Array localAddresses;
	{
		LOCK(g_cs_mapLocalHost);
		for (const auto &item : g_mapLocalHost) {
			Object rec;
			rec.push_back(Pair("address", item.first.ToString()));
			rec.push_back(Pair("port", item.second.nPort));
			rec.push_back(Pair("score", item.second.nScore));
			localAddresses.push_back(rec);
		}
	}
	obj.push_back(Pair("localaddresses", localAddresses));
	return obj;
}

/**
 * ��ȡ��� N����״̬��Ϣ
 * @param params �������
 * @param bHelp ���������Ϣ��
 * @return
 */
Value getcoinstate(const Array& params, bool bHelp) {
	if (bHelp || params.size() != 1) {
        throw runtime_error(
            "getcoinstate \"num\"\n"
			"\nget state data about the recently blocks.\n"
            "\nArguments:\n"
            "1.num   (numeric,required, > 0) The number of the recently blocks.\n"
            "\nResult:\n"
            "{\n"
            "  \"blocktime\": n,   (numeric)get the time of each block\n"
            "  \"difficulty\": n,   (numeric)get the difficulty of each block\n"
            "  \"transactions\": n, (numeric)get the transactions of each block\n"
        	"  \"fuel\": n, (numeric)get fuel of each block\n"
            "  \"blockminer\": n, (numeric)get the miner of each block\n"
            "}\n"
            "\nExamples:\n"
            + HelpExampleCli("getcoinstate", "\"5\"")
            + HelpExampleRpc("getcoinstate", "\"5\"")
       );
    }
	int i = 0, nHeight = 0;
	if (int_type == params[0].type()) {
		nHeight = params[0].get_int();
		if (nHeight < 1) {
			throw runtime_error("Block number out of range.");
		}
		if (nHeight > g_cChainActive.Height()) {   //��ֹ�������߶�
			nHeight = g_cChainActive.Height();
		}
	}
	CBlockIndex * pcBlockIndex = g_cChainActive.Tip();
	CBlock cBlock;
	Array blocktime;
	Array difficulty;
	Array transactions;
	Array fuel;
	Array blockminer;

	for (i = 0; (i < nHeight) && (pcBlockIndex != NULL); i++) {
		blocktime.push_back(pcBlockIndex->GetBlockTime());
		difficulty.push_back(GetDifficulty(pcBlockIndex));
		transactions.push_back((int) pcBlockIndex->m_unTx);
		fuel.push_back(pcBlockIndex->m_llFuel);
		cBlock.SetNull();
		if (ReadBlockFromDisk(cBlock, pcBlockIndex)) {
			string miner(boost::get<CRegID>(dynamic_pointer_cast<CRewardTransaction>(cBlock.vptx[0])->m_cAccount).ToString());
			blockminer.push_back(move(miner));
		}
		pcBlockIndex = pcBlockIndex->m_pPrevBlockIndex;
	}
	Object obj;
	obj.push_back(Pair("blocktime", blocktime));
	obj.push_back(Pair("difficulty", difficulty));
	obj.push_back(Pair("transactions", transactions));
	obj.push_back(Pair("fuel", fuel));
	obj.push_back(Pair("blockminer",blockminer));
    return obj;
}
