#pragma once

static String CommandToString(u8 cmd) {
	switch (cmd) {
	case 0x40: return "op_init";
	case 0x41: return "op_chdir";
	case 0x42: return "op_mkdir";
	case 0x43: return "op_rmdir";
	case 0x44: return "op_rename";
	case 0x45: return "op_delete";
	case 0x46: return "op_chmod";
	case 0x47: return "op_files";
	case 0x48: return "op_nfiles";
	case 0x49: return "op_create";
	case 0x4a: return "op_open";
	case 0x4b: return "op_close";
	case 0x4c: return "op_read";
	case 0x4d: return "op_write";
	case 0x4f: return "op_filedate";
	case 0x50: return "op_dskfre";
	case 0x51: return "nop drvctrl";
	case 0x52: return "nop getdbp";
	case 0x53: return "nop diskred";
	case 0x54: return "nop diskwrt";
	case 0x55: return "nop ioctl";
	case 0x56: return "nop abort";
	case 0x57: return "nop mediacheck";
	case 0x58: return "nop lock";
	default:return "Unknown op";
	}
}

static u8 Service_op_mkdir(u8 ID) { return CService::_DOSE_RDONLY; }
static u8 Service_op_rmdir(u8 ID) { return CService::_DOSE_RDONLY; }
static u8 Service_op_rename(u8 ID) { return CService::_DOSE_RDONLY; }
static u8 Service_op_delete(u8 ID) { return CService::_DOSE_RDONLY; }
static u8 Service_op_chmod(u8 ID) { return CService::_DOSE_RDONLY; }
static u8 Service_op_create(u8 ID) { return CService::_DOSE_RDONLY; }
static u8 Service_op_write(u8 ID) { return CService::_DOSE_RDONLY; }

static void RemoteExec() {
	Physical.pSendBuf->SetPos(0);
	Physical.pSendBuf->AdjustSizePos();

	u8 cmd = Physical.pReceiveBuf->ReadBEU8();
	u8 ID = cmd >> 5;
	cmd = (cmd & 0x1f) | 0x40;

	if ((cmd == 0x4c) || (cmd == 0x4d)) { ID = 0; } // op_read と op_write だけIDが2になる。remotedrv.c の send_read/send_write の cmd->command に直接0x4c/0x4dを代入しているのが原因っぽいけど実害は無い。

	Service_SetOLEDProgress("$" + String(cmd, HEX) + " " + CommandToString(cmd));

	DebugSerial.println("cmd=$" + String(cmd, HEX) + ", ID=" + String(ID) + ", cmdname=" + CommandToString(cmd));

	UInt32 ErrorCode = CService::_DOSE_ILGFNC;

	switch (ID) {
	case 0:
		switch (cmd) {
		case 0x40: ErrorCode = Service_Etc.op_init(ID); break;
		case 0x41: ErrorCode = Service_Etc.op_chdir(ID); break;
		case 0x42: ErrorCode = Service_op_mkdir(ID); break;
		case 0x43: ErrorCode = Service_op_rmdir(ID); break;
		case 0x44: ErrorCode = Service_op_rename(ID); break;
		case 0x45: ErrorCode = Service_op_delete(ID); break;
		case 0x46: ErrorCode = Service_op_chmod(ID); break;
		case 0x47: ErrorCode = Service_Files.op_files(ID); break;
		case 0x48: ErrorCode = Service_Files.op_nfiles(ID); break;
		case 0x49: ErrorCode = Service_op_create(ID); break;
		case 0x4a: ErrorCode = Service_Body.op_open(ID); break;
		case 0x4b: ErrorCode = Service_Body.op_close(ID); break;
		case 0x4c: ErrorCode = Service_Body.op_read(ID); break;
		case 0x4d: ErrorCode = Service_op_write(ID); break;
		case 0x4f: ErrorCode = Service_Body.op_filedate(ID); break;
		case 0x50: ErrorCode = Service_Etc.op_dskfre(ID); break;

		case 0x51: /* drvctrl */
		case 0x52: /* getdbp */
		case 0x53: /* diskred */
		case 0x54: /* diskwrt */
		case 0x55: /* ioctl */
		case 0x56: /* abort */
		case 0x57: /* mediacheck */
		case 0x58: /* lock */
		default:
			break;
		}
		break;
	default:
		DebugSerial.println("Unknown ID. ID=" + String(ID));
		break;
	}

	if (ErrorCode != CService::RES_Setup) {
		Physical.pSendBuf->SetSizeMax();
		Physical.pSendBuf->SetPos(0);
		Physical.pSendBuf->WriteBEU8(ErrorCode);
		Physical.pSendBuf->AdjustSizePos();

		if (ErrorCode != CService::RES_OK) {
			DebugSerial.println("command error. 0x" + String(ErrorCode, HEX));
		}
	}
}
