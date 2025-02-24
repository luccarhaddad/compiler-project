#include "symtab.h"

#include <log.h>
#include <stdlib.h>
#include <string.h>

Scope currentScope = NULL;
Scope scopeList    = NULL;

static int hash(const char* key) {
	int temp = 0;
	int i    = 0;
	while (key[i] != '\0') {
		temp = ((temp << SHIFT) + key[i]) % SIZE;
		++i;
	}
	return temp;
}

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

BucketList symbolTableLookupCurrentScope(const char* name) {
	const int  h      = hash(name);
	BucketList symbol = currentScope->hashTable[h];
	while (symbol != NULL && strcmp(name, symbol->name) != 0) symbol = symbol->next;
	return symbol;
}

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