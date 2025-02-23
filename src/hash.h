#ifndef HASH_H
#define HASH_H

#define TABLE_SIZE 100

typedef struct HashNode {
    char* key;
    int value;
    struct HashNode* next;
} HashNode;

typedef struct {
    HashNode* table[TABLE_SIZE];
} HashTable;

void insert(const char* key, int value);
int lookup(const char* key);

#endif // HASH_H