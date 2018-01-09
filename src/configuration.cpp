/*
 * configuration.h
 *
 *  Created on: 2016年9月8日
 *      Author: ranger.shi
 */

#include "configuration.h"

#include <memory>
#include "bignum.h"
#include "uint256.h"
#include "arith_uint256.h"
#include "util.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <vector>

using namespace std;

#include "main.h"
#include "uint256.h"

#include <stdint.h>
#include "syncdatadb.h"

#include <boost/assign/list_of.hpp> // for 'map_list_of()'

#define MAX_SUBSIDY_HEIGHT (1440 * 365 * 10)

namespace Checkpoints {
typedef map<int, uint256> MapCheckPoints; // the first parameter is  nHeight;
CCriticalSection g_cs_checkPoint;

// How many times we expect transactions after the last checkpoint to
// be slower. This number is a compromise, as it can't be accurate for
// every system. When reindexing from a fast disk with a slow CPU, it
// can be up to 20, while when downloading from a slow network with a
// fast multicore CPU, it won't be much higher than 1.
static const double SIGCHECK_VERIFICATION_FACTOR = 5.0;

struct CCheckpointData {
	MapCheckPoints *mapCheckpoints;
	int64_t nTimeLastCheckpoint;
	int64_t nTransactionsLastCheckpoint;
	double fTransactionsPerDay;
};

bool g_bEnabled = true;

// What makes a good checkpoint block?
// + Is surrounded by blocks with reasonable timestamps
//   (no blocks before with a timestamp after, none after with
//    timestamp before)
// + Contains no strange transactions
static MapCheckPoints mapCheckpoints = boost::assign::map_list_of(0,
		uint256S("596279e3dde27763577c8333ff5ba36f3c776adc728e265d9ea05607e9fe9630"));
static const CCheckpointData data = { &mapCheckpoints, 0,      // * UNIX timestamp of last checkpoint block
		0,      // * total number of transactions between genesis and last checkpoint
				//   (the tx=... number in the SetBestChain debug.log lines)
		0       // * estimated number of transactions per day after checkpoint
		};

static MapCheckPoints mapCheckpointsTestnet = boost::assign::map_list_of(0,
		uint256S("495f770912ec588e102b8e5ca3e7ac9ad01b7cb4bc180541fd933f2143f9045b"));

static const CCheckpointData dataTestnet = { &mapCheckpointsTestnet, 0, 0, 0 };

static MapCheckPoints mapCheckpointsRegtest = boost::assign::map_list_of(0,
		uint256S("6df311a80e7199e4f00f6eae81429712fa77c4ed4830c5b22b7582e87f7ff461"));
static const CCheckpointData dataRegtest = { &mapCheckpointsRegtest, 0, 0, 0 };

const CCheckpointData &Checkpoints() {
	if (SysCfg().NetworkID() == EM_TESTNET) {
		return dataTestnet;
	} else if (SysCfg().NetworkID() == EM_MAIN) {
		return data;
	} else {
		return dataRegtest;
	}
}

bool CheckBlock(int nHeight, const uint256& hash) { //nHeight 找不到或 高度和hash都能找到，则返回true
	if (!g_bEnabled) {
		return true;
	}
	const MapCheckPoints& checkpoints = *Checkpoints().mapCheckpoints;

	MapCheckPoints::const_iterator i = checkpoints.find(nHeight);
	if (i == checkpoints.end()) {
		return true;
	}
	return hash == i->second;
}

// Guess how far we are in the verification process at the given block index
double GuessVerificationProgress(CBlockIndex *pindex, bool fSigchecks) {
	if (pindex == NULL) {
		return 0.0;
	}
	int64_t nNow = time(NULL);

	double fSigcheckVerificationFactor = fSigchecks ? SIGCHECK_VERIFICATION_FACTOR : 1.0;
	double fWorkBefore = 0.0; // Amount of work done before pindex
	double fWorkAfter = 0.0;  // Amount of work left after pindex (estimated)
	// Work is defined as: 1.0 per transaction before the last checkpoint, and
	// fSigcheckVerificationFactor per transaction after.

	const CCheckpointData &data = Checkpoints();

	if (pindex->m_unChainTx <= data.nTransactionsLastCheckpoint) {
		double nCheapBefore = pindex->m_unChainTx;
		double nCheapAfter = data.nTransactionsLastCheckpoint - pindex->m_unChainTx;
		double nExpensiveAfter = (nNow - data.nTimeLastCheckpoint) / 86400.0 * data.fTransactionsPerDay;
		fWorkBefore = nCheapBefore;
		fWorkAfter = nCheapAfter + nExpensiveAfter * fSigcheckVerificationFactor;
	} else {
		double nCheapBefore = data.nTransactionsLastCheckpoint;
		double nExpensiveBefore = pindex->m_unChainTx - data.nTransactionsLastCheckpoint;
		double nExpensiveAfter = (nNow - pindex->m_unTime) / 86400.0 * data.fTransactionsPerDay;
		fWorkBefore = nCheapBefore + nExpensiveBefore * fSigcheckVerificationFactor;
		fWorkAfter = nExpensiveAfter * fSigcheckVerificationFactor;
	}

	return fWorkBefore / (fWorkBefore + fWorkAfter);
}

int GetTotalBlocksEstimate() {    // 获取mapCheckpoints 中保存最后一个checkpoint 的高度
	if (!g_bEnabled) {
		return 0;
	}

	const MapCheckPoints& checkpoints = *Checkpoints().mapCheckpoints;

	return checkpoints.rbegin()->first;
}

CBlockIndex* GetLastCheckpoint(const map<uint256, CBlockIndex*>& mapBlockIndex) {
	if (!g_bEnabled) {
		return NULL;
	}

	const MapCheckPoints& checkpoints = *Checkpoints().mapCheckpoints;

	BOOST_REVERSE_FOREACH(const MapCheckPoints::value_type& i, checkpoints){
	const uint256& hash = i.second;
	map<uint256, CBlockIndex*>::const_iterator t = mapBlockIndex.find(hash);
	if (t != mapBlockIndex.end()) {
		return t->second;
	}
}
	return NULL;
}

bool LoadCheckpoint() {
	LOCK(g_cs_checkPoint);
	SyncData::CSyncDataDb db;
	return db.LoadCheckPoint(*Checkpoints().mapCheckpoints);
}

bool GetCheckpointByHeight(const int nHeight, std::vector<int> &vCheckpoints) {
	LOCK(g_cs_checkPoint);
	MapCheckPoints& checkpoints = *Checkpoints().mapCheckpoints;
	std::map<int, uint256>::iterator iterMap = checkpoints.upper_bound(nHeight);
	while (iterMap != checkpoints.end()) {
		vCheckpoints.push_back(iterMap->first);
		++iterMap;
	}
	return !vCheckpoints.empty();
}

bool AddCheckpoint(int nHeight, uint256 hash) {
	LOCK(g_cs_checkPoint);
	MapCheckPoints& checkpoints = *Checkpoints().mapCheckpoints;
	checkpoints.insert(checkpoints.end(), make_pair(nHeight, hash));
	return true;
}

void GetCheckpointMap(std::map<int, uint256> &mapCheckpoints) {
	LOCK(g_cs_checkPoint);
	const MapCheckPoints& checkpoints = *Checkpoints().mapCheckpoints;
	mapCheckpoints = checkpoints;
}

}

//=========================================================================
//========以下是静态成员初始化的值=====================================================

const G_CONFIG_TABLE &IniCfg() {
	static G_CONFIG_TABLE * psCfg = NULL;
	if (psCfg == NULL) {
		psCfg = new G_CONFIG_TABLE();
	}
	assert(psCfg != NULL);
	return *psCfg;

}
const uint256 G_CONFIG_TABLE::GetIntHash(emNetWork type) const {

	switch (type) {
	case EM_MAIN: {
		return (uint256S((hashGenesisBlock_mainNet)));
	}
	case EM_TESTNET: {
		return (uint256S((hashGenesisBlock_testNet)));
	}
	case EM_REGTEST: {
		return (uint256S((hashGenesisBlock_regTest)));
	}
	default:
		assert(0);
	}
	return uint256S("");

}
const string G_CONFIG_TABLE::GetCheckPointPkey(emNetWork type) const {
	switch (type) {
	case EM_MAIN: {
		return CheckPointPK_MainNet;
	}
	case EM_TESTNET: {
		return CheckPointPK_TestNet;
	}
//			case EM_REGTEST: {
//				return std::move(uint256S(std::move(hashGenesisBlock_regTest)));
//			}
	default:
		assert(0);
	}
	return "";
}

const vector<string> G_CONFIG_TABLE::GetIntPubKey(emNetWork type) const {

	switch (type) {
	case EM_MAIN: {
		return (intPubKey_mainNet);
	}
	case EM_TESTNET: {
		return (initPubKey_testNet);
	}
	case EM_REGTEST: {
		return (initPubkey_regTest);
	}
	default:
		assert(0);
	}
	return vector<string>();
}

const uint256 G_CONFIG_TABLE::GetHashMerkleRoot() const {
	return (uint256S((HashMerkleRoot)));
}

vector<unsigned int> G_CONFIG_TABLE::GetSeedNodeIP() const {
	return pnSeed;
}

unsigned char* G_CONFIG_TABLE::GetMagicNumber(emNetWork type) const {
	switch (type) {
	case EM_MAIN: {
		return Message_mainNet;
	}
	case EM_TESTNET: {
		return Message_testNet;
	}
	case EM_REGTEST: {
		return Message_regTest;
	}
	default:
		assert(0);
	}
	return NULL;
}

vector<unsigned char> G_CONFIG_TABLE::GetAddressPrefix(emNetWork type, emBase58Type BaseType) const {

	switch (type) {
	case EM_MAIN: {
		return AddrPrefix_mainNet[BaseType];
	}
	case EM_TESTNET: {
		return AddrPrefix_testNet[BaseType];
	}
//			case EM_REGTEST: {
//				return Message_regTest;
//			}
	default:
		assert(0);
	}
	return vector<unsigned char>();

}

unsigned int G_CONFIG_TABLE::GetnDefaultPort(emNetWork type) const {
	switch (type) {
	case EM_MAIN: {
		return nDefaultPort_mainNet;
	}
	case EM_TESTNET: {
		return nDefaultPort_testNet;
	}
	case EM_REGTEST: {
		return nDefaultPort_regTest;
	}
	default:
		assert(0);
	}
	return 0;
}
unsigned int G_CONFIG_TABLE::GetnRPCPort(emNetWork type) const {
	switch (type) {
	case EM_MAIN: {
		return nRPCPort_mainNet;
	}
	case EM_TESTNET: {
		return nRPCPort_testNet;
	}
//			case EM_REGTEST: {
//				return Message_regTest;
//			}
	default:
		assert(0);
	}
	return 0;
}
unsigned int G_CONFIG_TABLE::GetnUIPort(emNetWork type) const {
	switch (type) {
	case EM_MAIN: {
		return nUIPort_mainNet;
	}
	case EM_TESTNET: {
		return nUIPort_testNet;
	}
	case EM_REGTEST: {
		return nUIPort_testNet;
	}
	default:
		assert(0);
	}
	return 0;
}
unsigned int G_CONFIG_TABLE::GetStartTimeInit(emNetWork type) const {
	switch (type) {
	case EM_MAIN: {
		return StartTime_mainNet;
	}
	case EM_TESTNET: {
		return StartTime_testNet;
	}
	case EM_REGTEST: {
		return StartTime_regTest;
	}
	default:
		assert(0);
	}
	return 0;
}

unsigned int G_CONFIG_TABLE::GetHalvingInterval(emNetWork type) const {
	switch (type) {
	case EM_MAIN: {
		return nSubsidyHalvingInterval_mainNet;
	}
//				case EM_TESTNET: {
//					return nSubsidyHalvingInterval_testNet;
//				}
	case EM_REGTEST: {
		return nSubsidyHalvingInterval_regNet;
	}
	default:
		assert(0);
	}
	return 0;
}

uint64_t G_CONFIG_TABLE::GetCoinInitValue() const {
	return InitialCoin;
}

uint64_t G_CONFIG_TABLE::GetBlockSubsidyCfg(int nHeight) const {

	if(nHeight <= MAX_SUBSIDY_HEIGHT) {

		return DefaultFee * COIN / 1000000;
	}
	return 0;
}

//=========================================================================
//========以下是静态成员初始化的值=====================================================
//=========================================================================

//名称
string G_CONFIG_TABLE::COIN_NAME = "tradechain";

//公钥-主网络
vector<string> G_CONFIG_TABLE::intPubKey_mainNet = {
		"023670951da383b05db84a1811b7932500045f231162ce205746c01958e2deb4c5",
		"034308d429918740d5fbcbecacaac16ac1435d6ae4fa6993ff51bd320142cb664e" };
//公钥-测试网络
vector<string> G_CONFIG_TABLE::initPubKey_testNet = {
		"0235e69c4e1506df20ea69b5921ce479e4befca37ed2c37477a8786462c8845f70",
		"02b6e02ca13af1c52de58aac07b97415bb8d7401bbac4545624e42f04af96691dc" };
//公钥-局域网络
vector<string> G_CONFIG_TABLE::initPubkey_regTest = {
		"03bf4ab939fd2a2e191bb718b54d8d709d3664f45fd47385f454fb784116e7c391",
		"02deaecc045a3d33fafa392d5220dd99cb7b339815100523ab2541ccbf39543af9",
		"033e8bd87467997d18547d244e110095711fa8bcdf4beacfbb5bc3ec3a0558d284",
		"0307153f43bced3bf2c1158905f3d9d49e0e414f432c49241f3cb94f7d0e135240",
		"031da887ccfc80dc6df90ef1ccefb3e45ed2cbc1fa9f2d0a406624a148238f6927", };

//公钥
string G_CONFIG_TABLE::CheckPointPK_MainNet = "0336b7abefe8acf3df5a6c5c69a091aa1bdf64004cb70fbb7da092f059d587bffd";
string G_CONFIG_TABLE::CheckPointPK_TestNet = "02921f795960dc0cb92c4a21ff0f1d88232a513f7a7647dae6c4b0402f095bf58e";

//创世块HASH
string G_CONFIG_TABLE::hashGenesisBlock_mainNet = "0x750f7ecd6509e2bb21f8dfff9cbf4aca004005286fde975c004cf36efef2136e";
string G_CONFIG_TABLE::hashGenesisBlock_testNet = "0xa2e2fc10313101980f771256f4a68aadbd90008f66e2907b295fced39fd8b255";
string G_CONFIG_TABLE::hashGenesisBlock_regTest = "0xe9cc63da05fea029e5211b7c3d7a74656bc77d4c8eab074c739db2f45298a383";

//梅根HASH

string G_CONFIG_TABLE::HashMerkleRoot = "0xa2dbaa2dad57a24495d4179f7cfd90c64270c09ea3d4b072bb65b67a36aac0e8";

//IP地址
/*
 * 47.93.96.33 0x21605d2f
 * 47.93.43.154 0x9a2b5d2f
 * 118.31.64.31 0x1f401f76
 * 106.15.200.115 0x73c80f6a
 * 112.74.32.158 0x9e204a70
 * 119.23.109.240 0xf06d1777
 */
vector<unsigned int> G_CONFIG_TABLE::pnSeed = { 0x21605d2f, 0x9a2b5d2f, 0x1f401f76, 0x73c80f6a, 0x9e204a70, 0xf06d1777 };

//网络协议魔=
unsigned char G_CONFIG_TABLE::Message_mainNet[MESSAGE_START_SIZE] = { 0xfa, 0xc4, 0x1b, 0xcc };
unsigned char G_CONFIG_TABLE::Message_testNet[MESSAGE_START_SIZE] = { 0xfb, 0xdb, 0x54, 0xbc };
unsigned char G_CONFIG_TABLE::Message_regTest[MESSAGE_START_SIZE] = { 0xfc, 0x5c, 0x6b, 0xac };

//修改地址前缀
vector<unsigned char> G_CONFIG_TABLE::AddrPrefix_mainNet[EM_MAX_BASE58_TYPES] = { { 66 }, { 53 }, { 123 }, { 0x41, 0x1C,
		0xCB, 0x3F }, { 0x41, 0x1C, 0x3D, 0x4A }, { 0 } };
vector<unsigned char> G_CONFIG_TABLE::AddrPrefix_testNet[EM_MAX_BASE58_TYPES] = { { 128 }, { 93 }, { 237 }, { 0x7F,
		0x57, 0x3F, 0x4D }, { 0x7F, 0x57, 0x5A, 0x2C }, { 0 } };

//网络端口
unsigned int G_CONFIG_TABLE::nDefaultPort_mainNet = 8795;
unsigned int G_CONFIG_TABLE::nDefaultPort_testNet = 18799;
unsigned int G_CONFIG_TABLE::nDefaultPort_regTest = 18791;

unsigned int G_CONFIG_TABLE::nRPCPort_mainNet = 18935;
unsigned int G_CONFIG_TABLE::nRPCPort_testNet = 18980;

unsigned int G_CONFIG_TABLE::nUIPort_mainNet = 4944;
unsigned int G_CONFIG_TABLE::nUIPort_testNet = 4962;
//修改时间
unsigned int G_CONFIG_TABLE::StartTime_mainNet = 1515253885;
unsigned int G_CONFIG_TABLE::StartTime_testNet = 1515253335;
unsigned int G_CONFIG_TABLE::StartTime_regTest = 1515233885;

//半衰期
unsigned int G_CONFIG_TABLE::nSubsidyHalvingInterval_mainNet = 2590000;
unsigned int G_CONFIG_TABLE::nSubsidyHalvingInterval_regNet = 500;

//修改发币初始值
uint64_t G_CONFIG_TABLE::InitialCoin = 4800000000;

//矿工费用
uint64_t G_CONFIG_TABLE::DefaultFee = 228310502;
