all: Makefile.broker Makefile.client Makefile.echo_worker
	mkdir -p zmqpp-root
	make -C zmqpp
	make PREFIX=${PWD}/zmqpp-root install -C zmqpp
	make -f Makefile.broker
	make -f Makefile.echo_worker
	make -f Makefile.client

clean:
	make -f Makefile.broker clean
	make -f Makefile.echo_worker clean
	make -f Makefile.client clean
