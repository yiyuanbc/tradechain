/*
 * guarantee_tests.cpp
 *
 *  Created on: 2015-04-24
 *      Author: frank
 */

#include "guarantee_tests.h"
#include "cycle_test_manger.h"
#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_reader.h"
using namespace std;
using namespace boost;
using namespace boost::assign;
using namespace json_spirit;

/*
 * ���Ե�ַ�����
 * */
#if 0
{
    "Address" : "dcmCbKbAfKrofNz35MSFupxrx4Uwn3vgjL",
    "KeyID" : "000f047f75144705b0c7f4eb30d205cd66f4599a",
    "RegID" : "1826-1437",
    "PublicKey" : "02bbb24c80a808cb6eb13de90c1dca99196bfce02bcf32812b7c4357a368877c68",
    "MinerPKey" : "02d546cc51b22621b093f08c679102b9b8ca3f1a07ea1d751de3f67c10670e635b",
    "Balance" : 129999825142,
    "CoinDays" : 0,
    "UpdateHeight" : 45433,
    "CurCoinDays" : 1457,
    "postion" : "inblock"
}
{
    "Address" : "dcmWdcfxEjRXUHk8LpygtgDHTpixoo3kbd",
    "KeyID" : "001e1384420cad01b0cd364ef064852b1bf3fd96",
    "RegID" : "1826-1285",
    "PublicKey" : "03d765b0f2bae7f6f61350b17bce5e57445cc286cada56d9c61987db5cbd533c43",
    "MinerPKey" : "025cae56b5faf1042f2d6610cde892f0cb1178282fb7b345b78611ccee4feab128",
    "Balance" : 128999717658,
    "CoinDays" : 0,
    "UpdateHeight" : 46859,
    "CurCoinDays" : 169,
    "postion" : "inblock"
}
{
    "Address" : "dcnGLkGud6c5bZJSUghzxvCqV45SJEwRcH",
    "KeyID" : "00429013e06bbcdc0529dd5b1117ddf4630544ad",
    "RegID" : "1826-1081",
    "PublicKey" : "02b509a4240ae08118ff2336981301cb2baf6207faf86aa1731a9ce8443e72f7f0",
    "MinerPKey" : "0394b395e1ef08f9c6e71eb2ecd70fe511f7ec0c0fe5a96c139fd4589b8f8a671c",
    "Balance" : 126999676405,
    "CoinDays" : 0,
    "UpdateHeight" : 45130,
    "CurCoinDays" : 1691,
    "postion" : "inblock"
}
#endif

#define ADDR_SEND_A        "dcmCbKbAfKrofNz35MSFupxrx4Uwn3vgjL"  //�ҵ���A  220700009d05 ���
// 1826-1437

#define ADDR_ACCEPT_B      "dcmWdcfxEjRXUHk8LpygtgDHTpixoo3kbd"  //�ӵ���B  220700000505 ����
// 1826-1285

#define ADDR_ARBITRATION_C "dcnGLkGud6c5bZJSUghzxvCqV45SJEwRcH"  //�ٲ���C   220700003904
// 1826-1081

//const static char DeveloperAccount[6]="\x00\x00\x00\x00\x14\x00";//!<�����ߵ�ַID
//#define ADDR_DeveloperAccount   "dk2NNjraSvquD9b4SQbysVRQeFikA55HLi"   //RegID = "0-20"
#define ADDR_DeveloperAccount   "DsSyKYzYBSgyEggq8o6SVD4DnPzETVbaUe"   //RegID = "86720-1"


//#define STR_APP_RETID  "47301-1"    //�ű�Ӧ��ID ����
//#define STR_APP_RETID  "47322-1"    //�ű�Ӧ��ID ����  47323
//#define STR_APP_RETID  "47323-1"    //�ű�Ӧ��ID ����  47323
#define STR_APP_RETID  "47018-1"    //
//#define HASH_sendhash     "7de1faafc2c9f14be5294f5f2b1082eaf92c7d66da5d42be1016e0988143318d"  //�ҵ�����hash ����
static const unsigned char HASH_sendhash[] ={
		0x14,0x96,0xb5,0xc0,0x3e,0xa9,0xa2,0x09,
		0xf3,0x97,0x05,0x3a,0x4d,0x32,0xdc,0x4a,
		0xe6,0x31,0x98,0x5e,0x14,0x8f,0x81,0x01,
		0xbb,0xf0,0x53,0xf7,0x4b,0x00,0x06,0x41
};

//!<�ٲ���C��������Ϣ
#define ARBITER_ARBITER_MONEYX      (2 * 1000000)      	//!<�ٲ÷���X
#define ARBITER_OVER_TIME_MONEY_YMAX  (1 * 100000000) 	//!<��ʱδ�о�������⳥����Y
#define ARBITER_CONFIG_MONEYZ       (1 * 1000000)       //!<������þ�����Z
#define ARBITER_OVER_TIME_HEIGHTT    (1 * 1440)  		//!<�о�����ʱ��T

//!<�ҵ��ߵ�������Ϣ
#define SEND_MONEYM     (2 * 100000000)    				//!<���׽��
#define SEND_HEIGHT     (2 * 1440)       				//!<ÿ�����׻��ڵĳ�ʱ�߶�

//#define  ARBITER_WINNER_MONEY  (1 * 100000000) //!<�þ���Ӯ�ҷ�����
#define  ARBITER_WINNER_MONEY  (198000000) //!<�þ���Ӯ�ҷ�����

CGuaranteeTest::CGuaranteeTest() :
		m_nNum(0), m_nStep(0), m_strTxHash(""), m_strAppRegId(STR_APP_RETID) {
}

emTEST_STATE CGuaranteeTest::Run(){
	cout<<"CGuaranteeTest run start"<<endl;
#if 0
    // ע��ipo�ű�
	if (!RegistScript()) {
		cout<<"CGuaranteeTest RegistScript err"<<endl;
		return EM_END_STATE;
	}

	/// �ȴ�ipo�ű���ȷ�ϵ�block��
	while (true) {
		if(WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif

//	Recharge();
	Withdraw();
//	WithdrawSomemoney();

//	Register(TX_REGISTER);
//	Register(TX_MODIFYREGISTER);
//	ArbitONOrOFF(TX_ARBIT_ON);
//	ArbitONOrOFF(TX_ARBIT_OFF);
//	UnRegister();
//	SendStartTrade();
//	SendCancelTrade();
//	AcceptTrade();
//    DeliveryTrade();
//	BuyerConfirm();
//	Arbitration();
//	RunFinalResult();
	cout<<"CGuaranteeTest run end"<<endl;
	return EM_END_STATE;
}

bool CGuaranteeTest::RegistScript(){
	const char* pKey[] = {
			"cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU",
			"cNcJkU44oG3etbWoEvY46i5qWPeE8jVb7K44keXxEQxsXUZ85MKU"};
	int nCount = sizeof(pKey) / sizeof(char*);
	m_cBasetest.ImportWalletKey(pKey, nCount);

	string strFileName("guarantee.bin");
	int nFee = m_cBasetest.GetRandomFee();
	int nCurHight;
	m_cBasetest.GetBlockHeight(nCurHight);
	string strRegAddr="dk2NNjraSvquD9b4SQbysVRQeFikA55HLi";

	//reg anony app
	Value regscript = m_cBasetest.RegisterAppTx(strRegAddr, strFileName, nCurHight, nFee + 1 *COIN);// + 20 *COIN
	if(m_cBasetest.GetHashFromCreatedTx(regscript, m_strTxHash)) {
		return true;
	}
	return false;
}

bool CGuaranteeTest::Recharge() {
	cout<<"Recharge start"<<endl;

	ST_APPACC tSenddata;
	memset(&tSenddata,0,sizeof(tSenddata));
	tSenddata.uchSysType = 0xff;
	tSenddata.uchType = 0x02;

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSenddata;
	string strSendContract = HexStr(cScriptData);
	uint64_t ullTempSend = COIN;//20 * COIN;
	//	uint64_t ullTempSend = (10000 + 5) * COIN;

    cout<<"Recharge data:"<<strSendContract<<endl;
    cout<<"Recharge strAppRegId:"<<m_strAppRegId<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SEND_A,strSendContract,0,0,ullTempSend);
	//   ADDR_DeveloperAccount  //RegID = "0-20"
	//   ADDR_SEND_A   //	Id = "1826-1437";
	//   ADDR_ACCEPT_B   //	Id = "1826-1285";
	//	ADDR_ARBITRATION_C  Id = "1826-1081"
	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;
	} else {
		cout << "Recharge err end" << endl;
		return false;
	}

#if 1
	// �ȴ����ױ�ȷ�ϵ�block��
	while (true) {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif
    cout<<"Recharge success end"<<endl;
	return false;
}

bool CGuaranteeTest::Withdraw() {
	cout<<"Withdraw start"<<endl;

	ST_APPACC tSendData;
	memset(&tSendData,0,sizeof(tSendData));
	tSendData.uchSysType = 0xff;
	tSendData.uchType = 0x01;
	tSendData.uchTypeAddr = EM_TX_REGID;

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSendData;
	string strSendContract = HexStr(cScriptData);
	uint64_t ullTempSend = 0;
    cout<<"Withdraw data:"<<strSendContract<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SEND_A,strSendContract,0,0,ullTempSend);
    // ADDR_DeveloperAccount  ADDR_ARBITRATION_C
	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;
	} else {
		cout << "Withdraw err end" << endl;
		return false;
	}

#if 1
	// �ȴ����ױ�ȷ�ϵ�block��
	while (true) {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif
	cout<<"Withdraw success end"<<endl;
	return false;
}

bool CGuaranteeTest::WithdrawSomemoney() {
	cout<<"WithdrawwSomemoney start"<<endl;

	ST_APPACC_MONEY tSendData;
	memset(&tSendData,0,sizeof(tSendData));
	tSendData.uchSysType = 0xff;
	tSendData.uchType = 0x03;
	tSendData.uchTypeAddr = EM_TX_REGID;
	tSendData.ullMoney = 10000 * COIN;
	// senddata.money = COIN;

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSendData;
	string strSendContract = HexStr(cScriptData);
	uint64_t ullTempSend = 0;
    cout<<"WithdrawwSomemoney data:"<<strSendContract<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_DeveloperAccount,strSendContract,0,0,ullTempSend);
    // ADDR_DeveloperAccount  ADDR_ARBITRATION_C
	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;
	} else {
		cout << "WithdrawwSomemoney err end" << endl;
		return false;
	}

#if 1
	/// �ȴ����ױ�ȷ�ϵ�block��
	while (true) {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif
	cout<<"WithdrawwSomemoney success end"<<endl;
	return false;
}

bool CGuaranteeTest::Register(unsigned char uchType) {
	ST_TX_REGISTER_CONTRACT tSendData;
	memset(&tSendData,0,sizeof(tSendData));
	if (uchType == TX_REGISTER) {
		tSendData.uchType = TX_REGISTER;
		cout << "TX_REGISTER start" << endl;
	} else if (uchType == TX_MODIFYREGISTER) {
		tSendData.uchType = TX_MODIFYREGISTER;
		cout << "TX_MODIFYREGISTER start" << endl;
	} else {
		cout << "Register input err" << endl;
		return false;
	}
	tSendData.ullArbiterMoneyX = ARBITER_ARBITER_MONEYX;
	tSendData.ullOvertimeMoneyYmax = ARBITER_OVER_TIME_MONEY_YMAX;
	tSendData.ullConfigMoneyZ = ARBITER_CONFIG_MONEYZ;//0x8234567812345678
	tSendData.unOvertimeheightT = ARBITER_OVER_TIME_HEIGHTT;
    strcpy(tSendData.arrchComment,"��ϵ�绰:13418656754");

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSendData;
	string strSendContract = HexStr(cScriptData);
	uint64_t ullTempSend = 0;
    cout<<"Register data:"<<strSendContract<<endl;
    cout<<"Register strAppRegId:"<<m_strAppRegId.c_str()<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_DeveloperAccount,strSendContract,0,0,ullTempSend);

	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;
	} else {
		cout << "Register fail" << endl;
		return false;
	}
#if 1
	/// �ȴ����ױ�ȷ�ϵ�block��
	while (true) {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif
	cout<<"Register success"<<endl;
	return true;
}

bool CGuaranteeTest::UnRegister() {
	cout<<"UnRegister start"<<endl;

	unsigned char uchSendData = TX_UNREGISTER;

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << uchSendData;
	string strSendContract = HexStr(cScriptData);
	uint64_t ullTempSend = 0;
	cout<<"UnRegister data:"<<strSendContract.c_str()<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_ARBITRATION_C,strSendContract,0,0,ullTempSend);

	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;
	} else {
		cout << "UnRegister fail" << endl;
		return false;
	}
#if 1
	/// �ȴ����ױ�ȷ�ϵ�block��
	while (true) {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif
	cout<<"UnRegister success end"<<endl;
	return false;
}

bool CGuaranteeTest::ArbitONOrOFF(unsigned char uchType) {
	cout<<"ArbitONOrOFF start"<<endl;

	unsigned char uchSendData;
	if (uchType == TX_ARBIT_ON) {
		uchSendData = TX_ARBIT_ON;
	} else {
		uchSendData = TX_ARBIT_OFF;
	}
	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << uchSendData;
	string strSendContract = HexStr(cScriptData);
	uint64_t ullTempSend = 0;
	cout<<"ArbitONOrOFF data:"<<strSendContract.c_str()<<endl;
	Value  retValue= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_ARBITRATION_C,strSendContract,0,0,ullTempSend);

	if (m_cBasetest.GetHashFromCreatedTx(retValue, m_strTxHash)) {
		m_nStep++;
	} else {
		cout << "ArbitONOrOFF fail" << endl;
		return false;
	}
#if 1
	/// �ȴ����ױ�ȷ�ϵ�block��
	while (true) {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif
	cout<<"ArbitONOrOFF success end"<<endl;
	return false;
}

bool CGuaranteeTest::SendStartTrade() {
	cout<<"SendStartTrade start"<<endl;

	ST_TX_SNED_CONTRACT tSendData;
	memset(&tSendData,0,sizeof(tSendData));
	tSendData.uchType = TX_SEND;
	// senddata.sendType = SEND_TYPE_BUY;   	//���޸� �ҵ�����
	tSendData.uchSendType = SEND_TYPE_SELL;   	//���޸� �ҵ�����

	// string arbitationID = "47046-1";
	unsigned int unHeight = 1826;   				//����   �ٲ���ID arbiterAddr_C
	unsigned short usIndex = 1081;

	memcpy(&tSendData.arrchArbitationID[0],&unHeight,4);
    memcpy(&tSendData.arrchArbitationID[4],&usIndex,2);
	tSendData.ullMoneyM = SEND_MONEYM;
	tSendData.unHeight = SEND_HEIGHT;
	strcpy(tSendData.arrchGoods,"С��3�ֻ�");
	strcpy(tSendData.arrchComment,"1������.��ϵ�绰:13418656754");

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSendData;
	string strSendContract = HexStr(cScriptData);
	uint64_t ullTempSend = 0;
	cout<<"SendStartTrade data:"<<strSendContract.c_str()<<endl;
	Value sendret;
	if (tSendData.uchSendType == SEND_TYPE_BUY) {
		sendret = m_cBasetest.CreateContractTx(m_strAppRegId, ADDR_SEND_A, strSendContract, 0, 0, ullTempSend); // ����д
	} else {
		sendret = m_cBasetest.CreateContractTx(m_strAppRegId, ADDR_ACCEPT_B, strSendContract, 0, 0, ullTempSend); // ����д
	}

	if (m_cBasetest.GetHashFromCreatedTx(sendret, m_strTxHash)) {
		m_nStep++;
	} else {
		cout << "SendStartTrade err end" << endl;
		return false;
	}
#if 1
		/// �ȴ����ױ�ȷ�ϵ�block��
	while (true) {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif
	cout<<"SendStartTrade success end"<<endl;
	return true;
}

bool CGuaranteeTest::SendCancelTrade() {
	cout<<"SendCancelTrade start"<<endl;

	ST_TX_CONTRACT tSendData;
	memset(&tSendData,0,sizeof(tSendData));
	tSendData.uchType = TX_CANCEL;
	// memcpy(senddata.txhash, uint256S(HASH_sendhash).begin(), sizeof(senddata.txhash)); //�����HASH
	memcpy(tSendData.arruchTxhash,HASH_sendhash, sizeof(tSendData.arruchTxhash)); //�����HASH
	tSendData.unHeight = SEND_HEIGHT;

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSendData;
	string strSendContract = HexStr(cScriptData);
	uint64_t ullTempSend = 0;
	cout<<"SendCancelTrade data:"<<strSendContract.c_str()<<endl;
	Value  sendret= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SEND_A,strSendContract,0,0,ullTempSend); // ȡ������
	// Value  sendret= m_cBasetest.CreateContractTx(strAppRegId,ADDR_ACCEPT_B,sendcontract,0,0,ullTempSend); //ȡ��������
	if (m_cBasetest.GetHashFromCreatedTx(sendret, m_strTxHash)) {
		m_nStep++;
	} else {
		cout << "SendCancelTrade err end" << endl;
		return false;
	}
#if 1
	// �ȴ����ױ�ȷ�ϵ�block��
	while (true) {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif
	cout<<"SendCancelTrade success end"<<endl;
	return true;
}

bool CGuaranteeTest::AcceptTrade() {
	cout<<"AcceptTrade start"<<endl;

	ST_TX_CONTRACT tSendData;
	memset(&tSendData,0,sizeof(tSendData));
	tSendData.uchType = TX_ACCEPT;
	memcpy(tSendData.arruchTxhash,HASH_sendhash, sizeof(tSendData.arruchTxhash)); //�����HASH
	tSendData.unHeight = SEND_HEIGHT;

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSendData;
	string strSendContract = HexStr(cScriptData);
	uint64_t ullTempSend = 0;
	cout<<"AcceptTrade data:"<<strSendContract.c_str()<<endl;
	// Value  sendret= m_cBasetest.CreateContractTx(strAppRegId,ADDR_ACCEPT_B,sendcontract,0,0,ullTempSend);//���ҽӵ�
	Value  sendret= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SEND_A,strSendContract,0,0,ullTempSend);//��ҽӵ�

	if (m_cBasetest.GetHashFromCreatedTx(sendret, m_strTxHash)) {
		m_nStep++;
	} else {
		cout << "AcceptTrade err end" << endl;
		return false;
	}
#if 1
	// �ȴ����ױ�ȷ�ϵ�block��
	while (true) {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif
	cout<<"AcceptTrade success end"<<endl;
	return true;
}

bool  CGuaranteeTest::DeliveryTrade() {
	cout<<"DeliveryTrade start"<<endl;

	ST_TX_CONTRACT tSendData;
	memset(&tSendData,0,sizeof(tSendData));
	tSendData.uchType = TX_DELIVERY;
	memcpy(tSendData.arruchTxhash,HASH_sendhash, sizeof(tSendData.arruchTxhash)); //�����HASH
	tSendData.unHeight = SEND_HEIGHT;

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSendData;
	string strSendContract = HexStr(cScriptData);
	uint64_t ullTempSend = 0;
	cout<<"DeliveryTrade data:"<<strSendContract.c_str()<<endl;
	Value  sendret= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_ACCEPT_B,strSendContract,0,0,ullTempSend);//���ҷ���

	if (m_cBasetest.GetHashFromCreatedTx(sendret, m_strTxHash)) {
		m_nStep++;
	} else {
		cout << "DeliveryTrade err end" << endl;
		return false;
	}
#if 1
	// �ȴ����ױ�ȷ�ϵ�block��
	while (true) {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif
	cout<<"DeliveryTrade success end"<<endl;
	return true;
}

bool CGuaranteeTest::BuyerConfirm() {
	cout<<"BuyerConfirm start"<<endl;
	ST_TX_CONTRACT tSendData;
	memset(&tSendData,0,sizeof(tSendData));
	tSendData.uchType = TX_BUYERCONFIRM;
	memcpy(tSendData.arruchTxhash,HASH_sendhash, sizeof(tSendData.arruchTxhash)); //�����HASH
	tSendData.unHeight = SEND_HEIGHT;

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSendData;
	string strSendContract = HexStr(cScriptData);
	uint64_t ullTempSend = 0;
	cout<<"BuyerConfirm data:"<<strSendContract.c_str()<<endl;
	Value  sendret= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SEND_A,strSendContract,0,0,ullTempSend); //����д

	if (m_cBasetest.GetHashFromCreatedTx(sendret, m_strTxHash)) {
		m_nStep++;
	} else {
		cout << "BuyerConfirm err end" << endl;
		return false;
	}
#if 1
	// �ȴ����ױ�ȷ�ϵ�block��
	while (true) {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif
	cout<<"BuyerConfirm success end"<<endl;
	return true;
}

bool CGuaranteeTest::Arbitration() {
	cout<<"Arbitration start"<<endl;

	ST_TX_Arbitration tSendData;
	memset(&tSendData,0,sizeof(tSendData));
	tSendData.uchType = TX_ARBITRATION;
	memcpy(tSendData.arruchTxhash,HASH_sendhash, sizeof(tSendData.arruchTxhash)); //�����HASH
	tSendData.unHeight = SEND_HEIGHT;
//	string arbitationID = "47046-1";
	unsigned int nHeight = 1826;   //����   �ٲ���ID arbiterAddr_C
	unsigned short usIndex = 1081;

	memcpy(&tSendData.arrchArbitationID[0],&nHeight,4);
	memcpy(&tSendData.arrchArbitationID[4],&usIndex,2);

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSendData;
	string strSendContract = HexStr(cScriptData);
	uint64_t ullTempSend = 0;
	cout<<"Arbitration data:"<<strSendContract.c_str()<<endl;
	Value  sendret= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_SEND_A,strSendContract,0,0,ullTempSend);// ����д ADDR_ARBITRATION_C

	if (m_cBasetest.GetHashFromCreatedTx(sendret, m_strTxHash)) {
		m_nStep++;

	} else {
		cout << "Arbitration err end" << endl;
		return false;
	}
#if 1
	// �ȴ����ױ�ȷ�ϵ�block��
	while (true) {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif
	cout<<"Arbitration success end"<<endl;
	return true;
}

bool CGuaranteeTest::RunFinalResult() {
	cout<<"RunFinalResult start"<<endl;

	ST_TX_FINALRESULT_CONTRACT tSendData;
	memset(&tSendData,0,sizeof(tSendData));
	tSendData.uchType = TX_FINALRESULT;
	memcpy(tSendData.arruchArbitHash, HASH_sendhash, sizeof(tSendData.arruchArbitHash)); //�����HASH
	tSendData.unOvertimeheightT = ARBITER_OVER_TIME_HEIGHTT;

	// string buyId = "1826-1437";
	unsigned int unHeight = 1826;   //����   Ӯ�� ID ��� Ӯ
	unsigned short usIndex = 1285;  //1437

    memcpy(&tSendData.arrchWinner[0],&unHeight,4);
    memcpy(&tSendData.arrchWinner[4],&usIndex,2);
	tSendData.ullWinnerMoney = ARBITER_WINNER_MONEY;

	// string sellerId = "1826-1285";
	unHeight 	= 1826;  //����   ���ID
	usIndex 	= 1437;  //1285
    memcpy(&tSendData.arrchLoser[0],&unHeight,4);
    memcpy(&tSendData.arrchLoser[4],&usIndex,2);
	tSendData.ullLoserMoney = SEND_MONEYM - ARBITER_WINNER_MONEY;  //  ���׽��M - Ӯ�ҷ����Ǯ  ����д

	CDataStream cScriptData(SER_DISK, g_sClientVersion);
	cScriptData << tSendData;
	string strSendContract = HexStr(cScriptData);
	uint64_t ullTempSend = 0;
	cout<<"RunFinalResult data:"<<strSendContract.c_str()<<endl;
	Value  sendret= m_cBasetest.CreateContractTx(m_strAppRegId,ADDR_ARBITRATION_C,strSendContract,0,0,ullTempSend);//ADDR_ARBITRATION_C

	if (m_cBasetest.GetHashFromCreatedTx(sendret, m_strTxHash)) {
		m_nStep++;
	} else {
		cout << "RunFinalResult err end" << endl;
		return false;
	}
#if 1
	// �ȴ����ױ�ȷ�ϵ�block��
	while (true) {
		if (WaitComfirmed(m_strTxHash, m_strAppRegId)) {
			break;
		}
	}
#endif
	cout<<"RunFinalResult success end"<<endl;
	return true;
}

BOOST_FIXTURE_TEST_SUITE(CGuaranteeTxTest,CGuaranteeTest)

BOOST_FIXTURE_TEST_CASE(Test,CGuaranteeTest)
{
	Run();
}
BOOST_AUTO_TEST_SUITE_END()

