#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"
#include <stdbool.h>

extern Scope currentScope;
extern Scope scopeList;

void       symbolTableInsert(const char* name, int lineno, int loc, ExpType type, StmtKind kind,
                             bool isArray, const char* scope);
BucketList symbolTableLookup(const char* name);
BucketList symbolTableLookupCurrentScope(const char* name);
void       symbolTableAddLineNumberToSymbol(const char* name, int lineno);
void       printSymbolTable();

#endif