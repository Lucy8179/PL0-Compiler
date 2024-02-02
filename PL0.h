#include <stdio.h>

#define NRW        12     // number of reserved words
#define TXMAX      500    // length of identifier table
#define MAXNUMLEN  14     // maximum number of digits in numbers
#define NSYM       13     // maximum number of symbols in array ssym and csym
#define MAXIDLEN   100     // length of identifiers

#define MAXADDRESS 32767  // maximum address
#define MAXLEVEL   32     // maximum depth of nesting block
#define CXMAX      50000    // size of code array

#define MAXSYM     33     // maximum number of symbols  

#define STACKSIZE  99999   // maximum storage

#define MAX_DIM	   20

#define TYPE_POINTER	100
#define TYPE_ARRAY	   	200
#define TYPE_VAR   		500


#define Epsilon -100

enum symtype
{
	SYM_NULL,
	SYM_IDENTIFIER,
	SYM_NUMBER,
	SYM_PLUS,
	SYM_MINUS,
	SYM_TIMES,
	SYM_SLASH,
	SYM_ODD,
	SYM_EQU,
	SYM_NEQ,
	SYM_LES,
	SYM_LEQ,
	SYM_GTR,
	SYM_GEQ,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_COMMA,
	SYM_SEMICOLON,
	SYM_PERIOD,
	SYM_BECOMES,
    SYM_BEGIN,
	SYM_END,
	SYM_IF,
	SYM_THEN,
	SYM_WHILE,
	SYM_DO,
	SYM_CALL,
	SYM_CONST,
	SYM_VAR,
	SYM_PROCEDURE,
	SYM_LBRACKET,
	SYM_RBRACKET,
	SYM_GETADDR,
	SYM_PRINT,
	SYM_SCOPE,
	SYM_TYPE
};

enum idtype
{
	ID_CONSTANT, 
	ID_VARIABLE, 
	ID_PROCEDURE, 
	ID_ARRAY
};

enum opcode
{
	LIT, OPR, LOD, STO, CAL, INT, JMP, JPC, LEA ,LODA, STOA, READDR, PRINT, NEWLINE, ADDR_ADD,ADDR_MIN
};

enum oprcode
{
	OPR_RET, OPR_NEG, OPR_ADD, OPR_MIN,
	OPR_MUL, OPR_DIV, OPR_ODD, OPR_EQU,
	OPR_NEQ, OPR_LES, OPR_LEQ, OPR_GTR,
	OPR_GEQ
};


typedef struct
{
	int f; // function code
	int l; // level
	int a; // displacement address
} instruction;

//////////////////////////////////////////////////////////////////////
char* err_msg[] =
{
/*  0 */    "",
/*  1 */    "Found ':=' when expecting '='.",
/*  2 */    "There must be a number to follow '='.",
/*  3 */    "There must be an '=' to follow the identifier.",
/*  4 */    "There must be an identifier to follow 'const', 'var', or 'procedure'.",
/*  5 */    "Missing ',' or ';'.",
/*  6 */    "Incorrect procedure name.",
/*  7 */    "Statement expected.",
/*  8 */    "Follow the statement is an incorrect symbol.",
/*  9 */    "'.' expected.",
/* 10 */    "';' expected.",
/* 11 */    "Undeclared identifier.",
/* 12 */    "Illegal assignment.",
/* 13 */    "':=' expected.",
/* 14 */    "There must be an identifier to follow the 'call'.",
/* 15 */    "A constant or variable can not be called.",
/* 16 */    "'then' expected.",
/* 17 */    "';' or 'end' expected.",
/* 18 */    "'do' expected.",
/* 19 */    "Incorrect symbol.",
/* 20 */    "Relative operators expected.",
/* 21 */    "Procedure identifier can not be in an expression.",
/* 22 */    "Missing ')'.",
/* 23 */    "The symbol can not be followed by a factor.",
/* 24 */    "The symbol can not be as the beginning of an expression.",
/* 25 */    "The number is too great.",
/* 26 */    "The left bracket should be followed by the number.",
/* 27 */    "Missing the right bracket ']'. ",
/* 28 */    "Could not get the addr! ",
/* 29 */    "Exceeding the dim of the pointer/array!",
/* 30 */    "False type. Do not support the operations of two addrs",
/* 31 */    "False type. Do not support the operation!",
/* 32 */    "There are too many levels.",
/* 33 */	"The addr of the var can not be got!",
/* 34 */	"The const var can not be assigned or modified!",
/* 35 */	"Unknown inst! Please check the inst code.",
/* 36 */	"Missing '(' .",
/* 37 */	"Identifier expected!",
/* 38 */	"Can not locate the identifier! ",
/* 39 */    "Identifier has been found, '::' is not expected! ",
/* 40 */	" "     
};

//////////////////////////////////////////////////////////////////////
char ch;         // last character read
int  sym;        // last symbol read
char id[MAXIDLEN + 1]; // last identifier read
int  num;        // last number read
int  cc;         // character count
int  ll;         // line length
int  kk;
int  err;
int  cx;         // index of current instruction to be generated.
int  level = 0;
int  tx = 0;	 //	table index

char line[200];

instruction code[CXMAX];

char* word[NRW + 1] =
{
	"", /* place holder */
	"begin", "call", "const", "do", "end","if",
	"odd", "procedure", "then", "var", "while", "print"
};

int wsym[NRW + 1] =
{
	SYM_NULL, SYM_BEGIN, SYM_CALL, SYM_CONST, SYM_DO, SYM_END,
	SYM_IF, SYM_ODD, SYM_PROCEDURE, SYM_THEN, SYM_VAR, SYM_WHILE,
	SYM_PRINT
};//word symbol

int ssym[NSYM + 1] =
{
	SYM_NULL, SYM_PLUS, SYM_MINUS, SYM_TIMES, SYM_SLASH,
	SYM_LPAREN, SYM_RPAREN, SYM_EQU, SYM_COMMA, SYM_PERIOD, SYM_SEMICOLON,
	SYM_LBRACKET, SYM_RBRACKET, SYM_GETADDR
};//symbol table

char csym[NSYM + 1] =
{
	' ', '+', '-', '*', '/', '(', ')', '=', ',', '.', ';',
	'[', ']', '&'
};//character symbol table

#define MAXINS   18
char* mnemonic[MAXINS] =
{
	"LIT", "OPR", "LOD", "STO", "CAL", "INT", "JMP", "JPC",
	"LEA", "LODA", "STOA", "READDR", "PRINT", "NEWLINE",
	"ADDR_ADD", "ADDR_MIN"
};

typedef struct KIND
{
	int type;
	int elem_num;
}kind_element;

typedef struct
{
	char name[MAXIDLEN + 1];
	int  kind;
	int  value;
	int  kindtable_en; //whether or not using the kindtable
	kind_element kindtable[MAX_DIM] ; //used for the array and pointer
	int kind_num;
} comtab;//identifier,stores the var and value

comtab table[TXMAX];

typedef struct
{
	char  name[MAXIDLEN + 1];
	int   kind;
	short level;
	short address;
} mask;//id mask for the specific procedure

//int type;
kind_element * typelist; //type we are analyzing
int kindtable_index = 0; 
int kindtable_size = 0;
int step = 1;

FILE* infile;

// EOF PL0.h
