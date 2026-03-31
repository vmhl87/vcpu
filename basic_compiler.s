# byte line_buffer[64];				line_buffer@0-63
null 64
# int line_num;						line_num@64-67
( 0 )
# int stack_top;					stack_top@68-71
( 0 )
# char *func = "func\0";			func@72-76
( "func" 0x00 )


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


# arg1: -5, ptr: -4
print_byte:
# out("0x");
out "0x"
# int i = 0 + arg1;				i@0-3
set @0 0
add1 @0 @-5
# int j = i/16;					j@4-7
set @4 @0
div @4 16
# if(j >= 10) goto alpha_0;
set @8 @4
sub @8 10
comp @12 @8 >=
cond print_byte_0 @12
# byte k = '0' + j;
set @8 "0"
add1 @8 @4
# out(k);
out @8 1
# goto end_0
goto print_byte_1
# alpha_0:
print_byte_0:
# byte k = 'a' + j - 10;
set @8 "a"
add1 @8 @4
sub1 @8 0x0a
# out(k);
out @8 1
# end_0;
print_byte_1:
# i -= j*16;
mul @4 16
sub @0 @4
# j = i;
set @4 @0
# if(j >= 10) goto alpha_1;
set @8 @4
sub @8 10
comp @12 @8 >=
cond print_byte_2 @12
# byte k = '0' + j;
set @8 "0"
add1 @8 @4
# out(k);
out @8 1
# goto end_1
goto print_byte_3
# alpha_1:
print_byte_2:
# byte k = 'a' + j - 10;
set @8 "a"
add1 @8 @4
sub1 @8 0x0a
# out(k);
out @8 1
# end_1;
print_byte_3:
# return;
ret


# ret: -13, addr1: -12, addr2: -8, ptr: -4
streq:
# loop_0:
streq_0:
# byte char = *addr1;				char@0-0
load &-12 @0 1
# byte ref = *addr2;				ref@1-1
load &-8 @1 1
# if(ref == 0) goto ret_yes;
comp1 @2 @1 =
cond streq_1 @2
# if(char == 0) goto ret_no;
comp1 @2 @0 =
cond streq_2 @2
# if(char != ref) goto ret_no;
set @2 @0 1
sub1 @2 @1
cond streq_2 @2
# addr1++;
add @-12 1
# addr2++;
add @-8 1
# goto loop_0;
goto streq_0
# ret_yes:
streq_1:
# return 0x01;
set @-13 0x01
ret
# ret_no:
streq_2:
# return 0x00;
set @-13 0x00
ret


process_line:
out "process_line called\n"
# if(!streq(func, line_buffer)) goto not_func;
set @1 0
set @5 72
call streq 13
nor @0 @0
cond process_line_0 @0
# out("found a function on line %d!\n", line_num);
out "found a function on line "
set @0 &64
call print_int 8
out "!\n"
# do function things
# return;
ret
# not_func:
process_line_0:
ret


main:
# int line_length = 0;					line_length@0-3
set @0 0
# read_loop:
main_0:
# byte char = in();						char@4-4
in @4 1
# if(char != EOF) goto not_exit;
comp1 @5 @4 !=
cond main_1 @5
# line_num++;
add &64 1
# process_line();
call process_line 9
# exit();
exit
# not_exit:
main_1:
# if(char != '\n') goto not_enter;
set @5 @4 1
sub1 @5 "\n"
comp1 @6 @5 !=
cond main_3 @6
# line_num++;
add &64 1
# process_line();
call process_line 9
# clear_loop:
main_2:
# line_length--;
sub @0 1
# line_buffer[line_length] = 0x00;
set @5 0x00
save &0 @5 1
# if(line_length > 0) goto clear_loop;
comp @5 @0 !<=
cond main_2 @5
# goto read_loop;
goto main_0
# not_enter:
main_3:
# line_buffer[line_length] = char;
save &0 @4 1
# line_length++;
add @0 1
# if(line_length != 64) goto read_loop;
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
