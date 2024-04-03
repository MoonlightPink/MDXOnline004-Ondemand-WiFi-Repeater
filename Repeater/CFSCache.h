#pragma once 

#include "incs.h"

class CFSCache {
private:
public:
	void DeleteDir(String path) {
		LittleFS.mkdir(path);
		File root = LittleFS.open(path);
		while (true) {
			File f = root.openNextFile();
			if (f == NULL) { break; }
			String fullpath = path + "/" + String(f.name());
			f.close();
			DebugSerial.println("Delete file in dir. fullpath:"+String(fullpath));
			LittleFS.remove(fullpath);
		}
	}
	void ClearAll() {
		DeleteDir(PDXBasePath);
		DeleteDir(NormalBasePath);
		DeleteDir(DirsDataBasePath);
		DeleteDir(FileSystemBasePath);
	}
	int GetFreeSize() {
		return LittleFS.totalBytes() - LittleFS.usedBytes();
	}
	void RequestFreeArea(int size) {
		size += 64 * 1024; // 64kbytesは残す

		if (GetFreeSize() < size) { DeleteDir(PDXBasePath); }

		if (GetFreeSize() < size) {
			DeleteDir(NormalBasePath);
			DeleteDir(DirsDataBasePath);
		}

		if (GetFreeSize() < size) { DebugSerial.println("CFSCache.RequestFreeArea(size=" + String(size) + ") error. 要求されたメモリを確保できませんでした。"); }
	}
};

static CFSCache FSCache;

static void CFSCache_RequestFreeArea(int size) { FSCache.RequestFreeArea(size); }