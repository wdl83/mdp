$(if $(MAKE_UTILS),,$(error MAKE_UTILS is not defined))

TARGET = mdp_client

LDFLAGS += \
	-Wl,--start-group \
	-lmdp_common \
	-lmdp_client \
	-Wl,--end-group \
	-lzmqpp \
	-lzmq \
	-lstdc++

CXXSRCS = \
	src/client.cpp

include $(MAKE_UTILS)/Makefile.rules
