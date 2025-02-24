#ifndef HASH_H
#define HASH_H

#define TABLE_SIZE 100

/**
 * Structure representing a node in the hash table.
 */
typedef struct HashNode {
	char*            key;   /**< Key of the hash node */
	int              value; /**< Value associated with the key */
	struct HashNode* next;  /**< Pointer to the next node in the chain */
} HashNode;

/**
 * Structure representing the hash table.
 */
typedef struct {
	HashNode* table[TABLE_SIZE]; /**< Array of pointers to hash nodes */
} HashTable;

/**
 * Inserts a key-value pair into the hash table.
 *
 * @param key The key to insert.
 * @param value The value associated with the key.
 */
void insert(const char* key, int value);

/**
 * Looks up a value by its key in the hash table.
 *
 * @param key The key to look up.
 * @return The value associated with the key, or -1 if the key is not found.
 */
int lookup(const char* key);

#endif // HASH_H