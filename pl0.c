// pl0 compiler source code

#pragma warning(disable:4996)


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "PL0.h"
#include "set.c"

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	printf("the sym is %d",sym);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
void getch(void)
{	
	//函数功能为读取下个字符并存到ch，line为读取该行的所有字符
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
	if(ch == '/'&&line[cc+1]=='/'){
		ll=cc=0;
		getch();
	}	
	//cc=当前读的一行的第cc个字符
	//line[80] 为当前一行的所有字符
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
// 分析当前读到的字符
// 为LL(k)文法进行词法分析
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];

	while (ch == ' '||ch == '\t')
		getch();
		//直接吞掉前面的空字符

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		}
		while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i]; // symbol is a reserved word
		else
			sym = SYM_IDENTIFIER;   // symbol is an identifier
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		}
		while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == ':')
	{//读到此符号，继续向前读并分析
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else if(ch == ':')
		{
			sym = SYM_SCOPE;   // ::
			getch();
		}
		else
		{
			sym = SYM_NULL;       // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else
	{ // other tokens
		i = NSYM;//' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';'等字符
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{	//具体功能详见2017版pdf教程
	symset s;

	if (! inset(sym, s1))
	{//如果当前符号不属于s1，先报错
		error(n);
		printf("the symbol is %d",sym);
		s = uniteset(s1, s2);
		while(! inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index

// enter object(constant, variable or procedre) into table.
void enter(int kind)
{	//管理符号表
	//常量直接存值
	//变量存level,kind,addr
	mask* mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	table[tx].kindtable_en=0;
	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask*) &table[tx];//直接转换类型，由comtab->mask,复用value域
		mk->level = level;
		mk->address = dx++;
		table[tx].kind_num=-1;
		table[tx].kindtable_en=0;
		break;
	case ID_PROCEDURE:
		mk = (mask*) &table[tx];
		mk->level = level;
		break;
	// case ID_POINTER:
	// 	mk = (mask*) &table[tx];
	// 	mk->level = level;
	// 	mk->address = dx++;
	// 	table[tx].kind_num=0;
	// 	table[tx].kindtable_en=1;
	// 	table[tx].kindtable[0].type=TYPE_POINTER;
	// 	break;
	case ID_ARRAY:
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		table[tx].kind_num=-1;
		table[tx].kindtable_en=1;
		table[tx].kindtable[0].type=TYPE_ARRAY;
		break;
	default:
		break;
	} // switch
} // enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	//这里直接返回最后同名的id的tx值
	return i;
} // position

//////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	//此处分析const后的语句
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	} 
	else	error(4);
	 // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration

int pointer_declaration(void)
{	
	int level=0; //指针的级数
	while(sym == SYM_TIMES){
		level++;
		getsym();
	}
	return level;
}

int dim_delcaration(void)
{
	int total_size=1;
	int index;
	//int dim_num=0;
	if(sym == SYM_LBRACKET)
		table[tx].kindtable_en=1;
	while(sym == SYM_LBRACKET){
		getsym();
		if(sym == SYM_NUMBER){
			//printf("detect the num");
			table[tx].kind_num++;
			index=table[tx].kind_num;
			total_size*=num;
			table[tx].kindtable[index].type=TYPE_ARRAY;
			table[tx].kindtable[index].elem_num=num;
		}
		else
			error(26);//"The left bracket should be followed by the number."
		getsym();
		if(sym != SYM_RBRACKET){
			error(27); //"Missing the right bracket. "
		}
		getsym();
	}
	return total_size;
}

//////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{	//变量赋值
	int pointer_level;
	int total_size;
	//int index=table[tx].kind_num;
	if (sym == SYM_IDENTIFIER)
	{
		//printf("READ the id!");
		getsym();
		if(sym == SYM_LBRACKET){
			enter(ID_ARRAY);
			table[tx].kindtable_en=1;
			total_size=dim_delcaration();
			dx+=total_size-1;
		}
		else{ 
			enter(ID_VARIABLE);//var or pointer
		}
	}
	else if(sym == SYM_TIMES){
		//printf("READ the stars!");
		pointer_level=pointer_declaration();
		vardeclaration();
		if(pointer_level!=0){
			table[tx].kindtable_en=1;
		}
		for(int i=0;i<pointer_level;i++){
			table[tx].kindtable[++table[tx].kind_num].type=TYPE_POINTER;
		}
	}
	else if(sym == SYM_LPAREN){
		getsym();
		vardeclaration();
		if(sym == SYM_RPAREN){
			getsym();
			dim_delcaration();
		}
		else{
			error(22);//"Missing ')'."
		}
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration


//////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{	//列出所有中间码
	int i;
	
	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

int update_step(void)
{
	//kindtable_index>=-1
	int final_type;
	//printf("kind_tableindex:%d, kindtablesize: %d\n",kindtable_index, kindtable_size);
	if(kindtable_index>=kindtable_size){
		//取地址后元素即为var
		step = 1;
		final_type=TYPE_VAR;
	}
	else{
		if(typelist[kindtable_index+1].type==TYPE_ARRAY){
			step=1;
			for(int i=1;kindtable_index+i<=kindtable_size&&typelist[kindtable_index+i].type==TYPE_ARRAY;i++){
				step*=typelist[kindtable_index+i].elem_num;
			}
		}
		else
			step=1;
		final_type=typelist[kindtable_index+1].type;
	}
	return final_type;
}

void Scope_analyze(int level_search, int start){
	int finish=0;
	int find_flag = 0;
	int i;
	do{
		if(sym != SYM_IDENTIFIER){
			error(36);
			break;
		}
		for(i =start;i <= tx; i++){
			if((!strcmp(table[i].name, id)) && ((mask*)(&table[i]))->level == level_search){
				find_flag = 1;
				break;
			}
			else if(table[i].kind == ID_PROCEDURE){
				find_flag = 0;
				break;
			}
		}
		if(find_flag == 0){
			error(38);//Can not locate the identifier!
		}
		else if(table[i].kind != ID_PROCEDURE){
			finish = 1; //找完了
		}
		else{
			start = i+1;
			level_search++;
		}
		getsym();
		if(sym != SYM_SCOPE)
			break;
		else if(finish){
			error(38);
			break;
		}
		else{
			getsym();
		}
	}while(sym == SYM_IDENTIFIER);
	if(finish == 0){
		error(21);
	}
	else if(finish == 1){
		gen(LOD, level-level_search, ((mask*)(&table[i]))->address);
	}
}

//////////////////////////////////////////////////////////////////////
int factor(symset fsys)
{	//因子
	//fsys为后继符号
	//printf("Now the symbol is %d",sym);
	int dim_reference(symset fsys);
	int expression(symset fsys);
	int i;
	symset set;
	set=createset(SYM_BECOMES);
	fsys=uniteset(fsys,set);
	int type_analyzing=TYPE_VAR;
	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.
	if (sym == SYM_IDENTIFIER)
	{
		if ((i = position(id)) == 0)
		{
			error(11); // Undeclared identifier.
		}
		else
		{
			mask* mk;
			mk = (mask*) &table[i];
			if(table[i].kind == ID_CONSTANT){
				gen(LIT, 0, table[i].value);
			}
			else if(table[i].kind == ID_PROCEDURE){
				getsym();
				if(sym != SYM_SCOPE)
					error(21); // Procedure identifier can not be in an expression.
				else{
					getsym();
					Scope_analyze(mk->level+1, i+1);
				}
			}
			else{
				if(table[i].kindtable_en==0){
					kindtable_size=-1;
					kindtable_index=-1;
					gen(LOD, level - mk->level, mk->address);
					getsym();
				}
				else if(table[i].kind==ID_ARRAY){
					gen(LEA, level-mk->level, mk->address);
					typelist=table[i].kindtable;
					kindtable_index=0;
					kindtable_size=table[i].kind_num;
					update_step();
					getsym();
					type_analyzing=dim_reference(fsys);
					if(type_analyzing == Epsilon)
						type_analyzing=TYPE_ARRAY;
					//printf("type is %d",type_analyzing);
				}
				else{
					//A pointer
					update_step();
					gen(LOD, level - mk->level, mk->address);
					typelist=table[i].kindtable;
					kindtable_index=0;
					kindtable_size=table[i].kind_num;
					getsym();
					type_analyzing=dim_reference(fsys);
					if(type_analyzing == Epsilon)
						type_analyzing=TYPE_POINTER;
				}
			}
		}
	}
	else if (sym == SYM_NUMBER)
	{
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		gen(LIT, 0, num);
		//printf("\nThe num is %d\n", num);
		getsym();
	}
	else if (sym == SYM_LPAREN)
	{
		getsym();
		set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
		type_analyzing = expression(set);
		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			//printf("\nIt's me!\n");
			error(22); // Missing ')'.
		}
		if(type_analyzing != TYPE_VAR){
			type_analyzing=dim_reference(set);
		}
	}
	else if(sym == SYM_MINUS) // UMINUS,  Expr -> '-' Expr
	{  
			getsym();
			type_analyzing = factor(fsys);
			if(type_analyzing != TYPE_VAR)
			error(30);
			else
			gen(OPR, 0, OPR_NEG);
	}
	else if(sym == SYM_GETADDR){
		getsym();
		type_analyzing=factor(fsys);
		//gen(READDR, 0, 0);
		if(kindtable_index<-1){
			error(33);
		}
		if(type_analyzing==TYPE_VAR){
			step=1;
			gen(READDR, 0, 0);
			if(kindtable_size>0 && kindtable_index>kindtable_size){
				kindtable_index--;
				type_analyzing=typelist[kindtable_index].type;
			}
			else if(kindtable_size==-1){
				//just a var
				kindtable_index=-2;
				type_analyzing=TYPE_POINTER;
			}
			else{
				error(33);
			}
		}
		else{
			step=1;
			if(type_analyzing==TYPE_ARRAY){
				kindtable_index--;
				update_step();
			}
			else{
				gen(READDR, 0, 0);
				kindtable_index--;
				update_step();
			}
			if(kindtable_index<0){
				type_analyzing=TYPE_ARRAY;
			}
			else{
				type_analyzing=typelist[kindtable_index].type;
			}
		}
	}
	else if(sym == SYM_TIMES){
		getsym();
		type_analyzing=factor(fsys);
		//printf("\ntype is %d",type_analyzing);
		if(type_analyzing==TYPE_VAR){
			error(31);
		}
		else{
			type_analyzing=update_step();
			kindtable_index++;
			update_step();
			if(type_analyzing!=TYPE_ARRAY){
					gen(LODA, 0, 0);
			}
		}
	}
	else if(sym == SYM_SCOPE){
		getsym();
		Scope_analyze(0,1);
	}
	//test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);//"The symbol can not be followed by a factor."
	destroyset(set);
	return type_analyzing;
} // factor

//////////////////////////////////////////////////////////////////////
int term(symset fsys)
{
	int type_analyzing;
	int mulop;
	symset set;
	
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_RBRACKET, SYM_NULL));
	type_analyzing = factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH)
	{
		if(type_analyzing != TYPE_VAR){
			error(30);
			break;
		}
		mulop = sym;
		getsym();
		type_analyzing=factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else
		{
			gen(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
	return type_analyzing;
} // term

//////////////////////////////////////////////////////////////////////
int expression(symset fsys)
{
	int addop;
	symset set;
	int type_before, type_now;
	int step_before;
	kind_element* save_typelist;
	int save_size,save_index;
	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS, SYM_NULL));

	type_before=term(set);

	type_now=type_before;
	while (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		step_before=step;

		//保存现场
		save_typelist=typelist;
		save_index=kindtable_index;
		save_size=kindtable_size;

		type_now = term(set);

		if (addop == SYM_PLUS)
		{
			if(type_before==TYPE_VAR&&type_now == TYPE_VAR){
				gen(OPR, 0, OPR_ADD);
				step_before=1;
				//type_before no change
			}
			else if(type_before!=TYPE_VAR&&type_now == TYPE_VAR){
				//恢复现场
				typelist=save_typelist;
				kindtable_index=save_index;
				kindtable_size=save_size;

				step=step_before;
				gen(LIT, 0, step);
				gen(OPR, 0, OPR_MUL);
				gen(OPR, 0, OPR_ADD);
				type_now=type_before;
			}
			else if(type_before==TYPE_VAR&&type_now!=TYPE_VAR){
				gen(ADDR_ADD, 0, step);
				type_before=type_now;
			}
			else
				error(30);//"False type. Do not support the operations of two addrs"
		}
		else
		{	//只允许指针减去偏移
			if(type_before!=TYPE_VAR&&type_now == TYPE_VAR){
				step=step_before;
				gen(LIT, 0, step);
				gen(OPR, 0, OPR_MUL);
				gen(OPR, 0, OPR_MIN);
				type_now=type_before;
				//type_before no change
			}
			else if(type_before == TYPE_VAR && type_now ==TYPE_VAR){
				gen(OPR, 0, OPR_MIN);
			}
			else if(type_before != TYPE_VAR && type_now != TYPE_VAR){
				gen(OPR, 0, OPR_MIN);
				step=1;
				type_before=TYPE_VAR;
				type_now=TYPE_VAR;
			}
			else 
				error(30);
		}
	} // while

	destroyset(set);
	return type_now;
} // expression

//////////////////////////////////////////////////////////////////////
void condition(symset fsys)
{
	int relop;
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		if (! inset(sym, relset))
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsym();
			expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition

int dim_reference(symset fsys)
{	
	//printf("enter the dim ref! sym is %d",sym);
	int exp_type;
	int final_type=TYPE_ARRAY;
	kind_element* save_typelist;
	int save_size,save_index;
	if(sym == SYM_LBRACKET)
	{
		while(sym == SYM_LBRACKET){
			if(final_type==TYPE_VAR){
				error(29);
			}
			getsym();
			//保存现场
			save_typelist=typelist;
			save_index=kindtable_index;
			save_size=kindtable_size;
			exp_type=expression(fsys);
			if(exp_type!=TYPE_VAR){
				error(30);
				//printf("\nme\n");
				//break;
			}
			//恢复现场
			typelist=save_typelist;
			kindtable_index=save_index;
			kindtable_size=save_size;
			final_type=update_step();
			kindtable_index++;
			//printf("check the dim ref! sym is %d",sym);
			if(sym == SYM_RBRACKET){
				gen(LIT, 0, step);
				gen(OPR, 0, OPR_MUL);
				gen(OPR, 0, OPR_ADD);
				if(final_type!=TYPE_ARRAY){
					gen(LODA, 0, 0);
				}
			}
			else
				error(27);
			getsym();
		}
		update_step();
		return final_type;
	}
	else 
		return Epsilon; 
}

//////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;
	int type_analyzing;
	if(inset(sym, facbegsys)){
		type_analyzing=factor(fsys);
		if(type_analyzing==TYPE_ARRAY){
			error(34);
		}
		else{
			gen(READDR, 0, 0);
			if (sym == SYM_BECOMES)
			{
				getsym();
			}
			else
			{
				error(13); // ':=' expected.
			}
			expression(fsys);
			gen(STOA, 0, 0);
		}
	}
	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (! (i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*) &table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	} 
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		cx1 = cx;
		gen(JPC, 0, 0);
		statement(fsys);
		code[cx1].a = cx;	
	}
	else if (sym == SYM_BEGIN)
	{	// block
		// 复合语句
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);/* 10"';' expected.",*/
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO, SYM_NULL);
		set = uniteset(set1, fsys);
		condition(set);
		destroyset(set1);
		destroyset(set);
		cx2 = cx;
		gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		statement(fsys);
		gen(JMP, 0, cx1);
		code[cx2].a = cx;
	}
	else if(sym == SYM_PRINT){
		getsym();
		if(sym != SYM_LPAREN){
			error(35);
		}
		do{
			getsym();
			if(sym == SYM_RPAREN)
				break;
			expression(fsys);
			//printf("1");
			gen(PRINT, 0, 0);
		}while(sym == SYM_COMMA);
		if(sym != SYM_RPAREN){
			error(22);
		}
		gen(NEWLINE, 0, 0);
		getsym();
	}
} // statement
			
//////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dx = 3;
	block_dx = dx;
	mk = (mask*) &table[tx];
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do	//dealing with declaration
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if
		block_dx = dx; //save dx before handling procedure call!
		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}


			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			level--;

			if (sym == SYM_SEMICOLON)
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);//"Incorrect procedure name."
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		} // while
		dx = block_dx; //restore dx after handling procedure call!
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		//test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	}
	while (inset(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
	listcode(cx0, cx);
	// for(int i =1;i <= tx; i++)
	// 	printf("table[%d].name is %s\n", i,table[i].name);
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	int val;
	int addr;
	int pop_val;
	instruction i,Lastcode; // instruction register

	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{	
		Lastcode= code[pc-1];
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
				break;
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
				break;
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
				break;
			} // switch
			break;
		case LOD:
			addr=base(stack, b, i.l) + i.a;
			//printf("addr is %d\n",addr);
			stack[++top] = stack[addr];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			//printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] == 0)
				pc = i.a;
			top--;
			break;
		case LEA:
			addr=base(stack, b, i.l) + i.a;
			stack[++top] = addr;
			break;
		case LODA:
			addr=stack [top];
			stack[top] = stack[addr];
			break;
		case STOA:
			val = stack[top];
			addr = stack [top-1];
			stack[addr]=val;
			top = top -2;
			break;
		case READDR:
			if(Lastcode.f==LODA||Lastcode.f==LOD){
				stack[top]=addr;
			}
			else
				error(28);//"Could not get the addr! "
			break;
		case ADDR_ADD:
			top--;
			stack[top] =stack[top]*i.a + stack[top + 1];
			break;
		case ADDR_MIN:
			top--;
			stack[top] =-stack[top]*i.a + stack[top + 1];
			break;
		case PRINT:
			printf("%d ", stack[top]);
			break;
		case NEWLINE:
			printf("\n");
			break;
		default:
			break;
		} // switch
	}
	while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
void main ()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	
	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE, SYM_NULL);
	facbegsys  = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_MINUS, SYM_GETADDR, SYM_TIMES, SYM_SCOPE, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
