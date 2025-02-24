#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"
#include <stdbool.h>

/**
 * The current scope in the symbol table.
 */
extern Scope currentScope;

/**
 * The list of all scopes in the symbol table.
 */
extern Scope scopeList;

/**
 * Inserts a symbol into the symbol table.
 *
 * @param name The name of the symbol.
 * @param lineno The line number where the symbol is declared.
 * @param loc The memory location of the symbol.
 * @param type The type of the expression.
 * @param kind The kind of statement.
 * @param isArray Whether the symbol is an array.
 * @param scope The scope of the symbol.
 */
void symbolTableInsert(const char* name, int lineno, int loc, ExpType type, StmtKind kind, bool isArray, const char* scope);

/**
 * Looks up a symbol in the symbol table.
 *
 * @param name The name of the symbol.
 * @return The bucket list containing the symbol.
 */
BucketList symbolTableLookup(const char* name);

/**
 * Looks up a symbol in the current scope of the symbol table.
 *
 * @param name The name of the symbol.
 * @return The bucket list containing the symbol.
 */
BucketList symbolTableLookupCurrentScope(const char* name);

/**
 * Looks up a symbol in a specific scope of the symbol table.
 *
 * @param name The name of the symbol.
 * @param scope The scope to look up the symbol in.
 * @return The bucket list containing the symbol.
 */
BucketList symbolTableLookupFromScope(const char* name, Scope scope);

/**
 * Adds a line number to an existing symbol in the symbol table.
 *
 * @param name The name of the symbol.
 * @param lineno The line number to add.
 */
void symbolTableAddLineNumberToSymbol(const char* name, int lineno);

/**
 * Prints the symbol table.
 */
void printSymbolTable();

#endif