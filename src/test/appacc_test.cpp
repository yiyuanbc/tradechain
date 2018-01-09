/*
 * appacc_test.cpp
 *
 *  Created on: 2015��3��29��
 *      Author: ranger.shi
 */


#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include "txdb.h"
#include "database.h"
#include <iostream>
#include  "boost/filesystem/operations.hpp"
#include  "boost/filesystem/path.hpp"
#include  "../vm/appaccount.h"
#include "../vm/vmrunevn.h"
#include "tx.h"
#include "util.h"

using namespace std;

int64_t g_arrllOpValue[8][8] = {
						{100*COIN, 10*COIN, 10*COIN, 30*COIN, 40*COIN, 30*COIN ,30*COIN, 20*COIN},    //false
						{1000*COIN, 200*COIN, 20*COIN, 30*COIN, 40*COIN, 20*COIN, 10*COIN, 20*COIN},  //false
						{500*COIN, 200*COIN, 20*COIN, 100*COIN, 200*COIN, 100*COIN, 300*COIN, 100*COIN}, //false
						{100*COIN, 10*COIN, 20*COIN, 50*COIN, 50*COIN, 10*COIN, 20*COIN, 30*COIN}, //false
						{200*COIN, 20*COIN, 30*COIN, 40*COIN, 30*COIN, 30*COIN, 40*COIN, 40*COIN},  //false
						{1000*COIN, 200*COIN, 20*COIN, 500*COIN, 800*COIN, 400*COIN, 200*COIN, 100*COIN}, //true
						{500*COIN, 200*COIN, 200*COIN, 300*COIN, 200*COIN, 50*COIN, 100*COIN, 50*COIN}, //true
						{600*COIN, 200*COIN, 20*COIN, 30*COIN, 50*COIN, 60*COIN, 70*COIN, 20*COIN}  //false
						};

// appacc_tests/key_test1


bool CheckAppAcct(int64_t arrllOpValue[]) {
	CRegID cSrcRegId(100, 1);
	CRegID cDesRegId(100, 2);
	CRegID cDesUser1RegId(100, 3);
	CRegID cDesUser2RegId(100, 4);
	CAccount cContractAcct;
	cContractAcct.m_ullValues = 100 * COIN;  //���ｫTX��100 COIN�ȳ�ֵ����Լ�˻��У�����ϵͳ�����˻���ɫ
	cContractAcct.m_cRegID = cDesRegId;

	CUserID cSrcUserId = cSrcRegId;
	CUserID cDesUserId = cDesRegId;
	vector_unsigned_char pContract;
	CTransaction cTx(cSrcUserId, cDesRegId, 10000, arrllOpValue[0], 1, pContract); //100 * COIN

	CVmRunEvn cVmRunEvn;
	vector<CVmOperate> vcAcctOper;

	vector_unsigned_char vDesUser1RegId = cDesUser1RegId.GetVec6();
	int64_t llTemp = arrllOpValue[1];  //10 * COIN
	CVmOperate cAcctAddOper;
	cAcctAddOper.m_uchNaccType = EM_REGID;
	cAcctAddOper.m_uchOpeatorType = EM_ADD_FREE;
	memcpy(cAcctAddOper.m_arruchAccountId, &vDesUser1RegId[0], 6);
	memcpy(cAcctAddOper.m_arruchMoney, &llTemp, sizeof(llTemp));
	vcAcctOper.push_back(cAcctAddOper);

	vector_unsigned_char vDesUser2RegId = cDesUser2RegId.GetVec6();
	llTemp = arrllOpValue[2];   //20 * COIN
	cAcctAddOper.m_uchNaccType = EM_REGID;
	cAcctAddOper.m_uchOpeatorType = EM_ADD_FREE;
	memcpy(cAcctAddOper.m_arruchAccountId, &vDesUser2RegId[0], 6);
	memcpy(cAcctAddOper.m_arruchMoney, &llTemp, sizeof(llTemp));
	vcAcctOper.push_back(cAcctAddOper);

	vector_unsigned_char vDesRegId = cDesRegId.GetVec6();
	llTemp = arrllOpValue[3];  //30 * COIN
	cAcctAddOper.m_uchNaccType = EM_REGID;
	cAcctAddOper.m_uchOpeatorType = EM_MINUS_FREE;
	memcpy(cAcctAddOper.m_arruchAccountId, &vDesRegId[0], 6);
	memcpy(cAcctAddOper.m_arruchMoney, &llTemp, sizeof(llTemp));
	vcAcctOper.push_back(cAcctAddOper);
	cVmRunEvn.InsertOutputData(vcAcctOper);

	CAppFundOperate cAppFundOper;
	cAppFundOper.m_uchOpeatorType = EM_ADD_FREE_OP;
	cAppFundOper.m_llMoney = arrllOpValue[4];       //20 * COIN
	cAppFundOper.m_uchAppuserIDlen = 6;
	memcpy(cAppFundOper.m_arruchAppuser, &vDesUser1RegId[0], 6);
	cAppFundOper.m_uchFundTaglen = 6;
	memcpy(cAppFundOper.m_arruchFundTag, &vDesUser1RegId[0], 6);
	cVmRunEvn.InsertOutAPPOperte(vDesUser1RegId, cAppFundOper);

	cAppFundOper.m_uchOpeatorType = EM_SUB_FREE_OP;
	cAppFundOper.m_llMoney = arrllOpValue[5];      //90 * COIN
	cAppFundOper.m_uchAppuserIDlen = 6;
	memcpy(cAppFundOper.m_arruchAppuser, &vDesUser2RegId[0], 6);
	cAppFundOper.m_uchFundTaglen = 6;
	memcpy(cAppFundOper.m_arruchFundTag, &vDesUser2RegId[0], 6);
	cVmRunEvn.InsertOutAPPOperte(vDesUser2RegId, cAppFundOper);

	cAppFundOper.m_uchOpeatorType = EM_ADD_TAG_OP;
	cAppFundOper.m_llMoney = arrllOpValue[6];     // 90 * COIN
	cAppFundOper.m_uchAppuserIDlen = 6;
	memcpy(cAppFundOper.m_arruchAppuser, &vDesUser2RegId[0], 6);
	cAppFundOper.m_uchFundTaglen = 6;
	memcpy(cAppFundOper.m_arruchFundTag, &vDesUser2RegId[0], 6);
	cVmRunEvn.InsertOutAPPOperte(vDesUser2RegId, cAppFundOper);

	cAppFundOper.m_uchOpeatorType = EM_SUB_TAG_OP;
	cAppFundOper.m_llMoney = arrllOpValue[7];  // 80 * COIN
	cAppFundOper.m_uchAppuserIDlen = 6;
	memcpy(cAppFundOper.m_arruchAppuser, &vDesUser1RegId[0], 6);
	cAppFundOper.m_uchFundTaglen = 6;
	memcpy(cAppFundOper.m_arruchFundTag, &vDesUser1RegId[0], 6);
	cVmRunEvn.InsertOutAPPOperte(vDesUser1RegId, cAppFundOper);

	return cVmRunEvn.CheckAppAcctOperate(&cTx);
}

BOOST_AUTO_TEST_SUITE(appacc_tests)

BOOST_AUTO_TEST_CASE(key_test1) {
	auto StrTVector = [&](string tag)
	{
		return vector<unsigned char>(tag.begin(),tag.end());
	};

	srand((int) time(NULL));

	vector<unsigned char> vuchAppuserId = StrTVector("test1");
	vector<unsigned char> vuchFundtag = StrTVector("foundtag");
	vector<unsigned char> vuchFundtag2 = StrTVector("foundtag2");

	CAppFundOperate cOpTe(vuchAppuserId, vuchFundtag, EM_ADD_TAG_OP, 500, 800000);
	BOOST_CHECK(cOpTe.GetFundTagV() == vuchFundtag);
	BOOST_CHECK(cOpTe.GetUint64Value() == 800000);
	BOOST_CHECK(cOpTe.getopeatortype() == EM_ADD_TAG_OP);

	vector<CAppFundOperate> vcOpArry;
	uint64_t ullAllmoney = 0;
	int nTimeout = (rand() % 15000) + 51;
	int nLoop = 500;
	int nMaxtimeout = nTimeout + nLoop + 1;
	for (int i = 0; i < nLoop; i++) {
		int64_t llTemp = ((rand() * rand()) % 15000000) + 20;
		ullAllmoney += llTemp;
		CAppFundOperate cOperate(vuchAppuserId, vuchFundtag, EM_ADD_TAG_OP, nTimeout + i, llTemp);
		vcOpArry.insert(vcOpArry.end(), cOperate);
	}

	CAppUserAccout cAccCount(vuchAppuserId);
	BOOST_CHECK(cAccCount.getaccUserId() == vuchAppuserId);      	//��ʼ����ID ������
	BOOST_CHECK(cAccCount.Operate(vcOpArry));               		//ִ�����еĲ�������
	BOOST_CHECK(cAccCount.getllValues() == 0);            			//��Ϊ������ȫ�ǼӶ����Ǯ�������ɽ�������0

	{
		CAppCFund cAppCFundTep;
		BOOST_CHECK(cAccCount.GetAppCFund(cAppCFundTep, vuchFundtag, nTimeout)); 					//��ȡ��Ӧ�Ķ�����
		BOOST_CHECK(cAppCFundTep.getValue() == vcOpArry[0].GetUint64Value());                    	//����Ľ����Ҫû������
		CAppCFund cAppCFundtep2;
		BOOST_CHECK(cAccCount.GetAppCFund(cAppCFundtep2, vuchFundtag, nMaxtimeout + 5) == false); 			//��ȡ��Ӧ�Ķ����� ��ʱʱ�䲻ͬ �����ȡ����

		cAccCount.AutoMergeFreezeToFree(0, nTimeout - 1);               	   						//�Զ��ϲ� ��ʱ�߶�û�е�  ����� 50 ��Ϊ�����ǩ�� time out de 51
		BOOST_CHECK(cAccCount.GetAppCFund(cAppCFundTep, vuchFundtag, nTimeout));			 		//û�кϲ��������û�б䶯
		BOOST_CHECK(cAppCFundTep.getValue() == vcOpArry[0].GetUint64Value());         				//û�кϲ��������û�б䶯
	}

	{
		vector<CAppFundOperate> vcOpArry2;
		CAppFundOperate cSubfreexeop(vuchAppuserId, vuchFundtag, EM_SUB_TAG_OP, nTimeout, 8);
		vcOpArry2.insert(vcOpArry2.end(), cSubfreexeop);
		BOOST_CHECK(cAccCount.Operate(vcOpArry2));               							//ִ�����еĲ�������
	}

	{
		CAppCFund cSubtemptep;
		BOOST_CHECK(cAccCount.GetAppCFund(cSubtemptep, vuchFundtag, nTimeout));        		//��ȡ��Ӧ�Ķ�����
		BOOST_CHECK(cSubtemptep.getValue() == (vcOpArry[0].GetUint64Value() - 8));    		//�����ȥ��8  ����Ƿ��
	}

	{
		vector<CAppFundOperate> vcOpArry2;
		CAppFundOperate revertfreexeop(vuchAppuserId, vuchFundtag, EM_ADD_TAG_OP, nTimeout, 8);
		vcOpArry2.clear();
		vcOpArry2.insert(vcOpArry2.end(), revertfreexeop);
		BOOST_CHECK(cAccCount.Operate(vcOpArry2));               							//ִ�����еĲ�������
	}

	{
		CAppCFund cReverttemptep;
		BOOST_CHECK(cAccCount.GetAppCFund(cReverttemptep, vuchFundtag, nTimeout));			//û�кϲ��������û�б䶯
		BOOST_CHECK(cReverttemptep.getValue() == vcOpArry[0].GetUint64Value());         	//û�кϲ��������û�б䶯
	}

	{         	//�ϲ���һ��
		CAppCFund cTep;
		cAccCount.AutoMergeFreezeToFree(0, nTimeout);                  						//�Զ��ϲ� ��0��
		BOOST_CHECK(cAccCount.GetAppCFund(cTep, vuchFundtag, nTimeout) == false); 			//�����Ҳ�������
		BOOST_CHECK(cAccCount.getllValues() == vcOpArry[0].GetUint64Value());;          	//�ϲ������ɽ�����û������
	}

	{           //��ȥȫ��
		CAppFundOperate cSubfreeop(vuchAppuserId, vuchFundtag, EM_SUB_FREE_OP, nTimeout, vcOpArry[0].GetUint64Value());
		vector<CAppFundOperate> vcOpArry2;
		vcOpArry2.insert(vcOpArry2.end(), cSubfreeop);
		BOOST_CHECK(cAccCount.Operate(vcOpArry2));               							//ִ�����еĲ�������
		BOOST_CHECK(cAccCount.getllValues() == 0);;                           				//Ǯ������Ժ˶�
	}

	{
		vector<CAppFundOperate> vcOpArry2;
		CAppFundOperate cAddfreeop(vuchAppuserId, vuchFundtag, EM_ADD_FREE_OP, nTimeout, vcOpArry[0].GetUint64Value()); //�ٴΰ����ݼӽ�ȥ
		vcOpArry2.clear();
		vcOpArry2.insert(vcOpArry2.end(), cAddfreeop);
		BOOST_CHECK(cAccCount.Operate(vcOpArry2));               							//ִ�����еĲ�������
		BOOST_CHECK(cAccCount.getllValues() == vcOpArry[0].GetUint64Value());               //���Ϻ� �ͻ�����
	}

	cAccCount.AutoMergeFreezeToFree(0, nMaxtimeout);     				//ȫ���ϲ�
	// BOOST_CHECK_MESSAGE(AccCount.getllValues() == allmoney, "" << allmoney << ' ' << AccCount.getllValues());        //���ƽ��
}

BOOST_AUTO_TEST_CASE(checkappacct_test) {
	for (int j = 0; j < 8; ++j) {
		for (int i = 0; i < 8; ++i) {
			cout << g_arrllOpValue[j][i] << " ";
		}
		cout << endl;
		int64_t llTxValue = g_arrllOpValue[j][0];
		int64_t llAcctMinusValue = g_arrllOpValue[j][3];
		int64_t llAcctSum = llTxValue - llAcctMinusValue;
		int64_t llAppAcctSum = g_arrllOpValue[j][4] - g_arrllOpValue[j][5] + g_arrllOpValue[j][6] - g_arrllOpValue[j][7];
		bool bIsCheck = (llAcctSum == llAppAcctSum);
		cout << "ischeck:" << bIsCheck << endl;
		BOOST_CHECK(CheckAppAcct(g_arrllOpValue[j]) == bIsCheck);
	}
}
BOOST_AUTO_TEST_SUITE_END()



