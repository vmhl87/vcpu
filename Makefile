all:
	gcc vcpu.c -o ./cpu -Wall -O2

debug:
	gcc vcpu.c -o ./cpu -Wall -g

comp0:
	./cpu -ax comp0/compiler.s comp0/.compiler.o
	cp comp0/.env comp0/env
