# byte line_buffer[64];				line_buffer@0-63
null 64
# int line_num;						line_num@64-67
( 0 )


# arg1: -8, ptr: -4
print_int:
# if(arg1 >= 0) goto label_0;
comp @0 @-8 >=
cond print_int_0 @0
#   out('-');
out "-"
#   arg1 *= -1;
mul @-8 -1
# label_0:
print_int_0:
# byte res[10];
# int count = 0;					count@10-13
set @10 0
# int i = arg1;						i@14-17
set @14 @-8
# label_1:
print_int_1:
#   int j = i/10;					j@18-21
set @18 @14
div @18 10
#   i -= j*10;
set @22 @18
mul @22 10
sub @14 @22
#   res[count] = '0' + i;
set @26 "0"
add @26 @14
save @10 @26 1
#   count++;
add @10 1
#   i = j;
set @14 @18
# if(i != 0) goto label_1;
comp @22 @14 !=
cond print_int_1 @22
# label_2:
print_int_2:
#   count--;
sub @10 1
#   byte l = res[count];
load @10 @22 1
#   out(l);
out @22 1
# if(count != 0) goto label_2;
comp @22 @10 !=
cond print_int_2 @22
# return;
ret


process_line:
out "process_line called\n"
ret


main:
# int line_length = 0;					line_length@0-3
set @0 0
# label_0:
main_0:
# byte char = in();						char@4-4
in @4 1
# if(char != EOF) goto label_1;
comp1 @5 @4 !=
cond main_1 @5
# line_num++;
add &64 1
# process_line();
call process_line 9
# exit();
exit
# label_1:
main_1:
# if(char != '\n') goto label_2;
set @5 @4 1
sub1 @5 "\n"
comp1 @6 @5 !=
cond main_2 @6
# line_num++;
add &64 1
# process_line();
call process_line 9
# line_length = 0;
set @0 0
# goto label_0;
goto main_0
# label_2:
main_2:
# line_buffer[line_length] = char;
save &0 @5 1
# line_length++;
add @0 1
# if(line_length != 64) goto label_0;
set @5 @0
sub @5 64
comp @9 @5 !=
cond main_0 @9
# out("line buffer length exceeded on line %d\n", line_num);
out "line buffer length exceeded on line "
set @5 &64
call print_int 13
out 0x0a
# exit();
exit
