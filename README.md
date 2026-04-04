A simple 32-bit virtual cpu, implementing a von neumann stack machine architecture.

Its instruction set and assembly format are very minimal. Heap allocations are not implemented (yet).

`vcpu.c` compiles into an executable that handles execution and assembling.
For example, to run the following "hello world" program:

```asm
main:
out "hello world\nwhat is your name? "

set @17 0
read_next_char:
in @16 1
save@ @17 @16 1
add @17 1

set @21 @16 1
sub1 @21 0x0a
add1 @21 0x00
cond read_next_char @21

sub @17 1
out "hello, "

set @21 0
write_next_char:
load@ @21 @25 1
add @21 1
out @25 1

set @25 @21
sub @25 @17
comp @29 @25 !=
cond write_next_char @25

out "!\n"
exit
```

`./cpu -a hello_world.s hello_world.o` produces the bytecode file `hello_world.o`.
Its contents can be printed using, say, the unix `hexdump` utility:

```
00000000  c3 00 00 00 00 00 00 00  58 1f 00 00 00 68 65 6c  |........X....hel|
00000010  6c 6f 20 77 6f 72 6c 64  0a 77 68 61 74 20 69 73  |lo world.what is|
00000020  20 79 6f 75 72 20 6e 61  6d 65 3f 20 24 11 00 00  | your name? $...|
00000030  00 00 00 00 00 45 10 00  00 00 d9 11 00 00 00 10  |.....E..........|
00000040  00 00 00 a3 11 00 00 00  01 00 00 00 15 15 00 00  |................|
00000050  00 10 00 00 00 a9 15 00  00 00 0a a1 15 00 00 00  |................|
00000060  00 6a 2d 00 00 00 15 00  00 00 ab 11 00 00 00 01  |.j-.............|
00000070  00 00 00 58 07 00 00 00  68 65 6c 6c 6f 2c 20 24  |...X....hello, $|
00000080  15 00 00 00 00 00 00 00  d1 15 00 00 00 19 00 00  |................|
00000090  00 a3 15 00 00 00 01 00  00 00 49 19 00 00 00 25  |..........I....%|
000000a0  19 00 00 00 15 00 00 00  9b 19 00 00 00 11 00 00  |................|
000000b0  00 bc 1d 00 00 00 19 00  00 00 6a 80 00 00 00 19  |..........j.....|
000000c0  00 00 00 58 02 00 00 00  21 0a 71                 |...X....!.q|
000000cb
```

`./cpu -x hello_world.o` executes the assembled program, yielding:

```
hello world
what is your name? vmhl87
hello, vmhl87!
```

A few more options are available:

- `./cpu -ax program.s`: assembles and executes immediately, without creating a bytecode file.

- Adding `v` to the flag enables verbose logging, e.g. `./cpu -xv program.o` logs executed instructions to stdout.

A short assembly/bytecode reference is available in [reference.txt](reference.txt).
