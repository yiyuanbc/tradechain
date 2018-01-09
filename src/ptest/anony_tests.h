/*
 * anony_tests.h
 *
 *  Created on: 2015-04-24
 *      Author: frank.shi
 */

#ifndef COIN_PTEST_ANONY_TESTS_H_
#define COIN_PTEST_ANONY_TESTS_H_

#include "cycle_test_base.h"
#include "../test/systestbase.h"
#include "../rpc/rpcclient.h"
#include "tx.h"

using namespace std;
using namespace boost;
using namespace json_spirit;

typedef struct  {
	unsigned char arruchSender[6];					//!<ת����ID������6�ֽڵ��˻�ID��
	int64_t llPayMoney;								//!<ת�˵���֧���Ľ��
	unsigned short usLen;             		        //!<����Ǯ�˻���Ϣ����
	IMPLEMENT_SERIALIZE
	(
			for (int i = 0; i < 6; i++) {
			    READWRITE(arruchSender[i]);
			}
			READWRITE(llPayMoney);
			READWRITE(usLen);
	)
}ST_CONTRACT;

typedef struct  {
	char  arrchAccount[6];						    	//!<����Ǯ��ID������6�ֽڵ��˻�ID��
	int64_t llReciMoney;						    	//!<�յ�Ǯ�Ľ��
	IMPLEMENT_SERIALIZE
	(
			for(int i = 0; i < 6; i++){
				READWRITE(arrchAccount[i]);
			}
			READWRITE(llReciMoney);
	)
}ST_ACCOUNT_INFO;

class CAnonyTest: public CycleTestBase {
 public:
	CAnonyTest();
	~CAnonyTest(){};
	virtual emTEST_STATE Run() ;
	bool RegistScript();
	bool CreateAnonyTx();
	void Initialize();

 private:
	int m_nNum;
	int m_nStep;
	string m_strTxHash;
	string m_strAppRegId;
	string m_strRegId;
};

#endif /* MLCOIN_PTEST_ANONY_TEST_H_ */
