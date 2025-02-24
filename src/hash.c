#include "hash.h"
#include <stdlib.h>
#include <string.h>

static HashTable hashTable;

static unsigned int hash(const char* key) {
	unsigned int hash = 0;
	while (*key) {
		hash = (hash << 5) + *key++;
	}
	return hash % TABLE_SIZE;
}

void insert(const char* key, int value) {
	unsigned int index     = hash(key);
	HashNode*    newNode   = malloc(sizeof(HashNode));
	newNode->key           = strdup(key);
	newNode->value         = value;
	newNode->next          = hashTable.table[index];
	hashTable.table[index] = newNode;
}

int lookup(const char* key) {
	unsigned int index = hash(key);
	HashNode*    node  = hashTable.table[index];
	while (node) {
		if (strcmp(node->key, key) == 0) {
			return node->value;
		}
		node = node->next;
	}
	return -1; // Key not found
}