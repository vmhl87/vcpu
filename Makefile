all:
	gcc vcpu.c -o ./cpu -Wall -O2

debug:
	gcc vcpu.c -o ./cpu -Wall -g
