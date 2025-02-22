/**
 * @file globals.h
 * @brief Global definitions and declarations for the project.
 */

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "log.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 211
#define SHIFT 4
#define MAX_MEMORY 1023

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/**
 * @brief The number of reserved words.
 */
#define MAXRESERVED 6

/**
 * @brief Type definition for tokens.
 */
typedef int TokenType;

/**
 * @brief External file pointers for various purposes.
 */
extern FILE* source;           /**< Source code text file. */
extern FILE* redundant_source; /**< Redundant source code text file. */
extern FILE* listing;          /**< Listing output text file. */
extern FILE* code;             /**< Code text file for TM simulator. */

/**
 * @brief Source line number for listing.
 */
extern int lineno;

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

/**
 * @brief Enumeration for node kinds in the syntax tree.
 */
typedef enum { StmtK, ExpK } NodeKind;

/**
 * @brief Enumeration for statement kinds in the syntax tree.
 */
typedef enum { IfK, WhileK, ReturnK, ParamK, VarK, FuncK, CompoundK } StmtKind;

/**
 * @brief Enumeration for expression kinds in the syntax tree.
 */
typedef enum { OpK, ConstK, IdK, CallK, AssignK, UnaryK } ExpKind;

/**
 * @brief Enumeration for expression types.
 */
typedef enum { Void, Integer, Boolean } ExpType;

#define MAXCHILDREN 3

/**
 * @brief Structure representing a list of line numbers.
 */
typedef struct LineListRecord {
	int                    lineno; /**< The line number. */
	struct LineListRecord* next;   /**< Pointer to the next line number in the list. */
}* LineList;

/**
 * @brief Structure representing a bucket in the symbol table.
 */
typedef struct BucketListRecord {
	char*                    name;           /**< The name of the symbol. */
	LineList                 lines;          /**< List of line numbers where the symbol appears. */
	int                      memoryLocation; /**< Memory location of the symbol. */
	ExpType                  type;           /**< The type of the symbol. */
	StmtKind                 kind;           /**< The kind of symbol (variable, function, etc.). */
	bool                     isArray;        /**< Whether the symbol is an array. */
	char*                    scope;          /**< The scope of the symbol. */
	struct BucketListRecord* next;           /**< Pointer to the next bucket in the list. */
}* BucketList;

/**
 * @brief Structure representing a scope in the symbol table.
 */
typedef struct ScopeRecord {
	char*               name;            /**< The name of the scope. */
	struct ScopeRecord* parent;          /**< Pointer to the parent scope. */
	struct ScopeRecord* next;            /**< Pointer to the next scope. */
	BucketList          hashTable[SIZE]; /**< Hash table containing the symbols in the scope. */
}* Scope;

/**
 * @brief Structure representing a node in the syntax tree.
 */
typedef struct treeNode {
	struct treeNode* child[MAXCHILDREN]; /**< Pointers to child nodes. */
	struct treeNode* sibling;            /**< Pointer to the sibling node. */
	struct treeNode* parent;             /**< Pointer to the parent node. */
	int              lineno;             /**< Line number associated with the node. */
	NodeKind         nodekind;           /**< Kind of the node (statement or expression). */
	union {
		StmtKind stmt; /**< Statement kind. */
		ExpKind  exp;  /**< Expression kind. */
	} kind;
	union {
		TokenType op;   /**< Operator token. */
		int       val;  /**< Integer value. */
		char*     name; /**< Identifier name. */
	} attr;
	ExpType type;    /**< Type for type checking of expressions. */
	Scope   scope;   /**< Scope associated with the node. */
	int     isArray; /**< Whether the node represents an array. */
} TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/**
 * @brief EchoSource = TRUE causes the source program to be echoed to the listing file with line
 * numbers during parsing.
 */
extern int EchoSource;

/**
 * @brief TraceScan = TRUE causes token information to be printed to the listing file as each token
 * is recognized by the scanner.
 */
extern int TraceScan;

/**
 * @brief TraceParse = TRUE causes the syntax tree to be printed to the listing file in linearized
 * form (using indents for children).
 */
extern int TraceParse;

/**
 * @brief TraceAnalyze = TRUE causes symbol table inserts and lookups to be reported to the listing
 * file.
 */
extern int TraceAnalyze;

/**
 * @brief TraceCode = TRUE causes comments to be written to the TM code file as code is generated.
 */
extern int TraceCode;

/**
 * @brief Error = TRUE prevents further passes if an error occurs.
 */
extern int Error;

#ifndef YYPARSER
#include "parser.h"
#define ENDFILE 0
#endif

#endif /* _GLOBALS_H_ */