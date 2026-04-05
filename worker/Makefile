$(if $(MAKE_UTILS),,$(error MAKE_UTILS is not defined))

TARGET = libmdp_worker

PUBLIC_HEADERS = include/mdp

CXXFLAGS += \
	-I include

CXXSRCS = \
	src/Worker.cpp \
	src/WorkerTask.cpp \
	src/ZMQWorkerContext.cpp

include $(MAKE_UTILS)/Makefile.a_rules
