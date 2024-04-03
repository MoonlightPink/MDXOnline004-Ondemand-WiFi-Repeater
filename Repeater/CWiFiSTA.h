#pragma once 

#include "incs.h"

class CWiFiSTA {
private:
public:
	static void Init() {
		WiFi.disconnect(true, true);
	}

	static void Disconnect() {
		CFSMain_Disconnect();
		WiFi.disconnect(true, true);
	}

	static bool Connect() {
		WiFi.disconnect(true, true);
		if (!Settings.isSetupComplete()) { return false; }
		WiFi.mode(WIFI_STA);
		WiFi.begin(Settings.SSID, Settings.Pass);
		return true;
	}

	static void Interval() {
		static wl_status_t LastWiFiStatus = WL_NO_SHIELD;
		wl_status_t WiFiStatus = WiFi.status();

		if (LastWiFiStatus != WiFiStatus) {
			LastWiFiStatus = WiFiStatus;
			DebugSerial.println("WiFiStatus changed. " +GetWifiStatus(WiFiStatus));
		}
	}

	static String GetWifiStatus(wl_status_t wifiStatus) {
		switch (wifiStatus) {
		case WL_CONNECTED: return("Connected to a WiFi network."); break;
		case WL_NO_SHIELD: return("No WiFi shield is present."); break;
		case WL_IDLE_STATUS: return("Idle."); break;
		case WL_NO_SSID_AVAIL: return("No SSID are available."); break;
		case WL_SCAN_COMPLETED: return("The scan networks is completed."); break;
		case WL_CONNECT_FAILED: return("The connection fails for all the attempts."); break;
		case WL_CONNECTION_LOST: return("The connection is lost."); break;
			// case 6: return("The password is incorrect."); break; // WL_CONNECT_WRONG_PASSWORD
		case WL_DISCONNECTED: return("Disconnected from a network."); break;
		default: return("Unknown WiFiStatus. " + String(wifiStatus)); break;
		}
	}

	static bool isConnect() {
		return WiFi.status() == WL_CONNECTED;
	}

	static String GetStatusStr() {
		return GetWifiStatus(WiFi.status());
	}
};

