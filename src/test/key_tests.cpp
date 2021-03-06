// Copyright (c) 2012-2013 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "key.h"

#include "base58.h"
#include "uint256.h"
#include "util.h"
#include "hash.h"
#include <string>
#include <vector>

#include <boost/test/unit_test.hpp>

using namespace std;

//static const string strSecret1C    ("Kwr371tjA9u2rFSMZjTNun2PXXP3WPZu2afRHTcta6KxEUdm1vEw");
//static const string strSecret2C    ("L3Hq7a8FEQwJkW1M2GNKDW28546Vp5miewcCzSqUD9kCAXrJdS3g");
//static const CCoinAddress addr1C("1NoJrossxPBKfCHuJXT4HadJrXRE9Fxiqs");
//static const CCoinAddress addr2C("1CRj2HyM1CXWzHAXLQtiGLyggNT9WQqsDs");

//static const string strSecret1C    ("cPRVcTPYmbwiZaLt4mez6v2a3sHKL3np5pc781BYUTYQEUue1j4K");
//static const string strSecret2C    ("cRNvsxg5RGLnp8UDf86TK4uRZZ2qZjNDFHSiAos3rdvB6SX2VC4G");
//static const CCoinAddress addr1C("n4PwAoA9zQ3SrndjB4wp8QWb9xPpNdtLsW");
//static const CCoinAddress addr2C("mfu2Z1UfUBkWQACSXVDr25h5UVU7LmC3xB");

static const string g_strAddressBad("1HV9Lc3sNHZxwj4Zk6fB38tEmBryq2cBiF");

#ifdef KEY_TESTS_DUMPINFO
void dumpKeyInfo(uint256 privkey)
{
    CKey key;
    key.resize(32);
    memcpy(&secret[0], &privkey, 32);
    vector<unsigned char> sec;
    sec.resize(32);
    memcpy(&sec[0], &secret[0], 32);
    printf("  * secret (hex): %s\n", HexStr(sec).c_str());

    for (int nCompressed=0; nCompressed<2; nCompressed++)
    {
        bool fCompressed = nCompressed == 1;
        printf("  * %s:\n", fCompressed ? "compressed" : "uncompressed");
        CCoinSecret bsecret;
        bsecret.SetSecret(secret, fCompressed);
        printf("    * secret (base58): %s\n", bsecret.ToString().c_str());
        CKey key;
        key.SetSecret(secret, fCompressed);
        vector<unsigned char> vchPubKey = key.GetPubKey();
        printf("    * pubkey (hex): %s\n", HexStr(vchPubKey).c_str());
        printf("    * address (base58): %s\n", CCoinAddress(vchPubKey).ToString().c_str());
    }
}
#endif

BOOST_AUTO_TEST_SUITE(key_tests)

BOOST_AUTO_TEST_CASE(key_test1) {
	CCoinSecret cBsecret1, cBsecret2, cBsecret1C, cBsecret2C, cBaddress1;
	string strSecret1C, strSecret2C;

	bool bRegTest = SysCfg().GetBoolArg("-regtest", false);
	bool bTestNet = SysCfg().GetBoolArg("-testnet", false);
	if (bTestNet && bRegTest) {
		fprintf(stderr, "Error: Invalid combination of -regtest and -testnet.\n");
		assert(0);
	}

	if (bRegTest || bTestNet) {
		strSecret1C = string("cRj6ybhjS297ovjMgCCySgbsYYmPHutBguWL7K7MssWEN1twRDmv");
		strSecret2C = string("cVJ4gxvCWZbobVzvVkw5NGy6MDYT4uV8NYHzn3hPkkDk6M5jRKnL");
	} else {
		strSecret1C = string("Kwr371tjA9u2rFSMZjTNun2PXXP3WPZu2afRHTcta6KxEUdm1vEw");
		strSecret2C = string("L3Hq7a8FEQwJkW1M2GNKDW28546Vp5miewcCzSqUD9kCAXrJdS3g");
	}

	// BOOST_CHECK( bsecret1.SetString (strSecret1));
	// BOOST_CHECK( bsecret2.SetString (strSecret2));
	BOOST_CHECK(cBsecret1C.SetString(strSecret1C));
	BOOST_CHECK(cBsecret2C.SetString(strSecret2C));
	BOOST_CHECK(!cBaddress1.SetString(g_strAddressBad));

	// CKey key1  = bsecret1.GetKey();
	// BOOST_CHECK(key1.IsCompressed() == false);
	// CKey key2  = bsecret2.GetKey();
	// BOOST_CHECK(key2.IsCompressed() == false);
	CKey cKey1C = cBsecret1C.GetKey();
	BOOST_CHECK(cKey1C.IsCompressed() == true);
	CKey cKey2C = cBsecret2C.GetKey();
	BOOST_CHECK(cKey1C.IsCompressed() == true);

	// CPubKey pubkey1  = key1. GetPubKey();
	// CPubKey pubkey2  = key2. GetPubKey();
	CPubKey cPubkey1C = cKey1C.GetPubKey();
	CPubKey cPubkey2C = cKey2C.GetPubKey();

	// BOOST_CHECK(addr1.Get()  == CTxDestination(pubkey1.GetID()));
	// BOOST_CHECK(addr2.Get()  == CTxDestination(pubkey2.GetID()));
	// BOOST_CHECK(addr1C.Get() == CTxDestination(pubkey1C.GetID()));
	// BOOST_CHECK(addr2C.Get() == CTxDestination(pubkey2C.GetID()));
	if (bRegTest || bTestNet) {
		CCoinAddress cAddr1Ct("dgZjR2S98gmdvXDzwKASxKiaGr9Dw1GD8F");
		CCoinAddress cAddr2Ct("dw3Ard8fFMvt7nRppEr9cDAoRCCvNxj6vE");

		BOOST_CHECK(cAddr1Ct.Get() == CTxDestination(cPubkey1C.GetKeyID()));
		BOOST_CHECK(cAddr2Ct.Get() == CTxDestination(cPubkey2C.GetKeyID()));
	} else {
		CCoinAddress cAddr1Cm("1NoJrossxPBKfCHuJXT4HadJrXRE9Fxiqs");
		CCoinAddress cAddr2Cm("1CRj2HyM1CXWzHAXLQtiGLyggNT9WQqsDs");

		BOOST_CHECK(cAddr1Cm.Get() == CTxDestination(cPubkey1C.GetKeyID()));
		BOOST_CHECK(cAddr2Cm.Get() == CTxDestination(cPubkey2C.GetKeyID()));
	}

	for (int n = 0; n < 16; n++) {
		string strMsg = strprintf("Very secret message %i: 11", n);
		uint256 cHashMsg = Hash(strMsg.begin(), strMsg.end());

		// normal signatures

		// vector<unsigned char> sign1, sign2, sign1C, sign2C;
		vector<unsigned char> sign1C, sign2C;
		BOOST_CHECK(cKey1C.Sign(cHashMsg, sign1C));
		BOOST_CHECK(cKey2C.Sign(cHashMsg, sign2C));
		BOOST_CHECK( cPubkey1C.Verify(cHashMsg, sign1C));
		BOOST_CHECK(!cPubkey1C.Verify(cHashMsg, sign2C));

		// BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign1));
		// BOOST_CHECK( pubkey2C.Verify(hashMsg, sign2));
		BOOST_CHECK(!cPubkey2C.Verify(cHashMsg, sign1C));
		BOOST_CHECK( cPubkey2C.Verify(cHashMsg, sign2C));

		// compact signatures (with key recovery)

		// vector<unsigned char> csign1, csign2, csign1C, csign2C;
		vector<unsigned char> vuchSign1C, vuchSign2C;

		// BOOST_CHECK(key1.SignCompact (hashMsg, csign1));
		// BOOST_CHECK(key2.SignCompact (hashMsg, csign2));
		BOOST_CHECK(cKey1C.SignCompact(cHashMsg, vuchSign1C));
		BOOST_CHECK(cKey2C.SignCompact(cHashMsg, vuchSign2C));

		CPubKey crkey1, crkey2, crkey1C, crkey2C;

		// BOOST_CHECK(rkey1.RecoverCompact (hashMsg, csign1));
		// BOOST_CHECK(rkey2.RecoverCompact (hashMsg, csign2));
		BOOST_CHECK(crkey1C.RecoverCompact(cHashMsg, vuchSign1C));
		BOOST_CHECK(crkey2C.RecoverCompact(cHashMsg, vuchSign2C));

		// BOOST_CHECK(rkey1  == pubkey1);
		// BOOST_CHECK(rkey2  == pubkey2);
		BOOST_CHECK(crkey1C == cPubkey1C);
		BOOST_CHECK(crkey2C == cPubkey2C);
	}
}

BOOST_AUTO_TEST_SUITE_END()
