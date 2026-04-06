$(if $(MAKE_UTILS),,$(error MAKE_UTILS is not defined))

TARGET = test_mdp_common

LDFLAGS += \
	-Wl,--start-group \
	-lmdp_common \
	-Wl,--end-group \
	-lzmqpp \
	-lzmq \
	-lgtest \
	-lgtest_main \
	-lm \
	-lstdc++ 

CXXSRCS = \
	src/MutualHeartbeatMonitor_tests.cpp \
	src/ZMQIdentity_tests.cpp \
	src/utils_tests.cpp

include $(MAKE_UTILS)/Makefile.rules
