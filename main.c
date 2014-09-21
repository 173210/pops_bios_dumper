/*
 * Copyright (C) 2014 173210 <root.3.173210@live.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <pspkernel.h>
#include <pspctrl.h>
#include <pspsysmem_kernel.h>
#include <systemctrl.h>
#include <stdio.h>

PSP_MODULE_INFO("POPS_BIOS_DUMPER", PSP_MODULE_KERNEL, 0, 0);

static SceUID thid;

int sceCtrlSetSamplingCycle371(int cycle);
int sceCtrlSetSamplingMode371(int mode);
int sceCtrlPeekBufferPositive371(SceCtrlData *pad_data, int count);

static void waitExit()
{
	SceCtrlData pad_data;

	pspDebugScreenPuts("\nPress X to exit.\n");

	sceCtrlSetSamplingCycle371(0);
	sceCtrlSetSamplingMode371(PSP_CTRL_MODE_DIGITAL);

	do {
		sceCtrlPeekBufferPositive371(&pad_data, 1);
	} while (!(pad_data.Buttons & PSP_CTRL_CROSS));

	sceKernelExitVSHVSH(NULL);
}

static int mainThread()
{
	const SceSize size = 524288;
	const int hdr[] = { 0x3C080013, 0x3508243F, 0x3C011F80, 0xAC281010 };

	SceUID fd;
	SceUID modid;
	SceModule2 *mod;
	int i, j;
	int ret;
	int *p;
	char path[sizeof("flash0:/kd/pops_0Xg")];

	pspDebugScreenInit();

	pspDebugScreenPuts("POPS BIOS Dumper by 173210\n\n"
		"Initializing...\n"
		" Loading Module...\n");
	sprintf(path, "flash0:/kd/pops_%02dg.prx", sceKernelGetModel() + 1);
	modid = sceKernelLoadModule(path, 0, NULL);
	if (modid < 0) {
		pspDebugScreenKprintf("  -> Failed 0x%08X\n", modid);
		waitExit();
		return modid;
	}
	pspDebugScreenPuts("Getting Module Information...\n");
	mod = (SceModule2 *)sceKernelFindModuleByUID(modid);
	if (mod == NULL) {
		pspDebugScreenKprintf(" -> Failed\n");
		waitExit();
		return -1;
	}

	// mod = (SceModule *)((int)mod + 4);

	pspDebugScreenPuts("Searching for BIOS...\n");
	for (i = 0; i < mod->nsegment; i++) {
		pspDebugScreenKprintf(" Searching Segment %d of %d (%d Bytes)\n",
			i, mod->nsegment - 1, mod->segmentsize[i]);
		for (p = (int *)mod->segmentaddr[i];
			(int)p < mod->segmentaddr[i] + mod->segmentsize[i] - size;
			p++) {
			for (j = 0; hdr[j] == *p; j++) {
				if (j >= sizeof(hdr) / sizeof(*hdr) - 1) {
					pspDebugScreenPuts("Dumping...\n"
						" Opening File...\n");
					fd = sceIoOpen("ms0:/PSX-BIOS.ROM",
						PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC,
						0777);
					if (fd < 0) {
						pspDebugScreenKprintf("  -> Failed 0x%08X\n"
							"Deinitialzing...\n"
							" Stopping Module...\n", fd);
						sceKernelStopModule(modid, 0, NULL, NULL, NULL);
						pspDebugScreenPuts(" Unloading Module...\n");
						sceKernelUnloadModule(modid);
						waitExit();
						return fd;
					}

					pspDebugScreenKprintf(" Writing...\n");
					ret = sceIoWrite(fd, p - j, size);
					if (ret < 0) {
						pspDebugScreenKprintf("  -> Failed 0x%08X\n"
							"Deinitialzing...\n"
							" Closing File...\n", ret);
						sceIoClose(fd);
						pspDebugScreenPuts(" Stopping Module...\n");
						sceKernelStopModule(modid, 0, NULL, NULL, NULL);
						pspDebugScreenPuts(" Unloading Module...\n");
						sceKernelUnloadModule(modid);
						waitExit();
						return ret;
					}

					pspDebugScreenKprintf(" Closing...\n");
					ret = sceIoClose(fd);
					if (ret) {
						pspDebugScreenKprintf("  -> Failed 0x%08X\n", ret);
					}

					pspDebugScreenPuts("Deinitialzing...\n"
						" Stopping Module...\n");
					sceKernelStopModule(modid, 0, NULL, NULL, NULL);
					pspDebugScreenPuts(" Unloading Module...\n");
					sceKernelUnloadModule(modid);
					waitExit();
					return ret;
				}
				p++;
			}
		}
	}

	pspDebugScreenPuts(" -> BIOS Not Found\n"
		"Deinitialzing...\n"
		" Stopping Module...\n");
	sceKernelStopModule(modid, 0, NULL, NULL, NULL);
	pspDebugScreenPuts(" Unloading Module...\n");
	sceKernelUnloadModule(modid);
	waitExit();
	return -2;
}

int module_start(SceSize arglen, void *argp)
{
	SceUID ret;

	thid = sceKernelCreateThread(module_info.modname, mainThread, 16, 4096, 0, NULL);
	if (thid < 0) {
		sceKernelSelfStopUnloadModule(1, 0, NULL);
		return thid;
	}

	ret = sceKernelStartThread(thid, arglen, argp);
	if (ret) {
		thid = -1;
		sceKernelDeleteThread(thid);
		sceKernelSelfStopUnloadModule(1, 0, NULL);
	}

	return ret;
}

int module_stop()
{
	if (thid >= 0)
		sceKernelTerminateDeleteThread(thid);
	return 0;
}

