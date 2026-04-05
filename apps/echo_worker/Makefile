$(if $(MAKE_UTILS),,$(error MAKE_UTILS is not defined))

TARGET = mdp_echo_worker

LDFLAGS += \
	-Wl,--start-group \
	-lmdp_common \
	-lmdp_worker \
	-Wl,--end-group \
	-lzmqpp \
	-lzmq \
	-lstdc++

CXXSRCS = \
	src/worker.cpp

include $(MAKE_UTILS)/Makefile.rules
