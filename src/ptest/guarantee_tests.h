/*
 * guarantee_tests.h
 *
 *  Created on: 2015-04-24
 *      Author: frank.shi
 */

#ifndef COIN_PTEST_GUARANTEE_TESTS_H_
#define COIN_PTEST_GUARANTEE_TESTS_H_

#include "cycle_test_base.h"
#include "../test/systestbase.h"
#include "../rpc/rpcclient.h"
#include "tx.h"

using namespace std;
using namespace boost;
using namespace json_spirit;

#define	TX_REGISTER   		0x01   		//ע���ٲ��˻�
#define TX_MODIFYREGISTER  	0x02 		// �޸��ٲ���ע����Ϣ
#define TX_ARBIT_ON     	0x03 		//�ٲÿ���
#define TX_ARBIT_OFF    	0x04 		//�ٲ���ͣ
#define	TX_UNREGISTER 		0x05 		//ע���ٲ��˻�
#define	TX_SEND  			0x06 		//�ҵ�
#define	TX_CANCEL  			0x07 		//ȡ���ҵ�
#define	TX_ACCEPT  			0x08 		//�ӵ�
#define TX_DELIVERY 		0x09		//����
#define	TX_BUYERCONFIRM  	0x0a 		//���ȷ���ջ�
#define	TX_ARBITRATION  	0x0b 		//�����ٲ�
#define	TX_FINALRESULT  	0x0c 		//�þ����

#define	SEND_TYPE_BUY   0x00   			//!<�ҵ� ��
#define	SEND_TYPE_SELL  0x01  			//!<�ҵ� ��


typedef struct {
	unsigned char uchSysType;               //0xff
	unsigned char uchType;            // 0x01 �ᬨ?��?  02 ��?ֵ��  03 �ᬨ?��?һ?������?��e��?
	unsigned char uchTypeAddr;            // 0x01 regid 0x02 base58
	uint64_t     ullMoney;

	IMPLEMENT_SERIALIZE
	(
		READWRITE(uchSysType);
		READWRITE(uchType);
		READWRITE(uchTypeAddr);
		READWRITE(ullMoney);
	)
} ST_APPACC_MONEY;

typedef struct {
	unsigned char uchSysType;               //0xff
	unsigned char uchType;            // 0x01 �ᬨ?��?  02 ��?ֵ��  03 �ᬨ?��?һ?������?��e��?
	unsigned char uchTypeAddr;            // 0x01 regid 0x02 base58
	// uint64_t     money;

	IMPLEMENT_SERIALIZE
	(
		READWRITE(uchSysType);
		READWRITE(uchType);
		READWRITE(uchTypeAddr);
//		READWRITE(money);
	)
} ST_APPACC;

enum emGETDAWEL{
	EM_TX_REGID 	= 0x01,
	EM_TX_BASE58 	= 0x02,
};

typedef struct {
	unsigned char uchType;            		//!<��������
	uint64_t ullArbiterMoneyX;             	//!<�ٲ÷���X
	uint64_t ullOvertimeMoneyYmax;  		//!<��ʱδ�о�������⳥����Y
	uint64_t ullConfigMoneyZ;              	//!<������þ�����Z
	unsigned int  unOvertimeheightT;  		//!<�о�����ʱ��T
	char  arrchComment[220];             	//!<��ע˵�� �ַ�����\0���������Ȳ����0
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(ullArbiterMoneyX);
			READWRITE(ullOvertimeMoneyYmax);
			READWRITE(ullConfigMoneyZ);
			READWRITE(unOvertimeheightT);
			for (int i = 0; i < 220; i++) {
				READWRITE(arrchComment[i]);
			}
	)

}ST_TX_REGISTER_CONTRACT;  //!<ע���ٲ��˻�

typedef struct {
	unsigned char uchType;            		//!<��������
	unsigned char uchSendType;         		//!<�ҵ�����:0 ��  1��
	char arrchArbitationID[6];        		//!<�ٲ���ID������6�ֽڵ��˻�ID��
	uint64_t ullMoneyM;                   	//!<���׽��
	unsigned int unHeight;           		//!<ÿ�����׻��ڵĳ�ʱ�߶�

	char arrchGoods[20];               		//!<��Ʒ��Ϣ  �ַ�����\0���������Ȳ����0
	char arrchComment[200];             	//!<��ע˵�� �ַ�����\0���������Ȳ����0
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			READWRITE(uchSendType);
			for(int i = 0; i < 6; i++) {
				READWRITE(arrchArbitationID[i]);
			}
			READWRITE(ullMoneyM);
			READWRITE(unHeight);
			for(int i = 0; i < 20; i++) {
				READWRITE(arrchGoods[i]);
			}
			for(int i = 0; i < 200; i++) {
				READWRITE(arrchComment[i]);
			}
	)
}ST_TX_SNED_CONTRACT;                  		//!<�ҵ�

typedef struct {
	unsigned char uchType;            		//!<��������
	unsigned char arruchTxhash[32];       	//!<�ҵ��Ľ���hash
	unsigned int unHeight;          		//!<ÿ�����׻��ڵĳ�ʱ�߶�
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			for (int i = 0; i < 32; i++) {
				READWRITE(arruchTxhash[i]);
			}
			READWRITE(unHeight);
	)
} ST_TX_CONTRACT;

typedef struct {
	unsigned char uchType;            		//!<��������
	unsigned char arruchTxhash[32];       	//!<�ҵ��Ľ���hash
	unsigned int unHeight;          		//!<ÿ�����׻��ڵĳ�ʱ�߶�
	char  arrchArbitationID[6];       		//!<�ٲ���ID������6�ֽڵ��˻�ID��
	IMPLEMENT_SERIALIZE
	(
			READWRITE(uchType);
			for (int i = 0; i < 32; i++) {
				READWRITE(arruchTxhash[i]);
			}
			READWRITE(unHeight);
			for (int i = 0; i < 6; i++) {
				READWRITE(arrchArbitationID[i]);
			}
	)
} ST_TX_Arbitration;  //!<�����ٲ�

typedef struct {
	unsigned char uchType;            		//!<��������
	unsigned char arruchArbitHash[32];      //!<�����ٲõĽ���hash
	unsigned int unOvertimeheightT;			//!<�о�����ʱ��T
	char 	arrchWinner[6];      			//!<Ӯ��ID������6�ֽڵ��˻�ID��
	uint64_t ullWinnerMoney;            	//!<���ջ�õĽ��
	char  arrchLoser[6];       				//!<���ID������6�ֽڵ��˻�ID��
	uint64_t ullLoserMoney;            		//!<���ջ�õĽ��
	IMPLEMENT_SERIALIZE
	(
		READWRITE(uchType);
			for (int i = 0; i < 32; i++) {
				READWRITE(arruchArbitHash[i]);
			}
		READWRITE(unOvertimeheightT);
		for (int i = 0; i < 6; i++) {
			READWRITE(arrchWinner[i]);
		}
		READWRITE(ullWinnerMoney);
		for (int i = 0; i < 6; i++) {
			READWRITE(arrchLoser[i]);
		}
		READWRITE(ullLoserMoney);
	)
}ST_TX_FINALRESULT_CONTRACT;        //!<���ղþ�


class CGuaranteeTest : public CycleTestBase {
 public:
	CGuaranteeTest();
	~CGuaranteeTest(){};
	virtual emTEST_STATE Run() ;
	bool RegistScript();

	bool Recharge(void);
	bool Withdraw(void);
	bool WithdrawSomemoney(void);

	bool Register(unsigned char uchType);
	bool ArbitONOrOFF(unsigned char uchType);
	bool UnRegister(void);
	bool SendStartTrade(void);
	bool SendCancelTrade(void);
	bool AcceptTrade(void);
	bool DeliveryTrade(void);
	bool BuyerConfirm(void);
    bool Arbitration(void);
    bool RunFinalResult(void);

 private:
	int m_nNum;
	int m_nStep;
	string m_strTxHash;
	string m_strAppRegId;//ע��Ӧ�ú��Id
};

#endif /* COIN_PTEST_GUARANTEE_TESTS_H_ */
