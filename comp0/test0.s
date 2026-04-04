func
main:

out "hello world!\nenter your name: "

var .name 32
var .i 4
set .i 0

loop1:
tmp .char 1
in .char 1
tmp .cmp 1
set .cmp .char 1
sub1 .cmp "\n"
nor .cmp .cmp
cond print .cmp
save@ .i .char 1
add .i 1
goto loop1
done

print:
out "hello "

var .j 4
set .j 0
loop2:
tmp .k 4
set .k .j
sub .k .i
tmp .b 1
comp .b .k =
cond end .b
done
tmp .char 1
load@ .j .char 1
out .char 1
add .j 1
goto loop2

end:
out "!\n"
exit
