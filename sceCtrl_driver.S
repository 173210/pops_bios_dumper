#include "pspimport.s"

	IMPORT_START	"sceCtrl_driver", 0x00010000
	IMPORT_FUNC	"sceCtrl_driver", 0x6D74BF08, sceCtrlSetSamplingCycle371
	IMPORT_FUNC	"sceCtrl_driver", 0x28E71A16, sceCtrlSetSamplingMode371
	IMPORT_FUNC	"sceCtrl_driver", 0xC4AAD55F, sceCtrlPeekBufferPositive371
