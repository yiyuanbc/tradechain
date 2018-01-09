/*
 * VmScript.h
 *
 *  Created on: Sep 2, 2014
 *      Author: ranger.shi
 */

#ifndef COIN_VM_SCRIPT_H_
#define COIN_VM_SCRIPT_H_

#include "serialize.h"
using namespace std;

/**
 * @brief Load script binary code class
 */
class CVmScript {
 public:
	vector<unsigned char> m_vuchRom;      		//!< Binary code
	vector<unsigned char> m_vuchScriptExplain;	// !<explain the binary code action
	int  m_nScriptType = 0;                    //!<�ű������� 0:8051,1:lua

 public:
	/**
	 * @brief
	 * @return
	 */
	bool IsValid() {
		///Binary code'size less 64k
		if ((m_vuchRom.size() > 64 * 1024) || (m_vuchRom.size() <= 0)) {
			return false;
		}
		if (m_vuchRom[0] != 0x02) {
			if (!memcmp(&m_vuchRom[0], "mylib = require", strlen("mylib = require"))) {
				m_nScriptType = 1;                    //lua�ű�
				return true;                    //lua�ű���ֱ�ӷ���
			} else {
				return false;
			}
		} else {
			m_nScriptType = 0;                    //8051�ű�
		}
		//!<ָ���汾��SDK���ϣ���ȥУ�� �˻�ƽ�⿪�ص�ȡֵ
		if (memcmp(&m_vuchRom[0x0004], "\x00\x02\x02", 3) >= 0) {
			if (!((m_vuchRom[0x0014] == 0x00) || (m_vuchRom[0x0014] == 0x01))) {
				return false;
			}
		}
		return true;
	}

	bool IsCheckAccount(void) {
		if (m_nScriptType) {
			return false;                    //lua�ű���ֱ�ӷ���(�ص��˻�ƽ��)
		}

		//!<ָ���汾��SDK���ϣ���ȥ��ȡ �˻�ƽ�⿪�ص�ȡֵ
		if (memcmp(&m_vuchRom[0x0004], "\x00\x02\x02", 3) >= 0) {
			if (m_vuchRom[0x0014] == 0x01) {
				return true;
			}
		}
		return false;
	}

	CVmScript();

	int getScriptType() {
		return m_nScriptType;
	}
	IMPLEMENT_SERIALIZE
	(
			READWRITE(m_vuchRom);
			READWRITE(m_vuchScriptExplain);
	)

	virtual ~CVmScript();
};

#endif /* COIN_VM_SCRIPT_H_ */
