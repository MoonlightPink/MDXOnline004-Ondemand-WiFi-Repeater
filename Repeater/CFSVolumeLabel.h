#pragma once 

#include "incs.h"

class CFSVolumeLabel {
public:
	String Name;
	void SetName(String text) {
		Name = text;
	}
	void SetName(byte* _Name, byte _NameLen) {
		Name = "";
		for (int idx = 0; idx < _NameLen; idx++) {
			Name += String((char)_Name[idx]);
		}
	}
	void SetName(CBuffer Buffer) {
		Name = "";
		u8 NameLen = Buffer.ReadU8();
		for (int idx = 0; idx < NameLen; idx++) {
			Name += String((char)Buffer.ReadU8());
		}
	}
};

