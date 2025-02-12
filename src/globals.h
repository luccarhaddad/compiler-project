#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 6

typedef int TokenType;

extern FILE* source;           /* source code text file */
extern FILE* redundant_source; /* source code text file */
extern FILE* listing;          /* listing output text file */
extern FILE* code;             /* code text file for TM simulator */

extern int lineno; /* source line number for listing */

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum { StmtK, ExpK } NodeKind;
typedef enum { IfK, WhileK, ReturnK, AssignK, ParamK, VarK, FuncK, CompoundK } StmtKind;
typedef enum { OpK, ConstK, IdK, CallK } ExpKind;
typedef enum { Void, Integer, Boolean } ExpType;

#define MAXCHILDREN 3

typedef struct treeNode {
	struct treeNode* child[MAXCHILDREN];
	struct treeNode* sibling;
	struct treeNode* parent;
	int              lineno;
	NodeKind         nodekind;
	union {
		StmtKind stmt;
		ExpKind  exp;
	} kind;
	union {
		TokenType op;
		int       val;
		char*     name;
	} attr;
	ExpType type; /* for type checking of exps */
	int isArray;
} TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* EchoSource = TRUE causes the source program to
 * be echoed to the listing file with line numbers
 * during parsing
 */
extern int EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern int TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern int Error;

#ifndef YYPARSER
#include "parser.h"
#define ENDFILE 0
#endif

#endif