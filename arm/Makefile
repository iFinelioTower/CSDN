PROJECT_DIR := $(shell pwd)

# Global
SDK_DIR = /home/luwenjiang/3128linux
TOOLSCHAIN_DIR = $(SDK_DIR)/buildroot/output/rockchip_rk3128_game

# CONFIG
CC = $(TOOLSCHAIN_DIR)/host/bin/arm-buildroot-linux-gnueabihf-gcc
STAGING_DIR = $(TOOLSCHAIN_DIR)/host/arm-buildroot-linux-gnueabihf/sysroot
ALSA_INCLUDE_DIR = $(TOOLSCHAIN_DIR)/build/alsa-lib-1.1.5
DESTDIR:=$(TOOLSCHAIN_DIR)/target

# OUTPUT
BIN = sqlite_test
      
OBJ = sqlite_test.o
      
CFLAGS =   -I./include \
	  -I$(STAGING_DIR)/usr/include \
	  -I$(ALSA_INCLUDE_DIR)/include \
      -I$(STAGING_DIR)/usr/include/libdrm \
      -I$(STAGING_DIR)/usr/include \
	  -L$(STAGING_DIR)/usr/lib \
      -L./build/lib/ \
      -I./build/include/ \
      -lsqlite3 \
	  -lpthread  -ldrm -lminigui_ths -ljpeg -lpng -lm \
	  -lavformat -lavcodec -lswscale -lavutil -lfreetype -lasound
	  
#install:
#	$(MAKE) $(BIN) install-am

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ) $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
    
clean:
	rm -rf $(OBJ) $(BIN)