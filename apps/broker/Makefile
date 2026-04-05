$(if $(MAKE_UTILS),,$(error MAKE_UTILS is not defined))

TARGET = mdp_broker

LDFLAGS += \
	-Wl,--start-group \
	-lmdp_common \
	-lmdp_broker \
	-Wl,--end-group \
	-lzmqpp \
	-lzmq \
	-lstdc++

CXXSRCS = \
	src/broker.cpp

include $(MAKE_UTILS)/Makefile.rules
