#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#define SIZE 211
#define SHIFT 4

typedef struct LineListRec {
   int                 lineno;
   struct LineListRec* next;
}* LineList;

typedef struct BucketListRec {
   char*                 name;
   LineList              lines;
   int                   memloc;
   struct BucketListRec* next;
}* BucketList;

static BucketList hashTable[SIZE];

void symbolTableInsert(char* name, int lineno, int loc);
int symbolTableLookup(const char* name);
void printSymbolTable();

#endif
