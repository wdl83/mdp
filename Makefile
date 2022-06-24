ifndef OBJ_DIR
OBJ_DIR = ${PWD}/obj
endif

all: Makefile.broker Makefile.client Makefile.echo_worker
	make -C zmqpp
	mkdir -p ${OBJ_DIR}/zmqpp
	make PREFIX=${OBJ_DIR}/zmqpp install -C zmqpp
	make -f Makefile.broker
	make -f Makefile.echo_worker
	make -f Makefile.client

clean:
	make -f Makefile.broker clean
	make -f Makefile.echo_worker clean
	make -f Makefile.client clean
