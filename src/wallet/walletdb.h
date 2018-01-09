// Copyright (c) 2009-2010 Satoshi Nakamoto
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef COIN_WALLET_WALLETDB_H_
#define COIN_WALLET_WALLETDB_H_

#include "key.h"

#include <list>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>
#include "stdio.h"
#include "db.h"

class CAccountInfo;
class CAccountingEntry;
struct ST_BlockLocator;
class CKeyPool;
class CMasterKey;
class CWallet;
class uint160;
class uint256;
class CRegID;
class CAccountTx;
class CKeyCombi;

/** Error statuses for the wallet database */
enum emDBErrors {
    EM_DB_LOAD_OK,
    EM_DB_CORRUPT,
    EM_DB_NONCRITICAL_ERROR,
    EM_DB_TOO_NEW,
    EM_DB_LOAD_FAIL,
    EM_DB_NEED_REWRITE
};

/*class CKeyMetadata
{
public:
    static const int CURRENT_VERSION=1;
    int nVersion;
    int64_t nCreateTime; // 0 means unknown

    CKeyMetadata()
    {
        SetNull();
    }
    CKeyMetadata(int64_t nCreateTime_)
    {
        nVersion = CKeyMetadata::CURRENT_VERSION;
        nCreateTime = nCreateTime_;
    }

    IMPLEMENT_SERIALIZE
    (
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(nCreateTime);
    )

    void SetNull()
    {
        nVersion = CKeyMetadata::CURRENT_VERSION;
        nCreateTime = 0;
    }
};*/


/** Access to the wallet database (wallet.dat) */
class CWalletDB: public CDB {
 public:
	CWalletDB(const std::string& strFilename, const char* pszMode = "r+", bool fFlushOnClose = true) :
			CDB(strFilename, pszMode, fFlushOnClose) {
	}

public:
	bool WriteCryptedKey(const CPubKey& pubkey, const std::vector<unsigned char>& vchCryptedSecret);
	bool WriteKeyStoreValue(const CKeyID &keyId, const CKeyCombi& KeyStoreValue, int nVersion);
	bool EraseKeyStoreValue(const CKeyID &keyId);
	bool WriteBlockTx(const uint256 &hash, const CAccountTx& atx);
	bool EraseBlockTx(const uint256& hash);
	bool WriteUnComFirmedTx(const uint256 &hash, const std::shared_ptr<CBaseTransaction> &tx);
	bool EraseUnComFirmedTx(const uint256& hash);
	bool WriteMasterKey(unsigned int nID, const CMasterKey& kMasterKey);
	bool EraseMasterKey(unsigned int nID);
	bool WriteVersion(const int version);
	bool WriteMinVersion(const int version);
	int  GetMinVersion(void);
	int  GetVersion(void);

	emDBErrors LoadWallet(CWallet* pwallet);
	static unsigned int g_unWalletDBUpdated;
	static bool Recover(CDBEnv& dbenv, string filename, bool fOnlyKeys);
	static bool Recover(CDBEnv& dbenv, string filename);

private:
	CWalletDB(const CWalletDB&);
	void operator=(const CWalletDB&);
};

bool BackupWallet(const CWallet& wallet, const string& strDest);

extern void ThreadFlushWalletDB(const string& kstrWalletFile);

extern void ThreadRelayTx(CWallet* pWallet);
#endif // COIN_WALLET_WALLETDB_H_
