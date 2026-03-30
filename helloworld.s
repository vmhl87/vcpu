main:
out "hello world\n"
out "what is your name? "

set @17 0
read_next_char:
in @16 1
save @17 @16 1
add @17 1

set @21 @16 1
sub1 @21 0x0a
add1 @21 0x00
cond read_next_char @21

sub @17 1
out "hello, "

set @21 0
write_next_char:
load @21 @25 1
add @21 1
out @25 1

set @25 @21
sub @25 @17
comp @29 @25 !=
cond write_next_char @25

out "!\n"
exit
