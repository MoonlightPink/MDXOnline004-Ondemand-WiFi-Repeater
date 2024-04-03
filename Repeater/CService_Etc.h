#pragma once 

#include "incs.h"

class CService_Etc {
public:
	u8 op_init(u8 ID) {
		return CService::RES_OK;
	}

	u8 op_chdir(u8 ID) {
		CService::Cdos_namebuf* pcmd_path = new CService::Cdos_namebuf(Physical.pReceiveBuf);

		DebugSerial.println("isRoot: " + String(pcmd_path->isRoot()));

		String path = pcmd_path->conv_namebufToSJISText(false);
		DebugSerial.println("path: " + path);

		if (CFSMain::GetFolderID(path) == CFSCommon::FolderID_Undef) { return CService::_DOSE_NODIR; }

		return CService::RES_OK;
	}

	u8 op_dskfre(u8 ID) {
		Physical.pSendBuf->SetSizeMax();
		Physical.pSendBuf->SetPos(0);

		UInt16 cluster = 1;
		UInt16 sectorsize = 1024;
		UInt32 freesize = 0;
		UInt32 totalsize = (UInt32)1 * sectorsize * cluster;

		Physical.pSendBuf->WriteBEU32(freesize); // res free bytes
		Physical.pSendBuf->WriteBEU16(freesize / sectorsize / cluster); // freeclu
		Physical.pSendBuf->WriteBEU16(totalsize / sectorsize / cluster); // totalclu
		Physical.pSendBuf->WriteBEU16(cluster); // clusect
		Physical.pSendBuf->WriteBEU16(sectorsize); // sectsize

		Physical.pSendBuf->AdjustSizePos();
		return CService::RES_Setup;
	}
};

static CService_Etc Service_Etc;
