/*
 * CSyncDataDb.h
 *
 *  Created on: Jun 14, 2014
 *      Author: ranger.shi
 */

#ifndef COIN_SYNCDATADB_H_
#define COIN_SYNCDATADB_H_

#include "sync.h"
#include <map>
#include "syncdata.h"
#include "net.h"
#include "leveldbwrapper.h"
#include <boost/shared_ptr.hpp>

namespace SyncData {
class CSyncCheckPoint {
 public:
	CSyncCheckPoint() :
			m_version(1), m_height(0), m_hashCheckpoint() {
	}
	void SetData(const CSyncData& data) {
		CDataStream sstream(data.GetMessageData(), SER_NETWORK, g_sProtocolVersion);
		sstream >> *this;
	}
	IMPLEMENT_SERIALIZE
	(
			READWRITE(this->m_version);
			nVersion = this->m_version;
			READWRITE(m_height);
			READWRITE(m_hashCheckpoint);
	)

 public:
	int m_version;
	int m_height;
	uint256 m_hashCheckpoint;      // checkpoint block
};

class CSyncDataDb {
 public:
	bool WriteCheckpoint(int nHeight, const CSyncData& cSyncData);
	bool ReadCheckpoint(int nHeight, CSyncData& cSyncData);
	bool ExistCheckpoint(int nHeight);
	bool LoadCheckPoint(std::map<int, uint256>& values);

 public:
	bool InitializeSyncDataDb(const boost::filesystem::path &path);
	void CloseSyncDataDb() {
		if (m_dbPoint) {
			m_dbPoint.reset();
		}
	}

 private:
	static boost::shared_ptr<CLevelDBWrapper> m_dbPoint;
};

} /* namespace SyncData */

#endif /* COIN_SYNCDATADB_H_ */
