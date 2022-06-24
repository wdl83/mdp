ifndef OBJ_DIR
OBJ_DIR = ${PWD}/obj
export OBJ_DIR
endif

ifndef DST_DIR
DST_DIR = ${PWD}/dst
export DST_DIR
endif

all: 
	make -C zmqpp
	mkdir -p ${OBJ_DIR}/zmqpp
	make PREFIX=${OBJ_DIR}/zmqpp install -C zmqpp
	make -f Makefile.broker
	make -f Makefile.echo_worker
	make -f Makefile.client

install:
	make -C zmqpp
	mkdir -p ${OBJ_DIR}/zmqpp
	make PREFIX=${OBJ_DIR}/zmqpp install -C zmqpp
	make PREFIX=${DST_DIR} install -C zmqpp
	make -f Makefile.broker install
	make -f Makefile.echo_worker install
	make -f Makefile.client install

clean:
	make -f Makefile.broker clean
	make -f Makefile.echo_worker clean
	make -f Makefile.client clean

purge:
	rm $(OBJ_DIR) -rf
