// Copyright (c) 2016 The Coin developers

// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COIN_NETBASE_H_
#define COIN_NETBASE_H_

#if defined(HAVE_CONFIG_H)
#include "coin-config.h"
#endif

#include "./compat/compat.h"
#include "serialize.h"

#include <stdint.h>
#include <string>
#include <vector>

#ifdef WIN32
// In MSVC, this is defined as a macro, undefine it to prevent a compile and link error
#undef SetPort
#endif

enum Network {
	NET_UNROUTABLE,
	NET_IPV4,
	NET_IPV6,
	NET_TOR,
	NET_MAX,
};

extern bool g_bNameLookup;

/** IP address (IPv6, or IPv4 using mapped IPv6 range (::FFFF:0:0/96)) */
class CNetAddr {
 public:
	CNetAddr();
	CNetAddr(const struct in_addr& ipv4Addr);
	explicit CNetAddr(const char *pszIp, bool bAllowLookup = false);
	explicit CNetAddr(const std::string &strIp, bool bAllowLookup = false);
	void Init();
	void SetIP(const CNetAddr& cIpIn);
	bool SetSpecial(const std::string &strName); // for Tor addresses
	bool IsIPv4() const;    // IPv4 mapped address (::FFFF:0:0/96, 0.0.0.0/0)
	bool IsIPv6() const;    // IPv6 address (not mapped IPv4, not Tor)
	bool IsRFC1918() const; // IPv4 private networks (10.0.0.0/8, 192.168.0.0/16, 172.16.0.0/12)
	bool IsRFC3849() const; // IPv6 documentation address (2001:0DB8::/32)
	bool IsRFC3927() const; // IPv4 autoconfig (169.254.0.0/16)
	bool IsRFC3964() const; // IPv6 6to4 tunnelling (2002::/16)
	bool IsRFC4193() const; // IPv6 unique local (FC00::/7)
	bool IsRFC4380() const; // IPv6 Teredo tunnelling (2001::/32)
	bool IsRFC4843() const; // IPv6 ORCHID (2001:10::/28)
	bool IsRFC4862() const; // IPv6 autoconfig (FE80::/64)
	bool IsRFC6052() const; // IPv6 well-known prefix (64:FF9B::/96)
	bool IsRFC6145() const; // IPv6 IPv4-translated address (::FFFF:0:0:0/96)
	bool IsTor() const;
	bool IsLocal() const;
	bool IsRoutable() const;
	bool IsValid() const;
	bool IsMulticast() const;
	enum Network GetNetwork() const;
	std::string ToString() const;
	std::string ToStringIP() const;
	unsigned int GetByte(int n) const;
	uint64_t GetHash() const;
	bool GetInAddr(struct in_addr* pipv4Addr) const;
	std::vector<unsigned char> GetGroup() const;
	int GetReachabilityFrom(const CNetAddr *paddrPartner = NULL) const;
	void print() const;

	CNetAddr(const struct in6_addr& pipv6Addr);
	bool GetIn6Addr(struct in6_addr* pipv6Addr) const;

	friend bool operator==(const CNetAddr& cNetAddrA, const CNetAddr& cNetAddrB);
	friend bool operator!=(const CNetAddr& cNetAddrA, const CNetAddr& cNetAddrB);
	friend bool operator<(const CNetAddr& cNetAddrA, const CNetAddr& cNetAddrB);

	IMPLEMENT_SERIALIZE
	(
			READWRITE(FLATDATA(m_chIp));
	)

 protected:
	unsigned char m_chIp[16]; // in network byte order
};

/** A combination of a network address (CNetAddr) and a (TCP) port */
class CService: public CNetAddr {
 public:
	CService();
	CService(const CNetAddr& cIp, unsigned short ushPortIn);
	CService(const struct in_addr& ipv4Addr, unsigned short ushPortIn);
	CService(const struct sockaddr_in& tAddr);
	explicit CService(const char *pszIpPort, int nPortDefault, bool bAllowLookup = false);
	explicit CService(const char *pszIpPort, bool bAllowLookup = false);
	explicit CService(const std::string& strIpPort, int nPortDefault, bool bAllowLookup = false);
	explicit CService(const std::string& strIpPort, bool bAllowLookup = false);
	void Init();
	void SetPort(unsigned short ushPortIn);
	unsigned short GetPort() const;
	bool GetSockAddr(struct sockaddr* pAddr, socklen_t *pAddrlen) const;
	bool SetSockAddr(const struct sockaddr* pAddr);
	friend bool operator==(const CService& cServiceA, const CService& cServiceB);
	friend bool operator!=(const CService& cServiceA, const CService& cServiceB);
	friend bool operator<(const CService& cServiceA, const CService& cServiceB);
	std::vector<unsigned char> GetKey() const;
	std::string ToString() const;
	std::string ToStringPort() const;
	std::string ToStringIPPort() const;
	void print() const;

	CService(const struct in6_addr& tIpv6Addr, unsigned short ushPortIn);
	CService(const struct sockaddr_in6& tAddr);

	IMPLEMENT_SERIALIZE
	(
			CService* pThis = const_cast<CService*>(this);
			READWRITE(FLATDATA(m_chIp));
			unsigned short ushPortN = htons(m_ushPort);
			READWRITE(ushPortN);
			if (fRead) {
				pThis->m_ushPort = ntohs(ushPortN);
			}
	)

 protected:
 	unsigned short m_ushPort; // host order
};

typedef std::pair<CService, int> proxyType;
extern int GetConnectTime();
enum Network ParseNetwork(std::string strNet);
void SplitHostPort(std::string strIn, int &nPortOut, std::string &strHostOut);
bool SetProxy(enum Network netWork, CService cAddrProxy, int nSocksVersion = 5);
bool GetProxy(enum Network netWork, proxyType &proxyInfoOut);
bool IsProxy(const CNetAddr &cNetAddr);
bool SetNameProxy(CService cAddrProxy, int nSocksVersion = 5);
bool HaveNameProxy();
bool LookupHost(const char *pszName, std::vector<CNetAddr>& vcIP, unsigned int nMaxSolutions = 0, bool bAllowLookup = true);
bool LookupHostNumeric(const char *pszName, std::vector<CNetAddr>& vcIP, unsigned int unMaxSolutions = 0);
bool Lookup(const char *pszName, CService& cAddr, int nPortDefault = 0, bool bAllowLookup = true);
bool Lookup(const char *pszName, std::vector<CService>& vcAddr, int nPortDefault = 0, bool bAllowLookup = true, unsigned int unMaxSolutions = 0);
bool LookupNumeric(const char *pszName, CService& cAddr, int nPortDefault = 0);
bool ConnectSocket(const CService &cAddrDest, SOCKET& hSocketRet, int nTimeout = GetConnectTime());
bool ConnectSocketByName(CService &cAddr, SOCKET& hSocketRet, const char *pszDest, int nPortDefault = 0, int nTimeout = GetConnectTime());
/** Return readable error string for a network error code */
std::string NetworkErrorString(int nErr);

#endif
