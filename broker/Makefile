$(if $(MAKE_UTILS),,$(error MAKE_UTILS is not defined))

TARGET = libmdp_broker

PUBLIC_HEADERS = include/mdp

CXXFLAGS += \
	-I include

CXXSRCS = \
	src/Broker.cpp \
	src/ZMQBrokerContext.cpp

include $(MAKE_UTILS)/Makefile.a_rules
