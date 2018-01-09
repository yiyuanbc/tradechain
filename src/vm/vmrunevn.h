/*
 * ScriptCheck.h
 *
 *  Created on: Sep 2, 2014
 *      Author: ranger.shi
 */

#ifndef COIN_VM_VMRUNEVN_H_
#define COIN_VM_VMRUNEVN_H_

#include "vmlua.h"
#include "serialize.h"
#include "script.h"
#include "main.h"
#include "txdb.h"
#include <memory>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"
#include "json/json_spirit_writer_template.h"

using namespace std;
class CVmOperate;

class CVmRunEvn {
	/**
	 * Run the script object
	 */

	shared_ptr<CVmlua> m_pLua;  	//ִ��lua�ű�
	/**
	 * vm before the account state
	 */
	vector<shared_ptr<CAccount> > m_RawAccont;
	/**
	 * vm operate the account  state
	 */
	vector<shared_ptr<CAccount> > m_NewAccont;
	/**
	 * current run the tx
	 */
	shared_ptr<CBaseTransaction> m_plistTx;
	/**
	 * run the script
	 */
	CVmScript m_cVmScript;
	/**
	 * the block height
	 */
	unsigned int m_unRunTimeHeight;
	/**
	 * vm before the app account state
	 */
	vector<shared_ptr<CAppUserAccout>> m_RawAppUserAccout;
	/**
	 * vm operate the app account  state
	 */
	vector<shared_ptr<CAppUserAccout>> m_NewAppUserAccout;
	CScriptDBViewCache *m_ScriptDBTip;
	CAccountViewCache *m_view;
	vector<CVmOperate> m_output;   	//����������
    bool  m_bIsCheckAccount;  		//У���˻�ƽ�⿪��

	map<vector<unsigned char >,vector<CAppFundOperate> > m_MapAppOperate;  //vector<unsigned char > �����accountId
	shared_ptr<vector<CScriptDBOperLog> > m_dblog;

 private:
	/**
	 * @brief The initialization function
	 * @param Tx: run the tx's contact
	 * @param view: Cache holds account
	 *  @param nheight: run the Environment the block's height
	 * @return : check the the tx and account is Legal true is legal false is unlegal
	 */
	bool intial(shared_ptr<CBaseTransaction> & Tx, CAccountViewCache& cView, int nHeight);
	/**
	 *@brief check aciton
	 * @param listoperate: run the script return the code,check the code
	 * @return : true check success
	 */
	bool CheckOperate(const vector<CVmOperate> &vcVmOperate);
	/**
	 *
	 * @param listoperate: through the vm return code ,The accounts plus money and less money
	 * @param view:
	 * @return true operate account success
	 */
	bool OpeatorAccount(const vector<CVmOperate>& vcVmOperate, CAccountViewCache& cView, const int nCurHeight);
	/**
	 * @brief find the vOldAccount from NewAccont if find success remove it from NewAccont
	 * @param vOldAccount: the argument
	 * @return:Return the object
	 */
	std::shared_ptr<CAccount> GetNewAccount(shared_ptr<CAccount>& vOldAccount);
	/**
	 * @brief find the Account from NewAccont
	 * @param Account: argument
	 * @return:Return the object
	 */
	std::shared_ptr<CAccount> GetAccount(shared_ptr<CAccount>& Account);
	/**
	 * @brief get the account id
	 * @param value: argument
	 * @return:Return account id
	 */
	vector_unsigned_char GetAccountID(CVmOperate cValue);
	// bool IsSignatureAccount(CRegID account);
	bool OpeatorAppAccount(const map<vector<unsigned char >,vector<CAppFundOperate> > mapOpMap, CScriptDBViewCache& cView);

	std::shared_ptr<CAppUserAccout> GetAppAccount(shared_ptr<CAppUserAccout>& AppAccount);

 public:
	/**
	 * A constructor.
	 */
	CVmRunEvn();
	/**
	 *@brief get be operate the account
	 * @return the variable RawAccont
	 */
	vector<shared_ptr<CAccount> > &GetRawAccont();
	/**
	 *@brief get after operate the account
	 * @return :the variable NewAccont
	 */
	vector<shared_ptr<CAccount> > &GetNewAccont();

	vector<shared_ptr<CAppUserAccout>> &GetRawAppUserAccount();

	vector<shared_ptr<CAppUserAccout>> &GetNewAppUserAccount();
	/**
	 * @brief  start to run the script
	 * @param Tx: run the tx
	 * @param view: the second argument
	 * @param nheight: block height
	 * @param nBurnFactor: Executing a step script to spending
	 * @return: tuple<bool,uint64_t,string>  bool represent the script run success
	 * uint64_t if the script run sucess Run the script calls the money ,string represent run the failed's  Reason
	 */
	tuple<bool,uint64_t,string> run(shared_ptr<CBaseTransaction>& Tx, CAccountViewCache& cView, CScriptDBViewCache& cVmDB, int nHeight,
			uint64_t ullBurnFactor, uint64_t &ullRunStep);
	/**
	 * @brief just for test
	 * @return:
	 */
	// shared_ptr<vector<CVmOperate> > GetOperate() const;
	const CRegID& GetScriptRegID();
	const CRegID &GetTxAccount();
	uint64_t GetValue() const;
	const vector<unsigned char>& GetTxContact();
	CScriptDBViewCache* GetScriptDB();
	CAccountViewCache * GetCatchView();
	int GetComfirHeight();
	uint256 GetCurTxHash();
	bool InsertOutputData(const vector<CVmOperate>& vcSource);
	void InsertOutAPPOperte(const vector<unsigned char>& vuchUserId, const CAppFundOperate &vcSource);
	shared_ptr<vector<CScriptDBOperLog> > GetDbLog();

	bool GetAppUserAccout(const vector<unsigned char> &vuchAppUserId, shared_ptr<CAppUserAccout> &sptrAcc);
	bool CheckAppAcctOperate(CTransaction* pcTx);
	void SetCheckAccount(bool bCheckAccount);
	virtual ~CVmRunEvn();
};

enum emACCOUNT_TYPE {
	// account type
	EM_REGID 		= 0x01,				//!< Registration accountid
	EM_BASE_58_ADDR = 0x02,			    //!< pulickey
};
/**
 * @brief after run the script,the script output the code
 */

class CVmOperate {
 public:
	unsigned char m_uchNaccType;      	//EM_REGID or EM_BASE_58_ADDR
	unsigned char m_arruchAccountId[34];	//!< m_arruchAccountId
	unsigned char m_uchOpeatorType;		//!OperType
	unsigned int m_unOutHeight;		//!< the transacion Timeout height
	unsigned char m_arruchMoney[8];			//!<The transfer amount

	IMPLEMENT_SERIALIZE
	(
			READWRITE(m_uchNaccType);
			for(int i = 0;i < 34;i++) {
				READWRITE(m_arruchAccountId[i]);
			}
			READWRITE(m_uchOpeatorType);
			READWRITE(m_unOutHeight);
			for(int i = 0;i < 8;i++) {
				READWRITE(m_arruchMoney[i]);
			}
	)
	CVmOperate() {
		m_uchNaccType = EM_REGID;
		memset(m_arruchAccountId, 0, 34);
		m_uchOpeatorType = EM_ADD_FREE;
		m_unOutHeight = 0;
		memset(m_arruchMoney, 0, 8);
	}
	Object ToJson();
};

//extern CVmRunEvn *pVmRunEvn; //�ṩ��lmylib.cpp��ʹ��
#endif /* COIN_VM_VMRUNEVN_H_ */
