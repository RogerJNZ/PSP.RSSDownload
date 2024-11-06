TARGET = DownloadLinks
OBJS = fileOperation.o network.o main.o

#To build for custom firmware:
BUILD_PRX = 1
PSP_FW_VERSION=371
PSP_EBOOT_ICON= downloadlinks.png

#CFLAGS = -O0 -G0 -Wall -g 
#CFLAGS = -O3 -frename-registers -G0 -Wall -E
CFLAGS = -O3 -frename-registers -G0 -Wall 
CXXFLAGS = $(CFLAGS) -fno-exceptions 
#CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBDIR =
STDLIBS= -losl -lpng -lz \
         -lcurl -lpsphprm -lpspsdk -lpspctrl -lpspumd -lpsprtc -lpsppower -lpspgu -lpspgum  -lpspaudiolib -lpspaudio -lpsphttp -lpspssl -lpspwlan \
         -lpspnet_adhocmatching -lpspnet_adhoc -lpspnet_adhocctl -lm -ljpeg  -lpsputility 
LIBS = $(STDLIBS)
LDFLAGS =
EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = DownloadLinks 4.3.0
#PSP_EBOOT_ICON = ICON0.PNG
PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
