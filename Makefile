TARGET = pops_bios_dumper
OBJS = main.o sceCtrl_driver.o

PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

CFLAGS = -O2 -G0 -Wall
LDFLAGS = -nostartfiles

LIBS = -lpspdebug -lpspge

BUILD_PRX = 1
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = POPS BIOS Dumper by 173210

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
