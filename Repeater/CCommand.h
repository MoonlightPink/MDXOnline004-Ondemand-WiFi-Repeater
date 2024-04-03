#pragma once 

#include "incs.h"

static void GetWiFiStatusText(CBuffer& Buffer) {
	Buffer.WriteString("SystemUpTime=" + String(millis()) + "\r\n");
	Buffer.WriteString("HardwareChip=" + String(ESP.getChipModel()) + " (rev." + String(ESP.getChipRevision()) + ")\r\n");
	Buffer.WriteString("HardwareCores=" + String(ESP.getChipCores()) + "\r\n");
	u32 ChipID = 0;
	for (int i = 0; i < 17; i = i + 8) {
		ChipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
	char tmp[64];
	sprintf(tmp, "%06x", ChipID);
	Buffer.WriteString("HardwareID=" + String(tmp) + "\r\n");
	Buffer.WriteString("CPUClock=" + String(F_CPU) + "\r\n");
	Buffer.WriteString("HeapMemoryFreeSize=" + String(ESP.getFreeHeap()) + "\r\n");
	Buffer.WriteString("CFSCacheFreeSize=" + String(FSCache.GetFreeSize()) + "\r\n");
	Buffer.WriteString("AppName=MDXOnline004 Ondemand WiFi Repeater\r\n");
	Buffer.WriteString("AppDateTime=" __DATE__ " " __TIME__ "\r\n");
	Buffer.WriteString("Lang=" + Settings.Lang + "\r\n");
	Buffer.WriteString("SSID(WiFi)=" + String(Settings.SSID.equals("") ? "Unprepared" : "Prepared") + "\r\n");
	Buffer.WriteString("Pass(WiFi)=" + String(Settings.Pass.equals("") ? "Unprepared" : "Prepared") + "\r\n");
	Buffer.WriteString("WiFiStatus=" + CWiFiSTA::GetStatusStr() + "\r\n");
	Buffer.WriteString("NetworkFileSystem=" + String(FSMain.isConnected ? "Prepared" : "Unprepared") + "\r\n");
}

class CCommand {
private:
	static const int ResBufSize = 1 * 1024;
	byte ResBuf[ResBufSize];
public:
	CBuffer Buffer;

	bool Exec(String path) {
		if (!path.startsWith("/CMD/")) { return false; }
		path = path.substring(5);
		DebugSerial.println("path=" + path);

		int SlashPos = path.indexOf('/');
		String Command, Param;
		if (SlashPos == -1) {
			Command = path;
			Param = "";
		} else {
			Command = path.substring(0, SlashPos);
			path = path.substring(SlashPos + 1);
			Param = "";
			for (int idx = 0; idx < path.length(); idx++) {
				char ch = path[idx];
				if (ch != '/') { Param += ch; }
			}
		}

		DebugSerial.println("Command=" + Command + ", Param=" + Param);

		Buffer.ResetBuffer(ResBuf, ResBufSize);
		Buffer.SetPos(0);

		if (Command.equals("SetLang")) { CWiFiSTA::Disconnect(); Buffer.WriteU8(Settings.Exec_SetLang(Buffer, Param)); }
		if (Command.equals("SetSSID")) { CWiFiSTA::Disconnect(); Buffer.WriteU8(Settings.Exec_SetSSID(Buffer, Param)); }
		if (Command.equals("SetPass")) { CWiFiSTA::Disconnect(); Buffer.WriteU8(Settings.Exec_SetPass(Buffer, Param)); }

		if (Command.equals("Disconnect")) { CWiFiSTA::Disconnect(); Buffer.WriteU8(CMD_RES_OK); }

		if (Command.equals("Connect")) {
			OLED_DrawDialog("WiFi connecting...", "", "", "Can interrupt");
			if (!Settings.isSetupComplete()) {
				CWiFiSTA::Disconnect();
				Buffer.WriteU8(CMD_RES_Error_SetupNotComplete);
			} else {
				if (!CWiFiSTA::Connect()) {
					CWiFiSTA::Disconnect();
					Buffer.WriteU8(CMD_RES_Error_SetupNotComplete);
				} else {
					bool interrupted = false;
					while (!CWiFiSTA::isConnect()) {
						if (PressedInterruptButton()) {
							CWiFiSTA::Disconnect();
							OLED_DrawDialog("WiFi connect.", "", "", "Aborted.");
							interrupted = true;
							break;
						}
						delay(1);
						CWiFiSTA::Interval();
					}
					if (interrupted) {
						Buffer.WriteU8(CMD_RES_Error_NetworkError);
					} else {
						if (!FSMain.Connect()) {
							Buffer.WriteU8(CMD_RES_Error_NetworkError);
						} else {
							Buffer.WriteU8(CMD_RES_OK);
						}
					}
				}
			}
		}

		if (Command.equals("GetWiFiStatusText")) { GetWiFiStatusText(Buffer); }

		if (Buffer.GetPos() == 0) { Buffer.WriteU8(CMD_RES_Error_UnknownCommand); }

		Buffer.AdjustSizePos();

		Buffer.SetPos(0);
		DebugSerial.println("CMDRES: $" + String(Buffer.ReadU8(), HEX) + " Size:" + Buffer.GetSize());

		return true;
	}


};

static CCommand Command;
