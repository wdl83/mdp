$(if $(MAKE_UTILS),,$(error MAKE_UTILS is not defined))

TARGET = libmdp_client

PUBLIC_HEADERS = include/mdp

CXXFLAGS += \
	-I include

CXXSRCS = \
	src/Client.cpp \
	src/ZMQClientContext.cpp

include $(MAKE_UTILS)/Makefile.a_rules
