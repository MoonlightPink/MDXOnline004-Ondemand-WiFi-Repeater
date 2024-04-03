#pragma once 

#include "incs.h"

class CWiFiTools {
private:
public:
	String Lang = "ENG";

	String BaseURI = "/MDXOnline004_X68000";

	bool HTTPGet_ins(String Filename, String Query) {
		if (LittleFS.exists(Filename)) { return true; }

		if (!CWiFiSTA::isConnect()) {
			OLED_DrawProgress("WiFi Reconnecting...");
			while (!CWiFiSTA::isConnect()) {
				yield();
				if (PressedInterruptButton()) { return false; }
			}
		}

		OLED_DrawProgress("HTTP Connect");

		WiFiClient client;
		HTTPClient http;
		http.setReuse(false);

		String URI = BaseURI + "?" + Query;
		DebugSerial.println("CWiFiTools.HTTPGet: URI=" + URI);

		if (!http.begin(client, "192.168.1.37", 12290, URI, false)) {
			DebugSerial.println("CWiFiTools.HTTPGet error. [begin false] URI=" + URI);
			return false;
		}

		int httprescode = http.GET();
		if (httprescode < 0) {
			DebugSerial.println("CWiFiTools.HTTPGet error. [httprescode<0] URI=" + URI + ", httprescode=" + http.errorToString(httprescode));
			return false;
		}

		int bodysize = http.getSize();
		if (bodysize < 0) {
			DebugSerial.println("CWiFiTools.HTTPGet error. [Invalid body size] URI=" + URI + ", bodysize=" + String(bodysize));
			return false;
		}

		CFSCache_RequestFreeArea(bodysize);
		fs::File fp = LittleFS.open(Filename, "w");

		WiFiClient& ws = http.getStream();

		static const int bufsize = 1 * 1024;
		static byte buf[bufsize];

		bool err = false;
		int Remain = bodysize;
		int OLED_LastValue = -1;
		while (1 <= Remain) {
			if (PressedInterruptButton()) {
				err = true;
				break;
			}
			int size = Remain;
			if (bufsize < size) { size = bufsize; }
			size = ws.read(buf, size);
			if (size < bufsize) {
				int OLED_Value = bodysize - Remain;
				if (OLED_LastValue != OLED_Value) {
					OLED_LastValue = OLED_Value;
					OLED_DrawProgress("Downloading...", "bytes.", OLED_Value, bodysize);
				}
			}
			if (size == 0) {
				delay(1);
				continue;
			}
			if (size < 0) {
				OLED_DrawProgress("HTTP download error.");
				DebugSerial.println("CWiFiTools.HTTPGet error. [Stream readed size under zero] URI=" + URI + ", size=" + String(size));
				err = true;
				break;
			}
			DebugSerial.println("Remain=" + String(Remain) + "\tbodysize=" + String(bodysize) + "\tsize=" + String(size));
			if (fp.write(buf, size) != size) {
				OLED_DrawProgress("LittleFS write error.");
				DebugSerial.println("CWiFiTools.HTTPGet error. [LittleFS write error] URI=" + URI + ", Filename=" + Filename);
				err = true;
				break;
			}
			Remain -= size;
		}

		fp.close();

		http.end();

		if (err) {
			LittleFS.remove(Filename);
			return false;
		}

		OLED_DrawProgress();

		return true;
	}

	bool HTTPGet(String Filename, String Query) {
		for (int loop = 0; loop < 10; loop++) {
			if (HTTPGet_ins(Filename, Query)) { return true; }
			delay(1000);
		}
		return false;
	}

	bool GetFolders(String Filename) { return HTTPGet(Filename, "Command=GetFolders&Lang=" + Lang); }
	bool GetPDXFiles(String Filename) { return HTTPGet(Filename, "Command=GetPDXFiles"); }
	bool GetDirsData(u16 ID, String Filename) { return HTTPGet(Filename, "Command=GetDirsData&ID=" + u16ToString(ID) + "&Lang=" + Lang); }
	bool GetNormalFile(String ID, String Filename) { return HTTPGet(Filename, "Command=GetNormalFile&ID=" + ID); }
	bool GetPDXFile(String ID, String Filename) { return HTTPGet(Filename, "Command=GetPDXFile&ID=" + ID); }
};

static CWiFiTools WiFiTools;
