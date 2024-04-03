#pragma once 

#include "incs.h"

class CLocalFile {
public:
	String Filename;
	int Size;
	const u8* pFlashData;
	CLocalFile(String _Filename, int _Size, const u8* _pFlashData) {
		Filename = _Filename;
		Size = _Size;
		pFlashData = _pFlashData;
	}
};

#include "LocalFiles.h"

static void LocalFiles_Init(bool Overwrite) {
	if (!LittleFS.mkdir(LocalFiles_BasePath)) {
		Serial.println("LocalFiles: " + String(LocalFiles_BasePath) + " mkdir failed.");
		return;
	}

	if (Overwrite) { FSCache.DeleteDir(LocalFiles_BasePath); }

	for (int idx = 0; idx < LocalFilesCount; idx++) {
		CLocalFile* pLocalFile = LocalFiles[idx];
		String fn = LocalFiles_BasePath + "/" + pLocalFile->Filename;
		if (LittleFS.exists(fn)) { continue; }
		DebugSerial.println("LittleFS: Create local file. Filename=" + fn + ", Size=" + String(pLocalFile->Size));
		fs::File fp = LittleFS.open(fn, "w");

		bool err = false;

		u32 Remain = pLocalFile->Size;
		u32 pBuf = (u32)(pLocalFile->pFlashData);
		while (!err && (4 <= Remain)) {
			u32 data = pgm_read_dword_far((u32*)pBuf);
			pBuf += 4;
			if (fp.write((u8)(data >> 0)) != 1) { err = true; };
			if (fp.write((u8)(data >> 8)) != 1) { err = true; };
			if (fp.write((u8)(data >> 16)) != 1) { err = true; };
			if (fp.write((u8)(data >> 24)) != 1) { err = true; };
			Remain -= 4;
		}
		if (!err && (1 <= Remain)) {
			u32 data = pgm_read_dword_far((u32*)pBuf);
			while (!err && (1 <= Remain)) {
				if (fp.write((u8)data) != 1) { err = true; };
				data >>= 8;
				Remain--;
			}
		}
		fp.close();
		if (err) {
			DebugSerial.println("File write error.");
			return;
		}
	}

	{
		File root = LittleFS.open(LocalFiles_BasePath);
		while (true) {
			File f = root.openNextFile();
			if (f == NULL) { break; }
			DebugSerial.println("LittleFS: LocalFiles: " + String(f.name()) + "\t" + String(f.size()));
			f.close();
		}
	}
}

class CFSLocal {
private:
	int LocalFileIndex = 0;

	File root;

public:
	void FilesReset() {
		root = LittleFS.open(LocalFiles_BasePath);
	}

	CFSCommon::CEntry* FilesGetNext(void) {
		File f = root.openNextFile();
		if (f == null) { return null; }

		static CFSCommon::CEntry Entry;

		CLocalFile* pLocalFile = LocalFiles[LocalFileIndex];

		Entry.Mode = CFSCommon::CEntry::Mode_Local;

		Entry.Attr = CFSCommon::Attr_File;

		Entry.Name = f.name();

		Entry.Size = f.size();
		Entry.LastWriteDT = 0x0000;
		Entry.MD5[0] = 0x00;

		LocalFileIndex++;

		return &Entry;
	}
	CFSCommon::CEntry* FindFile(String Filename) {
		FilesReset();
		while (true) {
			CFSCommon::CEntry* Entry = FilesGetNext();
			if (Entry == null) { return null; }
			if (Entry->Attr == CFSCommon::Attr_File) {
				if (CompSJISText_IgnoreCase(Filename, Entry->Name)) { return Entry; }
			}
		}
	}

	String GetRealFilename(CFSCommon::CEntry* Entry) {
		return LocalFiles_BasePath + "/" + Entry->Name;
	}
};

