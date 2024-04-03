#pragma once

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define CONFIG_DATASIZE     (1024) // config.h

#include <WiFi.h>
#include <HTTPClient.h>

#define DEBUG (false)

#define DebugSerial Serial

#include <LittleFS.h>

#include "PinsAssign.h"

#include "Common.h"

#include "CBuffer.h"

#include "OLED.h"

#include "CPhysical_JoyPort.h"

static const u8 CMD_RES_OK = 0x00;
static const u8 CMD_RES_Error_UnknownCommand = 0x01;
static const u8 CMD_RES_Error_File = 0x02;
static const u8 CMD_RES_Error_IlligalParam = 0x03;
static const u8 CMD_RES_Error_SetupNotComplete = 0x04;
static const u8 CMD_RES_Error_NetworkError = 0x05;

static const u8 CMD_RES_Error_ClientFile = 0xff; // Setup.xやStatus.xでファイルが開けなかったときに返る。

#include "CSettings.h"

static void CFSMain_VolumeLabel_SetName(String text);
static void CFSMain_FolderID_PDX_Set(u16 FolderID);
static void CFSMain_Disconnect();
static void CFSCache_RequestFreeArea(int size);

#include "CWiFiSTA.h"
#include "CWiFiTools.h"

#include "CFSCommon.h"
#include "CFSVolumeLabel.h"
#include "CFSCache.h"
#include "CFSLocal.h"
#include "CFSNetwork.h"
#include "CFSMain.h"

#include "CCommand.h"

#include "CService.h"
#include "CService_Etc.h"
#include "CService_Files.h"
#include "CService_Body.h"
#include "CService_Call.h"

