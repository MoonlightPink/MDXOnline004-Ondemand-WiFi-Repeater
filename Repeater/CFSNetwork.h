#pragma once

#include "incs.h"

class CFSNetworkFolders {
private:
	bool fpopened = false;
	fs::File fp;

	u8 ReadU8() {
		u8 res;
		fp.read(&res, 1);
		return res;
	}
	u16 ReadU16() {
		u16 res;
		fp.read((u8*)&res, 2);
		return res;
	}
	u32 ReadU24() {
		u32 res = 0;
		fp.read((u8*)&res, 3);
		return res;
	}
	u32 ReadU32() {
		u32 res;
		fp.read((u8*)&res, 4);
		return res;
	}

	int RootOffset;
public:
	void Init() {
		fp = LittleFS.open(FoldersENGFilename, "r");

		{
			u8 len = ReadU8();
			String Name = "";
			for (int idx = 0; idx < len; idx++) {
				Name += String((char)ReadU8());
			}
			CFSMain_VolumeLabel_SetName(Name);
		}

		RootOffset = fp.position();

		u16 FolderID = GetFolderID("/PDX");
		CFSMain_FolderID_PDX_Set(FolderID);
		if (FolderID == CFSCommon::FolderID_Undef) { DebugSerial.println("PDXフォルダが見つかりません。"); }
	}

	UInt16 GetFolderID(String FolderName) {
		if (DEBUG) { DebugSerial.println("FolderName: [" + FolderName + "]"); }

		fp.seek(RootOffset, SeekSet);

		if (FolderName.equals("")) { return 0; } // Root

		int FolderNamePos = 0;
		if (FolderName[FolderNamePos++] != '/') {
			DebugSerial.println("GetFolderID: 先頭が[/]以外 " + FolderName);
			return CFSCommon::FolderID_Undef;
		}


		while (true) {
			String CurrentName = "";
			while (true) {
				char ch = FolderName[FolderNamePos++];
				if (ch == 0x00) { break; }
				if (ch == '/') { break; }
				CurrentName += String(ch);
				if (0x81 <= ch && ch < 0xa0 || 0xe0 <= ch && ch < 0xf0) {
					ch = FolderName[FolderNamePos++];
					CurrentName += String(ch);
				}
			}

			if (DEBUG) { DebugSerial.println("Find dir name: [" + CurrentName + "]"); }

			while (true) {
				u8 Attr = ReadU8();
				u8 FoundNameLen = Attr & 0x1f;
				Attr &= CFSCommon::Attr_Mask;
				switch (Attr) {
				case CFSCommon::Attr_Term: return CFSCommon::FolderID_Undef;
				case CFSCommon::Attr_Dir: break;
				case CFSCommon::Attr_File: DebugSerial.println("GetFolderID: Attr is Attr_File. 0x" + String(Attr, HEX)); return CFSCommon::FolderID_Undef;
				case CFSCommon::Attr_Reserve: DebugSerial.println("GetFolderID: Attr is Attr_Reserve. 0x" + String(Attr, HEX)); return CFSCommon::FolderID_Undef;
				default: DebugSerial.println("GetFolderID: Attr is undef. 0x" + String(Attr, HEX)); return CFSCommon::FolderID_Undef;
				}

				String FoundName;
				for (int idx = 0; idx < FoundNameLen; idx++) {
					FoundName += String((char)ReadU8());
				}
				u16 FolderID = ReadU16();
				u16 FolderPos = ReadU16();

				if (DEBUG) { DebugSerial.println("Found dir name: [" + FoundName + "]"); }

				if (CompSJISText_IgnoreCase(CurrentName, FoundName)) {
					if (FolderName.length() <= FolderNamePos) {
						if (DEBUG) { DebugSerial.println("Found dir. ID=0x" + String(FolderID, HEX)); }
						return FolderID;
					}
					fp.seek(FolderPos, SeekSet);
					break;
				}
			}
		}
	}
};

class CFSNetworkNormal {
private:
	fs::File fp;

	u8 ReadU8() {
		u8 res;
		fp.read(&res, 1);
		return res;
	}
	u16 ReadU16() {
		u16 res;
		fp.read((u8*)&res, 2);
		return res;
	}
	u32 ReadU24() {
		u32 res = 0;
		fp.read((u8*)&res, 3);
		return res;
	}
	u32 ReadU32() {
		u32 res;
		fp.read((u8*)&res, 4);
		return res;
	}

	bool isEOF = false;
public:
	bool SetFolderID(UInt16 FolderID) {
		String fn = "";
		fn += DirsDataBasePath + DirsDataPrefix;
		fn += u16ToString(FolderID);
		fn += DirsDataENGSuffix;
		DebugSerial.println("Folder fn=" + fn);
		if (!WiFiTools.GetDirsData(FolderID, fn)) { return false; }
		fp = LittleFS.open(fn, "r");
		isEOF = false;
		return true;
	}
	void FilesReset() {
		fp.seek(0, SeekSet);
		isEOF = false;
	}
	CFSCommon::CEntry* FilesGetNext() {
		if (isEOF) { return null; }

		static CFSCommon::CEntry Entry;
		Entry.Mode = CFSCommon::CEntry::Mode_Normal;
		Entry.Attr = ReadU8();
		u8 NameLen = (byte)(Entry.Attr & 0x1f);
		Entry.Attr &= CFSCommon::Attr_Mask;
		switch (Entry.Attr) {
		case CFSCommon::Attr_Term:
			isEOF = true;
			return null;
		case CFSCommon::Attr_Dir:
		case CFSCommon::Attr_File:
			Entry.Name = "";
			for (int idx = 0; idx < NameLen; idx++) {
				Entry.Name += String((char)ReadU8());
			}
			if (Entry.Attr == CFSCommon::Attr_Dir) {
				Entry.Size = 0;
				Entry.LastWriteDT = 0;
				for (int idx = 0; idx < CFSCommon::MD5Len; idx++) {
					Entry.MD5[idx] = 0x00;
				}
				if (DEBUG) { DebugSerial.println("Found dir. Name:" + Entry.Name); }
			} else {
				Entry.Size = ReadU24();
				Entry.LastWriteDT = ReadU32();
				for (int idx = 0; idx < CFSCommon::MD5Len; idx++) {
					Entry.MD5[idx] = ReadU8();
				}
				if (DEBUG) { DebugSerial.println("Found file. Name:" + Entry.Name); }
			}
			return &Entry;
		case CFSCommon::Attr_Reserve: DebugSerial.println("FilesGetNext: Attr is Attr_Reserve. 0x" + String(Entry.Attr, HEX)); return null;
		default: DebugSerial.println("FilesGetNext: Attr is undef. 0x" + String(Entry.Attr, HEX)); return null;
		}
	}
	CFSCommon::CEntry* FindFile(String Filename) {
		FilesReset();
		while (true) {
			CFSCommon::CEntry* Entry = FilesGetNext();
			if (Entry == null) { isEOF = true; return null; }
			if (Entry->Attr == CFSCommon::Attr_File) {
				if (CompSJISText_IgnoreCase(Filename, Entry->Name)) {
					isEOF = true;
					return Entry;
				}
			}
		}
	}
	String GetRealFilename(CFSCommon::CEntry* Entry) {
		String ID = BufToString(Entry->MD5, NormalFilenameLen / 2);
		String fn = NormalBasePath + "/" + ID;
		if (!WiFiTools.GetNormalFile(ID, fn)) { return ""; }
		return fn;
	}
};

class CFSNetworkPDX {
private:
	fs::File fp;

	u8 ReadU8() {
		u8 res;
		fp.read(&res, 1);
		return res;
	}
	u16 ReadU16() {
		u16 res;
		fp.read((u8*)&res, 2);
		return res;
	}
	u32 ReadU32() {
		u32 res;
		fp.read((u8*)&res, 4);
		return res;
	}

	bool isEOF = false;
public:
	void Init() {
		fp = LittleFS.open(PDXFilesFilename, "r");
	}
	void FilesReset() {
		fp.seek(0, SeekSet);
		isEOF = false;
	}
	CFSCommon::CEntry* FilesGetNext() {
		if (isEOF) { return null; }

		static CFSCommon::CEntry Entry;

		Entry.Mode = CFSCommon::CEntry::Mode_PDX;
		Entry.Attr = CFSCommon::Attr_File;

		Entry.Size = ReadU8();
		if (Entry.Size == 0x00) {
			isEOF = true;
			return null;
		}

		Entry.Size = (Entry.Size & 0x7f) << 16;
		Entry.Size |= (u32)ReadU8() << 8;
		Entry.Size |= (u32)ReadU8() << 0;

		{
			char namebuf[PDXFilenameLen + 4 + 1];
			char hexs[0x10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
			for (int idx = 0; idx < PDXFilenameLen / 2; idx++) {
				u8 data = ReadU8();
				namebuf[idx * 2 + 0] = (byte)hexs[data >> 4];
				namebuf[idx * 2 + 1] = (byte)hexs[data & 0x0f];
				Entry.MD5[idx] = data;
			}
			for (int idx = 4; idx < CFSCommon::MD5Len; idx++) {
				Entry.MD5[idx] = 0x00;
			}
			namebuf[8 + 0] = (byte)'.';
			namebuf[8 + 1] = (byte)'p';
			namebuf[8 + 2] = (byte)'d';
			namebuf[8 + 3] = (byte)'x';
			namebuf[8 + 4 + 0] = 0x00;
			Entry.Name = String(namebuf);
		}

		if (DEBUG) { DebugSerial.println("PDX found: " + Entry.Name); }

		Entry.LastWriteDT = ReadU32();
		return &Entry;
	}
	CFSCommon::CEntry* FindFile(String Filename) {
		if (Filename.length() != 8 + 4) { return null; }
		if (Filename[8 + 0] != (byte)'.' || Filename[8 + 1] != (byte)'p' || Filename[8 + 2] != (byte)'d' || Filename[8 + 3] != (byte)'x') { return null; }

		DebugSerial.println("PDX find: " + Filename);

		const int FilenameBytesSize = PDXFilenameLen / 2;
		byte FilenameBytes[FilenameBytesSize];

		String HexStr = "0123456789abcdef";
		for (int idx = 0; idx < FilenameBytesSize; idx++) {
			byte data = 0x00;
			{
				int pos = HexStr.indexOf((char)Filename[idx * 2 + 0]);
				if (pos == -1) { DebugSerial.println("PDX.FindFile: PDXファイル名がHEXではありません。 " + Filename); return null; }
				data |= (byte)(pos << 4);
			}
			{
				int pos = HexStr.indexOf((char)Filename[idx * 2 + 1]);
				if (pos == -1) { DebugSerial.println("PDX.FindFile: PDXファイル名がHEXではありません。 " + Filename); return null; }
				data |= (byte)pos;
			}
			FilenameBytes[idx] = data;
		}

		FilesReset();
		while (true) {
			u32 Size = ReadU8();
			if (Size == 0x00) {
				isEOF = true;
				return null;
			}

			Size = (u32)(Size & 0x7f) << 16;
			Size |= (u32)ReadU8() << 8;
			Size |= (u32)ReadU8() << 0;

			bool chk = true;
			for (int idx = 0; idx < FilenameBytesSize; idx++) {
				if (FilenameBytes[idx] != ReadU8()) { chk = false; }
			}
			u32 LastWriteDT = ReadU32();

			if (!chk) { continue; }

			static CFSCommon::CEntry Entry;
			Entry.Mode = CFSCommon::CEntry::Mode_PDX;
			Entry.Attr = CFSCommon::Attr_File;

			Entry.Size = Size;

			Entry.Name = Filename;

			Entry.LastWriteDT = LastWriteDT;

			isEOF = true;
			return &Entry;
		}
	}

	String GetRealFilename(CFSCommon::CEntry* Entry) {
		String ID = Entry->Name;
		String fn = PDXBasePath + "/" + ID;
		ID = ID.substring(0, PDXFilenameLen);
		if (!WiFiTools.GetPDXFile(ID, fn)) { return ""; }
		return fn;
	}
};

