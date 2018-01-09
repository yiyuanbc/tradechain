/*
** $Id: ldblib.c,v 1.149 2015/02/19 17:06:21 roberto Exp $
** Interface from Lua to its debug API
** See Copyright Notice in lua.h
*/

//#define ldblib_c
//#define LUA_LIB

//#include "lprefix.h"
//
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//
//#include "lua.h"
//
//#include "lauxlib.h"
//#include "lualib.h"

#include "lua/lua.hpp"
//#include "hash.h"
//#include "key.h"
//#include "main.h"
#include <openssl/des.h>
#include <vector>
#include "vmrunevn.h"
#include "SafeInt3.hpp"

#define LUA_C_BUFFER_SIZE  500  //����ֵ������ֽڷ�ֹջ���

#if 0
static void setfield(lua_State *L,char * key,double value){
	 //Ĭ��ջ����table
	lua_pushstring(L,key);
	lua_pushnumber(L,value);
	lua_settable(L,-3);	//����һ�Լ�ֵ���Ԫ��
}
static void stackDump(lua_State *L){
	int i;
	int top = lua_gettop(L);
//	int top = 20;//debug
	for(i = 0;i < top;i++){
		int t = lua_type(L,-1 - i);
		switch(t){
		case LUA_TSTRING:
			LogPrint("vm","%d str =%s\n", i, lua_tostring(L,-1 - i));
			break;
		case LUA_TBOOLEAN:
			LogPrint("vm","boolean =%d\n",lua_toboolean(L,-1 - i));
			break;
		case LUA_TNUMBER:
			LogPrint("vm","number =%d\n",lua_tonumber(L,-1 - i));
			break;
		default:
			LogPrint("vm","default =%s\n",lua_typename(L,-1 - i));
			break;
		}
	   LogPrint("vm"," ");
	}
	LogPrint("vm","\n");
}
#endif
/*
 *  //3.������˽��ջ��������Ľ��*/
static inline int RetRstToLua(lua_State *L, const vector<unsigned char> &ResultData) {
	int nLen = ResultData.size();

	nLen = nLen > LUA_C_BUFFER_SIZE ? LUA_C_BUFFER_SIZE : nLen;
	if (nLen > 0) {	//���ջ�ռ��Ƿ�
		if (lua_checkstack(L, nLen)) {
			for (int i = 0; i < nLen; i++) {
				lua_pushinteger(L, (lua_Integer) ResultData[i]);
			}
			return nLen;
		} else {
			LogPrint("vm", "%s\r\n", "RetRstToLua stack overflow");
		}
	} else {
		LogPrint("vm", "RetRstToLua err nLen = %d\r\n", nLen);
	}
	return 0;
}
/*
 *  //3.������˽��ջ��沼�����ͷ���ֵ*/
static inline int RetRstBooleanToLua(lua_State *L, bool bFlag) {
	//���ջ�ռ��Ƿ�
	if (lua_checkstack(L, sizeof(int))) {
		lua_pushboolean(L, (int) bFlag);
		return 1;
	} else {
		LogPrint("vm", "%s\r\n", "RetRstBooleanToLua stack overflow");
		return 0;
	}
}

static inline int RetFalse(const string reason) {
	LogPrint("vm", "%s\r\n", reason.c_str());
	return 0;
}

static CVmRunEvn* GetVmRunEvn(lua_State *L) {
	CVmRunEvn* pcVmRunEvn = NULL;
	int nRes = lua_getglobal(L, "VmScriptRun");
	//LogPrint("vm", "GetVmRunEvn lua_getglobal:%d\n", res);

	if (LUA_TLIGHTUSERDATA == nRes) {
		if (lua_islightuserdata(L, -1)) {
			pcVmRunEvn = (CVmRunEvn*) lua_topointer(L, -1);
			//LogPrint("vm", "GetVmRunEvn lua_topointer:%p\n", pcVmRunEvn);
		}
	}
	lua_pop(L, 1);

	return pcVmRunEvn;
}

static bool GetKeyId(const CAccountViewCache &view, vector<unsigned char> &ret, CKeyID &cKeyId) {
	if (ret.size() == 6) {
		CRegID cReg(ret);
		cKeyId = cReg.getKeyID(view);
	} else if (ret.size() == 34) {
		string strAddr(ret.begin(), ret.end());
		cKeyId = CKeyID(strAddr);
	} else {
		return false;
	}
	if (cKeyId.IsEmpty()) {
		return false;
	}
	return true;
}

static bool GetArray(lua_State *L, vector<std::shared_ptr<std::vector<unsigned char> > > &ret) {
	//��ջ��ȡ�䳤������
	int nTotalLen = lua_gettop(L);
	if ((nTotalLen <= 0) || (nTotalLen > LUA_C_BUFFER_SIZE)) {
		LogPrint("vm", "totallen error\r\n");
		return false;
	}

	//printf("nLen = %d", nTotalLen);
	vector<unsigned char> vuchBuf;
	vuchBuf.clear();
	for (int i = 0; i < nTotalLen; i++) {
		if (!lua_isnumber(L, i + 1)) {
			LogPrint("vm", "%s\r\n", "data is not number");
			return false;
		}
		vuchBuf.insert(vuchBuf.end(), lua_tonumber(L, i + 1));
	}
	ret.insert(ret.end(), std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));
//	LogPrint("vm", "GetData:%s, nLen:%d\n", HexStr(vuchBuf).c_str(), vuchBuf.size());
	return true;
}

static bool GetDataInt(lua_State *L, int &intValue) {
	//��ջ��ȡint �߶�
	if (!lua_isinteger(L, -1 - 0)) {
		LogPrint("vm", "%s\r\n", "data is not integer");
		return false;
	} else {
		int value = (int) lua_tointeger(L, -1 - 0);
//		LogPrint("vm", "GetDataInt:%d\n", value);
		intValue = value;
		return true;
	}
}

static bool GetDataString(lua_State *L, vector<std::shared_ptr < std::vector<unsigned char> > > &ret) {
	//��ջ��ȡһ���ַ���
	if (!lua_isstring(L, -1 - 0)) {
		LogPrint("vm", "%s\r\n", "data is not string");
		return false;
	}
	vector<unsigned char> vuchBuf;
	vuchBuf.clear();
	const char *pStr = NULL;
	pStr = lua_tostring(L, -1 - 0);
	if (pStr && (strlen(pStr) <= LUA_C_BUFFER_SIZE)) {
		for (size_t i = 0; i < strlen(pStr); i++) {
			vuchBuf.insert(vuchBuf.end(), pStr[i]);
		}
		ret.insert(ret.end(), std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));
//		LogPrint("vm", "GetDataString:%s\n", pStr);
		return true;
	} else {
		LogPrint("vm", "%s\r\n", "lua_tostring get fail");
		return false;
	}
}

static bool getNumberInTable(lua_State *L, char * pKey, double &ret) {
	// ��table�ȡָ��pKey��Ӧ��һ��numberֵ

	//Ĭ��ջ����table����pKey��ջ
	lua_pushstring(L, pKey);
	lua_gettable(L, -2);  //���Ҽ�ֵΪkey��Ԫ�أ�����ջ��
	if (!lua_isnumber(L, -1)) {
		LogPrint("vm", "num get error! %s\n", lua_tostring(L,-1));
		lua_pop(L, 1); //ɾ�������Ĳ��ҽ��
		return false;
	} else {
		ret = lua_tonumber(L, -1);
//    	LogPrint("vm", "getNumberInTable:%d\n", ret);
		lua_pop(L, 1); //ɾ�������Ĳ��ҽ��
		return true;
	}
}

static bool getStringInTable(lua_State *L, char * pKey, string &strValue) {
	// ��table�ȡָ��pKey��Ӧ��stringֵ

	const char *pStr = NULL;
	//Ĭ��ջ����table����pKey��ջ
	lua_pushstring(L, pKey);
	lua_gettable(L, -2);  //���Ҽ�ֵΪkey��Ԫ�أ�����ջ��
	if (!lua_isstring(L, -1)) {
		LogPrint("vm", "string get error! %s\n", lua_tostring(L,-1));
	} else {
		pStr = lua_tostring(L, -1);
		if (pStr && (strlen(pStr) <= LUA_C_BUFFER_SIZE)) {
			string res(pStr);
			strValue = res;
//    		LogPrint("vm", "getStringInTable:%s\n", pStr);
			lua_pop(L, 1); //ɾ�������Ĳ��ҽ��
			return true;
		} else {
			LogPrint("vm", "%s\r\n", "lua_tostring get fail");
		}
	}
	lua_pop(L, 1); //ɾ�������Ĳ��ҽ��
	return false;
}

static bool getArrayInTable(lua_State *L, char * pKey, unsigned short usLen, vector<unsigned char> &vOut) {
	// ��table�ȡָ��pKey��Ӧ������

	if ((usLen <= 0) || (usLen > LUA_C_BUFFER_SIZE)) {
		LogPrint("vm", "usLen error\r\n");
		return false;
	}
	unsigned char uchValue = 0;
	vOut.clear();
	//Ĭ��ջ����table����key��ջ
	lua_pushstring(L, pKey);
	lua_gettable(L, 1);
	if (!lua_istable(L, -1)) {
		LogPrint("vm", "getTableInTable is not table\n");
		return false;
	}
	for (int i = 0; i < usLen; ++i) {
		lua_pushnumber(L, i + 1); //��������ջ
		lua_gettable(L, -2);
		if (!lua_isnumber(L, -1)) {
			LogPrint("vm", "getTableInTable is not number\n");
			return false;
		}
		uchValue = 0;
		uchValue = lua_tonumber(L, -1);
		vOut.insert(vOut.end(), uchValue);
		lua_pop(L, 1);
	}
	lua_pop(L, 1); //ɾ�������Ĳ��ҽ��
	return true;
}

static bool getStringLogPrint(lua_State *L, char * pKey, unsigned short usLen, vector<unsigned char> &vOut) {
	//��ջ��ȡ table��ֵ��һ���ַ���
	//�ú���ר����д��־����GetDataTableLogPrint��
	if ((usLen <= 0) || (usLen > LUA_C_BUFFER_SIZE)) {
		LogPrint("vm", "usLen error\r\n");
		return false;
	}

	//Ĭ��ջ����table����key��ջ
	lua_pushstring(L, pKey);
	lua_gettable(L, 1);

	const char *pStr = NULL;
	vOut.clear();
	lua_getfield(L, -2, pKey);
	//stackDump(L);
	if (!lua_isstring(L, -1)/*LUA_TSTRING != lua_type(L, -1)*/) {
		LogPrint("vm", "getStringLogPrint is not string\n");
		return false;
	}
	pStr = lua_tostring(L, -1 - 0);
	if (pStr && (strlen(pStr) == usLen)) {
		for (size_t i = 0; i < usLen; i++) {
			vOut.insert(vOut.end(), pStr[i]);
		}
//		LogPrint("vm", "getfieldTableString:%s\n", pStr);
		lua_pop(L, 1); //ɾ�������Ĳ��ҽ��
		return true;
	} else {
		LogPrint("vm", "%s\r\n", "getStringLogPrint get fail");
		lua_pop(L, 1); //ɾ�������Ĳ��ҽ��
		return false;
	}
}

static bool GetDataTableLogPrint(lua_State *L, vector<std::shared_ptr<std::vector<unsigned char> > > &ret) {
	//ȡ��־��key value
	if (!lua_istable(L, -1)) {
		LogPrint("vm", "GetDataTableLogPrint is not table\n");
		return false;
	}
	unsigned short usLen = 0;
	vector<unsigned char> vuchBuf;
	//ȡkey
	int nKey = 0;
	double dValue = 0;
	if (!(getNumberInTable(L, (char *) "nKey", dValue))) {
		LogPrint("vm", "nKey get fail\n");
		return false;
	} else {
		nKey = (int) dValue;
	}
	vuchBuf.clear();
	vuchBuf.insert(vuchBuf.end(), nKey);
	ret.insert(ret.end(), std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));

	//ȡvalue�ĳ���
	if (!(getNumberInTable(L, (char *) "length", dValue))) {
		LogPrint("vm", "length get fail\n");
		return false;
	} else {
		usLen = (unsigned short) dValue;
	}

	if (usLen > 0) {
		usLen = usLen > LUA_C_BUFFER_SIZE ? LUA_C_BUFFER_SIZE : usLen;
		if (nKey) {   //hex
			if (!getArrayInTable(L, (char *) "value", usLen, vuchBuf)) {
				LogPrint("vm", "valueTable is not table\n");
				return false;
			} else {
				ret.insert(ret.end(), std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));
			}
		} else { //string
			if (!getStringLogPrint(L, (char *) "value", usLen, vuchBuf)) {
				LogPrint("vm", "valueString is not string\n");
				return false;
			} else {
				ret.insert(ret.end(), std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));
			}
		}
		return true;
	} else {
		LogPrint("vm", "length error\n");
		return false;
	}
}

static bool GetDataTableDes(lua_State *L, vector<std::shared_ptr<std::vector<unsigned char> > > &ret) {
	if (!lua_istable(L, -1)) {
		LogPrint("vm", "is not table\n");
		return false;
	}
	double dValue = 0;
	vector<unsigned char> vuchBuf;

	int nDataLen = 0;
	if (!(getNumberInTable(L, (char *) "nDataLen", dValue))) {
		LogPrint("vm", "nDataLen get fail\n");
		return false;
	} else {
		nDataLen = (unsigned int) dValue;
	}

	if (nDataLen <= 0) {
		LogPrint("vm", "nDataLen <= 0\n");
		return false;
	}

	if (!getArrayInTable(L, (char *) "data", nDataLen, vuchBuf)) {
		LogPrint("vm", "data not table\n");
		return false;
	} else {

		ret.push_back(std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));
	}

	int nKeyLen = 0;
	if (!(getNumberInTable(L, (char *) "nKeyLen", dValue))) {
		LogPrint("vm", "nKeyLen get fail\n");
		return false;
	} else {
		nKeyLen = (unsigned int) dValue;
	}

	if (nKeyLen <= 0) {
		LogPrint("vm", "nKeyLen <= 0\n");
		return false;
	}

	if (!getArrayInTable(L, (char *) "key", nKeyLen, vuchBuf)) {
		LogPrint("vm", "key not table\n");
		return false;
	} else {

		ret.push_back(std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));
	}

	int nFlag = 0;
	if (!(getNumberInTable(L, (char *) "bFlag", dValue))) {
		LogPrint("vm", "bFlag get fail\n");
		return false;
	} else {
		nFlag = (unsigned int) dValue;
		CDataStream cTep(SER_DISK, g_sClientVersion);
		cTep << (nFlag == 0 ? 0 : 1);
		ret.push_back(std::make_shared<vector<unsigned char>>(cTep.begin(), cTep.end()));
	}

	return true;
}

static bool GetDataTableVerifySignature(lua_State *L, vector<std::shared_ptr<std::vector<unsigned char> > > &ret) {
	if (!lua_istable(L, -1)) {
		LogPrint("vm", "is not table\n");
		return false;
	}
	double dValue = 0;
	vector<unsigned char> vuchBuf;

	int nDataLen = 0;
	if (!(getNumberInTable(L, (char *) "nDataLen", dValue))) {
		LogPrint("vm", "nDataLen get fail\n");
		return false;
	} else {
		nDataLen = (unsigned int) dValue;
	}

	if (nDataLen <= 0) {
		LogPrint("vm", "nDataLen <= 0\n");
		return false;
	}

	if (!getArrayInTable(L, (char *) "data", nDataLen, vuchBuf)) {
		LogPrint("vm", "data not table\n");
		return false;
	} else {
		ret.push_back(std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));
	}

	int nKeyLen = 0;
	if (!(getNumberInTable(L, (char *) "nKeyLen", dValue))) {
		LogPrint("vm", "nKeyLen get fail\n");
		return false;
	} else {
		nKeyLen = (unsigned int) dValue;
	}

	if (nKeyLen <= 0) {
		LogPrint("vm", "nKeyLen <= 0\n");
		return false;
	}

	if (!getArrayInTable(L, (char *) "key", nKeyLen, vuchBuf)) {
		LogPrint("vm", "key not table\n");
		return false;
	} else {

		ret.push_back(std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));
	}

	int nHashLen = 0;
	if (!(getNumberInTable(L, (char *) "nHashLen", dValue))) {
		LogPrint("vm", "nHashLen get fail\n");
		return false;
	} else {
		nHashLen = (unsigned int) dValue;
	}

	if (nHashLen <= 0) {
		LogPrint("vm", "nHashLen <= 0\n");
		return false;
	}

	if (!getArrayInTable(L, (char *) "hash", nHashLen, vuchBuf)) {
		LogPrint("vm", "hash not table\n");
		return false;
	} else {

		ret.push_back(std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));
	}

	return true;
}

static int ExInt64MulFunc(lua_State *L) {
	int argc = lua_gettop(L);    /* number of arguments */
	if(argc != 2) {
    	return RetFalse("argc error\n");
	}

	if(!lua_isinteger(L,1)) {
    	return RetFalse("Int64Mul para1 error\n");
	}
	int64_t a = lua_tointeger(L,1);

	if(!lua_isinteger(L,2)) {
    	return RetFalse("Int64Mul para2 error\n");
	}
	int64_t b = lua_tointeger(L,2);

	int64_t c = 0;
	if(!SafeMultiply(a, b, c)) {
		return RetFalse("Int64Mul Operate overflow !\n");
	}

	lua_pushinteger(L, c);
	return 1;
}

static int ExInt64AddFunc(lua_State *L) {
	int argc = lua_gettop(L);    /* number of arguments */
	if(argc != 2) {
    	return RetFalse("argc error\n");
	}

	if(!lua_isinteger(L,1)) {
    	return RetFalse("Int64Add para1 error\n");
	}
	int64_t a = lua_tointeger(L,1);

	if(!lua_isinteger(L,2)) {
    	return RetFalse("Int64Add para2 error\n");
	}
	int64_t b = lua_tointeger(L,2);

	int64_t c = 0;
	if(!SafeAdd(a, b, c)) {
		return RetFalse("Int64Add Operate overflow !\n");
	}

	lua_pushinteger(L, c);
	return 1;
}

static int ExInt64SubFunc(lua_State *L) {
	int argc = lua_gettop(L);    /* number of arguments */
	if(argc != 2) {
    	return RetFalse("argc error\n");
	}

	if(!lua_isinteger(L,1)) {
    	return RetFalse("Int64Sub para1 error\n");
	}
	int64_t a = lua_tointeger(L,1);

	if(!lua_isinteger(L,2)) {
    	return RetFalse("Int64Sub para2 error\n");
	}
	int64_t b = lua_tointeger(L,2);

	int64_t c = 0;
	if(!SafeSubtract(a, b, c)) {
		return RetFalse("Int64Sub Operate overflow !\n");
	}

	lua_pushinteger(L, c);
	return 1;
}

static int ExInt64DivFunc(lua_State *L) {
	int argc = lua_gettop(L);    /* number of arguments */
	if(argc != 2) {
    	return RetFalse("argc error\n");
	}

	if(!lua_isinteger(L,1)) {
    	return RetFalse("Int64Div para1 error\n");
	}
	int64_t a = lua_tointeger(L,1);

	if(!lua_isinteger(L,2)) {
    	return RetFalse("Int64Div para2 error\n");
	}
	int64_t b = lua_tointeger(L,2);

	int64_t c = 0;
	if(!SafeDivide(a, b, c)) {
		return RetFalse("Int64Div Operate overflow !\n");
	}

	lua_pushinteger(L, c);
	return 1;
}

/**
 *bool SHA256(void const* pfrist, const unsigned short nLen, void * const pout)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ����Ҫ������hashֵ���ַ���
 */
static int ExSha256Func(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;

	if (!GetDataString(L, vuchRetdata) || vuchRetdata.size() != 1 || vuchRetdata.at(0).get()->size() <= 0) {
		return RetFalse("ExSha256Func para err0");
	}

	uint256 cRslt = Hash(&vuchRetdata.at(0).get()->at(0),
			&vuchRetdata.at(0).get()->at(0) + vuchRetdata.at(0).get()->size());

	CDataStream cTep(SER_DISK, g_sClientVersion);
	cTep << cRslt;
	vector<unsigned char> vuchTep1(cTep.begin(), cTep.end());
	return RetRstToLua(L, vuchTep1);
}

/**
 *unsigned short Des(void const* pdata, unsigned short nLen, void const* pkey, unsigned short nKeyLen, bool IsEn, void * const pOut,unsigned short poutlen)
 * �������ʽ���м�㴫����������������:
 * 1.��һ����Ҫ���������ݻ��߽�������
 * 2.�ڶ���ʽ���ܻ��߽��ܵ�keyֵ
 * 3.�����Ǳ�ʶ�����Ǽ��ܻ��ǽ���
 *
 * {
 * 	nDataLen = 0,
 * 	data = {},
 * 	nKeyLen = 0,
 * 	key = {},
 * 	bFlag = 0
 * }
 */
static int ExDesFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;

	if (!GetDataTableDes(L, vuchRetdata) || vuchRetdata.size() != 3) {
		return RetFalse(string(__FUNCTION__) + "para  err !");
	}

	DES_key_schedule tDesKey1, tDesKey2, tDesKey3;

	vector<unsigned char> vuchDesData;
	vector<unsigned char> vuchDesOut;
	unsigned char datalen_rest = vuchRetdata.at(0).get()->size() % sizeof(DES_cblock);
	vuchDesData.assign(vuchRetdata.at(0).get()->begin(), vuchRetdata.at(0).get()->end());
	if (datalen_rest) {
		vuchDesData.insert(vuchDesData.end(), sizeof(DES_cblock) - datalen_rest, 0);
	}

	const_DES_cblock tIn;
	DES_cblock tOut, tKey;

	vuchDesOut.resize(vuchDesData.size());

	unsigned char uchFlag = vuchRetdata.at(2).get()->at(0);
	if (uchFlag == 1) {
		if (vuchRetdata.at(1).get()->size() == 8) {
			//			printf("the des encrypt\r\n");
			memcpy(tKey, &vuchRetdata.at(1).get()->at(0), sizeof(DES_cblock));
			DES_set_key_unchecked(&tKey, &tDesKey1);
			for (unsigned int ii = 0; ii < vuchDesData.size() / sizeof(DES_cblock); ii++) {
				memcpy(&tIn, &vuchDesData[ii * sizeof(DES_cblock)], sizeof(tIn));
				DES_ecb_encrypt(&tIn, &tOut, &tDesKey1, DES_ENCRYPT);
				memcpy(&vuchDesOut[ii * sizeof(DES_cblock)], &tOut, sizeof(tOut));
			}
		} else if (vuchRetdata.at(1).get()->size() == 16) {
			//			printf("the 3 des encrypt\r\n");
			memcpy(tKey, &vuchRetdata.at(1).get()->at(0), sizeof(DES_cblock));
			DES_set_key_unchecked(&tKey, &tDesKey1);
			DES_set_key_unchecked(&tKey, &tDesKey3);
			memcpy(tKey, &vuchRetdata.at(1).get()->at(0) + sizeof(DES_cblock), sizeof(DES_cblock));
			DES_set_key_unchecked(&tKey, &tDesKey2);
			for (unsigned int ii = 0; ii < vuchDesData.size() / sizeof(DES_cblock); ii++) {
				memcpy(&tIn, &vuchDesData[ii * sizeof(DES_cblock)], sizeof(tIn));
				DES_ecb3_encrypt(&tIn, &tOut, &tDesKey1, &tDesKey2, &tDesKey3, DES_ENCRYPT);
				memcpy(&vuchDesOut[ii * sizeof(DES_cblock)], &tOut, sizeof(tOut));
			}
		} else {
			//error
			return RetFalse(string(__FUNCTION__) + "para  err !");
		}
	} else {
		if (vuchRetdata.at(1).get()->size() == 8) {
			memcpy(tKey, &vuchRetdata.at(1).get()->at(0), sizeof(DES_cblock));
			DES_set_key_unchecked(&tKey, &tDesKey1);
			for (unsigned int ii = 0; ii < vuchDesData.size() / sizeof(DES_cblock); ii++) {
				memcpy(&tIn, &vuchDesData[ii * sizeof(DES_cblock)], sizeof(tIn));
				DES_ecb_encrypt(&tIn, &tOut, &tDesKey1, DES_DECRYPT);
				memcpy(&vuchDesOut[ii * sizeof(DES_cblock)], &tOut, sizeof(tOut));
			}
		} else if (vuchRetdata.at(1).get()->size() == 16) {
			memcpy(tKey, &vuchRetdata.at(1).get()->at(0), sizeof(DES_cblock));
			DES_set_key_unchecked(&tKey, &tDesKey1);
			DES_set_key_unchecked(&tKey, &tDesKey3);
			memcpy(tKey, &vuchRetdata.at(1).get()->at(0) + sizeof(DES_cblock), sizeof(DES_cblock));
			DES_set_key_unchecked(&tKey, &tDesKey2);
			for (unsigned int ii = 0; ii < vuchDesData.size() / sizeof(DES_cblock); ii++) {
				memcpy(&tIn, &vuchDesData[ii * sizeof(DES_cblock)], sizeof(tIn));
				DES_ecb3_encrypt(&tIn, &tOut, &tDesKey1, &tDesKey2, &tDesKey3, DES_DECRYPT);
				memcpy(&vuchDesOut[ii * sizeof(DES_cblock)], &tOut, sizeof(tOut));
			}
		} else {
			//error
			return RetFalse(string(__FUNCTION__) + "para  err !");
		}
	}
	return RetRstToLua(L,vuchDesOut);
}

/**
 *bool SignatureVerify(void const* data, unsigned short nDataLen, void const* key, unsigned short nKeyLen,
		void const* phash, unsigned short nHashLen)
 * �������ʽ���м�㴫����������������:
 * 1.��һ����ǩ��������
 * 2.�ڶ������õ�ǩ����publickey
 * 3.������ǩ��֮ǰ��hashֵ
 *
 *{
 * 	nDataLen = 0,
 * 	data = {},
 * 	nKeyLen = 0,
 * 	key = {},
 * 	nHashLen = 0,
 * 	hash = {}
 * }
 */
static int ExVerifySignatureFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;

	if (!GetDataTableVerifySignature(L, vuchRetdata) || vuchRetdata.size() != 3 || vuchRetdata.at(1).get()->size() != 33
			|| vuchRetdata.at(2).get()->size() != 32) {
		return RetFalse(string(__FUNCTION__) + "para  err !");
	}

	CPubKey pk(vuchRetdata.at(1).get()->begin(), vuchRetdata.at(1).get()->end());
	vector<unsigned char> vuchHash(vuchRetdata.at(2).get()->rbegin(), vuchRetdata.at(2).get()->rend());
	uint256 cHash(vuchHash);
	auto tem = std::make_shared<std::vector<vector<unsigned char> > >();

	bool rlt = CheckSignScript(cHash, *vuchRetdata.at(0), pk);
	if (!rlt) {
		LogPrint("INFO", "ExVerifySignatureFunc call CheckSignScript verify signature failed!\n");
	}
	return RetRstBooleanToLua(L, rlt);
}


static int ExGetTxContractsFunc(lua_State *L) {

	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;
	if (!GetArray(L, vuchRetdata) || vuchRetdata.size() != 1 || vuchRetdata.at(0).get()->size() != 32) {
		return RetFalse(string(__FUNCTION__) + "para  err !");
	}

	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	vector<unsigned char> vuchHash(vuchRetdata.at(0).get()->rbegin(), vuchRetdata.at(0).get()->rend());
	CDataStream cTep1(vuchHash, SER_DISK, g_sClientVersion);
	uint256 cHash1;
	cTep1 >> cHash1;

	std::shared_ptr<CBaseTransaction> pBaseTx;

	if (GetTransaction(pBaseTx, cHash1, *pcVmRunEvn->GetScriptDB(), false)) {
		CTransaction *pcTx = static_cast<CTransaction*>(pBaseTx.get());
		return RetRstToLua(L, pcTx->m_vchContract);
	}

	return 0;
}

/**
 *void LogPrint(const void *pdata, const unsigned short nDataLen,PRINT_FORMAT bFlag )
 * �������ʽ���м�㴫����������������:
 * 1.��һ���Ǵ�ӡ���ݵı�ʾ���ţ�true��һʮ�����ƴ�ӡ,�������ַ����ĸ�ʽ��ӡ
 * 2.�ڶ����Ǵ�ӡ���ַ���
 */
static int ExLogPrintFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;
	if (!GetDataTableLogPrint(L, vuchRetdata) || vuchRetdata.size() != 2) {
		return RetFalse("ExLogPrintFunc para err1");
	}
	CDataStream cTep1(*vuchRetdata.at(0), SER_DISK, g_sClientVersion);
	bool bFlag;
	cTep1 >> bFlag;
	string pdata((*vuchRetdata[1]).begin(), (*vuchRetdata[1]).end());

	if (bFlag) {
		LogPrint("vm", "%s\r\n", HexStr(pdata).c_str());
	} else {
		LogPrint("vm", "%s\r\n", pdata.c_str());
	}
	return 0;
}


/**
 *unsigned short GetAccounts(const unsigned char *txhash,void* const paccoutn,unsigned short maxlen)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� hash
 */
static int ExGetTxAccountsFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;
	if (!GetArray(L, vuchRetdata) || vuchRetdata.size() != 1 || vuchRetdata.at(0).get()->size() != 32) {
		return RetFalse("ExGetTxAccountsFunc para err1");
	}

	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	vector<unsigned char> vuchHash(vuchRetdata.at(0).get()->rbegin(), vuchRetdata.at(0).get()->rend());

	CDataStream cTep1(vuchHash, SER_DISK, g_sClientVersion);
	uint256 cHash1;
	cTep1 >> cHash1;
//	LogPrint("vm","ExGetTxAccountsFunc:%s",cHash1.GetHex().c_str());
	std::shared_ptr<CBaseTransaction> pBaseTx;

//	auto tem = make_shared<std::vector<vector<unsigned char> > >();
	int nLen = 0;
	if (GetTransaction(pBaseTx, cHash1, *pcVmRunEvn->GetScriptDB(), false)) {
		CTransaction *pTx = static_cast<CTransaction*>(pBaseTx.get());
		vector<unsigned char> item = boost::get<CRegID>(pTx->m_cSrcRegId).GetVec6();
		nLen = RetRstToLua(L, item);
	}
	return nLen;
}

static int ExByteToIntegerFunc(lua_State *L) {
	//���ֽ�����ϳ�integer
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;
	if (!GetArray(L, vuchRetdata) || vuchRetdata.size() != 1
			|| ((vuchRetdata.at(0).get()->size() != 4) && (vuchRetdata.at(0).get()->size() != 8))) {
		return RetFalse("ExGetTxAccountsFunc para err1");
	}

	//�����ݷ���
	vector<unsigned char> vuchValue(vuchRetdata.at(0).get()->begin(), vuchRetdata.at(0).get()->end());
	CDataStream cTep1(vuchValue, SER_DISK, g_sClientVersion);

	if (vuchRetdata.at(0).get()->size() == 4) {
		unsigned int nHeight;
		cTep1 >> nHeight;

//		LogPrint("vm","%d\r\n", nHeight);
		if (lua_checkstack(L, sizeof(lua_Integer))) {
			lua_pushinteger(L, (lua_Integer) nHeight);
			return 1;
		} else {
			return RetFalse("ExGetTxAccountsFunc stack overflow");
		}
	} else {
		int64_t llValue = 0;
		cTep1 >> llValue;
//		LogPrint("vm","%lld\r\n", llValue);
		if (lua_checkstack(L, sizeof(lua_Integer))) {
			lua_pushinteger(L, (lua_Integer) llValue);
			return 1;
		} else {
			return RetFalse("ExGetTxAccountsFunc stack overflow");
		}
	}
}

static int ExIntegerToByte4Func(lua_State *L) {
	//��integerת����4�ֽ�����
	int nHeight = 0;
	if (!GetDataInt(L, nHeight)) {
		return RetFalse("ExGetBlockHashFunc para err1");
	}
	CDataStream cTep(SER_DISK, g_sClientVersion);
	cTep << nHeight;
	vector<unsigned char> TMP(cTep.begin(), cTep.end());
	return RetRstToLua(L, TMP);
}

static int ExIntegerToByte8Func(lua_State *L) {
	//��integerת����8�ֽ�����
	int64_t llValue = 0;
	if (!lua_isinteger(L, -1 - 0)) {
		LogPrint("vm", "%s\r\n", "data is not integer");
		return 0;
	} else {
		llValue = (int64_t) lua_tointeger(L, -1 - 0);
	}

	CDataStream cTep(SER_DISK, g_sClientVersion);
	cTep << llValue;
	vector<unsigned char> TMP(cTep.begin(), cTep.end());
	return RetRstToLua(L, TMP);
}
/**
 *unsigned short GetAccountPublickey(const void* const accounid,void * const pubkey,const unsigned short maxlength)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� �˻�id,�����ֽ�
 */
static int ExGetAccountPublickeyFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;
	if (!GetArray(L, vuchRetdata) || vuchRetdata.size() != 1
			|| !(vuchRetdata.at(0).get()->size() == 6 || vuchRetdata.at(0).get()->size() == 34)) {
		return RetFalse("ExGetAccountPublickeyFunc para err1");
	}

	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	CKeyID cAddrKeyId;
	if (!GetKeyId(*(pcVmRunEvn->GetCatchView()), *vuchRetdata.at(0).get(), cAddrKeyId)) {
		return RetFalse("ExGetAccountPublickeyFunc para err2");
	}
	CUserID userid(cAddrKeyId);
	CAccount cAccount;
	if (!pcVmRunEvn->GetCatchView()->GetAccount(userid, cAccount)) {
		return RetFalse("ExGetAccountPublickeyFunc para err3");
	}
	CDataStream cTep(SER_DISK, g_sClientVersion);
	vector<char> vchTe;
	cTep << cAccount.m_cPublicKey;
//    assert(cAccount.PublicKey.IsFullyValid());
	if (false == cAccount.m_cPublicKey.IsFullyValid()) {
		return RetFalse("ExGetAccountPublickeyFunc PublicKey invalid");
	}
	cTep >> vchTe;
	vector<unsigned char> vuchTep1(vchTe.begin(), vchTe.end());
	return RetRstToLua(L, vuchTep1);
}

/**
 *bool QueryAccountBalance(const unsigned char* const account,ST_INT64* const pBalance)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� �˻�id,�����ֽ�
 */
static int ExQueryAccountBalanceFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;
	if (!GetArray(L, vuchRetdata) || vuchRetdata.size() != 1
			|| !(vuchRetdata.at(0).get()->size() == 6 || vuchRetdata.at(0).get()->size() == 34)) {
		return RetFalse("ExQueryAccountBalanceFunc para err1");
	}

	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	CKeyID cAddrKeyId;
	if (!GetKeyId(*(pcVmRunEvn->GetCatchView()), *vuchRetdata.at(0).get(), cAddrKeyId)) {
		return RetFalse("ExQueryAccountBalanceFunc para err2");
	}

	CUserID userid(cAddrKeyId);
	CAccount cAccount;
	int nLen = 0;
	if (!pcVmRunEvn->GetCatchView()->GetAccount(userid, cAccount)) {
		nLen = 0;
	} else {
		uint64_t nbalance = cAccount.GetRawBalance();
		CDataStream cTep(SER_DISK, g_sClientVersion);
		cTep << nbalance;
		vector<unsigned char> TMP(cTep.begin(), cTep.end());
		nLen = RetRstToLua(L, TMP);
	}
	return nLen;
}

/**
 *unsigned long GetTxConFirmHeight(const void * const txhash)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ�����: hash,32���ֽ�
 */
static int ExGetTxConFirmHeightFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;

	if (!GetArray(L, vuchRetdata) || vuchRetdata.size() != 1 || vuchRetdata.at(0).get()->size() != 32) {
		return RetFalse("ExGetTxConFirmHeightFunc para err1");
	}
	uint256 cHash1(*vuchRetdata.at(0));
//	LogPrint("vm","ExGetTxContractsFunc1:%s",cHash1.GetHex().c_str());
	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	int nHeight = GetTxComfirmHigh(cHash1, *pcVmRunEvn->GetScriptDB());
	if (-1 == nHeight) {
		return RetFalse("ExGetTxConFirmHeightFunc para err2");
	} else {
		if (lua_checkstack(L, sizeof(lua_Number))) {
			lua_pushnumber(L, (lua_Number) nHeight);
			return 1;
		} else {
			LogPrint("vm", "%s\r\n", "ExGetCurRunEnvHeightFunc stack overflow");
			return 0;
		}
	}
}


/**
 *bool GetBlockHash(const unsigned long nHeight,void * const pblochHash)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� int���͵Ĳ���
 */
static int ExGetBlockHashFunc(lua_State *L) {
	int nHeight = 0;
	if (!GetDataInt(L, nHeight)) {
		return RetFalse("ExGetBlockHashFunc para err1");
	}

	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	if (nHeight <= 0 || nHeight >= pcVmRunEvn->GetComfirHeight()) //��ǰblock �ǲ����Ի�ȡhash��
			{
		return RetFalse("ExGetBlockHashFunc para err2");
	}

	if (g_cChainActive.Height() < nHeight) {	         //��ȡ�ȵ�ǰ�߶ȸߵ������ǲ����Ե�
		return RetFalse("ExGetBlockHashFunc para err3");
	}
	CBlockIndex *pcIndex = g_cChainActive[nHeight];
	uint256 cBlockHash = pcIndex->GetBlockHash();

//	LogPrint("vm","ExGetBlockHashFunc:%s",HexStr(cBlockHash).c_str());
	CDataStream cTep(SER_DISK, g_sClientVersion);
	cTep << cBlockHash;
	vector<unsigned char> TMP(cTep.begin(), cTep.end());
	return RetRstToLua(L, TMP);
}


static int ExGetCurRunEnvHeightFunc(lua_State *L) {
	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	int nHeight = pcVmRunEvn->GetComfirHeight();

	//���ջ�ռ��Ƿ�
	if (nHeight > 0) {
		if (lua_checkstack(L, sizeof(lua_Integer))) {
			lua_pushinteger(L, (lua_Integer) nHeight);
			return 1;
		} else {
			LogPrint("vm", "%s\r\n", "ExGetCurRunEnvHeightFunc stack overflow");
		}
	} else {
		LogPrint("vm", "ExGetCurRunEnvHeightFunc err nHeight =%d\r\n", nHeight);
	}
	return 0;
}

static bool GetDataTableWriteDataDB(lua_State *L, vector<std::shared_ptr<std::vector<unsigned char> > > &ret) {
	//ȡд���ݿ��key value
	if (!lua_istable(L, -1)) {
		LogPrint("vm", "GetDataTableWriteOutput is not table\n");
		return false;
	}
	unsigned short usLen = 0;
	vector<unsigned char> vuchBuf;
	//ȡkey
	string key = "";
	if (!(getStringInTable(L, (char *) "key", key))) {
		LogPrint("vm", "key get fail\n");
		return false;
	} else {
//		LogPrint("vm", "key:%s\n", key);
	}
	vuchBuf.clear();
	for (size_t i = 0; i < key.size(); i++) {
		vuchBuf.insert(vuchBuf.end(), key.at(i));
	}
	ret.insert(ret.end(), std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));

	//ȡvalue�ĳ���
	double dValue = 0;
	if (!(getNumberInTable(L, (char *) "length", dValue))) {
		LogPrint("vm", "length get fail\n");
		return false;
	} else {
		usLen = (unsigned short) dValue;
//    	 LogPrint("vm","usLen =%d\n",usLen);
	}
	if ((usLen > 0) && (usLen <= LUA_C_BUFFER_SIZE)) {
		if (!getArrayInTable(L, (char *) "value", usLen, vuchBuf)) {
			LogPrint("vm", "value is not table\n");
			return false;
		} else {
//			LogPrint("vm", "value:%s\n", HexStr(vuchBuf).c_str());
			ret.insert(ret.end(), std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));
		}
		return true;
	} else {
		LogPrint("vm", "usLen overflow\n");
		return false;
	}
}

/**
 *bool WriteDataDB(const void* const key,const unsigned char nKeyLen,const void * const value,const unsigned short valuelen,const unsigned long time)
 * �������ʽ���м�㴫����������������:
 * 1.��һ���� keyֵ
 * 2.�ڶ�����valueֵ
 */
static int ExWriteDataDBFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;

	if (!GetDataTableWriteDataDB(L, vuchRetdata) || vuchRetdata.size() != 2) {
		return RetFalse("ExWriteDataDBFunc key err1");
	}

	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	const CRegID cScriptId = pcVmRunEvn->GetScriptRegID();
	bool bFlag = true;
	CScriptDBViewCache* pcScriptDB = pcVmRunEvn->GetScriptDB();

	CScriptDBOperLog cOperlog;
//	int64_t step = (*vuchRetdata.at(1)).size() -1;
	if (!pcScriptDB->SetScriptData(cScriptId, *vuchRetdata.at(0), *vuchRetdata.at(1), cOperlog)) {
		bFlag = false;
	} else {
		shared_ptr<vector<CScriptDBOperLog> > m_dblog = pcVmRunEvn->GetDbLog();
		(*m_dblog.get()).push_back(cOperlog);
	}
	return RetRstBooleanToLua(L, bFlag);
}

/**
 *bool DeleteDataDB(const void* const key,const unsigned char nKeyLen)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� keyֵ
 */
static int ExDeleteDataDBFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;

	if (!GetDataString(L, vuchRetdata) || vuchRetdata.size() != 1) {
		LogPrint("vm", "ExDeleteDataDBFunc key err1");
		return RetFalse(string(__FUNCTION__) + "para  err !");
	}
	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}
	CRegID cScriptId = pcVmRunEvn->GetScriptRegID();

	CScriptDBViewCache* pcScriptDB = pcVmRunEvn->GetScriptDB();

	bool bFlag = true;
	CScriptDBOperLog cOperlog;
	int64_t llStep = 0;
	vector<unsigned char> vuchValue;
	if (pcScriptDB->GetScriptData(pcVmRunEvn->GetComfirHeight(), cScriptId, *vuchRetdata.at(0), vuchValue)) {
		llStep = llStep - (int64_t) (vuchValue.size() + 1);	         //ɾ�����ݽ���step
	}
	if (!pcScriptDB->EraseScriptData(cScriptId, *vuchRetdata.at(0), cOperlog)) {
		LogPrint("vm", "ExDeleteDataDBFunc error key:%s!\n", HexStr(*vuchRetdata.at(0)));
		bFlag = false;
	} else {
		shared_ptr<vector<CScriptDBOperLog> > m_dblog = pcVmRunEvn->GetDbLog();
		m_dblog.get()->push_back(cOperlog);
	}
	return RetRstBooleanToLua(L, bFlag);
}

/**
 *unsigned short ReadDataValueDB(const void* const key,const unsigned char nKeyLen, void* const value,unsigned short const maxbuffer)
 * �������ʽ���м�㴫��һ����������:
 * 1.��һ���� keyֵ
 */
static int ExReadDataValueDBFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;

	if (!GetDataString(L, vuchRetdata) || vuchRetdata.size() != 1) {
		return RetFalse("ExReadDataValueDBFunc key err1");
	}

	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	CRegID cScriptId = pcVmRunEvn->GetScriptRegID();

	vector_unsigned_char vuchValue;
	CScriptDBViewCache* pcScriptDB = pcVmRunEvn->GetScriptDB();
	int nLen = 0;
	if (!pcScriptDB->GetScriptData(pcVmRunEvn->GetComfirHeight(), cScriptId, *vuchRetdata.at(0), vuchValue)) {
		nLen = 0;
	} else {
		nLen = RetRstToLua(L, vuchValue);
	}
	return nLen;
}

#if 0
static int ExGetDBSizeFunc(lua_State *L) {
//	CVmRunEvn *pcVmRunEvn = (CVmRunEvn *)pVmEvn;
	CRegID cScriptId = pcVmRunEvn->GetScriptRegID();
	int nCount = 0;
	CScriptDBViewCache* pcScriptDB = pcVmRunEvn->GetScriptDB();
	if(!pcScriptDB->GetScriptDataCount(cScriptId,nCount))
	{
		return RetFalse("ExGetDBSizeFunc can't use");
	}
	else
	{
//		CDataStream cTep(SER_DISK, g_sClientVersion);
//		cTep << nCount;
//		vector<unsigned char> vuchTep1(cTep.begin(),cTep.end());
//        return RetRstToLua(L,vuchTep1);
		LogPrint("vm", "ExGetDBSizeFunc:%d\n", nCount);
	    lua_pushnumber(L,(lua_Number)nCount);
		return 1 ;
	}
}

/**
 *bool GetDBValue(const unsigned long index,void* const key,unsigned char * const nKeyLen,unsigned short maxkeylen,void* const value,unsigned short* const maxbuffer, unsigned long* const ptime)
 * �����ĵ�һ������index == 0������һ����������
 * 1.��һ���� indexֵ
 * �����ĵ�һ������index == 1��������������������
 * 1.��һ���� indexֵ
 * 2.�ڶ���keyֵ
 */
static int ExGetDBValueFunc(lua_State *L) {

	if (SysCfg().GetArg("-isdbtraversal", 0) == 0) {
    	return RetFalse("ExGetDBValueFunc can't use");
	}

	vector<std::shared_ptr < vector<unsigned char> > > vuchRetdata;
    if(!GetArray(L,vuchRetdata) ||(vuchRetdata.size() != 2 && vuchRetdata.size() != 1))
    {
    	return RetFalse("ExGetDBValueFunc index err1");
    }
	int index = 0;
	bool bFlag = true;
	memcpy(&index,&vuchRetdata.at(0).get()->at(0),sizeof(int));
	if(!(index == 0 ||(index == 1 && vuchRetdata.size() == 2)))
	{
	    return RetFalse("ExGetDBValueFunc para err2");
	}
	CRegID cScriptId = pcVmRunEvn->GetScriptRegID();

	vector_unsigned_char vuchValue;
	vector<unsigned char> vScriptKey;
	if(index == 1)
	{
		vScriptKey.assign(vuchRetdata.at(1).get()->begin(),vuchRetdata.at(1).get()->end());
	}

	CScriptDBViewCache* pcScriptDB = pcVmRunEvn->GetScriptDB();
	bFlag = pcScriptDB->GetScriptData(pcVmRunEvn->GetComfirHeight(),cScriptId,index,vScriptKey,vuchValue);
    int nLen = 0;
	if(bFlag){
	    nLen = RetRstToLua(L,vScriptKey) + RetRstToLua(L,vuchValue);
	}
	return nLen;
}
#endif
static int ExGetCurTxHash(lua_State *L) {
	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}
	uint256 cHash = pcVmRunEvn->GetCurTxHash();
	CDataStream cTep(SER_DISK, g_sClientVersion);
	cTep << cHash;
	vector<unsigned char> vuchTep1(cTep.begin(), cTep.end());

	vector<unsigned char> tep2(vuchTep1.rbegin(), vuchTep1.rend());
	return RetRstToLua(L, tep2);
}

/**
 *bool ModifyDataDBVavle(const void* const key,const unsigned char nKeyLen, const void* const pvalue,const unsigned short valuelen)
 * �м�㴫����������
 * 1.��һ���� key
 * 2.�ڶ����� value
 */
static int ExModifyDataDBValueFunc(lua_State *L)
 {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;
	if (!GetDataTableWriteDataDB(L, vuchRetdata) || vuchRetdata.size() != 2) {
		return RetFalse("ExModifyDataDBValueFunc key err1");
	}
	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	CRegID cScriptId = pcVmRunEvn->GetScriptRegID();
	bool bFlag = false;
	CScriptDBViewCache* pcScriptDB = pcVmRunEvn->GetScriptDB();

//	int64_t step = 0;
	CScriptDBOperLog cOperlog;
	vector_unsigned_char vTemp;
	if (pcScriptDB->GetScriptData(pcVmRunEvn->GetComfirHeight(), cScriptId, *vuchRetdata.at(0), vTemp)) {
		if (pcScriptDB->SetScriptData(cScriptId, *vuchRetdata.at(0), *vuchRetdata.at(1).get(), cOperlog)) {
			shared_ptr<vector<CScriptDBOperLog> > m_dblog = pcVmRunEvn->GetDbLog();
			m_dblog.get()->push_back(cOperlog);
			bFlag = true;
		}
	}

//	step =(((int64_t)(*vuchRetdata.at(1)).size())- (int64_t)(vTemp.size()) -1);
	return RetRstBooleanToLua(L, bFlag);
}

static bool GetDataTableWriteOutput(lua_State *L, vector<std::shared_ptr<std::vector<unsigned char> > > &ret) {
	if (!lua_istable(L, -1)) {
		LogPrint("vm", "GetDataTableWriteOutput is not table\n");
		return false;
	}
	double dValue = 0;
	unsigned short usLen = 0;
	vector<unsigned char> vuchBuf;
	CVmOperate cTemp;
	memset(&cTemp, 0, sizeof(cTemp));
	if (!(getNumberInTable(L, (char *) "addrType", dValue))) {
		LogPrint("vm", "addrType get fail\n");
		return false;
	} else {
		cTemp.m_uchNaccType = (unsigned char) dValue;
	}
	if (cTemp.m_uchNaccType == 1) {
		usLen = 6;
	} else if (cTemp.m_uchNaccType == 2) {
		usLen = 34;
	} else {
		LogPrint("vm", "error nacctype:%d\n", cTemp.m_uchNaccType);
		return false;
	}
	if (!getArrayInTable(L, (char *) "accountIdTbl", usLen, vuchBuf)) {
		LogPrint("vm", "accountidTbl not table\n");
		return false;
	} else {
		memcpy(cTemp.m_arruchAccountId, &vuchBuf[0], usLen);
	}
	if (!(getNumberInTable(L, (char *) "operatorType", dValue))) {
		LogPrint("vm", "opeatortype get fail\n");
		return false;
	} else {
		cTemp.m_uchOpeatorType = (unsigned char) dValue;
	}
	if (!(getNumberInTable(L, (char *) "outHeight", dValue))) {
		LogPrint("vm", "outheight get fail\n");
		return false;
	} else {
		cTemp.m_unOutHeight = (unsigned int) dValue;
	}

	if (!getArrayInTable(L, (char *) "moneyTbl", sizeof(cTemp.m_arruchMoney), vuchBuf)) {
		LogPrint("vm", "moneyTbl not table\n");
		return false;
	} else {
		memcpy(cTemp.m_arruchMoney, &vuchBuf[0], sizeof(cTemp.m_arruchMoney));
	}

	CDataStream cTep(SER_DISK, g_sClientVersion);
	cTep << cTemp;
	vector<unsigned char> vuchTep1(cTep.begin(), cTep.end());
	ret.insert(ret.end(), std::make_shared<vector<unsigned char>>(vuchTep1.begin(), vuchTep1.end()));
	return true;
}
/**
 *bool WriteOutput( const VM_OPERATE* data, const unsigned short conter)
 * �м�㴫��һ������ ,д CVmOperate�������
 * 1.��һ�������ָ��
 */
static int ExWriteOutputFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;

	if (!GetDataTableWriteOutput(L, vuchRetdata) || vuchRetdata.size() != 1) {
		return RetFalse("para err0");
	}
	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}
	vector<CVmOperate> vcSource;
	CVmOperate cTemp;
	int nSize = ::GetSerializeSize(cTemp, SER_NETWORK, g_sProtocolVersion);
	int nDataSize = vuchRetdata.at(0)->size();
	int nCount = nDataSize / nSize;
	if (nDataSize % nSize != 0) {
//	  assert(0);
		return RetFalse("para err1");
	}
	CDataStream cSs(*vuchRetdata.at(0), SER_DISK, g_sClientVersion);

	while (nCount--) {
		cSs >> cTemp;
		vcSource.push_back(cTemp);
	}
	if (!pcVmRunEvn->InsertOutputData(vcSource)) {
		return RetFalse("InsertOutput err");
	} else {
		/*
		 * ÿ���������Luaջ��˽�е�,���ѷ���ֵѹ��Luaջ�Ժ󣬸�ջ���Զ������*/
		return RetRstBooleanToLua(L, true);
	}
}


static bool GetDataTableGetScriptData(lua_State *L, vector<std::shared_ptr<std::vector<unsigned char> > > &ret) {
	if (!lua_istable(L, -1)) {
		LogPrint("vm", "GetDataTableGetScriptData is not table\n");
		return false;
	}
	vector<unsigned char> vuchBuf;
	//ȡ�ű�id
	if (!getArrayInTable(L, (char *) "id", 6, vuchBuf)) {
		LogPrint("vm", "idTbl not table\n");
		return false;
	} else {
		ret.insert(ret.end(), std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));
	}

	//ȡkey
	string strKey = "";
	if (!(getStringInTable(L, (char *) "key", strKey))) {
		LogPrint("vm", "key get fail\n");
		return false;
	} else {
//		LogPrint("vm", "key:%s\n", key);
	}

	vuchBuf.clear();
	for (size_t i = 0; i < strKey.size(); i++) {
		vuchBuf.insert(vuchBuf.end(), strKey.at(i));
	}
	ret.insert(ret.end(), std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));
	return true;
}

/**
 *bool GetScriptData(const void* const scriptID,void* const pkey,short len,void* const pvalve,short maxlen)
 * �м�㴫������������
 * 1.�ű���id��
 * 2.���ݿ��keyֵ
 */
static int ExGetScriptDataFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;

	if (!GetDataTableGetScriptData(L, vuchRetdata) || vuchRetdata.size() != 2 || vuchRetdata.at(0).get()->size() != 6) {
		return RetFalse("ExGetScriptDataFunc vuchTep1 err1");
	}
	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	vector_unsigned_char vuchValue;
	CScriptDBViewCache* pcScriptDB = pcVmRunEvn->GetScriptDB();
	CRegID cScriptId(*vuchRetdata.at(0));
	int nLen = 0;
	if (!pcScriptDB->GetScriptData(pcVmRunEvn->GetComfirHeight(), cScriptId, *vuchRetdata.at(1), vuchValue)) {
		nLen = 0;
	} else {
		//3.������˽��ջ��������Ľ��
		nLen = RetRstToLua(L, vuchValue);
	}
	/*
	 * ÿ���������Luaջ��˽�е�,���ѷ���ֵѹ��Luaջ�Ժ󣬸�ջ���Զ������*/
	return nLen;
}



/**
 * ȡĿ���˻�ID
 * @param ipara
 * @param pVmEvn
 * @return
 */
static int ExGetScriptIDFunc(lua_State *L) {
	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}
	//1.��luaȡ����
	//2.����C++�⺯�� ִ������
	vector_unsigned_char cScriptId = pcVmRunEvn->GetScriptRegID().GetVec6();
	//3.������˽��ջ��������Ľ��
	int nLen = RetRstToLua(L, cScriptId);
	/*
	 * ÿ���������Luaջ��˽�е�,���ѷ���ֵѹ��Luaջ�Ժ󣬸�ջ���Զ������*/
	return nLen; //number of results ����Lua�����˼�������ֵ
}
static int ExGetCurTxAccountFunc(lua_State *L) {
	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}
	//1.��luaȡ����
	//2.����C++�⺯�� ִ������
	vector_unsigned_char vUserId = pcVmRunEvn->GetTxAccount().GetVec6();

	//3.������˽��ջ��������Ľ��
	int nLen = RetRstToLua(L, vUserId);
	/*
	 * ÿ���������Luaջ��˽�е�,���ѷ���ֵѹ��Luaջ�Ժ󣬸�ջ���Զ������*/
	return nLen; //number of results ����Lua�����˼�������ֵ
}

static int GetCurTxPayAmountFunc(lua_State *L) {

	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}
	uint64_t ullValue = pcVmRunEvn->GetValue();

	CDataStream cTep(SER_DISK, g_sClientVersion);
	cTep << ullValue;
	vector<unsigned char> vuchTep1(cTep.begin(), cTep.end());
	int nLen = RetRstToLua(L, vuchTep1);
	/*
	 * ÿ���������Luaջ��˽�е�,���ѷ���ֵѹ��Luaջ�Ժ󣬸�ջ���Զ������*/
	return nLen; //number of results ����Lua�����˼�������ֵ
}


struct ST_APP_ID
{
	unsigned char uchIdLen;                    //!the nLen of the tag
	unsigned char arruchID[CAppCFund::MAX_TAG_SIZE];     //! the ID for the

	const vector<unsigned char> GetIdV() const {
//		assert(sizeof(ID) >= idlen);
		vector<unsigned char> vuchId(&arruchID[0], &arruchID[uchIdLen]);
		return (vuchId);
	}
}__attribute((aligned (1)));

static int GetUserAppAccValue(lua_State *L) {

	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;
	if (!lua_istable(L, -1)) {
		LogPrint("vm", "is not table\n");
		return 0;
	}
	double dValue = 0;
	vector<unsigned char> vuchBuf;
	ST_APP_ID tAccID;
	memset(&tAccID, 0, sizeof(tAccID));
	if (!(getNumberInTable(L, (char *) "idLen", dValue))) {
		LogPrint("vm", "idlen get fail\n");
		return 0;
	} else {
		tAccID.uchIdLen = (unsigned char) dValue;
	}
	if ((tAccID.uchIdLen < 1) || (tAccID.uchIdLen > sizeof(tAccID.arruchID))) {
		LogPrint("vm", "idlen is err\n");
		return 0;
	}
	if (!getArrayInTable(L, (char *) "idValueTbl", tAccID.uchIdLen, vuchBuf)) {
		LogPrint("vm", "idValueTbl not table\n");
		return 0;
	} else {
		memcpy(&tAccID.arruchID[0], &vuchBuf[0], tAccID.uchIdLen);
	}

	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	shared_ptr<CAppUserAccout> cSptrAcc;
	uint64_t ullValueData = 0;
	int nLen = 0;
	if (pcVmRunEvn->GetAppUserAccout(tAccID.GetIdV(), cSptrAcc)) {
		ullValueData = cSptrAcc->getllValues();
		CDataStream cTep(SER_DISK, g_sClientVersion);
		cTep << ullValueData;
		vector<unsigned char> TMP(cTep.begin(), cTep.end());
		nLen = RetRstToLua(L, TMP);
	}
	return nLen;
}

static bool GetDataTableOutAppOperate(lua_State *L, vector<std::shared_ptr<std::vector<unsigned char> > > &ret) {
	if (!lua_istable(L, -1)) {
		LogPrint("vm", "is not table\n");
		return false;
	}
	double dValue = 0;
	vector<unsigned char> vuchBuf;
	CAppFundOperate cTemp;
	memset(&cTemp, 0, sizeof(cTemp));
	if (!(getNumberInTable(L, (char *) "operatorType", dValue))) {
		LogPrint("vm", "opeatortype get fail\n");
		return false;
	} else {
		cTemp.m_uchOpeatorType = (unsigned char) dValue;
	}
	if (!(getNumberInTable(L, (char *) "outHeight", dValue))) {
		LogPrint("vm", "outheight get fail\n");
		return false;
	} else {
		cTemp.m_unOutHeight = (unsigned int) dValue;
	}

	if (!getArrayInTable(L, (char *) "moneyTbl", sizeof(cTemp.m_llMoney), vuchBuf)) {
		LogPrint("vm", "moneyTbl not table\n");
		return false;
	} else {
		memcpy(&cTemp.m_llMoney, &vuchBuf[0], sizeof(cTemp.m_llMoney));
	}
	if (!(getNumberInTable(L, (char *) "userIdLen", dValue))) {
		LogPrint("vm", "appuserIDlen get fail\n");
		return false;
	} else {
		cTemp.m_uchAppuserIDlen = (unsigned char) dValue;
	}
	if ((cTemp.m_uchAppuserIDlen < 1) || (cTemp.m_uchAppuserIDlen > sizeof(cTemp.m_arruchAppuser))) {
		LogPrint("vm", "appuserIDlen is err\n");
		return false;
	}
	if (!getArrayInTable(L, (char *) "userIdTbl", cTemp.m_uchAppuserIDlen, vuchBuf)) {
		LogPrint("vm", "useridTbl not table\n");
		return false;
	} else {
		memcpy(cTemp.m_arruchAppuser, &vuchBuf[0], cTemp.m_uchAppuserIDlen);
	}
	if (!(getNumberInTable(L, (char *) "fundTagLen", dValue))) {
		LogPrint("vm", "FundTaglen get fail\n");
		return false;
	} else {
		cTemp.m_uchFundTaglen = (unsigned char) dValue;
	}
	if ((cTemp.m_uchFundTaglen > 0) && (cTemp.m_uchFundTaglen <= sizeof(cTemp.m_arruchFundTag))) {
		if (!getArrayInTable(L, (char *) "fundTagTbl", cTemp.m_uchFundTaglen, vuchBuf)) {
			LogPrint("vm", "FundTagTbl not table\n");
			return false;
		} else {
			memcpy(cTemp.m_arruchFundTag, &vuchBuf[0], cTemp.m_uchFundTaglen);
		}
	}
	CDataStream cTep(SER_DISK, g_sClientVersion);
	cTep << cTemp;
	vector<unsigned char> vuchTep1(cTep.begin(), cTep.end());
	ret.insert(ret.end(), std::make_shared<vector<unsigned char>>(vuchTep1.begin(), vuchTep1.end()));
	return true;
}

static int GetUserAppAccFoudWithTag(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;
	unsigned int unSize(0);
	CAppFundOperate cTemp;
	unSize = ::GetSerializeSize(cTemp, SER_NETWORK, g_sProtocolVersion);

	if (!GetDataTableOutAppOperate(L, vuchRetdata) || vuchRetdata.size() != 1
			|| vuchRetdata.at(0).get()->size() != unSize) {
		return RetFalse("GetUserAppAccFoudWithTag para err0");
	}

	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	CDataStream cSs(*vuchRetdata.at(0), SER_DISK, g_sClientVersion);
	CAppFundOperate cUserfund;
	cSs >> cUserfund;

	shared_ptr<CAppUserAccout> sptrAcc;
	CAppCFund cFund;
	int nLen = 0;
	if (pcVmRunEvn->GetAppUserAccout(cUserfund.GetAppUserV(), sptrAcc)) {
		if (!sptrAcc->GetAppCFund(cFund, cUserfund.GetFundTagV(), cUserfund.m_unOutHeight)) {
			return RetFalse("GetUserAppAccFoudWithTag get fail");
		}
		CDataStream cTep(SER_DISK, g_sClientVersion);
		cTep << cFund.getValue();
		vector<unsigned char> TMP(cTep.begin(), cTep.end());
		nLen = RetRstToLua(L, TMP);
	}
	return nLen;
}

static bool GetDataTableAssetOperate(lua_State *L, int nIndex,
		vector<std::shared_ptr<std::vector<unsigned char> > > &ret) {
	if (!lua_istable(L, nIndex)) {
		LogPrint("vm", "is not table\n");
		return false;
	}
	double dValue = 0;
	vector<unsigned char> vuchBuf;
	CAssetOperate cTemp;
	memset(&cTemp, 0, sizeof(cTemp));

	if (!getArrayInTable(L, (char *) "toAddrTbl", 34, vuchBuf)) {
		LogPrint("vm", "toAddrTbl not table\n");
		return false;
	} else {

		ret.push_back(std::make_shared<vector<unsigned char>>(vuchBuf.begin(), vuchBuf.end()));
	}

	if (!(getNumberInTable(L, (char *) "outHeight", dValue))) {
		LogPrint("vm", "outheight get fail\n");
		return false;
	} else {
		cTemp.m_unOutHeight = (unsigned int) dValue;
	}

	if (!getArrayInTable(L, (char *) "moneyTbl", sizeof(cTemp.m_ullMoney), vuchBuf)) {
		LogPrint("vm", "moneyTbl not table\n");
		return false;
	} else {
		memcpy(&cTemp.m_ullMoney, &vuchBuf[0], sizeof(cTemp.m_ullMoney));
	}
	if (!(getNumberInTable(L, (char *) "fundTagLen", dValue))) {
		LogPrint("vm", "FundTaglen get fail\n");
		return false;
	} else {
		cTemp.m_uchFundTaglen = (unsigned char) dValue;
	}
	if ((cTemp.m_uchFundTaglen > 0) && (cTemp.m_uchFundTaglen <= sizeof(cTemp.m_vuchFundTag))) {
		if (!getArrayInTable(L, (char *) "fundTagTbl", cTemp.m_uchFundTaglen, vuchBuf)) {
			LogPrint("vm", "FundTagTbl not table\n");
			return false;
		} else {
			memcpy(cTemp.m_vuchFundTag, &vuchBuf[0], cTemp.m_uchFundTaglen);
		}
	}
	CDataStream cTep(SER_DISK, g_sClientVersion);
	cTep << cTemp;
	vector<unsigned char> vuchTep1(cTep.begin(), cTep.end());
	ret.insert(ret.end(), std::make_shared<vector<unsigned char>>(vuchTep1.begin(), vuchTep1.end()));
	return true;
}

/**
 *   д Ӧ�ò�������� pcVmRunEvn->MapAppOperate[0]
 * @param ipara
 * @param pVmEvn
 * @return
 */
static int ExWriteOutAppOperateFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;

	CAppFundOperate cTemp;
	unsigned int unSize = ::GetSerializeSize(cTemp, SER_NETWORK, g_sProtocolVersion);

	if (!GetDataTableOutAppOperate(L, vuchRetdata) || vuchRetdata.size() != 1
			|| (vuchRetdata.at(0).get()->size() % unSize) != 0) {
		return RetFalse("ExWriteOutAppOperateFunc para err1");
	}

	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	int nCount = vuchRetdata.at(0).get()->size() / unSize;
	CDataStream cSs(*vuchRetdata.at(0), SER_DISK, g_sClientVersion);

	int64_t llStep = -1;
	while (nCount--) {
		cSs >> cTemp;
		if (pcVmRunEvn->GetComfirHeight() > g_sFreezeBlackAcctHeight && cTemp.m_llMoney < 0) //����С��0,��ֹ �ϲ㴫����С��20150904
				{
			return RetFalse("ExWriteOutAppOperateFunc para err2");
		}
		pcVmRunEvn->InsertOutAPPOperte(cTemp.GetAppUserV(), cTemp);
		llStep += unSize;
	}

	/*
	 * ÿ���������Luaջ��˽�е�,���ѷ���ֵѹ��Luaջ�Ժ󣬸�ջ���Զ������*/
	return RetRstBooleanToLua(L, true);
}

static int ExGetBase58AddrFunc(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;

	if (!GetArray(L, vuchRetdata) || vuchRetdata.size() != 1 || vuchRetdata.at(0).get()->size() != 6) {
		return RetFalse("ExGetBase58AddrFunc para err0");
	}

	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	CKeyID cAddrKeyId;
	if (!GetKeyId(*pcVmRunEvn->GetCatchView(), *vuchRetdata.at(0).get(), cAddrKeyId)) {
		return RetFalse("ExGetBase58AddrFunc para err1");
	}
	string strCoinAddr = cAddrKeyId.ToAddress();

	vector<unsigned char> vTemp;
	vTemp.assign(strCoinAddr.c_str(), strCoinAddr.c_str() + strCoinAddr.length());
	return RetRstToLua(L, vTemp);
}

static int ExTransferContactAsset(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;

	if (!GetArray(L, vuchRetdata) || vuchRetdata.size() != 1 || vuchRetdata.at(0).get()->size() != 34) {
		return RetFalse(string(__FUNCTION__) + "para  err !");
	}
	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	vector<unsigned char> vuchSendkey;
	vector<unsigned char> vuchRecvkey;
	CRegID cScript = pcVmRunEvn->GetScriptRegID();

	CRegID cSendRegID = pcVmRunEvn->GetTxAccount();
	CKeyID cSendKeyID = cSendRegID.getKeyID(*pcVmRunEvn->GetCatchView());
	string strAddr = cSendKeyID.ToAddress();
	vuchSendkey.assign(strAddr.c_str(), strAddr.c_str() + strAddr.length());

	vuchRecvkey.assign(vuchRetdata.at(0).get()->begin(), vuchRetdata.at(0).get()->end());

	std::string recvaddr(vuchRecvkey.begin(), vuchRecvkey.end());
	if (strAddr == recvaddr) {
		LogPrint("vm", "%s\n", "send strAddr and recv strAddr is same !");
		return RetFalse(string(__FUNCTION__) + "send strAddr and recv strAddr is same !");
	}

	CKeyID cRecvKeyID;
	bool bValid = GetKeyId(*pcVmRunEvn->GetCatchView(), vuchRecvkey, cRecvKeyID);
	if (!bValid) {
		LogPrint("vm", "%s\n", "recv strAddr is not valid !");
		return RetFalse(string(__FUNCTION__) + "recv strAddr is not valid !");
	}

	std::shared_ptr<CAppUserAccout> cTemp = std::make_shared<CAppUserAccout>();
	CScriptDBViewCache* pContractScript = pcVmRunEvn->GetScriptDB();

	if (!pContractScript->GetScriptAcc(cScript, vuchSendkey, *cTemp.get())) {
		return RetFalse(string(__FUNCTION__) + "para  err3 !");
	}

	cTemp.get()->AutoMergeFreezeToFree(cScript.getHight(), g_cChainActive.Tip()->m_nHeight);

	uint64_t ullMoney = cTemp.get()->getllValues();

	int i = 0;
	CAppFundOperate cOp;
	memset(&cOp, 0, sizeof(cOp));

	if (ullMoney > 0) {
		cOp.m_llMoney = ullMoney;
		cOp.m_unOutHeight = 0;
		cOp.m_uchOpeatorType = EM_SUB_FREE_OP;
		cOp.m_uchAppuserIDlen = vuchSendkey.size();
		for (i = 0; i < cOp.m_uchAppuserIDlen; i++) {
			cOp.m_arruchAppuser[i] = vuchSendkey[i];
		}

		pcVmRunEvn->InsertOutAPPOperte(vuchSendkey, cOp);

		cOp.m_uchOpeatorType = EM_ADD_FREE_OP;
		cOp.m_uchAppuserIDlen = vuchRecvkey.size();
		for (i = 0; i < cOp.m_uchAppuserIDlen; i++) {
			cOp.m_arruchAppuser[i] = vuchRecvkey[i];
		}
		pcVmRunEvn->InsertOutAPPOperte(vuchRecvkey, cOp);
	}

	vector<CAppCFund> vcTemp = cTemp.get()->getFreezedFund();
	for (auto fund : vcTemp) {
		cOp.m_llMoney = fund.getValue();
		cOp.m_unOutHeight = fund.getHeight();
		cOp.m_uchOpeatorType = EM_SUB_TAG_OP;
		cOp.m_uchAppuserIDlen = vuchSendkey.size();
		for (i = 0; i < cOp.m_uchAppuserIDlen; i++) {
			cOp.m_arruchAppuser[i] = vuchSendkey[i];
		}

		cOp.m_uchFundTaglen = fund.getTag().size();
		for (i = 0; i < cOp.m_uchFundTaglen; i++) {
			cOp.m_arruchFundTag[i] = fund.getTag()[i];
		}

		pcVmRunEvn->InsertOutAPPOperte(vuchSendkey, cOp);

		cOp.m_uchOpeatorType = EM_ADD_TAG_OP;
		cOp.m_uchAppuserIDlen = vuchRecvkey.size();
		for (i = 0; i < cOp.m_uchAppuserIDlen; i++) {
			cOp.m_arruchAppuser[i] = vuchRecvkey[i];
		}

		pcVmRunEvn->InsertOutAPPOperte(vuchRecvkey, cOp);
	}

	return RetRstBooleanToLua(L, true);

}


static int ExTransferSomeAsset(lua_State *L) {
	vector<std::shared_ptr<vector<unsigned char> > > vuchRetdata;

	unsigned int unSize(0);
	CAssetOperate cTempAsset;
	unSize = ::GetSerializeSize(cTempAsset, SER_NETWORK, g_sProtocolVersion);

	if (!GetDataTableAssetOperate(L, -1, vuchRetdata) || vuchRetdata.size() != 2
			|| (vuchRetdata.at(1).get()->size() % unSize) != 0 || vuchRetdata.at(0).get()->size() != 34) {
		return RetFalse(string(__FUNCTION__) + "para err !");
	}
	CVmRunEvn* pcVmRunEvn = GetVmRunEvn(L);
	if (NULL == pcVmRunEvn) {
		return RetFalse("pcVmRunEvn is NULL");
	}

	CDataStream cSs(*vuchRetdata.at(1), SER_DISK, g_sClientVersion);
	CAssetOperate cAssetOp;
	cSs >> cAssetOp;

	vector<unsigned char> vuchSendkey;
	vector<unsigned char> vuchRecvkey;
	CRegID cScript = pcVmRunEvn->GetScriptRegID();

	CRegID cSendRegID = pcVmRunEvn->GetTxAccount();
	CKeyID cSendKeyID = cSendRegID.getKeyID(*pcVmRunEvn->GetCatchView());
	string strAddr = cSendKeyID.ToAddress();
	vuchSendkey.assign(strAddr.c_str(), strAddr.c_str() + strAddr.length());

	vuchRecvkey.assign(vuchRetdata.at(0).get()->begin(), vuchRetdata.at(0).get()->end());

	std::string recvaddr(vuchRecvkey.begin(), vuchRecvkey.end());
	if (strAddr == recvaddr) {
		LogPrint("vm", "%s\n", "send strAddr and recv strAddr is same !");
		return RetFalse(string(__FUNCTION__) + "send strAddr and recv strAddr is same !");
	}

	CKeyID cRecvKeyID;
	bool bValid = GetKeyId(*pcVmRunEvn->GetCatchView(), vuchRecvkey, cRecvKeyID);
	if (!bValid) {
		LogPrint("vm", "%s\n", "recv strAddr is not valid !");
		return RetFalse(string(__FUNCTION__) + "recv strAddr is not valid !");
	}

	uint64_t uTransferMoney = cAssetOp.GetUint64Value();
	if (0 == uTransferMoney) {
		return RetFalse(string(__FUNCTION__) + "Transfer Money is not valid !");
	}

	int nHeight = cAssetOp.getheight();
	if (nHeight < 0) {
		return RetFalse(string(__FUNCTION__) + "outHeight is not valid !");
	}

	int i = 0;
	CAppFundOperate cOp;
	memset(&cOp, 0, sizeof(cOp));
	vector<unsigned char> vtag = cAssetOp.GetFundTagV();
	cOp.m_uchFundTaglen = vtag.size();

	for (i = 0; i < cOp.m_uchFundTaglen; i++) {
		cOp.m_arruchFundTag[i] = vtag[i];
	}

	cOp.m_llMoney = uTransferMoney;
	cOp.m_unOutHeight = nHeight;
	cOp.m_uchAppuserIDlen = vuchSendkey.size();

	for (i = 0; i < cOp.m_uchAppuserIDlen; i++) {
		cOp.m_arruchAppuser[i] = vuchSendkey[i];
	}
	if (nHeight > 0)
		cOp.m_uchOpeatorType = EM_SUB_TAG_OP;
	else
		cOp.m_uchOpeatorType = EM_SUB_FREE_OP;
	pcVmRunEvn->InsertOutAPPOperte(vuchSendkey, cOp);

	if (nHeight > 0)
		cOp.m_uchOpeatorType = EM_ADD_TAG_OP;
	else
		cOp.m_uchOpeatorType = EM_ADD_FREE_OP;
	cOp.m_uchAppuserIDlen = vuchRecvkey.size();
	for (i = 0; i < cOp.m_uchAppuserIDlen; i++) {
		cOp.m_arruchAppuser[i] = vuchRecvkey[i];
	}
	pcVmRunEvn->InsertOutAPPOperte(vuchRecvkey, cOp);

	return RetRstBooleanToLua(L, true);

}

static int ExGetBlockTimestamp(lua_State *L) {
	int nHeight = 0;
	if (!GetDataInt(L, nHeight)) {
		return RetFalse("ExGetBlcokTimestamp para err1");
	}

	if (nHeight <= 0) {
		nHeight = g_cChainActive.Height() + nHeight;
		if (nHeight < 0) {
			return RetFalse("ExGetBlcokTimestamp para err2");
		}
	}

	CBlockIndex *pcIndex = g_cChainActive[nHeight];
	if (!pcIndex) {
		return RetFalse("ExGetBlcokTimestamp get time stamp error");
	}

	if (lua_checkstack(L, sizeof(lua_Integer))) {
		lua_pushinteger(L, (lua_Integer) pcIndex->m_unTime);
		return 1;
	}

	LogPrint("vm", "%s\r\n", "ExGetBlcokTimestamp stack overflow");
	return 0;
}

static const luaL_Reg mylib[] = { //
		{"Int64Mul", ExInt64MulFunc },			//
		{"Int64Add", ExInt64AddFunc },			//
		{"Int64Sub", ExInt64SubFunc },			//
		{"Int64Div", ExInt64DivFunc },			//
		{"Sha256", ExSha256Func },			//
		{"Des", ExDesFunc },			    //
		{"VerifySignature", ExVerifySignatureFunc },   //
		{"LogPrint", ExLogPrintFunc },         //
		{"GetTxContracts",ExGetTxContractsFunc},            //
		{"GetTxAccounts",ExGetTxAccountsFunc},
		{"GetAccountPublickey",ExGetAccountPublickeyFunc},
		{"QueryAccountBalance",ExQueryAccountBalanceFunc},
		{"GetTxConFirmHeight",ExGetTxConFirmHeightFunc},
		{"GetBlockHash",ExGetBlockHashFunc},


		{"GetCurRunEnvHeight",ExGetCurRunEnvHeightFunc},
		{"WriteData",ExWriteDataDBFunc},
		{"DeleteData",ExDeleteDataDBFunc},
		{"ReadData",ExReadDataValueDBFunc},
		{"GetCurTxHash",ExGetCurTxHash},
		{"ModifyData",ExModifyDataDBValueFunc},

		{"WriteOutput",ExWriteOutputFunc},
		{"GetScriptData",ExGetScriptDataFunc},
		{"GetScriptID",ExGetScriptIDFunc},
		{"GetCurTxAccount",ExGetCurTxAccountFunc},
		{"GetCurTxPayAmount",GetCurTxPayAmountFunc},

		{"GetUserAppAccValue",GetUserAppAccValue},
		{"GetUserAppAccFoudWithTag",GetUserAppAccFoudWithTag},
		{"WriteOutAppOperate",ExWriteOutAppOperateFunc},

		{"GetBase58Addr",ExGetBase58AddrFunc},
		{"ByteToInteger",ExByteToIntegerFunc},
		{"IntegerToByte4",ExIntegerToByte4Func},
		{"IntegerToByte8",ExIntegerToByte8Func},
		{"TransferContactAsset", ExTransferContactAsset},
		{"TransferSomeAsset", ExTransferSomeAsset},
		{"GetBlockTimestamp", ExGetBlockTimestamp},
		{NULL,NULL}

		};

/*
 * ע��һ����Luaģ��*/
#ifdef WIN_DLL
extern "C" __declspec(dllexport)int luaopen_mylib(lua_State *L)
#else
LUAMOD_API int luaopen_mylib(lua_State *L)
#endif
{
	luaL_newlib(L,mylib);//����һ��table,��mylibs���к�������ȥ
	return 1;
}

