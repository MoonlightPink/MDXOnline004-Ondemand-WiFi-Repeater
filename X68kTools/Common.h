
#define null (0)

typedef unsigned char u8;

static const u8 CMD_RES_OK = 0x00;
static const u8 CMD_RES_Error_UnknownCommand = 0x01;
static const u8 CMD_RES_Error_File = 0x02;
static const u8 CMD_RES_Error_IlligalParam = 0x03;
static const u8 CMD_RES_Error_SetupNotComplete = 0x04;
static const u8 CMD_RES_Error_NetworkError = 0x05;

static const u8 CMD_RES_Error_ClientFile = 0xff; // Setup.xやStatus.xでファイルが開けなかったときに返る。

static void ShowErrorMsg(u8 CMD_RES, bool ShowHeader = true) {
	const char* pmsg = null;

	switch (CMD_RES) {
	case CMD_RES_OK: pmsg = "CMD_RES_OK"; break;
	case CMD_RES_Error_UnknownCommand: pmsg = "CMD_RES_Error_UnknownCommand 未定義コマンド"; break;
	case CMD_RES_Error_File: pmsg = "CMD_RES_Error_File LittleFSファイルシステムエラー"; break;
	case CMD_RES_Error_IlligalParam: pmsg = "CMD_RES_Error_IlligalParam パラメータが異常です。"; break;
	case CMD_RES_Error_SetupNotComplete: pmsg = "CMD_RES_Error_SetupNotComplete セットアップが完了していません。"; break;
	case CMD_RES_Error_NetworkError: pmsg = "CMD_RES_Error_NetworkError ネットワークが異常です。（WiFi切断、サーバ停止など）"; break;

	case CMD_RES_Error_ClientFile: pmsg = "CMD_RES_Error_ClientFile リモートドライブ上で実行してください。"; break;
	}

	if (ShowHeader) { printf("Error: "); }
	printf("%s\r\n", pmsg);
}

