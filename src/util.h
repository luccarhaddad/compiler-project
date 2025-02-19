#ifndef _UTIL_H_
#define _UTIL_H_

#include "globals.h"

void      printToken(TokenType token, const char* tokenString);
TreeNode* newStmtNode(StmtKind kind);
TreeNode* newExpNode(ExpKind kind);
char*     copyString(const char* string);
void      printTree(const TreeNode* tree);
void      printLine();

#endif
