PWD = $(shell pwd)

ARCH = arm
CC = arm-linux-gnueabihf-gcc

BUILD_DIR = $(PWD)/app

# Project name
PROJ_NAME = melsec_driver
OUTPATH = $(PWD)/app
PROJ_PATH = $(PWD)

include $(PROJ_PATH)/melsec_mc_net/module.mk
include $(PROJ_PATH)/data_comm/module.mk

COBJS += $(CFILES:%.c=$(BUILD_DIR)/%.o)
CDEPS += $(CFILES:%.c=$(BUILD_DIR)/%.d)

LDLIBS += -L$(HIREDIS_DIR)/lib -lhiredis -pthread

ALLFLAGS = -g -Os

CFLAGS += -I/home/opt/hiredis/include
CFLAGS += $(ALLFLAGS) -MD -g3 -Wall -ggdb -std=gnu99 -rdynamic -funwind-tables -ffunction-sections 

.PHONY: proj clean

all: proj
	@mkdir -p $(BUILD_DIR)
	@echo ""


proj: $(OUTPATH)/$(PROJ_NAME)

$(OUTPATH)/$(PROJ_NAME): $(COBJS) $(LIBS)
	@echo Linking...
	$(CC) -o$@ $(COBJS) $(CFILES)

clean:
	rm -rf $(BUILD_DIR)


$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo Build... $$(basename $@)
	@echo Build... $@ >> $(BUILD_LOG)
	@if [ -e "$@" ]; then rm -f "$@"; fi
	@if [ -n "$(OVERRIDE_CFLAGS)" ]; then \
		echo $(CC) $(OVERRIDE_CFLAGS) $@ >> $(BUILD_LOG); \
		$(CC) $(OVERRIDE_CFLAGS) -c $< -o $@ 2>>$(ERR_LOG); \
	else \
		echo $(CC) $(CFLAGS) $@ >> $(BUILD_LOG); \
		$(CC) $(CFLAGS) -c $< -o $@ 2>>$(ERR_LOG); \
	fi; \
	if [ "$$?" != "0" ]; then \
		echo "Build... $$(basename $@) FAIL"; \
		echo "Build... $@ FAIL" >> $(BUILD_LOG); \
	else \
		echo "Build... $$(basename $@) PASS"; \
		echo "Build... $@ PASS" >> $(BUILD_LOG); \
	fi;

$(BUILD_DIR)/%.d: $(SOURCE_DIR)/%.c
	@mkdir -p $(dir $@)
	@set -e; rm -f $@; \
	export D_FILE="$@"; \
	export B_NAME=`echo $$D_FILE | sed 's/\.d//g'`; \
	if [ -n "$(OVERRIDE_CFLAGS)" ]; then \
		$(CC) -MM $(OVERRIDE_CFLAGS) $< > $@.$$$$; \
	else \
		$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	fi; \
	sed 's@\(.*\)\.o@'"$$B_NAME\.o $$B_NAME\.d"'@g' < $@.$$$$ > $@; \
	rm -f $@.$$$$


