#include "symtab.h"

#include <log.h>
#include <stdlib.h>
#include <string.h>

Scope currentScope = NULL;
Scope scopeList    = NULL;

/**
 * @brief Computes the hash value for a given key.
 *
 * @param key The key to hash.
 * @return The hash value.
 */
static int hash(const char* key) {
	int temp = 0;
	int i    = 0;
	while (key[i] != '\0') {
		temp = ((temp << SHIFT) + key[i]) % SIZE;
		++i;
	}
	return temp;
}

/**
 * @brief Inserts a symbol into the symbol table.
 *
 * @param name The name of the symbol.
 * @param lineno The line number where the symbol is found.
 * @param loc The memory location of the symbol.
 * @param type The type of the symbol.
 * @param kind The kind of symbol (variable, function, etc.).
 * @param isArray Whether the symbol is an array.
 * @param scope The scope of the symbol.
 */
void symbolTableInsert(const char* name, const int lineno, const int loc, const ExpType type,
                       const StmtKind kind, const bool isArray, const char* scope) {
	const int  h      = hash(name);
	BucketList symbol = currentScope->hashTable[h];

	while (symbol != NULL && strcmp(name, symbol->name) != 0) symbol = symbol->next;

	if (symbol == NULL) {
		symbol                     = (BucketList) malloc(sizeof(struct BucketListRecord));
		symbol->name               = strdup(name);
		symbol->lines              = (LineList) malloc(sizeof(struct LineListRecord));
		symbol->lines->lineno      = lineno;
		symbol->lines->next        = NULL;
		symbol->memoryLocation     = loc;
		symbol->type               = type;
		symbol->kind               = kind;
		symbol->isArray            = isArray;
		symbol->scope              = strdup(scope);
		symbol->next               = currentScope->hashTable[h];
		currentScope->hashTable[h] = symbol;
	} else {
		symbolTableAddLineNumberToSymbol(symbol->name, lineno);
	}
}

/**
 * @brief Looks up a symbol in all available scopes from current to parents.
 *
 * This function searches for a symbol by name starting from the current scope
 * and moving up to parent scopes until the symbol is found or no more parent
 * scopes are available.
 *
 * @param name The name of the symbol to look up.
 * @return A pointer to the BucketList containing the symbol, or NULL if not found.
 */
BucketList symbolTableLookup(const char* name) {
	if (!name) return NULL;
	Scope scope = currentScope;
	while (scope) {
		const int  h      = hash(name);
		BucketList symbol = scope->hashTable[h];
		while (symbol != NULL && strcmp(name, symbol->name) != 0) symbol = symbol->next;
		if (symbol) return symbol;
		scope = scope->parent;
	}
	return NULL;
}

/**
 * @brief Looks up a symbol in the current scope.
 *
 * This function searches for a symbol by name only within the current scope.
 *
 * @param name The name of the symbol to look up.
 * @return A pointer to the BucketList containing the symbol, or NULL if not found.
 */
BucketList symbolTableLookupCurrentScope(const char* name) {
	const int  h      = hash(name);
	BucketList symbol = currentScope->hashTable[h];
	while (symbol != NULL && strcmp(name, symbol->name) != 0) symbol = symbol->next;
	return symbol;
}

/**
 * @brief Looks up a symbol from a specific scope.
 *
 * This function searches for a symbol by name within a given scope.
 *
 * @param name The name of the symbol to look up.
 * @param scope The scope in which to search for the symbol.
 * @return A pointer to the BucketList containing the symbol, or NULL if not found.
 */
BucketList symbolTableLookupFromScope(const char* name, Scope scope) {
	if (!name) return NULL;
	while (scope) {
		const int  h      = hash(name);
		BucketList symbol = scope->hashTable[h];
		while (symbol != NULL && strcmp(name, symbol->name) != 0) symbol = symbol->next;
		if (symbol) return symbol;
		scope = scope->parent;
	}
	return NULL;
}

/**
 * @brief Adds a line number to an existing symbol in the symbol table.
 *
 * This function adds a new line number to the list of line numbers for a given symbol.
 * If the line number already exists in the list, it does nothing.
 *
 * @param name The name of the symbol.
 * @param lineno The line number to add.
 */
void symbolTableAddLineNumberToSymbol(const char* name, const int lineno) {
	BucketList symbol = symbolTableLookup(name);
	if (symbol) {
		LineList lineList = symbol->lines;
		LineList previous = NULL;
		while (lineList) {
			if (lineList->lineno == lineno) return;
			previous = lineList;
			lineList = lineList->next;
		}
		LineList newLine = malloc(sizeof(struct LineListRecord));
		newLine->lineno  = lineno;
		newLine->next    = NULL;
		previous->next   = newLine;
	}
}

/**
 * @brief Prints the contents of the symbol table.
 */
void printSymbolTable() {
	pc("Variable Name  Scope     ID Type  Data Type  Line Numbers\n");
	pc("-------------  --------  -------  ---------  -------------------------\n");

	Scope listOfScopes = scopeList;
	while (listOfScopes) {
		for (int i = 0; i < SIZE; i++) {
			if (listOfScopes->hashTable[i]) {
				BucketList symbol = listOfScopes->hashTable[i];
				while (symbol) {
					// Name
					pc("%-14s ", symbol->name);

					// Scope
					if (strcmp(symbol->scope, "global") == 0)
						pc("%-9s ", "");
					else
						pc("%-9s ", symbol->scope);

					// Id Type
					if (symbol->kind == FuncK) {
						pc("%-8s ", "fun");
					} else if (symbol->isArray) {
						pc("%-8s ", "array");
					} else {
						pc("%-8s ", "var");
					}

					// Data Type
					pc("%-9s ", symbol->type == Integer ? "int"
					            : symbol->type == Void  ? "void"
					                                    : "Unknown");

					pc(" ");
					LineList line = symbol->lines;
					while (line) {
						if (line->lineno > 0) pc("%2d ", line->lineno);
						line = line->next;
					}
					pc("\n");
					symbol = symbol->next;
				}
			}
		}
		listOfScopes = listOfScopes->next;
	}
}