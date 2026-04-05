$(if $(MAKE_UTILS),,$(error MAKE_UTILS is not defined))

TARGET = libmdp_common

PUBLIC_HEADERS = include/mdp

CXXFLAGS += \
	-I include

CXXSRCS = \
	src/MutualHeartbeatMonitor.cpp \
	src/ZMQIdentity.cpp

include $(MAKE_UTILS)/Makefile.a_rules
