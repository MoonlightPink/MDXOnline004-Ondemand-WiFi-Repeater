#pragma once 

#include "incs.h"

const String LocalFiles_BasePath = "/LocalFiles";

static String FileSystemBasePath = "/FileSystem";

static String FoldersENGFilename = FileSystemBasePath + "/Folders_ENG.dat";
static String FoldersJPNFilename = FileSystemBasePath + "/Folders_JPN.dat";

static String DirsDataBasePath = "/DirsData";
static String DirsDataPrefix =  "/Dir";
static String DirsDataENGSuffix = "_ENG.dat";
static String DirsDataJPGSuffix = "_JPG.dat";

static String NormalBasePath = "/Normal";

static String PDXFilesFilename = FileSystemBasePath + "/PDXFiles.dat";
static String PDXBasePath = "/PDX";

static const int NormalFilenameLen = 18;
static const int PDXFilenameLen = 8;

class CFSCommon {
private:
public:
	static const int MaxPathLen = 61;
	static const int MaxNameLen = 18;
	static const int MaxExtLen = 4;

	static const UInt16 FolderID_Undef = 0xffff;

	static const byte Attr_Mask = 0x60;
	static const byte Attr_Term = 0x00;
	static const byte Attr_Dir = 0x20;
	static const byte Attr_File = 0x40;
	static const byte Attr_Reserve = 0x60;

	static const int MD5Len = 16;

	class CEntry {
	private:
	public:
		static const byte Mode_Local = 0;
		static const byte Mode_Normal = 1;
		static const byte Mode_PDX = 2;
		byte Mode;

		byte Attr;
		String Name;
		UInt32 Size;
		UInt16 LastWriteDT;
		byte MD5[MD5Len];
		bool isDir() { return Attr == Attr_Dir; }
		bool isFile() { return Attr == Attr_File; }
	};
};

