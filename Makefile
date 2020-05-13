all: Makefile.broker Makefile.client Makefile.echo_worker
	make -f Makefile.broker
	make -f Makefile.echo_worker
	make -f Makefile.client

clean:
	rm *.o -f
	rm broker.elf -f
	rm echo_worker.elf -f
	rm client.elf -f
