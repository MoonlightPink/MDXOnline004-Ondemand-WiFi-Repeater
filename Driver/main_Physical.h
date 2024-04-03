
/* // ドライバモードは常にスーパーバイザーモードなので不要
static int ssp = 0;
static volatile void EnterSuper() {
	if (ssp != 0) {
		DebugPrintln("既にスーパーバイザーモードなのにスーパーバイザーモードに入ろうとしました。");
	}
	ssp = _iocs_b_super(0);
}

static volatile void LeaveSuper() {
	if (ssp == 0) {
		DebugPrintln("既にユーザーモードなのにスーパーバイザーモードから出ようとしました。");
	}
	_iocs_b_super(ssp);
	ssp = 0;
}
*/

static inline void wait() {
	for (volatile int a = 0; a < 100; a++) {
		asm volatile("nop;nop;nop;nop;nop; nop;nop;nop;nop;nop;");
	}
}

static u8 CRC8Table[0x100] = {
  0x00, 0x31, 0x62, 0x53, 0xc4, 0xf5, 0xa6, 0x97, 0xb9, 0x88, 0xdb, 0xea, 0x7d, 0x4c, 0x1f, 0x2e,
  0x43, 0x72, 0x21, 0x10, 0x87, 0xb6, 0xe5, 0xd4, 0xfa, 0xcb, 0x98, 0xa9, 0x3e, 0x0f, 0x5c, 0x6d,
  0x86, 0xb7, 0xe4, 0xd5, 0x42, 0x73, 0x20, 0x11, 0x3f, 0x0e, 0x5d, 0x6c, 0xfb, 0xca, 0x99, 0xa8,
  0xc5, 0xf4, 0xa7, 0x96, 0x01, 0x30, 0x63, 0x52, 0x7c, 0x4d, 0x1e, 0x2f, 0xb8, 0x89, 0xda, 0xeb,
  0x3d, 0x0c, 0x5f, 0x6e, 0xf9, 0xc8, 0x9b, 0xaa, 0x84, 0xb5, 0xe6, 0xd7, 0x40, 0x71, 0x22, 0x13,
  0x7e, 0x4f, 0x1c, 0x2d, 0xba, 0x8b, 0xd8, 0xe9, 0xc7, 0xf6, 0xa5, 0x94, 0x03, 0x32, 0x61, 0x50,
  0xbb, 0x8a, 0xd9, 0xe8, 0x7f, 0x4e, 0x1d, 0x2c, 0x02, 0x33, 0x60, 0x51, 0xc6, 0xf7, 0xa4, 0x95,
  0xf8, 0xc9, 0x9a, 0xab, 0x3c, 0x0d, 0x5e, 0x6f, 0x41, 0x70, 0x23, 0x12, 0x85, 0xb4, 0xe7, 0xd6,
  0x7a, 0x4b, 0x18, 0x29, 0xbe, 0x8f, 0xdc, 0xed, 0xc3, 0xf2, 0xa1, 0x90, 0x07, 0x36, 0x65, 0x54,
  0x39, 0x08, 0x5b, 0x6a, 0xfd, 0xcc, 0x9f, 0xae, 0x80, 0xb1, 0xe2, 0xd3, 0x44, 0x75, 0x26, 0x17,
  0xfc, 0xcd, 0x9e, 0xaf, 0x38, 0x09, 0x5a, 0x6b, 0x45, 0x74, 0x27, 0x16, 0x81, 0xb0, 0xe3, 0xd2,
  0xbf, 0x8e, 0xdd, 0xec, 0x7b, 0x4a, 0x19, 0x28, 0x06, 0x37, 0x64, 0x55, 0xc2, 0xf3, 0xa0, 0x91,
  0x47, 0x76, 0x25, 0x14, 0x83, 0xb2, 0xe1, 0xd0, 0xfe, 0xcf, 0x9c, 0xad, 0x3a, 0x0b, 0x58, 0x69,
  0x04, 0x35, 0x66, 0x57, 0xc0, 0xf1, 0xa2, 0x93, 0xbd, 0x8c, 0xdf, 0xee, 0x79, 0x48, 0x1b, 0x2a,
  0xc1, 0xf0, 0xa3, 0x92, 0x05, 0x34, 0x67, 0x56, 0x78, 0x49, 0x1a, 0x2b, 0xbc, 0x8d, 0xde, 0xef,
  0x82, 0xb3, 0xe0, 0xd1, 0x46, 0x77, 0x24, 0x15, 0x3b, 0x0a, 0x59, 0x68, 0xff, 0xce, 0x9d, 0xac,
};

// Joy 8255 GPIO Dir Desc
// 
// 1.1 PA0  WtoX Data.Bit0 Low:0, High:1.
// 1.2 PA1  WtoX Data.Bit1
// 1.3 PA2  WtoX Data.Bit2
// 1.4 PA3  WtoX Data.Bit3
// 1.6 PC6~ XtoW N.C.
// 1.7 PC7~ XtoW N.C.
// 1.8 PC4  XtoW Serial.Data N81, 2400
// 
// 2.1 PB0  WtoX Data.Bit4
// 2.2 PB1  WtoX Data.Bit5
// 2.3 PB2  WtoX Data.Bit6
// 2.4 PB3  WtoX Data.Bit7
// 2.6 PB5  WtoX Data.Clock Low:Can read, High:Changing.
// 2.7 PB6  WtoX Serial.Ready Low:Can send, High:Can not send.
// 2.8 PC5  XtoW Data.Recv Low:, High:.

#define Joy_PCA (*(volatile u8*)0xe9a001) // R 8255 ポート A(ジョイスティック #1)
#define Joy_PCB (*(volatile u8*)0xe9a003) // R 8255 ポート B(ジョイスティック #2)
#define Joy_PCC (*(volatile u8*)0xe9a005) // R/W 8255 ポート C(ジョイスティックコントロール)
#define Joy_Ctrl (*(volatile u8*)0xe9a007) // W 8255 コントロールワード(動作モード/ビット操作)

static volatile void JoyInit() {
	u8 Ctrl = 0x00;
	Ctrl |= 1 << 7; // bit 7    MDOE 0:ビット操作, 1:動作設定
	Ctrl |= 0 << 5; // bit 6～5 Group A Mode グループ A(ポート A とポート C 上位)の動作モード
	Ctrl |= 1 << 4; // bit 4    PORT A IN / OUT 0:出力, 1:入力
	Ctrl |= 0 << 3; // bit 3    PORT C(High) IN / OUT 0:出力, 1:入力
	Ctrl |= 0 << 2; // bit 2    Group B Mode グループ B(ポート B とポート C 下位)の動作モード
	Ctrl |= 1 << 1; // bit 1    PORT B IN / OUT 0:出力, 1:入力
	Ctrl |= 0 << 0; // bit 0    PORT C(Low) IN / OUT 0:出力, 1:入力 (起動時入力だけど、ADPCM関係の制御信号なのだから出力で良いよね？)
	Joy_Ctrl = Ctrl;
}

static volatile bool Joy_ReadDataClock() { return (Joy_PCB & (1 << 5)) != 0; }
static volatile bool Joy_ReadSerialReady() { return (Joy_PCB & (1 << 6)) != 0; }

#define Joy_PC_SerialData (4)
#define Joy_PC_DataRecv (5)
#define Joy_PC_NC1 (6)
#define Joy_PC_NC2 (7)

#define Joy_PC_BitSet(pin,f) { \
  u8 Ctrl = 0x00; \
  Ctrl |= 0 << 7; /* bit 7    MODE 0:ビット操作, 1:動作設定 */ \
  Ctrl |= pin << 1; \
  Ctrl |= (f ? 1 : 0) << 0; \
  Joy_Ctrl = Ctrl; \
}

#define Joy_PC_BitSet_Not(pin,f) { \
  u8 Ctrl = 0x00; \
  Ctrl |= 0 << 7; /* bit 7    MODE 0:ビット操作, 1:動作設定 */ \
  Ctrl |= pin << 1; \
  Ctrl |= (f ? 0 : 1) << 0; \
  Joy_Ctrl = Ctrl; \
}

static inline void MXDRV_Trap(u32 Command) {
	asm volatile(
		"move.l %[Command],%%d0;"
		"trap #4;"
		:
	: [Command] "d"(Command)
		: "cc", "d0"
		);
}
static inline void MXDRV_Pause() { MXDRV_Trap(0x06); }
static inline void MXDRV_Cont() { MXDRV_Trap(0x07); }

static inline void PCM8_Trap(u32 Command) {
	asm volatile(
		"move.l %[Command],%%d0;"
		"trap #2;"
		:
	: [Command] "d"(Command)
		: "cc", "d0"
		);
}
static inline void PCM8_End() { PCM8_Trap(0x0100); }
static inline void PCM8_Pause() { PCM8_Trap(0x0101); }
static inline void PCM8_Cont() { PCM8_Trap(0x0102); }

static inline volatile void Joy_SetSerialData(bool f) { Joy_PC_BitSet(Joy_PC_SerialData, f); } // 要スーパーバイザーモード
static inline volatile void Joy_SetDataRecv(bool f) { Joy_PC_BitSet(Joy_PC_DataRecv, f); } // 要スーパーバイザーモード
static inline volatile void Joy_SetNC1(bool f) { Joy_PC_BitSet_Not(Joy_PC_NC1, f); } // 要スーパーバイザーモード
static inline volatile void Joy_SetNC2(bool f) { Joy_PC_BitSet_Not(Joy_PC_NC2, f); } // 要スーパーバイザーモード

#define SerialWait ""                                         // NOPx0  47.8～48.8us 207039bps (207039/230400= 89.8%)
//#define SerialWait "nop;"                                     // NOPx1  51.3～52.3us 193050bps
//#define SerialWait "nop;nop;"                                 // NOPx2  55.6～56.7us 178253bps
//#define SerialWait "nop;nop;nop;"                             // NOPx3  59.6～60.7us 166389bps
//#define SerialWait "nop;nop;nop;nop;"                         // NOPx4  64.3～65.3us 154321bps
//#define SerialWait "nop;nop;nop;nop;nop;"                     // NOPx5  68.4～69.3us 145138bps
//#define SerialWait "nop;nop;nop;nop;nop;nop;"                 // NOPx6  72.6～73.3us 136986bps
//#define SerialWait "nop;nop;nop;nop;nop;nop;nop;"             // NOPx7  74.9～76.8us 132450bps
//#define SerialWait "nop;nop;nop;nop;nop;nop;nop;nop;"         // NOPx8  80.8～81.7us 123001bps
//#define SerialWait "nop;nop;nop;nop;nop;nop;nop;nop;nop;"     // NOPx9  84.8～85.6us 117379bps (117379/115200=101.8%)
//#define SerialWait "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;" // NOPx10 88.8～89.8us 111982bps (111982/115200= 97.2%)

static volatile void SerialWrite_Start() {
	asm volatile("ori #0x0700,%sr;"); // 割り込み禁止

	PCM8_Pause();
	PCM8_End();
}

static volatile void SerialWrite_u8(u8 _Data) {
	// ASCII:'e' 0x65 01100101 N81:0101001101

	volatile u8 Data = _Data;
	volatile u8 Temp;
	asm volatile(
		"move.b #0x09, 0xe9a007;" // Ctrl HIGH Init

		"move.b #0x08, 0xe9a007;" // Ctrl LOW StartBit
		"nop;nop;"
		SerialWait

		"move.b %[Data],%[Temp];"
		"and.b #0x01,%[Temp];"
		"or.b #0x08,%[Temp];"
		"move.b %[Temp], 0xe9a007;" // Ctrl
		SerialWait

		"lsr.b #1,%[Data];"
		"move.b %[Data],%[Temp];"
		"and.b #0x01,%[Temp];"
		"or.b #0x08,%[Temp];"
		"move.b %[Temp], 0xe9a007;" // Ctrl
		SerialWait

		"lsr.b #1,%[Data];"
		"move.b %[Data],%[Temp];"
		"and.b #0x01,%[Temp];"
		"or.b #0x08,%[Temp];"
		"move.b %[Temp], 0xe9a007;" // Ctrl
		SerialWait

		"lsr.b #1,%[Data];"
		"move.b %[Data],%[Temp];"
		"and.b #0x01,%[Temp];"
		"or.b #0x08,%[Temp];"
		"move.b %[Temp], 0xe9a007;" // Ctrl
		SerialWait

		"lsr.b #1,%[Data];"
		"move.b %[Data],%[Temp];"
		"and.b #0x01,%[Temp];"
		"or.b #0x08,%[Temp];"
		"move.b %[Temp], 0xe9a007;" // Ctrl
		SerialWait

		"lsr.b #1,%[Data];"
		"move.b %[Data],%[Temp];"
		"and.b #0x01,%[Temp];"
		"or.b #0x08,%[Temp];"
		"move.b %[Temp], 0xe9a007;" // Ctrl
		SerialWait

		"lsr.b #1,%[Data];"
		"move.b %[Data],%[Temp];"
		"and.b #0x01,%[Temp];"
		"or.b #0x08,%[Temp];"
		"move.b %[Temp], 0xe9a007;" // Ctrl
		SerialWait

		"lsr.b #1,%[Data];"
		"move.b %[Data],%[Temp];"
		"and.b #0x01,%[Temp];"
		"or.b #0x08,%[Temp];"
		"move.b %[Temp], 0xe9a007;" // Ctrl
		SerialWait

		"nop;nop;nop;nop;nop;nop;"
		"move.b #0x09, 0xe9a007;" // Ctrl HIGH StopBit
		"nop;nop;nop;nop;nop;nop;nop;nop;"
		SerialWait

		//		"move.b #0x08, 0xe9a007; move.b #0x09, 0xe9a007;" // 区切りをわかりやすくするためダミー信号を送る
		: [Data] "+d"(Data), [Temp]"=d"(Temp)
		:
		: "cc"
		);
}

static volatile void SerialWrite_buf(const u8* pBuf, int len) {
	for (int idx = 0; idx < len; idx++) {
		SerialWrite_u8(pBuf[idx]);
	}
}

static volatile void SerialWrite_End() {
	PCM8_Cont();
	asm volatile("andi #0xf8ff,%sr;"); // 割り込み許可
}

// DataRecv信号をLOWにしてから7us待てばデータが来ている。

static inline volatile u8 DataRead() {
	u8 Data, Temp;
	asm volatile(
		"nop;nop;nop;nop;nop; nop;nop;nop;nop;nop;"
		"nop;nop;nop;nop;nop; nop;nop;nop;nop;nop;"

		"move.b #0x0b, 0xe9a007;" // Joy_PC_DataRecv 4 HIGH
		"move.b 0xe9a001,%[Data];" // 下位4bit
		"move.b 0xe9a003,%[Temp];" // 上位4bit
		"move.b #0x0a, 0xe9a007;" // Joy_PC_DataRecv 4 LOW

		"and.b #0x0f,%[Data];"
		"lsl.b #4,%[Temp];"
		"or.b %[Temp],%[Data];"

		"nop;nop;nop;nop;nop; nop;nop;nop;nop;nop;"
		: [Data] "=d"(Data), [Temp]"=d"(Temp)
		:
		: "cc"
		);
	return Data;
}

#define ReadBufMaxSize (sizeof(union Tb))
static u8 ReadBufCheckSum;

static void volatile DataReadBuf(u8* ReadBuf, u32 Count) {
	if (Count == 0) {
		ReadBufCheckSum = 0;
		return;
	}

	// 8192bytes転送 NOPx3=135ms エラー100%, NOPx4=139ms エラー85%, NOPx5=141ms エラー57%, NOPx6=145ms エラー0%, NOPx7=148ms エラー0%, NOPx8=153ms エラー0%.

	volatile u32 rReadBuf = (u32)ReadBuf;
	volatile u8 Data, Temp;
	volatile u32 rCRC8Table = (u32)CRC8Table;
	asm volatile(
		"ori #0x0700,%%sr;" // 割り込み禁止

		"move.b #0,%[ReadBufCheckSum];"

		"loop:"
		"nop;nop;nop;nop; nop;nop;nop;nop;"

		"move.b #0x0b, 0xe9a007;" // Joy_PC_DataRecv 4 HIGH
		"move.b 0xe9a001,%[Data];" // 下位4bit
		"move.b 0xe9a003,%[Temp];" // 上位4bit
		"move.b #0x0a, 0xe9a007;" // Joy_PC_DataRecv 4 LOW (オシロで確認して信号が充分立ち上がってからLOWにする)

		"and.b #0x0f,%[Data];"
		"lsl.b #4,%[Temp];"
		"or.b %[Temp],%[Data];"

		"move.b %[Data],(%[ReadBuf])+;" // 配列に書き込み

		// チェックサム計算
		"eor.b %[Data],%[ReadBufCheckSum];"
		"move.b (%[ReadBufCheckSum],%[CRC8Table]),%[ReadBufCheckSum];"

		"sub #1,%[cnt];"
		"bne loop;"

		"andi #0xf8ff,%%sr;" // 割り込み許可
		: [Data] "=d"(Data), [ReadBufCheckSum] "=d"(ReadBufCheckSum), [Temp]"=d"(Temp), [cnt] "+d"(Count), [ReadBuf]"+a"(rReadBuf), [CRC8Table] "+a"(rCRC8Table)
		:
		: "cc"
		);
}

static void Physical_Init() {
	JoyInit();

	Joy_SetSerialData(true); // Serial N81
	Joy_SetDataRecv(true); // Data.Recv Low:Ready, High:Not ready.
	Joy_SetNC1(true); // 使わないのでオープンドレイン
	Joy_SetNC2(true); // 使わないのでオープンドレイン
}

static void Physical_End() {
	// 終了時は全てオープンドレイン
	Joy_SetSerialData(true);
	Joy_SetDataRecv(true);
	Joy_SetNC1(true);
	Joy_SetNC2(true);
}

static void serout(const void* buf, size_t len) {
	const u8* pBuf = (u8*)buf;

	if (EnableDebugPrint) {
		for (int idx = 0; idx < len; idx++) {
			const u8 Data = pBuf[idx];
			char buffer[2 + 1];
			itoa(Data, buffer, 16);
			DebugPrint(buffer);
			DebugPrint(",");
		}
		DebugPrintln("");
	}

	SerialWrite_Start();
	SerialWrite_u8(0xff);
	SerialWrite_u8(len >> 8);
	SerialWrite_u8(len >> 0);
	SerialWrite_buf(pBuf, len);
	SerialWrite_End();
}

static void serin(void* buf, size_t len) {
	u8* pBuf = (u8*)buf;

	DebugPrintlnInt("ReadBufMaxSize: ", ReadBufMaxSize);

	static int TotalCount = 0;
	static int ErrorCount = 0;

	while (true) {
		while (true) {
			const bool DataClock = Joy_ReadDataClock();
			if (!DataClock) { break; }
		}

		Joy_SetDataRecv(false);


		const u32 Count = (DataRead() << 8) | DataRead();
		const u8 CheckSum = DataRead();

		wait(); // どちらにもウェイトがないとダメ。明日調べる
		DebugPrintlnInt("Count: ", Count);
		DebugPrintlnU8("CheckSum: $", CheckSum);

		bool ReadError = false;

		if ((Count == 0) || (len < Count)) {
			DebugPrintln("Read buffer count overflow.");
			ReadError = true;
		} else {
			DataReadBuf(pBuf, Count);
			wait(); // どちらにもウェイトがないとダメ。明日調べる
			DebugPrintlnU8("ReadBufCheckSum: $", ReadBufCheckSum);
			if (CheckSum != ReadBufCheckSum) {
				DebugPrintln("Unmatch checksum.");
				ReadError = true;
			}
		}

		Joy_SetDataRecv(true);

		SerialWrite_Start();
		SerialWrite_u8(ReadError ? 0xff : 0x00); // Result: 0x00=OK, Other=Retry.
		SerialWrite_End();

		TotalCount++;
		if (!ReadError) { break; }

		ErrorCount++;
		DebugPrintln("Retry.");
	}

	if (1 <= ErrorCount) {
		DebugPrintlnInt("TotalCount: ", TotalCount);
		DebugPrintlnInt("ErrorCount: ", ErrorCount);
	}
}

void com_cmdres(void* wbuf, size_t wsize, void* rbuf, size_t rsize) {
	//	Physical_Init();
	while (true) {
		while (Joy_ReadSerialReady()) {
		}
		serout(wbuf, wsize);
		serin(rbuf, rsize);
		break;
	}
}

