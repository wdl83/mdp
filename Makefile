all: Makefile.broker Makefile.client Makefile.worker
	make -f Makefile.broker
	make -f Makefile.worker
	make -f Makefile.client

clean:
	rm *.o -f
	rm broker.elf -f
	rm worker.elf -f
	rm client.elf -f
