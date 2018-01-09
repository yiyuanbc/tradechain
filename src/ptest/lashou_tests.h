/*
 * lashou_tests.h
 *
 *  Created on: 2016��12��29��
 *      Author: ranger.shi
 */

#ifndef COIN_PTEST_LASHOU_TESTS_H_
#define COIN_PTEST_LASHOU_TESTS_H_

#include "cycle_test_base.h"
#include "../test/systestbase.h"
#include "../rpc/rpcclient.h"
#include "tx.h"

using namespace std;
using namespace boost;
using namespace json_spirit;

#if 1

#define TX_CONFIG   	 0x01//--������Ϣ���������ò����ʹ洢һЩȫ�ֱ�����
#define TX_MODIFIED 	 0X02//--�޸�������Ϣ
#define TX_REGISTER 	 0X03//--ע����Ϣ
#define TX_RECHARGE 	 0x04//--��ֵ
#define TX_WITHDRAW 	 0x05//--��������
#define TX_CLAIM_APPLY 	 0X06//--����������Ϣ
#define TX_CLAIM_OPERATE 0X07//--�������
#define TX_IMPORT_DATA   0X08//--�����û�����
#define TX_MODIFIED_TIME 0X09//--�޸ļ���ʱ��

typedef struct {
	unsigned char uchType;  	//!<��������
	uint32_t  uWatchDay;		//!�۲�������
	uint32_t  uMinBalance;		//!��������ֵ
	char arrchSuperAcc[35];		//!�����û�
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uWatchDay);
			READWRITE(uMinBalance);
			for (int i = 0; i < 35; i++) {
				READWRITE(arrchSuperAcc[i]);
			}
	)
}CONFIG_ST;  //!<ע��������Ϣ

typedef struct {
	unsigned char uchType;            	//!<��������
	char  arrchModityAcc[35];			//!<���׽��

	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			for (int i = 0; i < 35; i++) {
				READWRITE(arrchModityAcc[i]);
			}
	)
}MODIFIED_ST;//!<�޸��ʻ���Ϣ


typedef struct {
	unsigned char uchType;    	//!<��������
	uint32_t  uRegMoney;		//!ע����
	char arrchUserID[35];		//!�û�ID
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uRegMoney);
			for (int i = 0; i < 35; i++) {
				READWRITE(arrchUserID[i]);
			}
	)
}REGISTER_ST;  //!<ע���û���Ϣ


typedef struct {
	unsigned char uchType;    	//!<��������
	uint32_t  uMoney;			//!��ֵ���
	char arrchUserID[35];		//!�û�ID
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uMoney);
			for (int i = 0; i < 35; i++) {
				READWRITE(arrchUserID[i]);
			}
	)
}RECHARGE_ST;  //!<��ֵ������


typedef struct {
	unsigned char uchType;    //!<��������
	char arrchUserID[34];//!�û�ID
	char ApplyHash[35];//!�����HASH
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			for(int i = 0; i < 34; i++)
			{
				READWRITE(arrchUserID[i]);
			}

			for(int i = 0; i < 35; i++)
			{
				READWRITE(ApplyHash[i]);
			}

	)
}APPLY_ST;  //!<��������

typedef struct {
	unsigned char uchType;    		//!<��������
	uint32_t  uMoney;				//!��ֵ���
	char arrchUserID[34];			//!�û�ID
	uint32_t  uNumber;				//!����
	char arrchApplyHash[34*3+1];	//!�����HASH
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uMoney);
			for (int i = 0; i < 34; i++) {
				READWRITE(arrchUserID[i]);
			}
			READWRITE(uNumber);
			for (int i = 0; i < (34*3+1); i++) {
				READWRITE(arrchApplyHash[i]);
			}

	)
}CLAIMS_ST;  //!<�������

#define IMPORT_DATA_NNNN	90

typedef struct {
	unsigned char uchType; 			//!<��������
	uint32_t  uNumber;				//!��ֵ���
	struct {						//!<�������ݽṹ
		char arrchUserID[34];		//!�û�ID
		uint32_t uImportMoney;		//!ע����
		uint32_t uImportHight;		//!ע��߶�
	} ImportDataSt[IMPORT_DATA_NNNN];
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uNumber);
			for (int i = 0; i < IMPORT_DATA_NNNN; i++) {
				for (int j = 0; j < 34; j++) {
					READWRITE(ImportDataSt[i].arrchUserID[j]);
				}
				READWRITE(ImportDataSt[i].uImportMoney);
				READWRITE(ImportDataSt[i].uImportHight);
			}
	)
}IMPORT_ST;  //!<�������

//======================================================================
//======================================================================
//======================================================================

enum GETDAWEL{
	TX_REGID = 0x01,
	TX_BASE58 = 0x02,
};

typedef struct {
	unsigned char uchType;           	//!<��������
	uint64_t ullMaxMoneyByTime;       	//!<ÿ���޶�
	uint64_t ullMaxMoneyByDay;        	//!<ÿ���޶�
	char  arrchAddress[35];             //!<��ע˵�� �ַ�����\0���������Ȳ����0
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(ullMaxMoneyByTime);
			READWRITE(ullMaxMoneyByDay);
			for (int i = 0; i < 35; i++) {
				READWRITE(arrchAddress[i]);
			}
	)
}COMPANY_CONFIG;  //!<ע����ҵ������Ϣ

typedef struct {
	unsigned char uchType;            //!<��������
	uint64_t ullMaxMoneyByTime;       //!<ÿ���޶�
	uint64_t ullMaxMoneyByDay;        //!<ÿ���޶�
	// char  address[35];             //!<��ע˵�� �ַ�����\0���������Ȳ����0
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(ullMaxMoneyByTime);
			READWRITE(ullMaxMoneyByDay);
	// for(int i = 0; i < 220; i++)
	// {
	// 		READWRITE(address[i]);
	// }
	)
}COMPANY_CONFIG_MODIFY;  //!<�޸���ҵ������Ϣ

typedef struct {
	unsigned char uchType;            		//!<��������
	// uint64_t moneyM;                   	//!<���׽��

	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
//			READWRITE(moneyM);
	)
}COMPANY_RECHARGE;                  		//!<��ҵ������

typedef struct {
	unsigned char uchType;            		//!<��������
	char  arrchAddress[35];             	//!<��ע˵�� �ַ�����\0���������Ȳ����0
	uint64_t ullMoneyM;               		//!<���׽��

	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			for (int i = 0; i < 35; i++) {
				READWRITE(arrchAddress[i]);
			}
			READWRITE(ullMoneyM);
	)
} COMPANY_WITHDRAW;

class CLashouTest : public CycleTestBase {
 public:
	CLashouTest();
	~CLashouTest(){};
	virtual emTEST_STATE Run() ;
	bool RegistScript();

	bool Config(void);
	bool Modify(void);
	bool Register(void);
	bool Recharge(void);
	bool Withdraw(void);
	bool ApplyForClaims(void);
	bool ClaimsOperate(void);
	bool ImportDate(void);
	bool ImportDateNN(void);
	bool CodeTest(void);

 private:
	int m_nNum;
	int m_nStep;
	string m_strTxHash;
	string m_strAppRegId;               		//ע��Ӧ�ú��Id
};

#endif

#endif /* COIN_PTEST_LASHOU_TESTS_H_ */
