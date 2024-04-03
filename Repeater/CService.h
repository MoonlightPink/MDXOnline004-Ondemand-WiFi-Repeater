#pragma once 

#include "incs.h"

static String OLED_PrgTitle = "";
static String OLED_PrgDesc = "";
static float OLED_PrgValue = 0;
static float OLED_PrgMax = 0;

static void Service_SetOLEDProgress(String Title = "", String Desc = "", float Value = 0, float Max = 0) {
	OLED_PrgTitle = Title;
	OLED_PrgDesc = Desc;
	OLED_PrgValue = Value;
	OLED_PrgMax = Max;
}

class CService {
public:
	static const u8 RES_OK = 0x00;
	static const u8 RES_Setup = 0x01;

	// Human68k error code
	static const u8 _DOSE_ILGFNC = -1;
	static const u8 _DOSE_NOENT = -2;
	static const u8 _DOSE_NODIR = -3;
	static const u8 _DOSE_MFILE = -4;
	static const u8 _DOSE_ISDIR = -5;
	static const u8 _DOSE_BADF = -6;
	static const u8 _DOSE_BROKNMEM = -7;
	static const u8 _DOSE_NOMEM = -8;
	static const u8 _DOSE_ILGMPTR = -9;
	static const u8 _DOSE_ILGENV = -10;
	static const u8 _DOSE_ILGFMT = -11;
	static const u8 _DOSE_ILGARG = -12;
	static const u8 _DOSE_ILGFNAME = -13;
	static const u8 _DOSE_ILGPARM = -14;
	static const u8 _DOSE_ILGDRV = -15;
	static const u8 _DOSE_ISCURDIR = -16;
	static const u8 _DOSE_CANTIOC = -17;
	static const u8 _DOSE_NOMORE = -18;
	static const u8 _DOSE_RDONLY = -19;
	static const u8 _DOSE_EXISTDIR = -20;
	static const u8 _DOSE_NOTEMPTY = -21;
	static const u8 _DOSE_CANTREN = -22;
	static const u8 _DOSE_DISKFULL = -23;
	static const u8 _DOSE_DIRFULL = -24;
	static const u8 _DOSE_CANTSEEK = -25;
	static const u8 _DOSE_SUPER = -26;
	static const u8 _DOSE_DUPTHNAM = -27;
	static const u8 _DOSE_CANTSEND = -28;
	static const u8 _DOSE_THFULL = -29;
	static const u8 _DOSE_LCKFULL = -32;
	static const u8 _DOSE_LCKERR = -33;
	static const u8 _DOSE_BUSYDRV = -34;
	static const u8 _DOSE_SYMLOOP = -35;
	static const u8 _DOSE_EXISTFILE = -80;
	/*
$ffffffff        -1     Executed invalid function code
$fffffffe        -2     Specified file not found
$fffffffd        -3     Specified directory not found
$fffffffc        -4     Too many open files
$fffffffb        -5     Cannot access directory or volume label
$fffffffa        -6     Specified handle is not open
$fffffff9        -7     Memory manager region was destroyed
$fffffff8        -8     Not enough memory to execute
$fffffff7        -9     Invalid memory manager pointer specified
$fffffff6       -10     Illegal environment specified
$fffffff5       -11     Abnormal executable file format
$fffffff4       -12     Abnormal open access mode
$fffffff3       -13     Error in selecting a filename
$fffffff2       -14     Called with invalid parameter
$fffffff1       -15     Error in selecting a drive
$fffffff0       -16     Cannot remove current directory
$ffffffef       -17     Cannot ioctrl device
$ffffffee       -18     No more files found
$ffffffed       -19     Cannot write to specified file
$ffffffec       -20     Specified directory already registered
$ffffffeb       -21     Cannot delete because file exists
$ffffffea       -22     Cannot name because file exists
$ffffffe9       -23     Cannot create file because disk is full
$ffffffe8       -24     Cannot create file because directory is full
$ffffffe7       -25     Cannot seek to specified location
$ffffffe6       -26     Specified supervisor mode with supervisor status on
$ffffffe5       -27     Thread with same name exists
$ffffffe4       -28     Interprocess communication buffer is write-protected
$ffffffe3       -29     Cannot start any more background processes
$ffffffe0       -32     Not enough lock regions
$ffffffdf       -33     Locked; cannot access
$ffffffde       -34     Handler for specified drive is opened
$ffffffdd       -35     Symbolic link nest exceeded 16 steps (lndrv)
$ffffffb0       -80     File exists
	*/

	static byte GetAtr(bool isFile, bool isDir, bool isReadOnly = true) {
		byte res = 0x00;
		if (isFile) { res |= 0x20; } // regular file
		if (isDir) { res |= 0x10; } // directory
		if (isReadOnly) { res |= 0x01; } // read only
		return res;
	}

	class Cdos_namebuf {
	public:
		byte flag;
		byte drive;
		byte path[65];
		byte name1[8];
		byte ext[3];
		byte name2[10];
		Cdos_namebuf(CBuffer* pbuf) {
			flag = pbuf->ReadBEU8();
			drive = pbuf->ReadBEU8();
			for (int idx = 0; idx < sizeof(path); idx++) { path[idx] = pbuf->ReadBEU8(); }
			for (int idx = 0; idx < sizeof(name1); idx++) { name1[idx] = pbuf->ReadBEU8(); }
			for (int idx = 0; idx < sizeof(ext); idx++) { ext[idx] = pbuf->ReadBEU8(); }
			for (int idx = 0; idx < sizeof(name2); idx++) { name2[idx] = pbuf->ReadBEU8(); }
		}
		bool isRoot() {
			return (path[0] == '\t') && (path[1] == 0x00);
		}
		String conv_namebufToSJISText(bool full) {
			// namestsのパスをホストのパスに変換する
			// (derived from HFS.java by Makoto Kamada)

			byte bb[88 + 1];   // SJISでのパス名
			int k = 0;

			// パスの区切りを 0x09 -> '/' に変更
			for (int i = 0; i < 65;) {
				for (; i < 65 && path[i] == 0x09; i++); //0x09の並びを読み飛ばす
				if (i >= 65 || path[i] == 0x00) break; //ディレクトリ名がなかった
				bb[k++] = 0x2f;  //ディレクトリ名の手前の'/'
				for (; i < 65 && path[i] != 0x00 && path[i] != 0x09; i++) { bb[k++] = path[i]; } //ディレクトリ名
			}
			// 主ファイル名を展開する
			if (full) {
				bb[k++] = 0x2f;  //主ファイル名の手前の'/'
				for (int idx = 0; idx < sizeof(name1); idx++) { bb[k++] = name1[idx]; } //主ファイル名1
				for (int idx = 0; idx < sizeof(name2); idx++) { bb[k++] = name2[idx]; } //主ファイル名1
				for (; k > 0 && bb[k - 1] == 0x00; k--);  //主ファイル名2の末尾の0x00を切り捨てる
				for (; k > 0 && bb[k - 1] == 0x20; k--);  //主ファイル名1の末尾の0x20を切り捨てる
				bb[k++] = 0x2e;  //拡張子の手前の'.'
				for (int idx = 0; idx < sizeof(ext); idx++) { bb[k++] = ext[idx]; } //拡張子
				for (; k > 0 && bb[k - 1] == 0x20; k--); //拡張子の末尾の0x20を切り捨てる
				for (; k > 0 && bb[k - 1] == 0x2e; k--); //主ファイル名の末尾の0x2eを切り捨てる
			}
			bb[k] = 0x00;

			return String((char*)bb);
		}
		void dos_namebuf_GetWildCard(byte* w) {
			// (derived from HFS.java by Makoto Kamada)
			//検索するファイル名の順序を入れ替える
			//  主ファイル名1の末尾が'?'で主ファイル名2の先頭が'\0'のときは主ファイル名2を'?'で充填する

			for (int idx = 0; idx < sizeof(w); idx++) { w[idx] = 0x00; }
			for (int idx = 0; idx < 8; idx++) { w[idx] = name1[idx]; }    //主ファイル名1
			if (name1[7] == '?' && name2[0] == '\0') {  //主ファイル名1の末尾が'?'で主ファイル名2の先頭が'\0'
				for (int idx = 0; idx < 10; idx++) { w[8 + idx] = (byte)'?'; } //主ファイル名2
			} else {
				for (int idx = 0; idx < 10; idx++) { w[8 + idx] = name2[idx]; } //主ファイル名2
			}
			for (int i = 17; i >= 0 && (w[i] == '\0' || w[i] == ' '); i--) { w[i] = 0x00; } //主ファイル名1+主ファイル名2の空き
			for (int idx = 0; idx < 3; idx++) { w[18 + idx] = ext[idx]; } //拡張子
			for (int i = 20; i >= 18 && (w[i] == ' '); i--) { w[i] = 0x00; } //拡張子の空き
			//検索するファイル名を小文字化する
			for (int i = 0; i < 21; i++) {
				int c = w[i];
				if (0x81 <= c && c <= 0x9f || 0xe0 <= c && c <= 0xef) {  //SJISの1バイト目
					i++;
				} else {
					byte ch = w[i];
					if (((byte)'A' <= ch) && (ch <= (byte)'Z')) { w[i] = (byte)(ch - (byte)'A' + (byte)'a'); }
				}
			}
		}
	};
};
