#pragma once 

#include "incs.h"

class CSettings {
private:
	const String BasePath = "/Settings";

	const String LangFilename = "Lang.txt";
	const String SSIDFilename = "SSID.txt";
	const String PassFilename = "Pass.txt";

	bool WriteFile(String fn, String Data) {
		fn = BasePath + "/" + fn;
		fs::File fp = LittleFS.open(fn, "w");
		bool err = false;
		for (int idx = 0; idx < Data.length(); idx++) {
			if (fp.write(Data[idx]) != 1) {
				err = true;
				break;
			};
		}
		fp.close();
		if (err) {
			DebugSerial.println("Settings.WriteFile: ファイルの書き込みに失敗しました。 fn=" + fn);
			return false;
		}
		return true;
	}
	String ReadFile(String fn) {
		fn = BasePath + "/" + fn;
		if (!LittleFS.exists(fn)) { return ""; }
		fs::File fp = LittleFS.open(fn, "r");
		String res = "";
		while (true) {
			int ch = fp.read();
			if (ch == -1) { break; }
			res += (char)ch;
		}
		fp.close();
		return res;
	}
public:
	String Lang = "", SSID = "", Pass = "";

	void Init() {
		if (!LittleFS.mkdir(BasePath)) {
			Serial.println("CSettings: " + String(BasePath) + " mkdir failed.");
			return;
		}

		if (ReadFile(LangFilename).equals("")) { WriteFile(LangFilename, "ENG"); }

		Lang = ReadFile(LangFilename);
		SSID = ReadFile(SSIDFilename);
		Pass = ReadFile(PassFilename);

		DebugSerial.println("Settings: Lang=" + Lang);
		DebugSerial.println("Settings: SSID=" + String(SSID.equals("") ? "Unprepared" : "Prepared"));
		DebugSerial.println("Settings: Pass=" + String(Pass.equals("") ? "Unprepared" : "Prepared"));
	}

	u8 Exec_SetLang(CBuffer Buffer, String Param) {
		DebugSerial.println("Settings.Exec_SetLang: Param=" + Param);

		bool err = true;
		if (Param.equals("ENG")) { err = false; }
		if (Param.equals("JPN")) { err = false; }
		if (err) { return CMD_RES_Error_IlligalParam; }

		if (!WriteFile(LangFilename, Param)) {
			return CMD_RES_Error_File;
		} else {
			Lang = Param;
			return CMD_RES_OK;
		}
	}

	u8 Exec_SetSSID(CBuffer Buffer, String Param) {
		DebugSerial.println("Settings.Exec_SetSSID: Param=" + Param);

		bool err = Param.equals("");
		if (err) { return CMD_RES_Error_IlligalParam; }

		if (!WriteFile(SSIDFilename, Param)) {
			return CMD_RES_Error_File;
		} else {
			SSID = Param;
			return CMD_RES_OK;
		}
	}

	u8 Exec_SetPass(CBuffer Buffer, String Param) {
		DebugSerial.println("Settings.Exec_SetPass: Param=" + Param);

		bool err = Param.equals("");
		if (err) { return CMD_RES_Error_IlligalParam; }

		if (!WriteFile(PassFilename, Param)) {
			return CMD_RES_Error_File;
		} else {
			Pass = Param;
			return CMD_RES_OK;
		}
	}

	bool isSetupComplete() {
		bool res = true;
		if (Lang.equals("")) { res = false; }
		if (SSID.equals("")) { res = false; }
		if (Pass.equals("")) { res = false; }
		return res;
	}
};

static CSettings Settings;
