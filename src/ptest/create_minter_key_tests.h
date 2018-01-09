/*
 * createminterkey_tests.h
 *
 *  Created on: 2015��4��13��
 *      Author: ranger.shi
 */

#ifndef COIN_PTEST_CREATE_MINTER_KEY_TESTS_H_
#define COIN_PTEST_CREATE_MINTER_KEY_TESTS_H_

class CCreateMinerkey: public SysTestBase {
 public:
	void CreateAccount();
	bool SelectAccounts();
	string GetOneAccount();
	CCreateMinerkey() {
	};
	virtual ~CCreateMinerkey();

 private:
	vector<string> m_vstrAccount;
	map<string, uint64_t> m_mapSendValue;
};

#endif /* COIN_PTEST_CREATE_MINTER_KEY_TESTS_H_ */
