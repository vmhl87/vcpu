#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef uint32_t byte4;
typedef uint8_t byte;
typedef uint8_t bool;

#define MAX_LINE_LENGTH 64
#define STACK_SIZE 4096

bool verbose;
#define log if(verbose) printf

void fail(bool test, const char *str){
	if(!test) return;

	printf(str);
	printf("\n");
	exit(1);
}

int line_num = -1;

void fail_p(int test, const char *str1, char *str2){
	if(!test) return;

	if(line_num != -1) printf("error on line %d:\n\t", line_num);
	printf(str1, str2);
	printf("\n");
	exit(1);
}

// ------------------------------------------

byte stack[STACK_SIZE];
byte4 stack_ptr;

void stack_push(byte data){
log("    push 0x%02X\n", data);
	stack[stack_ptr++] = data;
}

void stack_push_4(int data){
log("    push %d\n", data);
	*((int*) (stack+stack_ptr)) = data;
	stack_ptr += 4;
}

void stack_push_n(int bytes, byte *data){
	while(bytes--){
log("    push 0x%02X\n", *data);
		stack[stack_ptr++] = *(data++);
	}
}

byte get_stack(int i){
	return stack[i];
}

int get_stack_4(int i){
	return *((int*) (stack+i));
}

void set_stack(int i, byte data){
	stack[i] = data;
}

void set_stack_4(int i, int data){
	*((int*) (stack+i)) = data;
}

// ------------------------------------------

struct label_map_node{
	struct label_map_node *c[256];
	byte4 loc;
	int id;
};

struct label_map_node label_map_data[256];
int label_map_fill;

byte4 label_id_table[256];
int label_id_table_fill;

int label_ref_table[256][2];
int label_ref_table_fill;

struct label_map_node* _find(char *label){
	struct label_map_node *p = label_map_data;
	char *l = label;

	while(l[0] != 0){
		if(p->c[l[0]] == NULL){
			p->c[l[0]] = label_map_data+(++label_map_fill);
			p->c[l[0]]->id = -1;

			fail_p(label_map_fill == 256, "label_map_data capacity reached when parsing \"%s\"", label);
		}

		p = p->c[l[0]], ++l;
	}

	return p;
}

void label_map_set(char *label, byte4 loc){
	struct label_map_node *p = _find(label);

	if(p->id == -1) p->id = label_id_table_fill++;

	label_id_table[p->id] = (p->loc = loc);
}

int label_map_get_id(char *label){
	struct label_map_node *p = _find(label);

	if(p->id == -1) p->id = label_id_table_fill++;

	return p->id;
}

void label_map_save_ref(int loc, int id){
	label_ref_table[label_ref_table_fill][0] = loc;
	label_ref_table[label_ref_table_fill][1] = id;
	++label_ref_table_fill;
}

void populate_label_refs(){
	for(int i=0; i<label_ref_table_fill; ++i){
		int loc = label_ref_table[i][0];
		int id = label_ref_table[i][1];
		set_stack_4(loc, label_id_table[id]);
	}
}

byte4 label_map_get(char *label){
	struct label_map_node *p = label_map_data;
	char *l = label;

	while(l[0] != 0){
		fail_p(p->c[l[0]] == NULL, "could not locate label: \"%s\"", label);
		p = p->c[l[0]], ++l;
	}

	return p->loc;
}

// ------------------------------------------

int grab_token(int start, char line[MAX_LINE_LENGTH]){
	bool string_mode = 0;
	for(int i=start; i<MAX_LINE_LENGTH; ++i){
		if(string_mode){
			if(line[i] == '\"') string_mode = 0;
			continue;
		}

		if(line[i] == '\"') string_mode = 1;
		if(line[i] == ' ' || line[i] == 0){
			for(; i<MAX_LINE_LENGTH && line[i] == ' '; ++i)
				line[i] = 0;
			return i;
		}
	}

	fail_p(1, "encountered end of line while reading \"%s\"", line+start);
	return -1;
}

struct data{
	short type; // 0: raw, 1: relative addr, 2: absolute addr
	int data, bytes;
};

struct data parse_data(char *_arg){
	char *arg = _arg;

	struct data ret = {};
	ret.type = 0;

	int sign = 1;

	if(arg[0] == '\"'){
		ret.bytes = 0;
		ret.type = 0;
		++arg;

		while(arg[0] != '\"'){
			if(arg[0] == '\\'){
				switch(arg[1]){
					case '0':
						_arg[ret.bytes++] = 0x00;
						break;

					case 'n':
						_arg[ret.bytes++] = 0x0a;
						break;

					case 't':
						_arg[ret.bytes++] = 0x09;
						break;

					case '\"':
						_arg[ret.bytes++] = 0x22;

					case '\\':
						_arg[ret.bytes++] = 0x5c;
						break;

					fail_p(1, "invalid escape sequence: %s", _arg);
				}

				++arg;

			}else _arg[ret.bytes++] = arg[0];

			++arg;
		}

		if(ret.bytes < 4){
			ret.data = 0;
			for(int i=0; i<ret.bytes; ++i) ret.data = (ret.data << 4) | _arg[i];
		}

		return ret;

	}else if(arg[0] == '@'){
		ret.type = 1, ret.bytes = 4;
		if(arg[1] == '-') sign *= -1, ++arg;
		++arg;

	}else if(arg[0] == '&'){
		ret.type = 2, ret.bytes = 4, ++arg;

	}else if(arg[0] == '-'){
		sign *= -1, ret.bytes = 4, ++arg;
	}

	if(arg[0] == 0) goto fail;

	if(arg[0] == '0'){
		if(arg[1] == 'x'){
			ret.bytes = 0;
			ret.data = 0;
			arg += 2;

			int chars = 0;
			byte b = 0;

			while((arg[0] >= '0' && arg[0] <= 'F') || (arg[0] >= 'a' && arg[0] <= 'f') || arg[0] == 0){
				if(arg[0] == 0) break;
				b = (b<<4) | (arg[0] >= 'a' ? arg[0]+10-'a' : (arg[0] >= 'A' ? arg[0]+10-'A' : arg[0]-'0'));
				if((++chars) % 2 == 0) *((byte*) (_arg+(ret.bytes++))) = b, b = 0;
				++arg;
			}

			if(chars%2 != 0 || arg[0] != 0) goto fail;
			
			if(ret.bytes < 4){
				ret.data = 0;
				for(int i=0; i<ret.bytes; ++i) ret.data = (ret.data << 4) | _arg[i];
				ret.data *= sign;

			}else if(sign != 1) goto fail;

		}else if(arg[1] == 0){
			ret.data = 0, ret.bytes = 4;

		}else goto fail;

	}else{
		ret.data = 0, ret.bytes = 4;

		while((arg[0] >= '0' && arg[0] <= '9') || arg[0] == 0){
			if(arg[0] == 0) break;
			ret.data = ret.data*10 + arg[0]-'0';
			++arg;
		}
		
		ret.data *= sign;
		
		if(arg[0] != 0) goto fail;
	}

	return ret;

fail:
	fail_p(1, "error processing data: \"%s\"", _arg);
	return ret;
}

void parse(char line[MAX_LINE_LENGTH]){
	for(int i=0, j=0; i<MAX_LINE_LENGTH; j^=(line[i++]=='\"')) if(line[i] == '#' && !j) line[i] = 0;
	for(int i=0; i<MAX_LINE_LENGTH; ++i) if(line[i] == '\t') line[i] = ' ';
	if(line[0] == 0 || line[0] == ' ') return;

	bool is_label = 0;

	{
		bool m[256] = {};
		m['_'] = 1, m['-'] = 1, m['.'] = 1;
		for(char i='a'; i<='z'; ++i) m[i] = 1;
		for(char i='A'; i<='Z'; ++i) m[i] = 1;
		for(char i='0'; i<='9'; ++i) m[i] = 1;

		for(int i=0; i<MAX_LINE_LENGTH; ++i){
			if(line[i] == ':') is_label = 1;
			if(!m[line[i]]) break;
		}
	};

	if(is_label){
		for(int i=0; i<MAX_LINE_LENGTH; ++i) if(line[i] == ':') line[i] = 0;
		label_map_set(line, stack_ptr);
		return;
	}

	int start = grab_token(0, line);

log("instruction: \"%s\"\n", line);

	if(!strcmp(line, "set")){
		char *arg1 = line+start;
		start = grab_token(start, line);
		char *arg2 = line+start;
		start = grab_token(start, line);
		char *arg3 = line+start;
		start = grab_token(start, line);

log("  arg 1: \"%s\", arg 2: \"%s\", arg 3: \"%s\"\n", arg1, arg2, arg3);

		struct data addr1 = parse_data(arg1);
		struct data addr2 = parse_data(arg2);
		struct data addr3;

		int bytes = 4;

		if(arg3[0] != 0){
			addr3 = parse_data(arg3);
			fail_p(addr3.type != 0 || addr3.bytes > 4,
				"third argument to 'set' must be numeric: \"%s\"", arg3);
			bytes = addr3.data;

		}else if(addr2.type == 0) bytes = addr2.bytes;

if(bytes != 4 && bytes != 1) log("    (bytes %d)\n", bytes);

		byte inst = 0x20;
		if(bytes == 1) inst = 0x10;
		else if(bytes != 4) inst = 0x30;

		stack_push(inst | (addr1.type << 2) | addr2.type);
		stack_push_4(addr1.data);

		if(bytes == 1){
			if(addr2.type == 0){
				if(addr2.bytes == 1) stack_push(arg2[0]);
				else stack_push(addr2.data);

			}else stack_push_4(addr2.data);

		}else if(bytes == 4){
			stack_push_4(addr2.data);

		}else{
			stack_push_4(bytes);
			if(addr2.type == 0) stack_push_n(addr2.bytes, (byte*) arg2);
			else stack_push_4(addr2.data);
		}

	}else if(!strcmp(line, "in")){
		char *arg1 = line+start;
		start = grab_token(start, line);
		char *arg2 = line+start;
		start = grab_token(start, line);

log("  arg 1: \"%s\", arg 2: \"%s\"\n", arg1, arg2);

		struct data addr1 = parse_data(arg1);
		struct data addr2 = parse_data(arg2);

		fail_p(addr1.type == 0, "first argument to 'in' must be address: \"%s\"", arg1);
		fail_p(addr2.bytes != 4, "invalid size parameter: \"%s\"", arg2);

		byte inst = 0x44 | addr1.type;
		if(addr2.data != 1) inst |= 0x10;

		stack_push(inst);
		if(addr2.data != 1) stack_push_4(addr2.data);
		stack_push_4(addr1.data);

	}else if(!strcmp(line, "out")){
		char *arg1 = line+start;
		start = grab_token(start, line);
		struct data addr1 = parse_data(arg1);

		if(addr1.type == 0){
log("  arg 1: \"%s\"", arg1);

			if(addr1.bytes == 1){
				stack_push(0x48);
				stack_push(arg1[0]);

			}else{
				stack_push(0x58);
				stack_push_4(addr1.bytes);
				stack_push_n(addr1.bytes, (byte*) arg1);
			}

		}else{
			char *arg2 = line+start;
			start = grab_token(start, line);

log("  arg 1: \"%s\", arg 2: \"%s\"\n", arg1, arg2);

			struct data addr2 = parse_data(arg2);

			fail_p(addr2.bytes != 4, "invalid size parameter: \"%s\"", arg2);

			byte inst = 0x48 | addr1.type;
			if(addr2.data != 1) inst |= 0x10;

			stack_push(inst);
			if(addr2.data != 1) stack_push_4(addr2.data);
			stack_push_4(addr1.data);
		}

	}else if(!strcmp(line, "goto")){
		char *arg1 = line+start;
		start = grab_token(start, line);

log("  arg 1: \"%s\"\n", arg1);

		char m[256] = {};
		m['@'] = 1, m['-'] = 1;
		for(int i=0; i<10; ++i) m['0'+i] = 1;

		if(m[arg1[0]]){
			struct data addr1 = parse_data(arg1);

			fail_p(addr1.type == 0, "first argument to 'goto' may not be raw: \"%s\"", arg1);

			byte inst = 0x60 | addr1.type;

			stack_push(inst);
			stack_push_4(addr1.data);

		}else{
			int addr1 = label_map_get_id(arg1);
			label_map_save_ref(stack_ptr+1, addr1);

			stack_push(0x62);
			stack_push_4(0x00);  // addr1
		}

	}else if(!strcmp(line, "call")){
		char *arg1 = line+start;
		start = grab_token(start, line);
		char *arg2 = line+start;
		start = grab_token(start, line);

log("  arg 1: \"%s\", arg 2: \"%s\"\n", arg1, arg2);

		int addr1 = label_map_get_id(arg1);
		label_map_save_ref(stack_ptr+1, addr1);

		struct data addr2 = parse_data(arg2);

		fail_p(addr2.type != 0, "second argument to 'call' must be raw: \"%s\"", arg2);
		fail_p(addr2.bytes != 4, "invalid size parameter: \"%s\"", arg2);

		stack_push(0x66);
		stack_push_4(0x00);  // addr1
		stack_push(0x6c);
		stack_push_4(addr2.data);

	}else if(!strcmp(line, "cond")){
		char *arg1 = line+start;
		start = grab_token(start, line);
		char *arg2 = line+start;
		start = grab_token(start, line);

log("  arg 1: \"%s\", arg 2: \"%s\"\n", arg1, arg2);

		int addr1 = label_map_get_id(arg1);
		label_map_save_ref(stack_ptr+1, addr1);

		struct data addr2 = parse_data(arg2);

		fail_p(addr2.type != 1, "second argument to 'cond' must be relative address: \"%s\"", arg2);

		stack_push(0x6a);
		stack_push_4(0x00);  // addr1
		stack_push_4(addr2.data);

	}else if(!strcmp(line, "ret")){
		stack_push(0x70);

	}else if(!strcmp(line, "exit")){
		stack_push(0x71);

	}else if(!strcmp(line, "add") || !strcmp(line, "mul") || !strcmp(line, "sub") || !strcmp(line, "div")){
		char *arg1 = line+start;
		start = grab_token(start, line);
		char *arg2 = line+start;
		start = grab_token(start, line);
		char *arg3 = line+start;
		start = grab_token(start, line);

log("  arg 1: \"%s\", arg 2: \"%s\", arg 3: \"%s\"\n", arg1, arg2, arg3);

		struct data addr1 = parse_data(arg1);
		struct data addr2 = parse_data(arg2);

		fail_p(addr1.type == 0, "first argument to <math> must be address: \"%s\"", arg1);
		fail_p(addr2.bytes > 4, "second argument to <math> cannot be more than 4 bytes: \"%s\"", arg2);

		byte inst = addr2.type == 0 ? 0xa2 : 0x90;
		if(addr1.type & 1) inst |= 0x01;
		if(addr2.type & 1) inst |= 0x02;

		if(line[0] == 'a') inst |= 0;
		if(line[0] == 'm') inst |= 4;
		if(line[0] == 's') inst |= 8;
		if(line[0] == 'd') inst |= 12;

		stack_push(inst);
		stack_push_4(addr1.data);
		stack_push_4(addr2.data);

	}else if(!strcmp(line, "add1") || !strcmp(line, "mul1") || !strcmp(line, "sub1") || !strcmp(line, "div1")){
		char *arg1 = line+start;
		start = grab_token(start, line);
		char *arg2 = line+start;
		start = grab_token(start, line);
		char *arg3 = line+start;
		start = grab_token(start, line);

log("  arg 1: \"%s\", arg 2: \"%s\", arg 3: \"%s\"\n", arg1, arg2, arg3);

		struct data addr1 = parse_data(arg1);
		struct data addr2 = parse_data(arg2);

		fail_p(addr1.type == 0, "first argument to <math1> must be address: \"%s\"", arg1);
		fail_p(addr2.type == 0 && addr2.bytes > 1,
				"second argument to <math1> cannot be more than 1 byte: \"%s\"", arg2);

		byte inst = addr2.type == 0 ? 0xa0 : 0x80;
		if(addr1.type & 1) inst |= 0x01;
		if(addr2.type & 1) inst |= 0x02;

		if(line[0] == 'a') inst |= 0;
		if(line[0] == 'm') inst |= 4;
		if(line[0] == 's') inst |= 8;
		if(line[0] == 'd') inst |= 12;

		stack_push(inst);
		stack_push_4(addr1.data);
		stack_push(addr2.data);
		
	}else if(!strcmp(line, "comp")){
		char *arg1 = line+start;
		start = grab_token(start, line);
		char *arg2 = line+start;
		start = grab_token(start, line);
		char *arg3 = line+start;
		start = grab_token(start, line);

log("  arg 1: \"%s\", arg 2: \"%s\", arg 3: \"%s\"\n", arg1, arg2, arg3);

		struct data addr1 = parse_data(arg1);
		struct data addr2 = parse_data(arg2);

		fail_p(addr1.type != 1, "first argument to 'comp' must be relative address: \"%s\"", arg1);
		fail_p(addr2.type != 1, "second argument to 'comp' must be relative address: \"%s\"", arg2);

		byte inst = 0xb8;

		for(int i=0; arg3[i]; ++i){
			if(arg3[i] == '!') inst |= 4;
			else if(arg3[i] == '>') inst |= 2;
			else if(arg3[i] == '<') inst |= 1;
			else if(arg3[i] == '=') inst |= 0;
			else fail_p(1, "invalid comparator: \"%s\"", arg3);
		}

		stack_push(inst);
		stack_push_4(addr1.data);
		stack_push_4(addr2.data);

	}else if(!strcmp(line, "and") || !strcmp(line, "or") || !strcmp(line, "nand") || !strcmp(line, "nor")){
		char *arg1 = line+start;
		start = grab_token(start, line);
		char *arg2 = line+start;
		start = grab_token(start, line);

log("  arg 1: \"%s\", arg 2: \"%s\"", arg1, arg2);

		struct data addr1 = parse_data(arg1);
		struct data addr2 = parse_data(arg2);

		fail_p(addr1.type == 0, "first argument to <bool> must be address: \"%s\"", arg1);
		fail_p(addr2.type == 0, "second argument to <bool> must be address: \"%s\"", arg2);

		byte inst = 0xc0;

		if(addr1.type == 1) inst |= 1;
		if(addr2.type == 1) inst |= 2;
		
		if(line[0] == 'n') inst |= 8, ++line;
		if(line[0] == 'a') inst |= 4;

		stack_push(inst);
		stack_push_4(addr1.data);
		stack_push_4(addr2.data);

	}else if(!strcmp(line, "load") || !strcmp(line, "save")){
		char *arg1 = line+start;
		start = grab_token(start, line);
		char *arg2 = line+start;
		start = grab_token(start, line);
		char *arg3 = line+start;
		start = grab_token(start, line);

log("  arg 1: \"%s\", arg 2: \"%s\", arg 3: \"%s\"\n", arg1, arg2, arg3);

		struct data addr1 = parse_data(arg1);
		struct data addr2 = parse_data(arg2);

		fail_p(addr1.type == 0, "first argument to 'load' must be address: \"%s\"", arg1);
		fail_p(addr2.type != 1, "second argument to 'load' must be relative address: \"%s\"", arg2);

		int bytes = 4;

		if(arg3[0] != 0){
			struct data addr3 = parse_data(arg3);
			fail_p(addr3.type != 0, "size argument to 'load' must be raw: \"%s\"", arg3);
			bytes = addr3.data;
		}

		byte inst = 0xd0 | (addr1.type & 1);
		if(bytes == 4) inst |= 2;
		else if(bytes != 1) inst |= 4;
		if(line[0] == 's') inst |= 8;

		stack_push(inst);
		stack_push_4(addr1.data);
		stack_push_4(addr2.data);
		if(bytes != 1 && bytes != 4) stack_push_4(bytes);

	}else if(!strcmp(line, "(")){
		while(1){
			char *arg = line+start;
			start = grab_token(start, line);

			if(arg[0] == ')') break;

			if(label_map_data[0].c[arg[0]] != NULL){
				int addr = label_map_get_id(arg);
				label_map_save_ref(stack_ptr, addr);
				stack_push_4(0x00);  // addr

			}else{
				struct data addr = parse_data(arg);

				fail_p(addr.type != 0, "expected raw data: \"%s\"", arg);

				if(addr.bytes == 1) stack_push(arg[0]);
				else if(addr.bytes == 4) stack_push_4(addr.data);
				else stack_push_n(addr.bytes, (byte*) arg);
			}
		}

	}else if(!strcmp(line, "null")){
		char *arg = line+start;
		start = grab_token(start, line);

log("  arg 1: \"%s\"\n", arg);
		
		struct data addr = parse_data(arg);

		fail_p(addr.type != 0, "first argument to 'null' must be numeric: \"%s\"", arg);
		fail_p(addr.bytes > 4, "first argument to 'null' must be at most 4 bytes: \"%s\"", arg);

		stack_ptr += addr.data;

	}else fail_p(1, "invalid instruction: \"%s\"", line);

log("\n");
}

// ------------------------------------------

byte4 ip, sp;

void execute(){
	while(1){
		byte inst = stack[ip];

		byte4 next_inst = ip;

		byte high = inst >> 4, low = inst & 15;

		switch(high){
			case 1:
				int dest = get_stack_4(ip+1);
				if(low & 4) dest += sp;
				int src = get_stack_4(ip+5);
				if(low & 1) src += sp;
				if((low & 3) == 0){
log("set_1\t@%d\t0x%02X\tdest:%d dat:0x%02X\n", ip, inst, dest, get_stack(ip+5));
					stack[dest] = get_stack(ip+5), next_inst = ip+6;

				}else{
log("set_1\t@%d\t0x%02X\tsrc:%d dest:%d dat:0x%02X\n", ip, inst, src, dest, stack[src]);
					stack[dest] = stack[src], next_inst = ip+9;
				}

				break;

			case 2:
				dest = get_stack_4(ip+1);
				if(low & 4) dest += sp;
				src = get_stack_4(ip+5);
				if(low & 1) src += sp;
				if((low & 3) == 0){
					set_stack_4(dest, get_stack_4(ip+5));
log("set_4\t@%d\t0x%02X\tdest:%d dat:%d\n", ip, inst, dest, get_stack_4(dest));

				}else{
					set_stack_4(dest, get_stack_4(src));
log("set_4\t@%d\t0x%02X\tsrc:%d dest:%d dat:%d\n", ip, inst, src, dest, get_stack_4(dest));
				}

				next_inst = ip+9;
				break;

			case 3:
				int len = get_stack_4(ip+5);
				dest = get_stack_4(ip+1);
				if(low & 4) dest += sp;
				src = get_stack_4(ip+9);
				if(low & 1) src += sp;
				if((low & 3) == 0){
log("set_n\t@%d\t0x%02X\tdest:%d len:%d\n", ip, inst, dest, len);
					memcpy(stack+dest, stack+ip+9, len), next_inst = ip+9+len;

				}else{
log("set_n\t@%d\t0x%02X\tsrc:%d dest:%d len:%d\n", ip, inst, src, dest, len);
					memcpy(stack+dest, stack+src, len), next_inst = ip+13;
				}
				break;

			case 4:
				if(low & 8){
					int src = get_stack_4(ip+1);
					if(low & 1) src += sp;
					if((low & 3) == 0){
log("out_1\t@%d\t0x%02X\tdat:0x%02X\n", ip, inst, get_stack(ip+1));
						fputc((char) get_stack(ip+1), stdout), fflush(stdout);
						next_inst = ip+2;

					}else{
log("out_1\t@%d\t0x%02X\tsrc:%d dat:0x%02X\n", ip, inst, src, get_stack(src));
						fputc((char) get_stack(src), stdout), fflush(stdout);
						next_inst = ip+5;
					}
				}else{
					int src = get_stack_4(ip+1);
					if(low & 1) src += sp;
					if((low & 3) == 0){
log("----\t@%d\t0x%02X\n", ip, inst);
						fail(1, "cannot read to non-address");

					}else{
						fread((char*) stack+src, 1, 1, stdin);
log("in_1\t@%d\t0x%02X\tsrc:%d dat:0x%02X\n", ip, inst, src, get_stack(src));
						next_inst = ip+5;
					}
				}
				break;

			case 5:
				if(low & 8){
					int len = get_stack_4(ip+1);
					int src = get_stack_4(ip+5);
					if(low & 1) src += sp;
					if((low & 3) == 0){
log("out_n\t@%d\t0x%02X\tlen:%d\n", ip, inst, len);
						fwrite(stack+ip+5, sizeof(char), len, stdout), fflush(stdout);
						next_inst = ip+5+len;

					}else{
log("out_4\t@%d\t0x%02X\tsrc:%d len:%d\n", ip, inst, src, len);
						fwrite(stack+src, sizeof(char), len, stdout), fflush(stdout);
						next_inst = ip+9;
					}
				}else{
					int len = get_stack_4(ip+1);
					int src = get_stack_4(ip+5);
					if(low & 1) src += sp;
					if((low & 3) == 0){
log("----\t@%d\t0x%02X\n", ip, inst);
						fail(1, "cannot read to non-address");

					}else{
log("in_4\t@%d\t0x%02X\tsrc:%d len:%d\n", ip, inst, src, len);
						fread((char*) stack+src, 1, len, stdin);
						next_inst = ip+9;
					}
				}
				break;

			case 6:
				if(low & 4){
					if(low & 8){
						int amt = get_stack_4(ip+1);
						sp -= amt;
						next_inst = ip+5;
log("call_2\t@%d\t0x%02X\tamt:%d\n", ip, inst, amt);
					}else{
						int amt = get_stack_4(ip+6);
						sp += amt;
						set_stack_4(sp-4, ip+5);
						next_inst = get_stack_4(ip+1);
						if(low & 1) next_inst += sp;
log("call_1\t@%d\t0x%02X\tto:%d amt:%d\n", ip, inst, next_inst, amt);
					}
				}else{
					if(low & 8){
						byte4 loc = get_stack_4(ip+1);
						if(low & 1) loc += sp;
						byte val = get_stack(sp+get_stack(ip+5));
						if(val > 0x00) next_inst = loc;
						else next_inst = ip+9;
log("cond\t@%d\t0x%02X\tto:%d val:0x%02X\n", ip, inst, loc, val);
					}else{
						next_inst = get_stack_4(ip+1);
						if(low & 1) next_inst += sp;
log("goto\t@%d\t0x%02X\tto:%d\n", ip, inst, next_inst);
					}
				}
				break;

			case 7:
				if(low == 1){
log("exit\n");
					return;

				}else if(low == 0){
					next_inst = get_stack_4(sp-4);
log("ret\t@%d\t0x%02X\tto:%d\n", ip, inst, next_inst);

				}else printf("unimplemented instruction: 0x%02X\n", inst);
				break;

			case 8:
				dest = get_stack_4(ip+1);
				if(low & 1) dest += sp;
				src = get_stack_4(ip+5);
				if(low & 2) src += sp;
				if((low & 12) == 0){
log("add_1\t@%d\t0x%02X\tdest:0x%02X, src:0x%02X\n", ip, inst, get_stack(dest), get_stack(src));
					set_stack(dest, get_stack(dest)+get_stack(src));
				}else if((low & 12) == 4){
log("mul_1\t@%d\t0x%02X\tdest:0x%02X, src:0x%02X\n", ip, inst, get_stack(dest), get_stack(src));
					set_stack(dest, get_stack(dest)*get_stack(src));
				}else if((low & 12) == 8){
log("sub_1\t@%d\t0x%02X\tdest:0x%02X, src:0x%02X\n", ip, inst, get_stack(dest), get_stack(src));
					set_stack(dest, get_stack(dest)-get_stack(src));
				}else if((low & 12) == 12){
log("div_1\t@%d\t0x%02X\tdest:0x%02X, src:0x%02X\n", ip, inst, get_stack(dest), get_stack(src));
					set_stack(dest, get_stack(dest)/get_stack(src));
				}
				next_inst = ip+9;
				break;

			case 9:
				dest = get_stack_4(ip+1);
				if(low & 1) dest += sp;
				src = get_stack_4(ip+5);
				if(low & 2) src += sp;
				if((low & 12) == 0){
log("add_4\t@%d\t0x%02X\tdest:%d, src:%d\n", ip, inst, get_stack_4(dest), get_stack_4(src));
					set_stack_4(dest, get_stack_4(dest)+get_stack_4(src));
				}else if((low & 12) == 4){
log("mul_4\t@%d\t0x%02X\tdest:%d, src:%d\n", ip, inst, get_stack_4(dest), get_stack_4(src));
					set_stack_4(dest, get_stack_4(dest)*get_stack_4(src));
				}else if((low & 12) == 8){
log("sub_4\t@%d\t0x%02X\tdest:%d, src:%d\n", ip, inst, get_stack_4(dest), get_stack_4(src));
					set_stack_4(dest, get_stack_4(dest)-get_stack_4(src));
				}else if((low & 12) == 12){
log("div_4\t@%d\t0x%02X\tdest:%d, src:%d\n", ip, inst, get_stack_4(dest), get_stack_4(src));
					set_stack_4(dest, get_stack_4(dest)/get_stack_4(src));
				}
				next_inst = ip+9;
				break;

			case 0xa:
				dest = get_stack_4(ip+1);
				if(low & 1) dest += sp;
				if(low & 2){
					int amt = get_stack_4(ip+5);
					if((low & 12) == 0){
log("add_4\t@%d\t0x%02X\tdest:%d, amt:%d\n", ip, inst, get_stack_4(dest), amt);
						set_stack_4(dest, get_stack_4(dest)+amt);
					}else if((low & 12) == 4){
log("mul_4\t@%d\t0x%02X\tdest:%d, amt:%d\n", ip, inst, get_stack_4(dest), amt);
						set_stack_4(dest, get_stack_4(dest)*amt);
					}else if((low & 12) == 8){
log("sub_4\t@%d\t0x%02X\tdest:%d, amt:%d\n", ip, inst, get_stack_4(dest), amt);
						set_stack_4(dest, get_stack_4(dest)-amt);
					}else if((low & 12) == 12){
log("div_4\t@%d\t0x%02X\tdest:%d, amt:%d\n", ip, inst, get_stack_4(dest), amt);
						set_stack_4(dest, get_stack_4(dest)/amt);
					}
					next_inst = ip+9;
				}else{
					byte amt = get_stack(ip+5);
					if((low & 12) == 0){
log("add_1\t@%d\t0x%02X\tdest:0x%02X, amt:0x%02X\n", ip, inst, get_stack(dest), amt);
						set_stack(dest, get_stack(dest)+amt);
					}else if((low & 12) == 4){
log("mul_1\t@%d\t0x%02X\tdest:0x%02X, amt:0x%02X\n", ip, inst, get_stack(dest), amt);
						set_stack(dest, get_stack(dest)*amt);
					}else if((low & 12) == 8){
log("sub_1\t@%d\t0x%02X\tdest:0x%02X, amt:0x%02X\n", ip, inst, get_stack(dest), amt);
						set_stack(dest, get_stack(dest)-amt);
					}else if((low & 12) == 12){
log("div_1\t@%d\t0x%02X\tdest:0x%02X, amt:0x%02X\n", ip, inst, get_stack(dest), amt);
						set_stack(dest, get_stack(dest)/amt);
					}
					next_inst = ip+6;
				}
				break;

			case 0xb:
				byte4 loc = sp+get_stack_4(ip+1);
				if(low & 8){
					byte res = 0;
					int val = *((int*) (stack+sp+get_stack_4(ip+5)));
					if(val == 0) res = 1;
					if(low & 1) res |= val < 0;
					if(low & 2) res |= val > 0;
					if(low & 4) res ^= 1;
log("comp_4\t@%d\t0x%02X\tdest:0x%02X, val:%d, res:0x%02X\n", ip, inst, loc, val, res);
					set_stack(loc, res);
				}else{
					byte res = 0, val = get_stack(sp+get_stack_4(ip+5));
					if(val == 0x00) res = 1;
					if(low & 2) res |= val > 0x00;
					if(low & 4) res ^= 1;
log("comp_1\t@%d\t0x%02X\tdest:0x%02X, val:0x%02X, res:0x%02X\n", ip, inst, loc, val, res);
					set_stack(loc, res);
				}
				next_inst = ip+9;
				break;

			case 0xc:
				dest = get_stack_4(ip+1);
				if(low & 1) dest += sp;
				src = get_stack_4(ip+5);
				if(low & 2) src += sp;
				byte res = get_stack(src);
				if(low & 4) res = res && get_stack(dest);
				else res = res || get_stack(dest);
				if(low & 8) res = !res;
				set_stack(dest, res);
log("bool\t@%d\t0x%02X\tdest:%d, src:%d, and:%d, invert:%d\n", ip, inst, dest, src, low&4, low&8);
				next_inst = ip+9;
				break;

			case 0xd:
				byte4 addr = get_stack_4(sp+get_stack_4(ip+1));
				if(low & 1) addr += sp;
				loc = sp+get_stack_4(ip+5);
				if(low & 8){
					if(low & 4){
						int len = get_stack_4(ip+9);
log("save_n\t@%d\t0x%02X\taddr:%d, from:%d, len:%d\n", ip, inst, addr, loc, len);
						memcpy(stack+addr, stack+loc, len);
						next_inst = ip+13;

					}else{
						if(low & 2){
log("save_4\t@%d\t0x%02X\taddr:%d, from:%d\n", ip, inst, addr, loc);
							set_stack_4(addr, get_stack_4(loc));
							next_inst = ip+9;

						}else{
log("save_1\t@%d\t0x%02X\taddr:%d, from:%d\n", ip, inst, addr, loc);
							set_stack(addr, get_stack(loc));
							next_inst = ip+9;
						}
					}
				}else{
					if(low & 4){
						int len = get_stack_4(ip+9);
log("load_n\t@%d\t0x%02X\taddr:%d, to:%d, len:%d\n", ip, inst, addr, loc, len);
						memcpy(stack+loc, stack+addr, len);
						next_inst = ip+13;

					}else{
						if(low & 2){
log("load_4\t@%d\t0x%02X\taddr:%d, to:%d\n", ip, inst, addr, loc);
							set_stack_4(loc, get_stack_4(addr));
							next_inst = ip+9;
						}else{
log("load_1\t@%d\t0x%02X\taddr:%d, to:%d\n", ip, inst, addr, loc);
							set_stack(loc, get_stack(addr));
							next_inst = ip+9;
						}
					}
				}
				break;

				fail_p(1, "invalid instruction: \"0x%02X\"", (char*) inst);
		}

		if(ip == next_inst){
log("----\t@%d\t0x%02X\n", ip, inst);
			fail(1, "program halted");
		}

		ip = next_inst;
	}
}

// ------------------------------------------

void assemble(const char *file){
	FILE *f = fopen(file, "r");
	fail_p(f == NULL, "failed to open \'%s\'", (char*) file);

	char line[MAX_LINE_LENGTH] = {};

	for(int _c=0, i=0; _c != EOF;){
		_c = fgetc(f);

		if(_c == EOF || _c == '\n'){
			++line_num, parse(line);

			for(int j=0; j<i; ++j) line[j] = 0;
			i = 0;

		}else if(i < 127) line[i++] = _c;
	}

	populate_label_refs();

	line_num = -1;

	ip = label_map_get("main");
	sp = stack_ptr;

	fclose(f);
}

void write_bytecode(const char *file){
	FILE *f = fopen(file, "w");
	fail_p(f == NULL, "failed to open \'%s\'", (char*) file);
	fail(sp == 0, "empty stack");

	fwrite(&sp, 4, 1, f);
	fwrite(&ip, 4, 1, f);
	fwrite(stack, 1, sp, f);

	fclose(f);
}

void read_bytecode(const char *file){
	FILE *f = fopen(file, "r");
	fail_p(f == NULL, "failed to open \'%s\'", (char*) file);

	fread(&sp, 4, 1, f);
	fread(&ip, 4, 1, f);
	fread(stack, 1, sp, f);

	fclose(f);
}

int main(int argc, const char *argv[]){
	if(argc == 2 && !strcmp(argv[1], "-h")){
		printf(
			"[tiny virtual machine]\tusage:\n\n"
			"assemble:\t%s -a source.s program.o\n"
			"execute:\t%s -x program.o\n\n",
			argv[0], argv[0]
		);

	}else if(argc > 1 && argv[1][0] == '-'){
		bool flag[256] = {};
		for(int i=1; argv[1][i]; ++i) flag[argv[1][i]] = 1;

		int flagged = 0;
		for(int i=0; i<256; ++i) if(flag[i]) ++flagged;

		if(flag['v']) verbose = 1, --flagged;

		if(argc == 3 && flag['x'] && flagged == 1){
			read_bytecode(argv[2]);
			execute();

		}else if(argc == 3 && flag['a'] && flag['x'] && flagged == 2){
			assemble(argv[2]);
			execute();

		}else if(argc == 4 && flag['a'] && flag['x'] && flagged == 2){
			assemble(argv[2]);
			write_bytecode(argv[3]);
			execute();

		}else if(argc == 4 && flag['a'] && flagged == 1){
			assemble(argv[2]);
			write_bytecode(argv[3]);

		}else fail(1, "invalid arguments");

	}else fail(1, "invalid arguments");
}
