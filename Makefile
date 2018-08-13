all: Makefile.broker Makefile.client Makefile.worker
	make -f Makefile.broker
	make -f Makefile.worker
	make -f Makefile.client

clean:
	rm -f broker.elf
	rm -f worker.elf
	rm -f client.elf
