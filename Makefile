
K1DIR = /home1/ar2366_kernel/
KDIR = /home1/gplus33/kernel/
#PWD=/home/doris
PWD := $(shell pwd)


ifeq "$(CROSS)" "arm-none-linux-gnueabi-"
MODULE_NAME = i2c-adap-gpl8300
obj-m := $(MODULE_NAME).o gp_gpio_test.o gpio.o 
endif


ifeq "$(CROSS)" "arm-linux-androideabi-"
MODULE_NAME = pixcir_i2c_ts
obj-m := pixcir_i2c_ts.o
endif



CC = $(CROSS)gcc
CPP = $(CROSS)g++
LD = $(CROSS)ld

STRIPTOOL = $(CROSS)strip
STRIP   = $(STRIPTOOL) --remove-section=.note --remove-section=.comment $(NAME)

AR	= $(CROSS)ar

CFLAGS	= 
#DEBUG	= -g -Wall
#DEBUG	= -O2 -Wall -fPIC -static
DEBUG	= -Wall -static
IFLAGS 	= -I. -I./include -L. -L./libs
LDFLAG	= -lpthread -lrt

ifeq "$(CROSS)" "arm-none-linux-gnueabi-"
#LDFLAG	= -lpthread -lrt -lasound
LDFLAG	= -lpthread -lrt -ltinyalsa
endif


RHYTHM_DEPEND_FILES = RhythmMain.o Event.o BaseApp.o RhythmApp.o Misc.o
FPC_DEPEND_FILES = FpcMain.o Event.o FpcTb.o FpcApp.o FpcData.o Misc.o BaseApp.o Sd55App.o Sd55Data.o
DORIS_DEPEND_FILES = FifoMain.o Event.o App.o DorisApp.o Pool.o Misc.o FiFoApp.o
TARGET = doris i2c_dump spidev_fdx spidev_test fpc serialio ks mbreak qs pwm_ctrl AdConvert \
			rhythm JkFreqSet JkSpeedSet JkSpuGet JkInclineSet JkCalibSet JkParamGet JkParamSet \
			wsc

ifeq "$(CROSS)" "arm-none-linux-gnueabi-"
#TARGET += $(MODULE_NAME).ko
#TARGET += modules
endif


ifeq "$(CROSS)" "arm-linux-androideabi-"
#TARGET += $(MODULE_NAME).ko
TARGET = modules
IFLAGS 	= -I. -I/opt/android-ndk-r8/platforms/android-4/arch-mips/usr/include
endif


all: $(TARGET)

wsc: wsc.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) wsc.o $(LDFLAG)

JkParamSet: JkParamSet.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) JkParamSet.o $(LDFLAG)

JkParamGet: JkParamGet.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) JkParamGet.o $(LDFLAG)

JkCalibSet: JkCalibSet.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) JkCalibSet.o $(LDFLAG)

JkInclineSet: JkInclineSet.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) JkInclineSet.o $(LDFLAG)

JkSpuGet: JkSpuGet.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) JkSpuGet.o $(LDFLAG)

JkSpeedSet: JkSpeedSet.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) JkSpeedSet.o $(LDFLAG)

JkFreqSet: JkFreqSet.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) JkFreqSet.o $(LDFLAG)

AdConvert: AdConvert.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) AdConvert.o $(LDFLAG)

pwm_ctrl: pwm_ctrl.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) pwm_ctrl.o $(LDFLAG)

mbreak: mbreak.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) mbreak.o $(LDFLAG)

qs: QsMain.o Event.o BaseApp.o Misc.o QsApp.o Event.o FpcTb.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) QsMain.o Event.o BaseApp.o Misc.o QsApp.o FpcTb.o $(LDFLAG)

ks: KsMain.o Event.o BaseApp.o Pool.o Misc.o KsApp.o Event.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) KsMain.o Event.o BaseApp.o Misc.o KsApp.o $(LDFLAG)

serialio: serialio.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) serialio.o $(LDFLAG)

spidev_test: spidev_test.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) spidev_test.o $(LDFLAG)

spidev_fdx: spidev_fdx.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) spidev_fdx.o $(LDFLAG)

rhythm: $(RHYTHM_DEPEND_FILES)
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $(RHYTHM_DEPEND_FILES) $(LDFLAG)

doris: $(DORIS_DEPEND_FILES)
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $(DORIS_DEPEND_FILES) $(LDFLAG)

fpc: $(FPC_DEPEND_FILES)
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $(FPC_DEPEND_FILES) $(LDFLAG)

i2c_dump: i2c_dump.o
	$(CPP) -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) i2c_dump.o $(LDFLAG)

.c.o:
	$(CC) -c -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $<

.cpp.o:
	$(CPP) -c -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $<



ifeq "$(CROSS)" "arm-linux-androideabi-"

pixcir_i2c_ts.ko:
	make -C $(K1DIR) M=$(PWD) V=1 modules

modules:
	make -C $(K1DIR) M=$(PWD) V=1 modules

endif



ifeq "$(CROSS)" "arm-none-linux-gnueabi-"

i2c-adap-gpl8300.ko:
	#make -C $(KDIR) M=$(PWD) V=1 ARCH=arm CROSS_COMPILE=$(CROSS) modules
	make -C $(KDIR) M=$(PWD) V=1 modules

gpio.ko:
	make -C $(KDIR) M=$(PWD) V=1 modules

gp_gpio_test.ko:
	make -C $(KDIR) M=$(PWD) V=1 modules

modules:
	make -C $(KDIR) M=$(PWD) V=1 modules

endif



clean:
	rm -rf *.o *.a core a.out out.txt out.bin nohup* .#* $(TARGET) test *.ko *.cmd *.mod.c .tmp_versions modules.order Module.symvers *.mod.*

