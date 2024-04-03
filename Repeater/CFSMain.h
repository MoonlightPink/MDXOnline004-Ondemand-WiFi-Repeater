#pragma once 

#include "incs.h"

class CFSMain {
private:
	static CFSLocal FSLocal;
	static CFSNetworkFolders FSNetworkFolders;
	static CFSNetworkNormal FSNetworkNormal;
	static CFSNetworkPDX FSNetworkPDX;
public:
	static bool isConnected;

	static CFSVolumeLabel VolumeLabel;

	static UInt16 FolderID_PDX;
	static bool FoilderID_isPDX(UInt16 FolderID) { return FolderID == FolderID_PDX; }

	static UInt16 GetFolderID(String FolderName) {
		DebugSerial.println("GetFolderID: FolderName=" + FolderName);
		if (isConnected) { return FSNetworkFolders.GetFolderID(FolderName); }
		return (FolderName.length() == 0) ? (UInt16)0 : CFSCommon::FolderID_Undef; // 未接続時はルートだけ有効
	}

	class CFolder {
	private:
		CFSLocal* Local = null;
		CFSNetworkNormal* Normal = null;
		CFSNetworkPDX* PDX = null;
	public:
		void Init() {
			Local = null;
			Normal = null;
			PDX = null;
		}
		void SetFolderID(ushort FolderID) {
			Init();

			if (!isConnected) {
				if (DEBUG) { DebugSerial.println("SetFolderID: Local. FID=$" + String(FolderID, HEX)); }
				Local = &FSLocal;
			} else {
				if (FoilderID_isPDX(FolderID)) {
					if (DEBUG) { DebugSerial.println("SetFolderID: NetworkPDX. FID=$" + String(FolderID, HEX)); }
					PDX = &FSNetworkPDX;
				} else {
					if (DEBUG) { DebugSerial.println("SetFolderID: NetworkNormal. FID=$" + String(FolderID, HEX)); }
					Normal = &FSNetworkNormal;
					if (!Normal->SetFolderID(FolderID)) { Normal = null; }
				}
			}

			FilesReset();
		}

		void FilesReset() {
			if (Local != null) { Local->FilesReset(); return; }
			if (Normal != null) { Normal->FilesReset(); return; }
			if (PDX != null) { PDX->FilesReset(); return; }
			DebugSerial.println("FilesReset: All null.");
			return;
		}
		CFSCommon::CEntry* FilesGetNext() {
			if (Local != null) { return Local->FilesGetNext(); }
			if (Normal != null) { return Normal->FilesGetNext(); }
			if (PDX != null) { return PDX->FilesGetNext(); }
			DebugSerial.println("FilesGetNext: All null.");
			return null;
		}
		CFSCommon::CEntry* FindFile(String Filename) {
			if (Local != null) { return Local->FindFile(Filename); }
			if (Normal != null) { return Normal->FindFile(Filename); }
			if (PDX != null) { return PDX->FindFile(Filename); }
			DebugSerial.println("FindFile: All null.");
			return null;
		}
	};
	CFolder Folder;

	String GetRealFilename(CFSCommon::CEntry* Entry) {
		switch (Entry->Mode) {
		case CFSCommon::CEntry::Mode_Local: return FSLocal.GetRealFilename(Entry);
		case CFSCommon::CEntry::Mode_Normal: return FSNetworkNormal.GetRealFilename(Entry);
		case CFSCommon::CEntry::Mode_PDX: return FSNetworkPDX.GetRealFilename(Entry);
		default:
			DebugSerial.println("GetRealFilename: 未定義ファイルモード Entry.Mode=" + String(Entry->Mode));
			return "";
		}
	}

	void Disconnect() {
		isConnected = false;

		VolumeLabel.SetName("MDXO_Disconnected");

		FolderID_PDX = CFSCommon::FolderID_Undef;

		Folder.Init();

		FSCache.ClearAll();

		DebugSerial.println("FSMain: Disconnected.");
	}

	bool Connect() { // この関数を呼ぶ前に、WiFi接続が確立していなければならない。
		if (isConnected) { return true; }

		WiFiTools.Lang = Settings.Lang;

		FSCache.ClearAll();

		FolderID_PDX = CFSCommon::FolderID_Undef;

		if (!WiFiTools.GetFolders(FoldersENGFilename)) { Disconnect(); return false; }
		FSNetworkFolders.Init();

		if (!WiFiTools.GetPDXFiles(PDXFilesFilename)) { Disconnect(); return false; }
		FSNetworkPDX.Init();

		Folder.Init();

		DebugSerial.println("FSMain: Connected.");

		VolumeLabel.SetName("MDXO_Connected");

		isConnected = true;

		return true;
	}
};

CFSLocal CFSMain::FSLocal;
CFSNetworkFolders CFSMain::FSNetworkFolders;
CFSNetworkNormal CFSMain::FSNetworkNormal;
CFSNetworkPDX CFSMain::FSNetworkPDX;

bool CFSMain::isConnected;

CFSVolumeLabel CFSMain::VolumeLabel;

UInt16 CFSMain::FolderID_PDX;

static CFSMain FSMain;

static void CFSMain_VolumeLabel_SetName(String text) { CFSMain::VolumeLabel.SetName(text); }
static void CFSMain_FolderID_PDX_Set(u16 FolderID) { CFSMain::FolderID_PDX = FolderID; }
static void CFSMain_Disconnect() { FSMain.Disconnect(); }
