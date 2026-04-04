# byte line_buffer[64];							line_buffer@0-63
null 64
# int line_num;									line_num@64-67
( 0 )
# int stack_top;								stack_top@68-71
( 0 )
# int stack_tmp_top;							stack_tmp_top@72-75
( 0 )
# int func_header;								func_header@76-79
( 0 )
# struct textentry { char name[16], int data };
# textentry map_data[64];						map_data@80-1359
null 1280
# int map_top;									map_top@1360-1363
( 0 )
# int map_tmp_top;								map_tmp_top@1364-1367
( 0 )
# char *func = "func\0";						func@1368-1372
( "func" 0x00 )
# char *ret = "ret\0";							ret@1373-1376
( "ret" 0x00 )
# char *arg = "arg\0";							arg@1377-1380
( "arg" 0x00 )
# char *var = "var\0";							var@1381-1384
( "var" 0x00 )
# char *tmp = "tmp\0";							tmp@1385-1388
( "tmp" 0x00 )
# char *call = "tmp\0";							call@1389-1393
( "call" 0x00 )
# char *done = "tmp\0";							done@1394-1398
( "done" 0x00 )


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
save@ @10 @26 1
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
load@ @10 @22 1
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


# str: -8, ptr: -4
print_str:
# byte* ptr = str;						ptr@0-3
set @0 @-8
# loop:
print_str_0:
# byte char = *ptr;						char@4-4
load& @0 @4 1
# if(char != 0x00) goto not_null;
cond print_str_1 @4
# return;
ret
# not_null:
print_str_1:
# out(char);
out @4 1
# ptr++;
add @0 1
# goto loop;
goto print_str_0


# ret: -13, addr1: -12, addr2: -8, ptr: -4
streq:
# loop_0:
streq_0:
# byte char = *addr1;				char@0-0
load& @-12 @0 1
# byte ref = *addr2;				ref@1-1
load& @-8 @1 1
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


# str: -12, val: -8, ptr: -4
insert_map:
# memcpy(map_data + map_tmp_top*20, str, 16)
set @0 @-12
load& @0 @4 16
set @0 &1364
mul @0 20
add @0 80
save& @0 @4 16
# *(map_data + map_tmp_top*20 + 16) = val;
set @0 &1364
mul @0 20
add @0 16
add @0 80
save& @0 @-8
# map_tmp_top++;
add &1364 1
# return;
ret


# ret: -12, str: -8, ptr: -4
find_map:
# int i = 0;											i@0-3
set @0 0
# loop:
find_map_0:
# if(i != map_tmp_top) goto process;
set @4 @0
sub @4 &1364
comp @8 @4 !=
cond find_map_1 @8
# out("error on line %d: variable not found: %s\n", line_num, str);
out "error on line "
set @0 &64
call print_int 8
out ": variable not found: "
set @0 @-8
call print_str 8
out 0x0a
# exit;
exit
# process:
find_map_1:
# if(!streq(map_data + i*20, str)) goto not_found;
set @5 @0
mul @5 20
add @5 80
set @9 @-8
call streq 17
nor @4 @4
cond find_map_2 @4
# return *(map_data + i*20 + 16);
mul @0 20
add @0 80
add @0 16
load& @0 @4
set @-12 @4
ret
# not_found:
find_map_2:
# i++;
add @0 1
# goto loop;
goto find_map_0


# ret: -12, str: -8, ptr: -4
split:
# byte* ptr = str;						ptr@0-3
set @0 @-8
# loop:
split_0:
# byte char = *ptr;						char@4-4
load& @0 @4 1
# if(char != 0x00) goto not_null;
cond split_1 @4
# return ptr+1;
set @-12 @0
add @-12 1
ret
# not_null:
split_1:
# if(char != 0x0a) goto not_space;
set @5 @4 1
sub1 @5 0x20
cond split_2 @5
# *ptr = 0x00;
set @5 0x00
save& @0 @5 1
# return ptr+1;
set @-12 @0
add @-12 1
ret
# not_space;
split_2:
# ptr++;
add @0 1
# goto loop;
goto split_0


# ret: -12, str: -8, ptr: -4
parse_int:
# int res = 0;									res@0-3
set @0 0
# byte* ptr = str;								ptr@4-7
set @4 @-8
# loop:
parse_int_0:
# byte char = *ptr;								char@8-8
load& @4 @8
# if(char >= '0' && char <= '9') goto digit;
set @9 0
set @9 @8 1
set @13 @9
sub @13 "0"
comp @17 @13 >=
set @13 @9
sub @13 "9"
comp @18 @13 <=
and @17 @18
cond parse_int_1 @17
# return res;
set @-12 @0
ret
# digit:
parse_int_1:
# res = res*10 + (char - '0');
set @9 0
set @9 @8 1
sub @9 "0"
mul @0 10
add @0 @9
# ptr++;
add @4 1
# goto loop;
goto parse_int_0


process_line:
# if(!streq(func, line_buffer)) goto not_func;
set @1 0
set @5 1368
call streq 13
nor @0 @0
cond process_line_0 @0
# stack_top = 4;
set &68 4
# stack_tmp_top = 4;
set &72 4
# func_header = 4;
set &76 4
# map_top = 0;
set &1360 0
# map_tmp_top = 0;
set &1364 0
# return;
ret
# not_func:
process_line_0:
# if(!streq(ret, line_buffer)) goto not_ret;
set @1 0
set @5 1373
call streq 13
nor @0 @0
cond process_line_1 @0
# do return value things
# return;
ret
# not_ret:
process_line_1:
# if(!streq(arg, line_buffer)) goto not_arg;
set @1 0
set @5 1377
call streq 13
nor @0 @0
cond process_line_2 @0
# do argument things
# return;
ret
# not_arg:
process_line_2:
# if(!streq(var, line_buffer)) goto not_var;
set @1 0
set @5 1381
call streq 13
nor @0 @0
cond process_line_3 @0
# int arg1 = split(0);								arg1@0-3
set @4 0
call split 12
# int arg2 = split(arg1);							arg2@4-7
set @8 @0
call split 16
# split(arg2);
set @12 @4
call split 20
# byte* loc = line_buffer+arg1;						loc@8-11
set @8 @0
# int size = parse_int(line_buffer+arg2);			size@12-15
set @16 @4
call parse_int 24
# map_tmp_top = map_top;
set &1364 &1360
# insert_map(loc, stack_top);
set @16 @8
set @20 &68
call insert_map 28
# stack_top += size;
add &68 @12
# stack_tmp_top = stack_top;
set &72 &68
# map_top = map_tmp_top;
set &1360 &1364
# return;
ret
# not_var:
process_line_3:
# if(!streq(tmp, line_buffer)) goto not_tmp;
set @1 0
set @5 1385
call streq 13
nor @0 @0
cond process_line_4 @0
# int arg1 = split(0);								arg1@0-3
set @4 0
call split 12
# int arg2 = split(arg1);							arg2@4-7
set @8 @0
call split 16
# split(arg2);
set @12 @4
call split 20
# byte* loc = line_buffer+arg1;						loc@8-11
set @8 @0
# int size = parse_int(line_buffer+arg2);			size@12-15
set @16 @4
call parse_int 24
# insert_map(loc, stack_tmp_top);
set @16 @8
set @20 &72
call insert_map 28
# stack_tmp_top += size;
add &72 @12
# return;
ret
# not_tmp:
process_line_4:
# if(!streq(call, line_buffer)) goto not_call;
set @1 0
set @5 1389
call streq 13
nor @0 @0
cond process_line_5 @0
# byte* j = split(0);					j@0-3
set @4 0
call split 12
# split(j);
set @8 @0
call split 16
# out("call %s %d\n", j, stack_tmp_top - func_header + 4);
out "call "
call print_str 8
out 0x20
set @0 &72
sub @0 &76
add @0 4
call print_int 8
out 0x0a
# return;
ret
# not_call:
process_line_5:
# if(!streq(done, line_buffer)) goto normal;
set @1 0
set @5 1394
call streq 13
nor @0 @0
cond process_line_6 @0
# stack_tmp_top = stack_top;
set &72 &68
# map_tmp_top = map_top;
set &1364 &1360
# return;
ret
# normal:
process_line_6:
# int i = 0;									i@0-3
set @0 0
# byte char = line_buffer[i];					char@4-4
load& @0 @4
# if(char != 0x00) goto loop;
cond process_line_6_0 @4
# return;
ret
# loop:
process_line_6_0:
# byte char = line_buffer[i];					char@4-4
load& @0 @4
# if(char != 0x00) goto not_null;
cond process_line_6_1 @4
# out(0x0a);
out 0x0a
# return;
ret
# not_null:
process_line_6_1:
# if(char != '.') goto not_var_start;
set @5 @4 1
sub @5 "."
cond process_line_6_2 @5
# int j = split(i);								j@5-8
set @9 @0
call split 17
# int val = find_map(line_buffer+i);			val@9-12
set @13 @0
call find_map 21
# out("@%d ", val-func_header);
out "@"
set @13 @9
sub @13 &76
call print_int 21
out 0x20
# i = j;
set @0 @5
# goto loop;
goto process_line_6_0
# not_var_start:
process_line_6_2:
# out(char);
out @4 1
# i++;
add @0 1
# goto loop;
goto process_line_6_0


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
main_2_0:
# if(line_length > 0) goto skip;
comp @5 @0 !<=
cond main_2_1 @5
# goto read_loop;
goto main_0
# skip:
main_2_1:
# line_length--;
sub @0 1
# line_buffer[line_length] = 0x00;
set @5 0x00
save& @0 @5 1
# goto clear_loop;
goto main_2_0
# not_enter:
main_3:
# line_buffer[line_length] = char;
save& @0 @4 1
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
