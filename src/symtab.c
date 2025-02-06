#include "symtab.h"

#include <log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 */
void symbolTableInsert(char* name, const int lineno, const int loc) {
	const int  h = hash(name);
	BucketList l = hashTable[h];
	while (l != NULL && strcmp(name, l->name) != 0)
		l = l->next;

	if (l == NULL) {
		l                = (BucketList) malloc(sizeof(struct BucketListRec));
		l->name          = name;
		l->lines         = (LineList) malloc(sizeof(struct LineListRec));
		l->lines->lineno = lineno;
		l->memloc        = loc;
		l->lines->next   = NULL;
		l->next          = hashTable[h];
		hashTable[h]     = l;
	} else {
		LineList t = l->lines;
		while (t->next != NULL) t = t->next;
		t->next         = (LineList) malloc(sizeof(struct LineListRec));
		t->next->lineno = lineno;
		t->next->next   = NULL;
	}
}

/**
 * @brief Looks up a symbol in the symbol table.
 *
 * @param name The name of the symbol to look up.
 * @return The memory location of the symbol, or -1 if not found.
 */
int symbolTableLookup(const char* name) {
	const int  h = hash(name);
	BucketList l = hashTable[h];
	while (l != NULL && strcmp(name, l->name) != 0)
		l = l->next;
	if (l == NULL)
		return -1;
	return l->memloc;
}

/**
 * @brief Prints the contents of the symbol table.
 */
void printSymbolTable() {
	pc("Variable Name  Location   Line Numbers\n");
	pc("-------------  --------   ------------\n");
	for (int i = 0; i < SIZE; ++i) {
		if (hashTable[i] != NULL) {
			BucketList l = hashTable[i];
			while (l != NULL) {
				LineList t = l->lines;
				pc("%-14s ", l->name);
				pc("%-8d  ", l->memloc);
				while (t != NULL) {
					pc("%4d ", t->lineno);
					t = t->next;
				}
				pc("\n");
				l = l->next;
			}
		}
	}
}