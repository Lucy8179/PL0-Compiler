#ifndef SET_H
#define SET_H

typedef struct snode
{
	int elem;
	struct snode* next;
} snode, *symset;

symset phi;			//null
symset declbegsys; 	//declare begin(const, var, procedure...) set 
symset statbegsys; 	//statement begin(begin, call, if,...)
symset facbegsys; 	//factor begin(id, num, op,...)
symset relset;		//relation set(>, <, >=, ...)

symset createset(int data, .../* SYM_NULL */);
void destroyset(symset s);
symset uniteset(symset s1, symset s2);
int inset(int elem, symset s); //search element,true or false

#endif
// EOF set.h
