#pragma once 

#include "incs.h"

class CService_Body {
private:
	class CFileHandle {
	private:
	public:
		UInt32 fcb = 0;
		u32 FileSize = 0;
		u32 FilePos = 0;
		UInt32 LastWriteDT = 0;
		CBuffer* pBuffer = null;
		fs::File fp;

		bool Open(UInt32 _fcb, String path) {
			if (Command.Exec(path)) {
				pBuffer = &Command.Buffer;
				pBuffer->SetPos(0);

				FileSize = pBuffer->GetSize();
				FilePos = 0;
				LastWriteDT = 0x0000;

				fcb = _fcb;
				return true;
			}

			int SlashPos = -1;
			for (int idx = 0; idx < path.length(); idx++) {
				if (path[idx] == '/') { SlashPos = idx; }
			}
			if (SlashPos == -1) {
				DebugSerial.println("フルパスではなかった。");
				return false;
			}
			UInt16 FolderID = CFSMain::GetFolderID(path.substring(0, SlashPos));
			if (FolderID == CFSCommon::FolderID_Undef) {
				DebugSerial.println("FolderIDが見つからない。 " + path.substring(0, SlashPos));
				return false;
			}
			FSMain.Folder.SetFolderID(FolderID);
			CFSCommon::CEntry* Entry = FSMain.Folder.FindFile(path.substring(SlashPos + 1));
			if (Entry == null) {
				DebugSerial.println("FileEntryが見つからない。 " + path.substring(SlashPos + 1));
				return false;
			}

			String RealFilename = FSMain.GetRealFilename(Entry);
			DebugSerial.println("RealFilename=" + RealFilename);
			if (RealFilename.equals("") || !LittleFS.exists(RealFilename)) {
				DebugSerial.println("RealFileが見つからない。 " + RealFilename);
				return false;
			}

			pBuffer = null;
			fp = LittleFS.open(RealFilename, "r");

			FileSize = Entry->Size;
			FilePos = 0;
			LastWriteDT = Entry->LastWriteDT;

			fcb = _fcb;

			return true;
		}
		void Close() {
			if (fcb != 0) {
				fcb = 0;
				fp.close();
			}
		}
		int GetRemain() {
			if (fcb != 0) {
				return FileSize - FilePos;
			}
			return 0;
		}
		u8 ReadU8() {
			if (fcb != 0) {
				FilePos++;
				if (pBuffer != null) {
					return pBuffer->ReadU8();
				} else {
					return fp.read();
				}
			}
			return 0;
		}
		void ReadBuf(byte* buf, int size) {
			if (fcb != 0) {
				if (pBuffer != null) {
					pBuffer->ReadBuf(buf, size);
				} else {
					fp.read(buf, size);
				}
				FilePos += size;
			}
		}
		void SetPos(int pos) {
			if (fcb != 0) {
				FilePos = pos;
				if (pBuffer != null) {
					pBuffer->SetPos(FilePos);
				}
			}
		}
		int GetPos() {
			if (fcb != 0) {
				return FilePos;
			}
			return 0;
		}
	};

	static const int FileHandlesCount = 16;
	CFileHandle FileHandles[FileHandlesCount];

	bool FileHandles_isOpened(UInt32 fcb) {
		for (int idx = 0; idx < FileHandlesCount; idx++) {
			if (FileHandles[idx].fcb == fcb) { return true; }
		}
		return false;
	}

	int FileHandles_GetEmptyIndex(UInt32 fcb) {
		for (int idx = 0; idx < FileHandlesCount; idx++) {
			if (FileHandles[idx].fcb == 0) { return idx; }
		}
		return -1;
	}

	bool FileHandles_Open(UInt32 fcb, String path) {
		int idx = FileHandles_GetEmptyIndex(fcb);
		if (idx == -1) {
			DebugSerial.println("空きハンドルが無い。");
			return false;
		}
		if (!FileHandles[idx].Open(fcb, path)) { return false; }
		return true;
	}

	CFileHandle* FileHandles_Get(UInt32 fcb) {
		for (int idx = 0; idx < FileHandlesCount; idx++) {
			if (FileHandles[idx].fcb == fcb) { return &FileHandles[idx]; }
		}
		return null;
	}

	bool FileHandles_Close(UInt32 fcb) {
		CFileHandle* fh = FileHandles_Get(fcb);
		if (fh == null) {
			DebugSerial.println("開いているハンドルが無い。");
			return false;
		}
		fh->Close();
		return true;
	}

public:
	byte op_open(u8 ID) {
		u8 cmd_mode = Physical.pReceiveBuf->ReadBEU8();
		u32 cmd_fcb = Physical.pReceiveBuf->ReadBEU32();
		CService::Cdos_namebuf* pcmd_path = new CService::Cdos_namebuf(Physical.pReceiveBuf);

		DebugSerial.println("cmd_mode: 0x" + String(cmd_mode, HEX));
		// DebugSerial.println("cmd_fcb: 0x" + String(cmd_fcb, HEX));

		String path = pcmd_path->conv_namebufToSJISText(true);
		DebugSerial.println("path: " + path);

		if (FileHandles_isOpened(cmd_fcb)) {
			DebugSerial.println("op_open: 既に開いているファイルポインタで開こうとしたけど、このエラーは無視して、古いFileHandleを閉じる。");
			FileHandles_Close(cmd_fcb);
		}

		switch (cmd_mode) {
		case 0: { // Read
			String fn = pcmd_path->conv_namebufToSJISText(true);
			if (!FileHandles_Open(cmd_fcb, fn)) { return CService::_DOSE_ILGARG; }

			CFileHandle* fh = FileHandles_Get(cmd_fcb);
			DebugSerial.println("op_open: " + String(fn) + "," + String(fh->FileSize));

			Physical.pSendBuf->SetSizeMax();
			Physical.pSendBuf->SetPos(0);
			Physical.pSendBuf->WriteBEU8(CService::RES_OK);
			Physical.pSendBuf->WriteBEU32(fh->FileSize);
			Physical.pSendBuf->AdjustSizePos();
			return CService::RES_Setup;
		}
		case 1: break; // Write
		case 2: break; // Read/Write
		default: break; // Error
		}

		return CService::_DOSE_ILGARG;
	}

	byte op_close(u8 ID) {
		u32 cmd_fcb = Physical.pReceiveBuf->ReadBEU32();
		// DebugSerial.println("cmd_fcb: 0x" + String(cmd_fcb, HEX));

		CFileHandle* fh = FileHandles_Get(cmd_fcb);

		if (fh == null) {
			DebugSerial.println("op_close: 開いていないファイルポインタを閉じようとした。");
		} else {
			fh->Close();
			return CService::RES_OK;
		}

		return CService::_DOSE_ILGARG;
	}

	byte op_read(u8 ID) {
		u32 cmd_fcb = Physical.pReceiveBuf->ReadBEU32();
		u32 cmd_pos = Physical.pReceiveBuf->ReadBEU32();
		u16 cmd_len = Physical.pReceiveBuf->ReadBEU16();

		// DebugSerial.println("cmd_fcb: 0x" + String(cmd_fcb, HEX));

		CFileHandle* fh = FileHandles_Get(cmd_fcb);

		if (fh == null) {
			DebugSerial.println("op_close: 開いていないファイルポインタで読み込もうとした。");
		} else {
			fh->SetPos(cmd_pos);
			if (fh->GetRemain() < cmd_len) { cmd_len = fh->GetRemain(); }
			DebugSerial.println("FileRead: Pos=" + String(fh->FilePos) + ",Len=" + String(cmd_len) + ",Remain=" + String(fh->GetRemain()) + ",Size=" + String(fh->FileSize));

			Physical.pSendBuf->SetSizeMax();
			Physical.pSendBuf->SetPos(0);
			Physical.pSendBuf->WriteBEU16(cmd_len);

			{
				byte* pSendRaw = Physical.pSendBuf->GetRaw();
				int pos = Physical.pSendBuf->GetPos();
				pSendRaw += pos;
				fh->ReadBuf(pSendRaw, cmd_len);
				Physical.pSendBuf->SetPos(pos + cmd_len);
			}

			Service_SetOLEDProgress("Send file body.","bytes.", fh->FilePos, fh->FileSize);

			Physical.pSendBuf->AdjustSizePos();
			return CService::RES_Setup;
		}

		return CService::_DOSE_ILGARG;

	}

	byte op_filedate(u8 ID) {
		u32 cmd_fcb = Physical.pReceiveBuf->ReadBEU32();
		u32 cmd_timedate = Physical.pReceiveBuf->ReadBEU32();

		CFileHandle* fh = FileHandles_Get(cmd_fcb);

		if (fh == null) {
			DebugSerial.println("op_close: 開いていないファイルポインタで読み込もうとした。");
		} else {

		}

		u32 res = 0xffffff00 | CService::_DOSE_ILGARG;

		if (fh == null) {
			DebugSerial.println("op_close: 開いていないファイルポインタの日時を操作しようとした。");
		} else {
			if (cmd_timedate == 0) {   // 更新日時取得
				res = fh->LastWriteDT;
			} else { // 更新日時設定
				res = 0xffffff00 | CService::_DOSE_RDONLY;
			}
		}

		Physical.pSendBuf->SetSizeMax();
		Physical.pSendBuf->SetPos(0);
		Physical.pSendBuf->WriteBEU32(res);
		Physical.pSendBuf->AdjustSizePos();
		return CService::RES_Setup;
	}
};

static CService_Body Service_Body;
