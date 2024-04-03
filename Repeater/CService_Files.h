#pragma once 

#include "incs.h"

class CService_Files {
private:
	static const int Namelen = 21;
	byte WildCard[Namelen];

	bool CheckMatch(byte cmd_attr, byte atr, String name) {
		if ((atr & cmd_attr) == 0) { return false; } // 属性がマッチしなかった

		byte w2[Namelen];
		for (int idx = 0; idx < sizeof(w2); idx++) { w2[idx] = 0x00; }

		{ // ファイル名を分解する
			int dotpos = -1;
			for (int idx = 0; idx < name.length(); idx++) {
				if (name[idx] == '.') { dotpos = idx; }
			}
			if (dotpos == -1) {
				for (int idx = 0; idx < name.length(); idx++) { w2[idx] = name[idx]; }
			} else {
				for (int idx = 0; idx < dotpos; idx++) { w2[idx] = name[idx]; }
				for (int idx = 0; ; idx++) {
					byte ch = name[dotpos + 1 + idx];
					if (ch == 0x00) { break; }
					w2[18 + idx] = ch;
				}
			}
		}

		{ //ファイル名を比較する
			int f = 0x20;  //0x00=次のバイトはSJISの2バイト目,0x20=次のバイトはSJISの2バイト目ではない
			int i;
			for (i = 0; i < Namelen; i++) {
				int c = w2[i];
				int d = WildCard[i];
				if (d != '?' && ('A' <= c && c <= 'Z' ? c | f : c) != d) { break; }  //検索するファイル名の'?'以外の部分がマッチしない。SJISの2バイト目でなければ小文字化してから比較する
				f = f != 0x00 && (0x81 <= c && c <= 0x9f || 0xe0 <= c && c <= 0xef) ? 0x00 : 0x20;  //このバイトがSJISの2バイト目ではなくてSJISの1バイト目ならば次のバイトはSJISの2バイト目
			}
			if (i < Namelen) { return false; } //ファイル名がマッチしなかった
		}

		return true;
	}

	void Store(byte atr, UInt32 LastWriteDT, UInt32 filelen, String name) {
		Physical.pSendBuf->WriteBEU8(0x00); // dummy
		Physical.pSendBuf->WriteBEU8(atr);
		Physical.pSendBuf->WriteBEU32(LastWriteDT);
		Physical.pSendBuf->WriteBEU32(filelen);
		for (int idx = 0; idx < 24; idx++) {
			if (idx < name.length()) {
				Physical.pSendBuf->WriteBEU8(name[idx]);
			} else {
				Physical.pSendBuf->WriteBEU8(0x00);
			}
		}
	}

	void StoreVolumeLabel() {
		Store(0x08, 0, 0, CFSMain::VolumeLabel.Name);
	}

	void StoreEntry(byte atr, UInt32 LastWriteDT, UInt32 filelen, String name) {
		Store(atr, LastWriteDT, filelen, name);
	}

	byte FindAttr;

	u8 GetResultCode(int Count) {
		return (Count == 0) ? CService::_DOSE_NOMORE : CService::RES_OK;
	}

public:
	u8 op_files(u8 ID) {
		u8 cmd_attr = Physical.pReceiveBuf->ReadBEU8();
		u8 cmd_num = Physical.pReceiveBuf->ReadBEU8();
		u32 cmd_fcb = Physical.pReceiveBuf->ReadBEU32();

			DebugSerial.println("cmd_attr: 0x" + String(cmd_attr, HEX));
			DebugSerial.println("cmd_num: " + String(cmd_num));
			// DebugSerial.println("cmd_fcb: 0x" + String(cmd_fcb, HEX));

		CService::Cdos_namebuf* pcmd_path = new CService::Cdos_namebuf(Physical.pReceiveBuf);

			DebugSerial.println("isRoot: " + String(pcmd_path->isRoot()));

		String path = pcmd_path->conv_namebufToSJISText(false);
			DebugSerial.println("path: " + path);

		pcmd_path->dos_namebuf_GetWildCard(&WildCard[0]);

		if (DEBUG) {
			DebugSerial.println("WildCard: ");
			for (int idx = 0; idx < Namelen; idx++) { DebugSerial.print(String(WildCard[idx], HEX) + ","); }
			DebugSerial.println();
			for (int idx = 0; idx < Namelen; idx++) { DebugSerial.print((char)(WildCard[idx])); }
			DebugSerial.println();
		}

		FindAttr = cmd_attr;

		// ルートディレクトリかつボリューム名が必要な場合（手抜きでボリュームラベルしか返さない）
		if (pcmd_path->isRoot() && (FindAttr == 0x08)) {
			Physical.pSendBuf->SetSizeMax();
			Physical.pSendBuf->SetPos(2);
			byte res_num = 0;
			if (WildCard[0] == '?' && WildCard[18] == '?') {    //検索するファイル名が*.*のとき
				if (res_num < cmd_num) {
					StoreVolumeLabel();
					res_num++;
				}
			}
			Physical.pSendBuf->AdjustSizePos();
			Physical.pSendBuf->SetPos(0);
			Physical.pSendBuf->WriteBEU8(GetResultCode(res_num));
			Physical.pSendBuf->WriteBEU8(res_num);
			return CService::RES_Setup;
		} else {
			UInt16 FolderID = CFSMain::GetFolderID(path);
			if (FolderID == CFSCommon::FolderID_Undef) { // ディレクトリが存在しない場合に_DOSE_NOENTを返すと正常動作しない
				DebugSerial.println("Not found folder.");
				return CService::_DOSE_NODIR;
			}
			FSMain.Folder.SetFolderID(FolderID);
			FSMain.Folder.FilesReset();

			Physical.pSendBuf->SetSizeMax();
			Physical.pSendBuf->SetPos(2);
			byte res_num = 0;
			if (!pcmd_path->isRoot() && ((FindAttr & CService::GetAtr(false, true)) != 0)) { // Root back dir
				if (WildCard[0] == '?' && WildCard[18] == '?') {    //検索するファイル名が*.*のとき
					 DebugSerial.println("Add. Change dir entry."); 
					//StoreEntry(CService::GetAtr(false, true), 0x0000, 0, "."); res_num++;
					StoreEntry(CService::GetAtr(false, true), 0x0000, 0, ".."); res_num++;
				}
			}
			while (true) {
				if (cmd_num <= res_num) { break; }
				CFSCommon::CEntry* pEntry = FSMain.Folder.FilesGetNext();
				if (pEntry == null) { break; }
				if (DEBUG) { DebugSerial.println("Found. Name=" + pEntry->Name); }
				byte atr = CService::GetAtr(pEntry->isFile(), pEntry->isDir());
				if (CheckMatch(FindAttr, atr, pEntry->Name)) {
					if (DEBUG) { DebugSerial.println("Match."); }
					StoreEntry(atr, pEntry->LastWriteDT, pEntry->Size, pEntry->Name);
					res_num++;
				}
			}
			DebugSerial.println("res_num=" + String(res_num));

			Physical.pSendBuf->AdjustSizePos();
			Physical.pSendBuf->SetPos(0);
			Physical.pSendBuf->WriteBEU8(GetResultCode(res_num));
			Physical.pSendBuf->WriteBEU8(res_num);
			return CService::RES_Setup;
		}
	}

	u8 op_nfiles(u8 ID) {
		u8 cmd_num = Physical.pReceiveBuf->ReadBEU8();
		u32 cmd_fcb = Physical.pReceiveBuf->ReadBEU32();

		if (DEBUG) {
			DebugSerial.println("cmd_num: " + String(cmd_num));
			// DebugSerial.println("cmd_fcb: 0x" + String(cmd_fcb, HEX));
		}

		Physical.pSendBuf->SetSizeMax();
		Physical.pSendBuf->SetPos(2);
		byte res_num = 0;
		while (true) {
			if (cmd_num <= res_num) { break; }
			CFSCommon::CEntry* pEntry = FSMain.Folder.FilesGetNext();
			if (pEntry == null) { break; }
			byte atr = CService::GetAtr(pEntry->isFile(), pEntry->isDir());
			if (CheckMatch(FindAttr, atr, pEntry->Name)) {
				StoreEntry(atr, pEntry->LastWriteDT, pEntry->Size, pEntry->Name);
				res_num++;
			}
		}
		Physical.pSendBuf->AdjustSizePos();
		Physical.pSendBuf->SetPos(0);
		Physical.pSendBuf->WriteBEU8(GetResultCode(res_num));
		Physical.pSendBuf->WriteBEU8(res_num);
		return CService::RES_Setup;
	}
};

static CService_Files Service_Files;
