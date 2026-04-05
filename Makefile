export MAKE_UTILS := $(PWD)/make_utils

include $(MAKE_UTILS)/Makefile.defs

export INSTALL_DIR
export INSTALL_INCLUDE_DIR := $(INSTALL_DIR)/include
export INSTALL_LIB_DIR := $(INSTALL_DIR)/lib
export BUILD_DIR
CFLAGS += \
	-I $(PWD)/modules \
	-I $(INSTALL_INCLUDE_DIR) \
	-DENABLE_TRACE
export CFLAGS
CXXFLAGS += \
	-I $(PWD)/modules \
	-I $(INSTALL_INCLUDE_DIR) \
	-DENABLE_TRACE
export CXXFLAGS
LDFLAGS += \
	-L $(INSTALL_LIB_DIR)
export LDFLAGS

all: purge clean_zmqpp build_broker build_client build_echo_worker
install: purge clean_zmqpp install_broker install_client install_echo_worker

# BEGIN DEPS: zmqpp library ---------------------------------------------------#
clean_zmqpp: 
	make clean -C modules/zmqpp

compile_zmqpp: 
	make -C modules/zmqpp

install_zmqpp: compile_zmqpp
	mkdir -p $(INSTALL_DIR)
	make PREFIX=$(INSTALL_DIR) install -C modules/zmqpp
# END DEPS: zmqpp library -----------------------------------------------------#

# BEGIN LIBS ------------------------------------------------------------------#
build_libcommon: install_zmqpp
	make -C common

install_libcommon: build_libcommon
	make install -C common

build_libbroker: install_libcommon
	make -C broker

install_libbroker: build_libbroker
	make install -C broker

build_libworker: install_libcommon
	make -C worker

install_libworker: build_libworker
	make install -C worker

build_libclient: install_libcommon
	make -C client

install_libclient: build_libclient
	make install -C client
# END LIBS --------------------------------------------------------------------#

# BEGIN APPS ------------------------------------------------------------------#
build_broker: install_libcommon install_libbroker
	make -C apps/broker

install_broker: build_broker
	make install -C apps/broker

build_client: install_libcommon install_libclient
	make -C apps/client

install_client: build_client
	make install -C apps/client

build_echo_worker: install_libcommon install_libworker
	make -C apps/echo_worker

install_echo_worker: build_echo_worker
	make install -C apps/echo_worker
# END APPS --------------------------------------------------------------------#

clean:
	rm $(BUILD_DIR) -rf

purge:
	rm $(BUILD_DIR) -rf
	rm $(INSTALL_DIR) -rf
