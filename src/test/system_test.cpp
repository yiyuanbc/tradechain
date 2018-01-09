#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "init.h"
#include "miner.h"
#include "uint256.h"
#include "util.h"
#include <boost/test/unit_test.hpp>
#include "rpc/rpcclient.h"
#include "tx.h"
#include "wallet/wallet.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include "vm/script.h"
#include "rpc/rpcserver.h"
#include "noui.h"
#include "ui_interface.h"
#include "systestbase.h"
#include <boost/algorithm/string/predicate.hpp>

using namespace std;
using namespace boost;

#define ACCOUNT_ID_SIZE 6
#define MAX_ACCOUNT_LEN 20
#pragma pack(1)
typedef struct tag_INT64 {
	unsigned char data[8];
} ST_INT64;

typedef struct tagACCOUNT_ID {
	char accounid[MAX_ACCOUNT_LEN];
} ST_ACCOUNT_ID;

typedef struct {
	unsigned char uchType;
	ST_ACCOUNT_ID arrtVregID[3];
	long lHeight;
	ST_INT64 tPay;
} ST_CONTRACT_DATA;
#pragma pack()

class CSystemTest : public SysTestBase {
 public:
	enum {
		EM_ID1_FREE_TO_ID2_FREE = 1,
		EM_ID2_FREE_TO_ID3_FREE,
		EM_ID3_FREE_TO_ID3_SELF,
		EM_ID3_SELF_TO_ID2_FREE,
		EM_ID3_FREE_TO_ID2_FREE,
		EM_UNDEFINED_OPER
	};

	CSystemTest() {
		m_nOldBlockHeight 	= 0;
		m_nNewBlockHeight 	= 0;
		m_nTimeOutHeight 	= 100;
		m_ullOldMoney 		= 0;
		m_ullNewMoney 		= 0;
		m_strFileName 		= "unit_test.bin";
		m_strAddr1 			= "dsjkLDFfhenmx2JkFMdtJ22TYDvSGgmJem";
	}

	~CSystemTest() {
	}

 public:
	bool IsTxConfirmdInWallet(int nBlockHeight, const uint256& cTxHash) {
		string strHash = "";
		if (!SysTestBase::GetBlockHash(nBlockHeight, strHash)) {
			return false;
		}

		uint256 cBlockHash(uint256S(strHash));
		auto itAccountTx = g_pwalletMain->m_mapInBlockTx.find(cBlockHash);
		if (g_pwalletMain->m_mapInBlockTx.end() == itAccountTx) {
			return false;
		}
		for (const auto &item : itAccountTx->second.m_mapAccountTx) {
			if (cTxHash == item.first) {
				return true;
			}
		}
		return false;
	}

	bool GetTxIndexInBlock(const uint256& cTxHash, int& nIndex) {
		CBlockIndex* pcIndex = g_cChainActive.Tip();
		CBlock cBlock;
		if (!ReadBlockFromDisk(cBlock, pcIndex)){
			return false;
		}
		cBlock.BuildMerkleTree();
		std::tuple<bool, int> ret = cBlock.GetTxIndex(cTxHash);
		if (!std::get<0>(ret)) {
			return false;
		}

		nIndex = std::get<1>(ret);
		return true;
	}

	bool GetRegScript(map<string, string>& mapRegScript) {
		CRegID cRegId;
		vector<unsigned char> vuchScript;

		if (g_pScriptDBTip == nullptr) {
			return false;
		}
		assert(g_pScriptDBTip->Flush());

		int nCount(0);
		if (!g_pScriptDBTip->GetScriptCount(nCount)) {
			return false;
		}
		if (!g_pScriptDBTip->GetScript(0, cRegId, vuchScript)) {
			return false;
		}
		string strRegID = HexStr(cRegId.GetVec6());
		string strScript = HexStr(vuchScript.begin(), vuchScript.end());
		mapRegScript.insert(make_pair(strRegID, strScript));

		while (g_pScriptDBTip->GetScript(1, cRegId, vuchScript)) {
			strRegID = HexStr(cRegId.GetVec6());
			strScript = HexStr(vuchScript.begin(), vuchScript.end());
			mapRegScript.insert(make_pair(strRegID, strScript));
		}

		return true;
	}

	bool CheckRegScript(const string& strRegID, const string& strPath) {
		map<string, string> mapRegScript;
		if (!GetRegScript(mapRegScript)) {
			return false;
		}

		string strFileData;
		if (!GetFileData(strPath, strFileData)) {
			return false;
		}

		for (const auto& item : mapRegScript) {
			if (strRegID == item.first) {
				if (strFileData == item.second) {
					return true;
				}
			}
		}

		return false;
	}

	bool GetFileData(const string& strFilePath, string& strFileData) {
		FILE* pFile = fopen(strFilePath.c_str(), "rb+");
		if (!pFile) {
			return false;
		}

		unsigned long ulSize;
		fseek(pFile, 0, SEEK_END);
		ulSize = ftell(pFile);
		rewind(pFile);

		// allocate memory to contain the whole file:
		char *pBuffer = (char*) malloc(sizeof(char) * ulSize);
		if (pBuffer == NULL) {
			return false;
		}

		if (fread(pBuffer, 1, ulSize, pFile) != ulSize) {
			if (pBuffer) {
				free(pBuffer);
			}
			throw runtime_error("read script file error");
		}

		CVmScript cVmScript;
		cVmScript.m_vuchRom.insert(cVmScript.m_vuchRom.end(), pBuffer, pBuffer + ulSize);
		string strDesp("this is description");
		cVmScript.m_vuchScriptExplain.assign(strDesp.begin(), strDesp.end());
		CDataStream cDs(SER_DISK, g_sClientVersion);
		cDs << cVmScript;

		vector<unsigned char> vuchVscript;
		vuchVscript.assign(cDs.begin(), cDs.end());

		if (pFile) {
			fclose(pFile);
		}
		if (pBuffer) {
			free(pBuffer);
		}

		strFileData = HexStr(vuchVscript);
		return true;
	}

	bool IsScriptAccCreatedEx(const uint256& cTxHash, int nConfirmHeight) {
		int nIndex = 0;
		if (!GetTxIndexInBlock(uint256(uint256S(m_strTxHash)), nIndex)) {
			return false;
		}

		CRegID cRegID(nConfirmHeight, nIndex);
		return IsScriptAccCreated(HexStr(cRegID.GetVec6()));
	}

 protected:
	int m_nOldBlockHeight;
	int m_nNewBlockHeight;
	int m_nTimeOutHeight;
	static const int m_nFee = 1*COIN + 100000;
	uint64_t m_ullOldMoney;
	uint64_t m_ullNewMoney;
	string m_strTxHash;
	string m_strFileName;
	string m_strAddr1;
};

/*
 * ���Խű��˻�һ����ϵͳ�е�����
 */
BOOST_FIXTURE_TEST_SUITE(system_test,CSystemTest)
BOOST_FIXTURE_TEST_CASE(acct_process,CSystemTest)
{
#if 0
	ResetEnv();
	vector<map<int,string> >vDataInfo;
	vector<CAccountLog> vLog;
	for (int i = 0; i < m_nTimeOutHeight; i++) {
		//0:����ע��ű�����
		Value valueRes = RegisterAppTx(strAddr1,strFileName , m_nTimeOutHeight, nFee);
		BOOST_CHECK(GetHashFromCreatedTx(valueRes,strTxHash));

		//1:�ڿ�
		nOldMoney = GetBalance(strAddr1);
		BOOST_CHECK(GenerateOneBlock());
		SysTestBase::GetBlockHeight(m_nNewBlockHeight);

		//2:ȷ��Ǯ�Ѿ��۳�
		nNewMoney = GetBalance(strAddr1);
		BOOST_CHECK(nNewMoney == nOldMoney - nFee);

		//3:ȷ�Ͻű��˺��Ѿ�����
		int nIndex = 0;
		BOOST_CHECK(GetTxIndexInBlock(uint256(uint256S(strTxHash)), nIndex));
		CRegID regID(m_nNewBlockHeight, nIndex);
		BOOST_CHECK(IsScriptAccCreated(HexStr(regID.GetVec6())));

		//4:���Ǯ�������ȷ�Ͻ������Ƿ��д˱ʽ���
		BOOST_CHECK(IsTxConfirmdInWallet(m_nNewBlockHeight, uint256(uint256S(strTxHash))));

		//5:ͨ��listregscript ��ȡ�����Ϣ��һһ�˶ԣ����Ƿ�������һ��
		string strPath = SysCfg().GetDefaultTestDataPath() + strFileName;
		BOOST_CHECK(CheckRegScript(HexStr(regID.GetVec6()), strPath));

		//6:Gettxoperationlog ��ȡ����log���鿴�Ƿ���ȷ
		BOOST_CHECK(GetTxOperateLog(uint256(uint256S(strTxHash)), vLog));
//		BOOST_CHECK(1 == vLog.size() && 1 == vLog[0].vOperFund.size() && 1 == vLog[0].vOperFund[0].vFund.size());
		BOOST_CHECK(strAddr1 == vLog[0].keyID.ToAddress());
//		BOOST_CHECK(vLog[0].vOperFund[0].operType == MINUS_FREE && vLog[0].vOperFund[0].vFund[0].value == nFee);

		map<int,string> mapData;
		mapData.insert(make_pair(nIndex,strTxHash));
		vDataInfo.push_back(std::move(mapData));
		ShowProgress("acct_process progress: ",(int)(((i+1)/(float)100) * 100));

	}

	for(int i = vDataInfo.size()-1;i>=0;i--) {
		map<int,string> mapData = vDataInfo[i];
		BOOST_CHECK(1 == mapData.size());

//		int nTxIndex = mapData.begin()->first;
		string strTxHash = mapData.begin()->second;
		uint256 txHash(uint256S(strTxHash));

		SysTestBase::GetBlockHeight(m_nOldBlockHeight);
		nOldMoney = GetBalance(strAddr1);

		//8:�ع�
		BOOST_CHECK(DisConnectBlock(1));

		//9.1:����˻��������Ƿ����
		nNewMoney = GetBalance(strAddr1);
		SysTestBase::GetBlockHeight(m_nNewBlockHeight);
		BOOST_CHECK(m_nOldBlockHeight - 1 == m_nNewBlockHeight);
		BOOST_CHECK(nNewMoney-nFee == nOldMoney);

		//9.2:���ű��˻��Ƿ�ɾ��
		CRegID regID(m_nOldBlockHeight, mapData.begin()->first);
		BOOST_CHECK(!IsScriptAccCreated(HexStr(regID.GetVec6())));

		//9.3:�����Ƿ��Ѿ��Ѿ��ŵ�Ǯ����δȷ�Ͻ�����
		BOOST_CHECK(IsTxUnConfirmdInWallet(txHash));

		//9.4:��齻���Ƿ���mempool��
		BOOST_CHECK(IsTxInMemorypool(txHash));

		//9.5:���operationlog �Ƿ�������»�ȡ
		BOOST_CHECK(!GetTxOperateLog(txHash, vLog));
	}

	//��ջ���
	ResetEnv();
	SysTestBase::GetBlockHeight(m_nNewBlockHeight);
	BOOST_CHECK(0 == m_nNewBlockHeight);
#endif // 0
}

BOOST_AUTO_TEST_SUITE_END()

